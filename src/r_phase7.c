/*
  CALICO

  Renderer phase 7 - Visplanes
*/

static void R_MapPlane(void)
{
   // CALICO_TODO
   /*
mp_ystep	.equr	r0
mp_axstep	.equr	r1
mp_aystep	.equr	r2
mp_count	.equr	r3
mp_a1pixel	.equr	r4
mp_FREE		.equr	r5
mp_remaining .equr	r6
mp_xremain	.equr	r7
mp_yremain	.equr	r8
mp_ffff		.equr	r9
mp_ffff0000	.equr	r12

mp_blitter	.equr	r15
mp_y		.equr	r16
mp_3fffff	.equr	r17
mp_x2		.equr	r18
mp_angle	.equr	r19
mp_distance	.equr	r20
mp_length	.equr	r21
mp_x		.equr	r22
mp_light	.equr	r23
mp_xfrac	.equr	r24
mp_yfrac	.equr	r25
mp_xstep	.equr	r26
mp_blitcommand	.equr	r27

;
; set up for multiple R_MapPlanes
;                                                             // Jag-specific setup
	movei	#$f02200,mp_blitter                                  mp_blitter = 0xf02200
	movei	#$ffff,mp_ffff                                       mp_ffff = 0xffff
	movei	#$ffff0000,mp_ffff0000                               mp_ffff0000 = 0xffff0000
	movei	#$3fffff,mp_3fffff                                   mp_3fffff = 0x3fffff
	movei	#1+(1<<11)+(1<<13)+(1<<30)+(12<<21),mp_blitcommand   mp_blitcommand = 0x41802801

;	x2 = parm>>16;
;	y = (parm>>8)&0xff;
;	x1 = parm&0xff;
;	x = x1;
;	count = x2 - x+1;
mp_entry:
	load	(FP),mp_x2                                            mp_x2 = *FP
	move	mp_x2,mp_y                                            mp_y = mp_x2
	move	mp_y,mp_x                                             mp_x = mp_y
	shlq	#24,mp_x                                              mp_x <<= 24
	shrq	#24,mp_x                                              mp_x >>= 24
	shrq	#16,mp_x2                                             mp_x2 >>= 16
	shlq	#16,mp_y                                              mp_y <<= 16
	shrq	#24,mp_y                                              mp_y >>= 24
	move	mp_x2,mp_remaining                                    mp_remaining = mp_x2
	sub		mp_x,mp_remaining                                  mp_remaining -= mp_x
	addq	#1,mp_remaining                                       mp_remaining += 1

	or		mp_remaining,mp_remaining                             mp_remaining |= mp_remaining
	movei	#mp_linedone,scratch                                  scratch = &mp_linedone
	jump	EQ,(scratch) ; nothing to draw (sholdn't happen)      if(!mp_remaining) goto scratch
	nop
			
;	distance = (planeheight*yslope[y])>>12;
	movei	#_yslope,scratch                                      scratch = &yslope
	add		mp_y,scratch                                       scratch += mp_y
	add		mp_y,scratch                                       scratch += mp_y
	loadw	(scratch),scratch ; yslope[y]                         scratch = *scratch
	movefa	alt_planeheight,mp_distance                        mp_distance = alt_planeheight
	mult	scratch,mp_distance                                   mp_distance *= scratch
	shrq	#12,mp_distance                                       mp_distance >>= 12
	
;	length = (distance*distscale[x1])>>14;
	move mp_x,scratch2                                        scratch2 = mp_x
	shlq	#1,scratch2                                         scratch2 <<= 1
	movei	#_distscale,mp_length                               mp_length = &distscale
	add		scratch2,mp_length                               mp_length += scratch2
	loadw	(mp_length),mp_length                               mp_length = *mp_length
	mult	mp_distance,mp_length                               mp_length *= mp_distance
	shrq	#14,mp_length                                       mp_length >>= 14
	
;	angle = (planeangle + xtoviewangle[x1])>>ANGLETOFINESHIFT;
	movefa	alt_planeangle,scratch                           scratch = alt_planeangle
	shlq	#1,scratch2                                         scratch2 <<= 1
	movei	#_xtoviewangle,mp_angle                             mp_angle = &xtoviewangle
	add		scratch2,mp_angle		; + x1*4                    mp_angle += scratch2
	load	(mp_angle),mp_angle                                 mp_angle = *mp_angle
	add		scratch,mp_angle                                 mp_angle += scratch
	shrq	#19,mp_angle                                        mp_angle >>= ANGLETOFINESHIFT
	
	shlq	#2,mp_angle ; used to index int tables              mp_angle <<= 2
	movei	#_finesine,scratch                                  scratch = &finesine
	add		scratch,mp_angle                                 mp_angle += scratch
	movei	#8192,scratch                                       scratch = FINEANGLES
	load	(mp_angle),scratch2 ; finesine[angle]               scratch2 = *mp_angle
	add		scratch,mp_angle                                 mp_angle += scratch
	load	(mp_angle),mp_xfrac ; finecosine[angle]             mp_xfrac = *mp_angle
 
;	xfrac = planex + (((finecosine[angle]>>1)*length)>>4);
	movefa	alt_planex,scratch                               scratch = alt_planex
	sharq	#1,mp_xfrac                                         mp_xfrac >>= 1
	imult	mp_length,mp_xfrac                                  mp_xfrac *= mp_length
	sharq	#4,mp_xfrac                                         mp_xfrac >>= 4
	add		scratch,mp_xfrac                                 mp_xfrac += scratch

;	yfrac = planey - (((finesine[angle]>>1)*length)>>4);
	movefa	alt_planey,mp_yfrac                              mp_yfrac = alt_planey
	sharq	#1,scratch2                                         scratch2 >>= 1
	imult	mp_length,scratch2                                  scratch2 *= mp_length
	sharq	#4,scratch2                                         scratch2 >>= 4
	sub		scratch2,mp_yfrac                                mp_yfrac -= scratch2
 
;	xstep = (distance*basexscale)>>4;
;	if (!xstep)
;		axstep = 1;
;	else if (xstep < 0)
;		axstep = -xstep;
;	else
;		axstep = xstep;
	movefa	alt_basexscale,mp_xstep                          mp_xstep = alt_basexscale
	imult	mp_distance,mp_xstep                                mp_xstep *= mp_distance
	sharq	#4,mp_xstep                                         mp_xstep >>= 4
	jr		NE,mp_xnotzero                                      if(mp_xstep) goto mp_xnotzero
	move	mp_xstep,mp_axstep ; delay slot                       mp_axstep = mp_xstep // before jump
	moveq	#1,mp_axstep                                        mp_axstep = 1
	moveq	#1,mp_xstep                                         mp_xstep = 1
mp_xnotzero:
	abs		mp_axstep                                        mp_axstep = abs(mp_axstep)

;	light = planelight; // - ((planelightscale*distance)>>16);
;	if (light < 0)
;		light = 0;
;	light = -((255-light)<<14); // should be from 0 to -0x800000
;	light &= 0xffffff;
	movefa	alt_plane_lightcoef,mp_light                     mp_light = alt_plane_lightcoef
	div		mp_distance,mp_light                             mp_light /= mp_distance

; do something else while dividing

;	ystep = (distance*baseyscale)>>4;	
;	if (!ystep)
;		aystep = 1;
;	else if (ystep < 0)
;		aystep = -ystep;
;	else
;		aystep = ystep;
	movefa	alt_baseyscale,mp_ystep                          mp_ystep = alt_baseyscale
	imult	mp_distance,mp_ystep                                mp_ystep *= mp_distance
	sharq	#4,mp_ystep                                         mp_ystep >>= 4
	jr		NE,mp_ynotzero                                      if(mp_ystep) goto mp_ynotzero
	move	mp_ystep,mp_aystep ; delay slot                        mp_aystep = mp_ystep // before jump
	moveq	#1,mp_aystep		                                  mp_aystep = 1
	moveq	#1,mp_ystep		                                     mp_ystep = 1
mp_ynotzero:
	abs		mp_aystep                                        mp_aystep = abs(mp_aystep)

; finish light calculations
	movefa	alt_plane_lightsub,scratch                       scratch = alt_plane_lightsub
	sub		scratch,mp_light                                 mp_light -= scratch
	
	movefa	alt_plane_lightmax,scratch                       scratch = alt_plane_lightmax
	cmp		scratch,mp_light                                 if(scratch > mp_light)
	jr		S_GT,mp_lightless                                      goto mp_lightless
	nop
	move	scratch,mp_light	                                  mp_light = scratch
mp_lightless:
	movefa	alt_plane_lightmin,scratch                       scratch = alt_plane_lightmin
	cmp		scratch,mp_light                                 if(scratch < mp_light)
	jr		S_LT,mp_lightmore                                      goto mp_lightmore
	nop
	move	scratch,mp_light	                                  mp_light = scratch
mp_lightmore:

	movei	#255,scratch                                        scratch = 255
	sub		mp_light,scratch                                 scratch -= mp_light
	shlq	#14,scratch                                         scratch <<= 14
	neg		scratch                                          scratch = -scratch
	movei	#$ffffff,mp_light                                   mp_light = 0xffffff
	and		scratch,mp_light                                 mp_light &= scratch

;-----------------
;
; setup blitter
;
;-----------------

mp_wait1: // busy loop
	load	(mp_blitter+14),scratch                             scratch = *(mp_blitter+14)
	btst	#0,scratch                                          if(!scratch)
	jr		EQ,mp_wait1                                            goto mp_wait1
	nop
	
   // JAG-SPECIFIC SETUP FOLLOWS (similar to R_DrawSpan)
; *(int *)0xf0221c = (ystep&0xffff0000)+((xstep>>16)&0xffff); // a1 increment 
	move	mp_ystep,scratch                                    scratch = mp_ystep
	and		mp_ffff0000,scratch                              scratch &= mp_ffff0000
	move	mp_xstep,scratch2                                   scratch2 = mp_xstep
	shrq	#16,scratch2                                        scratch2 >>= 16
	or		scratch2,scratch                                    scratch |= scratch2
	store	scratch,(mp_blitter+7)                              *(mp_blitter+7) = scratch
		
; *(int *)0xf02220 = (xstep&0xffff) + (ystep<<16); // a1 increment frac 	
	move	mp_xstep,scratch                                    scratch = mp_xstep
	and		mp_ffff,scratch                                  scratch &= mp_ffff
	move	mp_ystep,scratch2                                   scratch2 = mp_ystep
	shlq	#16,scratch2                                        scratch2 <<= 16
	or		scratch2,scratch                                    scratch |= scratch2
	store	scratch,(mp_blitter+8)                              *(mp_blitter+8) = scratch

; *(int *)0xf02218 = (yfrac<<16)+(xfrac&0xffff); // a1 pixel frac 
	move	mp_yfrac,scratch                                    scratch = mp_yfrac
	shlq	#16,scratch                                         scratch <<= 16
	move	mp_xfrac,scratch2                                   scratch2 = mp_xfrac
	and		mp_ffff,scratch2                                 scratch2 &= mp_ffff
	or		scratch2,scratch                                    scratch |= scratch2
	store	scratch,(mp_blitter+6)                              *(mp_blitter+6) = scratch

; *(int *)0xf02230 = (y<<16) + x; // a2 pixel pointers
	shlq	#16,mp_y                                            mp_y <<= 16
	add		mp_x,mp_y                                        mp_y += mp_x
	store	mp_y,(mp_blitter+12)                                *(mp_blitter+12) = mp_y
	
; *(int *)0xf02270 = light; // iinc
	movei	#$f02270,scratch                                    scratch = 0xf02270 // B_IINC
	store	mp_light,(scratch)                                  *scratch = mp_light
			
;	count = 0;
	moveq	#0,mp_count                                         mp_count = 0

mp_stillremaining:
;===============
;
; x axis
;
;===============
; xfrac = (xfrac + count*xstep)&0x3fffff;
; xpos = xfrac;
; if(xstep >= 0)
;    xpos = 0x400000 - xpos;
; xremain = (xpos / axstep);
	move	mp_axstep,scratch                                   scratch = mp_axstep
	move	mp_axstep,scratch2                                  scratch2 = mp_axstep
	shrq	#16,scratch2                                        scratch2 >>= 16
	mult	mp_count,scratch                                    scratch *= mp_count
	mult	mp_count,scratch2                                   scratch2 *= mp_count
	shlq	#16,scratch2                                        scratch2 <<= 16
	add		scratch2,scratch                                 scratch += scratch2
	or		mp_xstep,mp_xstep                                   mp_xstep |= mp_xstep
	jr		PL,mp_addxpos                                       if( <= ) goto mp_addxpos //????
	nop
	sub		scratch,mp_xfrac                                 mp_xfrac -= scratch
	and		mp_3fffff,mp_xfrac                               mp_xfrac &= mp_3fffff
	jr		T,mp_xadded                                         goto mp_xadded
	move	mp_xfrac,mp_xremain ; delay slot                       mp_xremain = mp_xfrac // before jump

mp_addxpos:
	add		scratch,mp_xfrac                                 mp_xfrac += scratch
	and		mp_3fffff,mp_xfrac                               mp_xfrac &= mp_3fffff
	movefa	alt_400000,mp_xremain                            mp_xremain = alt_400000
	sub		mp_xfrac,mp_xremain                              mp_xremain -= mp_xfrac
mp_xadded:
	div		mp_axstep,mp_xremain                             mp_xremain /= mp_axstep

;===============
;
; y axis
;
;===============
; yfrac = (yfrac + count*ystep)&0x3fffff;
; yremain = (ypos / aystep) + 1;
	move	mp_aystep,scratch                                   scratch = mp_aystep
	move	mp_aystep,scratch2                                  scratch2 = mp_aystep
	shrq	#16,scratch2                                        scratch2 >>= 16
	mult	mp_count,scratch                                    scratch *= mp_count
	mult	mp_count,scratch2                                   scratch2 *= mp_count
	shlq	#16,scratch2                                        scratch2 <<= 16
	add		scratch2,scratch                                 scratch += scratch2
	or		mp_ystep,mp_ystep                                   mp_ystep |= mp_ystep
	jr		PL,mp_addypos                                       if( <= ) goto mp_addypos // ????
	nop
	sub		scratch,mp_yfrac                                 mp_yfrac -= scratch
	and		mp_3fffff,mp_yfrac                               mp_yfrac &= mp_3fffff
	jr		T,mp_yadded                                         goto mp_yadded
	move	mp_yfrac,mp_yremain ; delay slot                       mp_yremain = mp_yfrac // before jump

mp_addypos:
	add		scratch,mp_yfrac                                 mp_yfrac += scratch
	and		mp_3fffff,mp_yfrac                               mp_yfrac &= mp_3fffff
	movefa	alt_400000,mp_yremain                            mp_yremain = alt_400000
	sub		mp_yfrac,mp_yremain                              mp_yremain -= mp_yfrac
	
mp_yadded:
	div		mp_aystep,mp_yremain                             mp_yremain /= mp_aystep

;====================
; prepare blitter registers
;====================
	
; *(int *)0xf0220c = (yfrac&0xffff0000)+(xfrac>>16); // a1 pixel 
	move	mp_yfrac,mp_a1pixel                                 mp_a1pixel = mp_yfrac
	and		mp_ffff0000,mp_a1pixel                           mp_a1pixel &= mp_ffff0000
	move	mp_xfrac,scratch2                                   scratch2 = mp_xfrac
	shrq	#16,scratch2                                        scratch2 >>= 16
	or		scratch2,mp_a1pixel                                 mp_a1pixel |= scratch2

;=====================
;		count = remaining;
;		if (xremain < count)
;			count = xremain;
;		if (yremain < count)
;			count = yremain;
; ramaining will allways be at least 1
; xremain and yremain will allways be 0 or greater
;=====================

	move	mp_remaining,mp_count                               mp_count = mp_remaining
	subq	#1,mp_count                                         mp_count -= 1
	cmp		mp_xremain,mp_count                              if(mp_xremain > mp_count)
	jr		U_GT,mp_notxremain                                     goto mp_notxremain
	nop
	move	mp_xremain,mp_count                                 mp_count = mp_xremain
mp_notxremain:
	cmp		mp_yremain,mp_count                              if(mp_yremain > mp_count)
	jr		U_GT,mp_notyremain                                    goto mp_notyremain
	nop
	move	mp_yremain,mp_count                                 mp_count = mp_yremain
mp_notyremain:
	
	addq	#1,mp_count                                         mp_count += 1
	
;=====================
; program blitter
;=====================

;	while ( ! (*(int *)0xf02238 & 1) )			// wait for blitter to finish
;	;
mp_wait2: // busy loop
	load	(mp_blitter+14),scratch                             scratch = *(mp_blitter+14)
	btst	#0,scratch                                          if(!scratch)
	jr		EQ,mp_wait2                                            goto mp_wait2
	move	mp_count,scratch ; harmless delay slot
	
	store	mp_a1pixel,(mp_blitter+3) ; source location         *(mp_blitter+3) = mp_a1pixel
	bset	#16,scratch ; one outer loop                        scratch |= (1<<16)
	store	scratch,(mp_blitter+15) ; count register            *(mp_blitter+15) = scratch
	store	mp_blitcommand,(mp_blitter+14) ; command register   *(mp_blitter+14) = mp_blitcommand
;===========================

;		remaining -= count;
	movei	#mp_stillremaining,scratch                          scratch = &mp_stillremaining
	sub		mp_count,mp_remaining ; delay slot               mp_remaining -= mp_count
	jump	NE,(scratch)                                        if(mp_remaining) goto *scratch
	nop

;
; all done with this line, see if there are more to do
;
mp_linedone:
	addq	#4,FP                                               FP += 4
	cmp		FP,pl_stopfp                                     if(FP != pl_stopfp)
	movei	#mp_entry,scratch                                   scratch = &mp_entry
	jump	NE,(scratch)                                           goto *scratch
	load	(FP),RETURNPOINT ; harmless delay slot              RETURNPOINT = *FP // before jump
	jump	T,(RETURNPOINT)  ; go back to R_DrawPlanes          return;
	addq	#4,FP            ; delay slot
   */
}

