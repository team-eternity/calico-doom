/*
  CALICO

  Renderer phase 8 - Sprites
*/

#include "r_local.h"

#define OPENMARK 0xff00

static int spropening[SCREENWIDTH + 1];

static void R_DrawVisSprite(vissprite_t *vis)
{
   // CALICO_TODO
   /*
dv_blitter		.equr	r15
dv_bottom		.equr	r16
dv_column		.equr	r17
dv_frac			.equr	r18
dv_topclip		.equr	r19
dv_spryscale	.equr	r20
dv_bottomclip	.equr	r21
dv_x			.equr	r22

dv_col_offset	.equr	r23
dv_oldcolread	.equr	r24
dv_top			.equr	r25

dv_patch		.equr	r13
dv_iscale		.equr	r12
dv_xfrac		.equr	r9
dv_sprtop		.equr	r8
dv_stopx		.equr	r7
dv_fracstep		.equr	r6

dv_vis		.equr	r15

	load	(FP),dv_vis

	btst	#0,dv_vis				; scoreboard bug	
	load	(dv_vis+8),dv_patch		; vis->patch
	load	(dv_vis+6),dv_iscale	; vis->yiscale	 
 	load	(dv_vis+2),dv_xfrac		; vis->startfrac
	load	(dv_vis+5),dv_spryscale		; spryscale
	
	shrq	#8,dv_spryscale			; make fit in 16 bits
	
	load	(dv_vis+7),MATH_A			; vis->texturemid
 
;======= FixedMul 16 * +-32 ===
 
	move    MATH_A,MATH_SIGN
	abs     MATH_A
	move    MATH_A,RETURNVALUE
	shrq	#16,RETURNVALUE
	mult    dv_spryscale,RETURNVALUE
	mult    dv_spryscale,MATH_A
	shlq	#8,RETURNVALUE
	shrq	#8,MATH_A

	btst    #31,MATH_SIGN
	jr		EQ,fixednotneg
	add		MATH_A,RETURNVALUE		; delay slot
	
	neg     RETURNVALUE
fixednotneg:
;====================
	movei #5898240,dv_sprtop
	sub RETURNVALUE,dv_sprtop

	movei	#$f02238,r0
blitwait1:
	load	(r0),r1
	btst	#0,r1
	jr		EQ,blitwait1
	nop
	
	movei	#15737344,r0
	load	(dv_vis+14),r1		; vis->pixels
	store	r1,(r0)

	movei	#15737360,r0
	move	dv_iscale,r1
	sharq	#16,r1
	store	r1,(r0)

	movei	#15737364,r0
	move	dv_iscale,r1 ; local iscale
	shlq	#16,r1
	shrq	#16,r1		; mask off high bits
	store 	r1,(r0)

	load	(dv_vis+9),r0	; vis->colormap
	movei	#255,r1
	sub		r0,r1
	shlq	#14,r1
	neg		r1
	shlq	#8,r1
	shrq	#8,r1			; mask off high 8 bits
	movei	#$f02270,r0
	store	r1,(r0)			; blitter iinc
	
;===========================
;
; for x = vis->x1 ; x<stopx ; x++, xfrac+= fracstep
;
;===========================
	load	(dv_vis),dv_x
	load	(dv_vis+1),dv_stopx
	load	(dv_vis+4),dv_fracstep
	
	movei	#$f02200,dv_blitter

	movei	#L414,r0
	jump	T,(r0)
	addq	#1,dv_stopx				; delay slot

L411:

	move	dv_xfrac,dv_column
	shrq	#16,dv_column
	shlq	#1,dv_column
	addq	#8,dv_column
	add		dv_patch,dv_column
	loadw	(dv_column),dv_column
	add		dv_patch,dv_column
	 
	move	dv_x,r0 ;(x)
	shlq	#2,r0
	movei	#_spropening,r1
	add		r1,r0
	load	(r0),dv_bottomclip
	load	(dv_column),dv_oldcolread	; start first read now
	move	dv_bottomclip,dv_topclip
	shrq	#8,dv_topclip

	shlq	#24,dv_bottomclip
	shrq	#24,dv_bottomclip		; mask off upper 24 bits


	movei	#nextpost,r0
	jump	T,(r0)
	subq	#1,dv_bottomclip		; delay slot

;
; column loop
;
L415:
	mult	dv_spryscale,dv_top
	shlq	#8,dv_top
	add		dv_sprtop,dv_top

	mult	dv_spryscale,dv_bottom
	shlq	#8,dv_bottom
	add		dv_top,dv_bottom
	 
	movei	#65535,r0
	add		r0,dv_top
	sharq	#16,dv_top

	subq	#1,dv_bottom
	sharq	#16,dv_bottom
		
;
; clip to bottom
;
	cmp		dv_bottom,dv_bottomclip ;(bottom)(bottomclip)
	jr		PL,bottomclipped
	nop
	move	dv_bottomclip,dv_bottom ;(bottomclip)(bottom)
bottomclipped:

;
; clip to top
;
	cmp		dv_topclip,dv_top
	jr		PL,topclipped
	move	dv_col_offset,dv_frac			; delay slot

	move	dv_topclip,r0
	sub		dv_top,r0
	
	move	dv_iscale,dv_frac
	shrq	#16,dv_frac
	mult	r0,dv_frac
	shlq	#16,dv_frac
	move	dv_iscale,scratch
	mult	r0,scratch
	add		scratch,dv_frac	; delay slot

	move 	dv_topclip,dv_top
	add		dv_col_offset,dv_frac
	
topclipped:
;
; calc count
;
	sub		dv_top,dv_bottom
	movei	#nextpost,scratch
	jump	MI,(scratch)
	addq	#1,dv_bottom			; delay slot

;
; program blitter
;
blitwait2:
	load	(dv_blitter+14),r1
	btst	#0,r1
	jr		EQ,blitwait2
	move	dv_frac,r1 ;(frac)			; delay slot
	
	shrq	#16,r1
	shlq	#16,dv_frac
	store	r1,(dv_blitter+3)			; a1 pixel
	shrq	#16,dv_frac
	store 	dv_frac,(dv_blitter+6)		; a1 frac

	move	dv_top,r1
	shlq	#16,r1
	add		dv_x,r1
	store	r1,(dv_blitter+12)

	shlq	#16,dv_bottom
	addq	#1,dv_bottom
	store	dv_bottom,(dv_blitter+15)		; count

	movei	#1098919681,r1
	store	r1,(dv_blitter+14)

;
; next post
;	
nextpost:

; a post record has four bytes: topdelta length pixelofs*2
	addq	#4,dv_column				; advance to next post
	btst	#15,dv_oldcolread			; last column marker
	move	dv_oldcolread,dv_col_offset
	jr		NE,nextx
	move	dv_col_offset,dv_top		; harmless delay slot
	
	load	(dv_column),dv_oldcolread	; start next read now
	
	shrq	#24,dv_top
	move	dv_col_offset,dv_bottom
	shlq	#8,dv_bottom
	shrq	#24,dv_bottom
		
	movei	#L415,scratch
	jump	T,(scratch)
	shlq	#16,dv_col_offset		; delay slot, leave offset <<16 

;
; next x
;
nextx:
	addq	#1,dv_x
	add		dv_fracstep,dv_xfrac

L414:
	cmp		dv_x,dv_stopx
	movei	#L411,scratch
	jump	U_LT,(scratch)
	nop

	jump T,(RETURNPOINT)
	nop
   */
}

//
// Compare the vissprite to a viswall. Similar to R_PointOnSegSide, but less accurate.
//
static boolean R_SegBehindPoint(viswall_t *viswall, int dx, int dy)
{
   fixed_t x1, y1, sdx, sdy;

   x1  = viswall->seg->v1->x;
   y1  = viswall->seg->v1->y;
   sdx = viswall->seg->v2->x;
   sdy = viswall->seg->v2->y;

   sdx -= x1;
   sdy -= y1;
   dx  -= x1;
   dy  -= y1;

   sdx /= FRACUNIT;
   sdy /= FRACUNIT;
   dx  /= FRACUNIT;
   dy  /= FRACUNIT;

   dx  *= sdy;
   sdx *=  dy;

   return (sdx < dx);
}

//
// Clip a sprite to the openings created by walls
//
static void R_ClipVisSprite(vissprite_t *vis)
{
   int     x;          // r15
   int     x1;         // FP+5
   int     x2;         // r22
   fixed_t gz;         // FP+8
   int     gzt;        // FP+9
   int     scalefrac;  // FP+3
   int     r1;         // FP+7
   int     r2;         // r18
   int     silhouette; // FP+4
   byte   *topsil;     // FP+6
   byte   *bottomsil;  // r21
   int     opening;    // r16
   int     top;        // r19
   int     bottom;     // r20
   
   viswall_t *ds;      // r17

   x1  = vis->x1;
   x2  = vis->x2;
   gz  = (vis->gz  - viewz) / (1 << 10);
   gzt = (vis->gzt - viewz) / (1 << 10);
   
   scalefrac = vis->yscale;
   
   x = vis->x1;

   while(x <= x2)
   {
      spropening[x] = SCREENHEIGHT;
      ++x;
   }
   
   ds = lastwallcmd;
   do
   {
      --ds;

      if(ds->start > x2 || ds->stop < x1 || ds->scalefrac < scalefrac || ds->scale2 < scalefrac ||
         !(ds->actionbits & (AC_TOPSIL | AC_BOTTOMSIL | AC_SOLIDSIL)))
      {
         continue;
      }

      if(ds->scalefrac <= scalefrac || ds->scale2 <= scalefrac)
      {
         if(R_SegBehindPoint(ds, vis->gx, vis->gy))
            continue;
      }

      r1 = ds->start < x1 ? x1 : ds->start;
      r2 = ds->stop  > x2 ? x2 : ds->stop;

      silhouette = (ds->actionbits & (AC_TOPSIL | AC_BOTTOMSIL | AC_SOLIDSIL));

      if(silhouette == AC_SOLIDSIL)
      {
         x = r1;
         while(x <= r2)
         {
            spropening[x] = (SCREENHEIGHT << 8);
            ++x;
         }
         continue;
      }

      topsil    = ds->topsil;
      bottomsil = ds->bottomsil;

      if(silhouette == AC_BOTTOMSIL)
      {
         x = r1;
         while(x <= r2)
         {
            opening = spropening[x];
            if((opening & 0xff) == SCREENHEIGHT)
               spropening[x] = (opening & OPENMARK) + bottomsil[x];
            ++x;
         }
      }
      else if(silhouette == AC_TOPSIL)
      {
         x = r1;
         while(x <= r2)
         {
            opening = spropening[x];
            if(!(opening & OPENMARK))
               spropening[x] = (topsil[x] << 8) + (opening & 0xff);
            ++x;
         }
      }
      else if(silhouette == (AC_TOPSIL | AC_BOTTOMSIL))
      {
         x = r1;
         while(x <= r2)
         {
            top    = spropening[x];
            bottom = top & 0xff;
            top >>= 8;
            if(bottom == SCREENHEIGHT)
               bottom = bottomsil[x];
            if(top == 0)
               top = topsil[x];
            spropening[x] = (top << 8) + bottom;
            ++x;
         }
      }
   }
   while(ds != viswalls);
}