static void R_PlaneLoop(void)
{
   // CALICO_TODO
   /*
pl_L_topstarts	.equr	r0
pl_L_checkbottomdif .equr r1
pl_L_topdif		.equr	r2
pl_L_next		.equr	r3
pl_L_bottomstarts .equr	r4
pl_L_xloop		.equr	r5
pl_L_bottomdif	.equr	r6

pl_spanstart	.equr	r15
pl_pl			.equr	r15

pl_x			.equr	r16
pl_stopx		.equr	r17
pl_t1			.equr	r18
pl_t2			.equr	r19
pl_b1			.equr	r20
pl_b2			.equr	r21
pl_oldtop		.equr	r22
pl_oldbottom	.equr	r23
pl_openptr		.equr	r24
pl_ff			.equr	r25
pl_cmdhigh		.equr	r26

pl_stopfp		.equr	r13 ; must stay constant across R_MapPlane

 load	(FP),pl_pl ; get plane                                             pl = *(FP)

 move	pl_pl,pl_openptr                                                   pl_openptr = pl
 load	(pl_pl+3),pl_x     ; pl_x = pl->minx                               pl_x = pl->minx
 addq	#6*4,pl_openptr    ; pl_openptr = pl->open                         pl_openptr += &visplane_t::open
 load	(pl_pl+4),pl_stopx ; pl_stopx = pl->maxx                           pl_stopx = pl->maxx
 
 cmp	pl_x,pl_stopx      ; see if there is any open space                if(pl_x > pl_stopx)
 jump	U_GT,(RETURNPOINT) ; nothing to map                                   return;
 addq	#2,pl_stopx        ; pl_stopx = pl->maxx+2 (harmless delay slot)         pl_stopx += 2 // before jump
 
 subq 	#4,FP            ; space for returnpoint                        FP -= 4
 store	RETURNPOINT,(FP) ; save returnpoint before pushing cmds         *FP = RETURNPOINT
 move	FP,pl_stopfp        ; when command que is back here, stop          pl_stopfp = FP

 move	pl_x,scratch                                                       scratch = pl_x
 shlq	#1,scratch                                                         scratch <<= 1
 add	scratch,pl_openptr  ; pl_openptr = &pl->open[x-1]                  pl_openptr += scratch
 subq	#2,pl_openptr                                                      pl_openptr -= 2
 
 movei	#_spanstart,pl_spanstart ; allow indexed loads on spanstart[]   pl_spanstart = &spanstart
 movei	#$ff,pl_ff                                                      pl_ff = 0xff
  
 movei	#pl_topstarts,pl_L_topstarts                                    pl_L_topstarts = &pl_topstarts
 movei	#pl_checkbottmdif,pl_L_checkbottomdif                           pl_L_checkbottomdiff = &pl_checkbottmdiff
 movei	#pl_topdif,pl_L_topdif                                          pl_L_topdif = &pl_topdif
 movei	#pl_next,pl_L_next                                              pl_L_next = &pl_next
 movei	#pl_bottomstarts,pl_L_bottomstarts                              pl_L_bottomstarts = &pl_bottomstarts
 movei	#pl_xloop,pl_L_xloop                                            pl_L_xloop = &pl_xloop
 movei	#pl_bottomdif,pl_L_bottomdif                                    pl_L_bottomdif = &pl_bottomdif

;	oldtop = open[x-1];
;	oldbottom = oldtop&0xff;
;	oldtop >>= 8;
 loadw	(pl_openptr),pl_t1                                              pl_t1 = *pl_openptr
 addq	#2,pl_openptr                                                      pl_openptr += 2
 move	pl_t1,pl_b1                                                        pl_b1 = pl_t1
 and	pl_ff,pl_b1                                                        pl_b1 &= pl_ff
 shrq	#8,pl_t1                                                           pl_t1 >>= 8
 
 loadw	(pl_openptr),pl_t2 ; delay sloted                               pl_t2 = *pl_openptr
;----------------------
;
pl_xloop:
;
;-----------------------
;		t1 = oldtop;
;		b1 = oldbottom;
;		t2 = open[x];
;		b2 = t2&0xff;
;		t2 >>= 8;
;		oldtop = t2;
;		oldbottom = b2;

 move	pl_t2,pl_b2                                                      pl_b2 = pl_t2
 and	pl_ff,pl_b2                                                      pl_b2 &= pl_ff
 shrq	#8,pl_t2                                                         pl_t2 >>= 8
 move	pl_x,pl_cmdhigh                                                  pl_cmdhight = pl_x
 subq	#1,pl_cmdhigh                                                    pl_cmdhigh -= 1
 shlq	#16,pl_cmdhigh                                                   pl_cmdhigh <<= 16
	
;------------------------
;
; top diffs
;
;------------------------
; if (t1 != t2)
 move	pl_t2,pl_oldtop                                                  pl_toptop = pl_t2
 cmp	pl_t1,pl_oldtop                                                  if(pl_t1 == pl_t2)
 move	pl_b2,pl_oldbottom                                                  pl_oldbottom = pl_b2
 jump	EQ,(pl_L_checkbottomdif)                                            goto *pl_L_checkbottomdif
 addq	#2,pl_openptr ; reordered delay slot                                pl_openptr += 2 // before jump
  
pl_topdif:
 cmp	pl_t1,pl_t2                                                      if(pl_t1 > pl_t2)
 jump	U_GT,(pl_L_topstarts)                                               goto *pl_L_topstarts
 nop
 jump	EQ,(pl_L_topstarts)                                              if(pl_t1 == pl_t2)
 cmp	pl_t1,pl_b1 ; harmless delay slot                                   goto *pl_L_topstarts
 jump	U_GT,(pl_L_topstarts)                                            if(pl_t1 > pl_b1)
                                                                          goto *pl_L_topstarts
;
; R_MapPlane ( ((x-1)<<16) + (t1<<8) + spanstart[t1]);
;
 move	pl_t1,scratch ; harmless delay slot                              scratch = pl_t1 // before jump ^^^
 move	pl_t1,scratch2                                                   scratch2 = pl_t1
 shlq	#8,scratch                                                       scratch <<= 8
 shlq	#2,scratch2                                                      scratch2 <<= 2
 or		pl_cmdhigh,scratch                                            scratch |= pl_cmdhigh
 load	(pl_spanstart+scratch2),scratch2                                 scratch2 = *(pl_spanstart + scratch2)
 subq	#4,FP                                                            FP -= 4
 or		scratch2,scratch                                              scratch |= scratch2
 addqt	#1,pl_t1         ; t1++                                       pl_t1 += 1
 jump	T,(pl_L_topdif)                                                  goto *(pl_L_topdif)
 store	scratch,(FP) ; delay slot                                        *(FP) = scratch // before jump
 
;
; top dif spanstarts
;
pl_topstarts:
 cmp	pl_t2,pl_t1                                                      if(pl_t2 > pl_t1)
 jump	U_GT,(pl_L_checkbottomdif)                                          goto *pl_L_checkbottomdif
 move	pl_t2,scratch2 ; harmless delay slot                                scratch2 = pl_t2 // before jump
 jump	EQ,(pl_L_checkbottomdif)                                         if(pl_t2 == pl_t1) goto *pl_L_checkbottomdif
 shlq	#2,scratch2    ; harmless delay slot                                scratch2 <<= 2 // before jump
 cmp	pl_t2,pl_b2                                                      if(pl_t2 > pl_b2)
 jump	U_GT,(pl_L_checkbottomdif)                                          goto *pl_L_checkbottomdif
 nop

; spanstart[t2] = x
 addqt	#1,pl_t2                                                        pl_t2 += 1
 jr		T,pl_topstarts                                                  goto pl_topstarts (loop)
 store	pl_x,(pl_spanstart+scratch2) ; delay slot                          *(pl_spanstart + scratch2) = pl_x // before jump
  
;------------------------
;
; bottom diffs
; 
;------------------------
pl_checkbottmdif:
; if (b1 != b2)
 cmp	pl_b1,pl_b2                                                       if(b1 == b2)
 jump	EQ,(pl_L_next)                                                       goto *pl_L_next
 
pl_bottomdif:
 cmp	pl_b1,pl_b2 ; harmless delay slot                                 if(pl_b1 <= pl_b2) goto *pl_L_bottomstarts
 jump	U_LE,(pl_L_bottomstarts)
 cmp	pl_b1,pl_t1 ; harmless delay slot                                    if(pl_b1 == pl_t1) goto pl_bottomplane
 jr		EQ,pl_bottomplane
 move	pl_b1,scratch ; harmless delay slot                                      scratch = pl_b1 // before jump ^
 jump	U_LE,(pl_L_bottomstarts)                                                 goto *pl_L_bottomstarts
pl_bottomplane:
;
;R_MapPlane ( ((x-1)<<16) + (b1<<8) + spanstart[b1]);
;
 shlq	#8,scratch ; harmless delay slot                                         scratch <<= 8 // before jump ^^^
 move	pl_b1,scratch2                                                   scratch2 = pl_b1
 or		pl_cmdhigh,scratch                                            scratch |= pl_cmdhigh
 shlq	#2,scratch2                                                      scratch2 <<= 2
 load	(pl_spanstart+scratch2),scratch2                                 scratch2 = *(pl_spanstart + scratch2)
 subq	#4,FP                                                            FP -= 4
 or		scratch2,scratch                                              scratch |= scratch2
 subqt	#1,pl_b1         ; b1--                                       pl_b1 -= 1
 jump	T,(pl_L_bottomdif)                                               goto *pl_L_bottomdif
 store	scratch,(FP) ; delay slot                                        *FP = scratch // before jump
 
;
; bottom dif spanstarts
;
pl_bottomstarts:
 cmp	pl_b2,pl_b1                                                      if(pl_b2 <= pl_b1)
 jump	U_LE,(pl_L_next)                                                    goto *pl_L_next
 cmp	pl_b2,pl_t2 ; harmless delay slot                                   if(pl_b2 == pl_t2)
 jr		EQ,pl_bottommark                                                    goto pl_bottommark
 move	pl_b2,scratch2 ; harmless delay slot                                   scratch2 = pl_b2 // before jump ^^
 jump	U_LT,(pl_L_next)                                                 goto pl_L_next
pl_bottommark:
 shlq	#2,scratch2 ; harmless delay slot                                scratch2 <<= 2 // before jump ^^
; spanstart[b2] = x
; b2--
 subqt	#1,pl_b2                                                      pl_b2 -= 1
 jump	T,(pl_L_bottomstarts)                                            goto *pl_L_bottomstarts
 store	pl_x,(pl_spanstart+scratch2) ; delay slot                     *(pl_spanstart + scratch2) = pl_x
 
;------------------------
;
; next
; 
;------------------------
pl_next:
 addq	#1,pl_x                                                          pl_x += 1
 move	pl_oldbottom,pl_b1                                               pl_b1 = pl_oldbottom
 cmp	pl_x,pl_stopx                                                    if(pl_x != pl_stopx)
 move	pl_oldtop,pl_t1                                                  pl_t1 = pl_oldtop
 jump	NE,(pl_L_xloop)                                                     goto *pl_L_xloop
 loadw	(pl_openptr),pl_t2 ; delay slot                                  pl_t2 = *pl_openptr // before jump

;------------------------
;
; all done calculating, so execute the plane commands
; 
;------------------------
 cmp	FP,pl_stopfp                                                     if(FP != pl_stopfp)
 jr		NE,pl_isadraw                                                    goto pl_isadraw
 nop

; nothing to draw
 load	(FP),RETURNPOINT
 jump	T,(RETURNPOINT)
 addq	#4,FP					; delay slot
 
pl_isadraw:                                                            call R_MapPlane
;
; fall through into R_MapPlane
;
   */
}