//
// Render all sprites
//
void R_Sprites(void)
{
   // Jag-specific blitter setup
   /*
   movei #15737348,r0                         r0 = 0xf02204
   movei #145440,r1                           r1 = 0x23820
   store r1,(r0)                              *r0 = r1
   
   movei #15737384,r0                         r0 = 0xf02228
   movei #145952,r1                           r1 = 0x23A20
   store r1,(r0)                              *r0 = r1
   */

   ptrdiff_t i = 0, count = lastsprite_p - vissprites;
   vissprite_t *best = NULL;

   // draw mobj sprites
   while(i < count)
   {
      fixed_t bestscale = MAXINT;
      vissprite_t *ds = vissprites;

      while(ds != lastsprite_p)
      {
         if(ds->xscale < bestscale)
         {
            bestscale = ds->xscale;
            best = ds;
         }
         ++ds;
      }

      if(best->patch != NULL)
      {
         R_ClipVisSprite(best);
         R_DrawVisSprite(best);
      }

      best->xscale = MAXINT;

      ++i;
   }

   // draw psprites
   while(lastsprite_p < vissprite_p)
   {
      ptrdiff_t stopx = lastsprite_p->x2 + 1;
      i = lastsprite_p->x1;
      
      // clear out the clipping array across the range of the psprite
      while(i < stopx)
      {
         spropening[i] = SCREENHEIGHT;
         ++i;
      }

      R_DrawVisSprite(lastsprite_p);
      
      ++lastsprite_p;
   }
}

// EOF