void R_DrawPlanes(void)
{
   // CALICO_TODO: R_DrawPlanes
   /*
	movei #36,scratch
	sub scratch,FP

	nop
	store	RETURNPOINT,(FP+1)
	
;================
; load constants into alternate register bank
;================
	movei	#$400000,r0                   r0 = 0x400000
	moveta	r0,alt_400000              alt_400000 = r0

	movei	#_viewx,r0                    r0 = &viewx
	load	(r0),r0                       r0 = *r0
	moveta	r0,alt_planex              alt_planex = r0
 
	movei	#_viewy,r0                    r0 = &viewy
	load	(r0),r0                       r0 = *r0
	neg		r0                         r0 = -r0
	moveta	r0,alt_planey              alt_planey = r0
  
	movei	#_viewangle,r0                r0 = &viewangle
	load	(r0),r0                       r0 = *r0
	moveta	r0,alt_planeangle          alt_planeangle = r0
   
	movei	#1073741824,r1                r1 = ANG90
	sub		r1,r0                      r0 -= r1
	shrq	#19,r0                        r0 >>= ANGLETOFINESHIFT
	move	r0,r17 ;(angle)               angle = r0

 move r17,r1 ;(angle)                  r1 = angle
 shlq #2,r1                            r1 <<= 2
 movei #_finecosine,r2                 r2 = &finecosine
 load (r2),r2                          r2 = *r2
 move r1,r3                            r3 = r1
 add r2,r3                             r3 += r2
 load (r3),r2                          r2 = *r3
 movei #80,r3                          r3 = 80
 move r3,MATH_SIGN                     MATH_SIGN = r3
 move r3,MATH_B                        MATH_B = r3
 xor r2,MATH_SIGN                      MATH_SIGN ^= r2
 abs MATH_B                            MATH_B = abs(MATH_B)
 abs r2                                r2 = abs(r2)
 div MATH_B,r2                         r2 /= MATH_B
 btst #31, MATH_SIGN                   if(MATH_SIGN >= 0)
 jr EQ,L71                                goto L71
 nop
 neg r2                                r2 = -r2
L71:
	moveta	r2,alt_basexscale          alt_basexscale = r2
 
 movei #_finesine,r2                   r2 = &finesine
 add r2,r1                             r1 += r2
 load (r1),r1                          r1 = *r1
 move r3,MATH_SIGN                     MATH_SIGN = r3
 move r3,MATH_B                        MATH_B = r3
 xor r1,MATH_SIGN                      MATH_SIGN ^= r1
 abs MATH_B                            MATH_B = abs(MATH_B)
 abs r1                                r1 = abs(r1)
 div MATH_B,r1                         r1 /= MATH_B
 btst #31, MATH_SIGN                   if(MATH_SIGN >= 0)
 jr EQ,L72                                goto L72
 nop
 neg r1                                r1 = -r1
L72:
 neg r1                                r1 = -r1 // (^^^ ???)
	moveta	r1,alt_baseyscale          alt_baseyscale = r1

L53:                                   // Jag-specific setup
 movei #_junk,r0                       r0 = &junk
 movei #15737400,r1                    r1 = 0xf02238
 load (r1),r1                          r1 = *r1
 store r1,(r0)                         *r0 = r1
 move r1,r0                            r0 = r1
 moveq #1,r1                           r1 = 1
 and r1,r0                             r0 &= r1
 moveq #0,r1                           r1 = 0
 cmp r0,r1                             if(r0 == r1)
 movei #L53,scratch                       goto L53
 jump EQ,(scratch)
 nop

 movei #15737348,r0                    r0 = 0xf02204
 movei #,r1                            r1 = 208928 // 0x33020
 store r1,(r0)                         *r0 = r1

 movei #15737384,r0                    r0 = 0xf02228
 movei #80416,r1                       r1 = 80416 // 0x13A20
 store r1,(r0)                         *r0 = r1

 movei #_visplanes+348,r0              r0 = &visplanes + sizeof(visplane_t)
 move r0,r15 ;(pl)                     pl = r0

 movei #L59,r0                         goto L59
 jump T,(r0)
 nop

L56: // start loop

 move r15,r0 ;(pl)                     r0 = pl
 addq #12,r0                           r0 += &visplane_t::minx
 load (r0),r0                          r0 = *r0
 move r15,r1 ;(pl)                     r1 = pl
 addq #16,r1                           r1 += &visplane_t::maxx
 load (r1),r1                          r1 = *r1
 cmp r0,r1                             if(r0 <= r1)
 movei #L61,scratch                       goto L61
 jump PL,(scratch)
 nop

 movei #L57,r0                         goto L57
 jump T,(r0)
 nop

L61:
L63:
 movei #_junk,r0                       r0 = &junk 
 movei #15737400,r1                    r1 = 0xf02238
 load (r1),r1                          r1 = *r1
 store r1,(r0)                         *r0 = r1
 move r1,r0                            r0 = r1
 moveq #1,r1                           r1 = 1
 and r1,r0                             r0 &= r1
 moveq #0,r1                           r1 = 0
 cmp r0,r1                             if(r0 == r1)
 movei #L63,scratch                       goto L63
 jump EQ,(scratch)
 nop

	load (r15+1),r1	; (pl)            r1 = *(pl+&visplane_t::picnum)
	movei #$f02200,r0		; plane source r0 = 0xf02200
	store r1,(r0)                       *r0 = r1

	load (r15),r0 ;(pl)                 r0 = pl->height
	abs r0                              r0 = abs(r0)
	moveta	r0,alt_planeheight         alt_planeheight = r0

L66:
 move r15,r0 ;(pl)                     r0 = pl
 addq #8,r0                            r0 += &visplane_t::lightlevel
 load (r0),r0                          r0 = *r0
 move r0,r16 ;(light)                  light = r0
 movei #255,r1                         r1 = 255
 sub r16,r1 ;(light)                   r1 -= light
 shlq #1,r1                            r1 <<= 1
 move r16,r2 ;(light)                  r2 = light
 sub r1,r2                             r2 -= r1
 moveta r2,alt_plane_lightmin          alt_plane_lightmin = r2
 moveq #0,r1                           r1 = 0
 cmp r2,r1                             if(r2 == r1)
 movei #L68,scratch                       goto L68
 jump EQ,(scratch)
 nop
 jump MI,(scratch)                     if(r2 > r1)
 nop                                      goto L68

 moveq #0,r1                           r1 = 0
 moveta	r1,alt_plane_lightmin         alt_plane_lightmin = r1

L68:
	moveta	r16,alt_plane_lightmax     alt_plane_lightmax = light
	
	movei	#160,r1                       r1 = 160 // SCREENWIDTH?
	movefa	alt_plane_lightmin,r2      r2 = alt_plane_lightmin
	move	r16,r3 ;(light)               r3 = light
	sub		r2,r3                      r3 -= r2
	move	r3,r4                         r4 = r3
	imult	r1,r3                         r3 *= r1
	movei	#640,r2                       r2 = 640
 	div		r2,r3                      r3 /= r2
	moveta	r3,alt_plane_lightsub      alt_plane_lightsub = r3

	shlq	#11,r4                        r4 <<= SLOPEBITS
	moveta	r4,alt_plane_lightcoef     alt_plane_lightcoef = r4
 
 move r15,r0 ;(pl)                     r0 = pl
 addq #16,r0                           r0 += &visplane_t::maxx
 load (r0),r0                          r0 = *r0
 shlq #1,r0                            r0 <<= 1
 move r15,r1 ;(pl)                     r1 = pl
 addq #26,r1                           r1 += &visplane_t::open[1]
 add r1,r0                             r0 += r1
 movei #65280,r1                       r1 = OPENMASK
 storew r1,(r0)                        *r0 = r1

 move r15,r0 ;(pl)                     r0 = pl
 addq #12,r0                           r0 += &visplane_t::minx
 load (r0),r0                          r0 = *r0
 shlq #1,r0                            r0 <<= 1
 subq #2,r0                            r0 -= 2
 move r15,r1 ;(pl)                     r1 = pl
 addq #24,r1                           r1 &= &visplane_t::open[0]
 add r1,r0                             r0 += r1
 movei #65280,r1                       r1 = OPENMASK
 storew r1,(r0)                        *r0 = r1

	store	r15,(FP) ; arg[] ;(pl)        (*FP) = r15
	movei	#_R_PlaneLoop,r0              r0 = R_PlaneLoop
	move	PC,RETURNPOINT
	jump	T,(r0)                        call R_PlaneLoop
	addq	#6,RETURNPOINT
	load	(FP),r15
	
L57: // loop increment
 movei #348,r0                         r0 = sizeof(visplane_t)                
 move r15,r1 ;(pl)                     r1 = pl
 add r0,r1                             r1 += r0
 move r1,r15 ;(pl)                     pl = r1

L59: // end loop
 move r15,r0 ;(pl)                     r0 = pl
 movei #_lastvisplane,r1               r1 = &lastvisplane
 load (r1),r1                          r1 = *r1
 cmp r0,r1                             if(r0 < r1)
 movei #L56,scratch                       goto L56
 jump U_LT,(scratch)
 nop

 movei #_phasetime+28,r0
 movei #_samplecount,r1
 load (r1),r1
 store r1,(r0)

 movei #_gpucodestart,r0
 movei #_ref8_start,r1
 store r1,(r0)

L52:
	load	(FP+1),RETURNPOINT
	movei	#36,scratch
	jump	T,(RETURNPOINT)
	add		scratch,FP ; delay slot
   */
}

// EOF

