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

	load	(FP),dv_vis                                                vis = *FP

	btst #0,dv_vis ; scoreboard bug                            
	load (dv_vis+8),dv_patch ; vis->patch                            dv_patch = vis->patch
	load (dv_vis+6),dv_iscale ; vis->yiscale                         dv_iscale = vis->yiscale
 	load (dv_vis+2),dv_xfrac ; vis->startfrac                        dv_xfrac = vis->startfrac
	load (dv_vis+5),dv_spryscale ; spryscale                         dv_spryscale = vis->yscale
	
	shrq #8,dv_spryscale ; make fit in 16 bits                       dv_spryscale >>= 8
	
	load (dv_vis+7),MATH_A ; vis->texturemid                         MATH_A = vis->texturemid
 
;======= FixedMul 16 * +-32 ===
 
	move MATH_A,MATH_SIGN
	abs  MATH_A
	move MATH_A,RETURNVALUE
	shrq #16,RETURNVALUE
	mult dv_spryscale,RETURNVALUE
	mult dv_spryscale,MATH_A
	shlq #8,RETURNVALUE
	shrq #8,MATH_A

	btst #31,MATH_SIGN
	jr   EQ,fixednotneg
	add  MATH_A,RETURNVALUE ; delay slot
	
	neg  RETURNVALUE
fixednotneg:
;====================
	movei #5898240,dv_sprtop                                         dv_sprtop = 5898240 // ???
	sub   RETURNVALUE,dv_sprtop                                      dv_sprtop -= RETURNVALUE

	movei #$f02238,r0                                                r0 = 0xf02238
blitwait1: // busy loop
	load (r0),r1                                                     r1 = *r0
	btst #0,r1                                                       if(!r1)
	jr   EQ,blitwait1                                                   goto blitwait1
	nop
	
	movei #15737344,r0                                               r0 = 0xf02200
	load  (dv_vis+14),r1 ; vis->pixels                               r1 = vis->pixels 
	store r1,(r0)                                                    *r0 = r1

	movei #15737360,r0                                               r0 = 0xf02210
	move  dv_iscale,r1                                               r1 = dv_iscale
	sharq #16,r1                                                     r1 >>= 16
	store r1,(r0)                                                    *r0 = r1

	movei #15737364,r0                                               r0 = 0xf02214
	move  dv_iscale,r1 ; local iscale                                41 = dv_iscale
	shlq  #16,r1                                                     r1 <<= 16
	shrq  #16,r1 ; mask off high bits                                r1 >>= 16
	store r1,(r0)                                                    *r0 = r1

	load  (dv_vis+9),r0 ; vis->colormap                              r0 = vis->colormap
	movei #255,r1                                                    r1 = 255
	sub   r0,r1                                                      r1 -= r0
	shlq  #14,r1                                                     r1 <<= 14
	neg   r1                                                         r1 = -r1
	shlq  #8,r1                                                      r1 <<= 8
	shrq  #8,r1 ; mask off high 8 bits                               r1 >>= 8
	movei #$f02270,r0                                                r0 = 0xf02270 // B_IINC
	store r1,(r0) ; blitter iinc                                     *r0 = r1
	
;===========================
;
; for x = vis->x1 ; x<stopx ; x++, xfrac+= fracstep
;
;===========================
	load (dv_vis),dv_x                                           dv_x = vis->x1
	load (dv_vis+1),dv_stopx                                     dv_stopx = vis->x2
	load (dv_vis+4),dv_fracstep                                  dv_fracstep = vis->xiscale
	
	movei #$f02200,dv_blitter                                    dv_blitter = 0xf02200

	movei #L414,r0                                               goto L414
	jump  T,(r0)
	addq  #1,dv_stopx ; delay slot                                  dv_stopx += 1 // before jump^^

L411: // start loop

	move  dv_xfrac,dv_column                                     dv_column = dv_xfrac
	shrq  #16,dv_column                                          dv_column >>= 16
	shlq  #1,dv_column                                           dv_column <<= 1
	addq  #8,dv_column                                           dv_column += 8
	add   dv_patch,dv_column                                     dv_column += dv_patch
	loadw (dv_column),dv_column                                  dv_column = *dv_column
	add   dv_patch,dv_column                                     dv_column += dv_patch
	 
	move  dv_x,r0 ;(x)                                           r0 = dv_x
	shlq  #2,r0                                                  r0 <<= 2
	movei #_spropening,r1                                        r1 = &spropening
	add   r1,r0                                                  r0 += r1
	load  (r0),dv_bottomclip                                     dv_bottomclip = *r0
	load  (dv_column),dv_oldcolread ; start first read now       dv_oldcolread = *dv_column
	move  dv_bottomclip,dv_topclip                               dv_topclip = dv_bottomclip
	shrq  #8,dv_topclip                                          dv_topclip >>= 8

	shlq #24,dv_bottomclip                                       dv_bottomclip <<= 24
	shrq #24,dv_bottomclip ; mask off upper 24 bits              dv_bottomclip >>= 24


	movei #nextpost,r0                                           
	jump  T,(r0)                                                 goto nextpost
	subq  #1,dv_bottomclip ; delay slot

;
; column loop
;
L415:
	mult dv_spryscale,dv_top                                     dv_top *= dv_spryscale
	shlq #8,dv_top                                               dv_top <<= 8
	add  dv_sprtop,dv_top                                        dv_top += dv_sprtop

	mult dv_spryscale,dv_bottom                                  dv_bottom *= dv_spryscale
	shlq #8,dv_bottom                                            dv_bottom <<= 8
	add  dv_top,dv_bottom                                        dv_bottom += dv_top
	 
	movei #65535,r0                                              r0 = 65535
	add   r0,dv_top                                              dv_top += r0
	sharq #16,dv_top                                             dv_top >>= 16

	subq  #1,dv_bottom                                           dv_bottom -= 1
	sharq #16,dv_bottom                                          dv_bottom >>= 16
		
;
; clip to bottom
;
	cmp dv_bottom,dv_bottomclip ;(bottom)(bottomclip)            if(dv_bottom <= dv_bottomclip)
	jr  PL,bottomclipped                                            goto bottomclipped
	nop
   move dv_bottomclip,dv_bottom ;(bottomclip)(bottom)           dv_bottom = dv_bottomclip

bottomclipped:
;
; clip to top
;
	cmp dv_topclip,dv_top                                        if(dv_topclip <= dv_top)
	jr  PL,topclipped                                               goto topclipped
	move dv_col_offset,dv_frac ; delay slot                      dv_frac = dv_col_offset

	move dv_topclip,r0                                           r0 = dv_topclip
	sub  dv_top,r0                                               r0 -= dv_top
	
	move dv_iscale,dv_frac                                       dv_frac = dv_iscale
	shrq #16,dv_frac                                             dv_frac >>= 16
	mult r0,dv_frac                                              dv_frac *= r0
	shlq #16,dv_frac                                             dv_frac <<= 16
	move dv_iscale,scratch                                       scratch = dv_iscale
	mult r0,scratch                                              scratch *= r0
	add  scratch,dv_frac ; delay slot                            dv_frac += scratch // delay slot...

	move dv_topclip,dv_top                                       dv_top = dv_topclip
	add  dv_col_offset,dv_frac                                   dv_frac += dv_col_offset
	
topclipped:
;
; calc count
;
	sub   dv_top,dv_bottom                                       dv_bottom -= dv_top
	movei #nextpost,scratch                                      
	jump  MI,(scratch)                                           if(??? > ???) goto nextpost
	addq  #1,dv_bottom ; delay slot                              dv_bottom += 1 // before jump

;
; program blitter
;
blitwait2:
	load (dv_blitter+14),r1                                      r1 = *(dv_blitter+14)
	btst #0,r1                                                   if(!r1)
	jr   EQ,blitwait2                                            goto blitwait2
	move dv_frac,r1 ;(frac) ; delay slot                         r1 = dv_frac // delay slot
	
	shrq  #16,r1                                                 r1 >>= 16
	shlq  #16,dv_frac                                            dv_frac <<= 16
	store r1,(dv_blitter+3) ; a1 pixel                           *(dv_blitter+3) = r1
	shrq  #16,dv_frac                                            dv_frac >>= 16
	store dv_frac,(dv_blitter+6) ; a1 frac                       *(dv_blitter+6) = dv_frac

	move dv_top,r1                                               r1 = dv_top
	shlq #16,r1                                                  r1 <<= 16
	add  dv_x,r1                                                 r1 += dv_x
	store r1,(dv_blitter+12)                                     *(dv_blitter+12) = r1

	shlq  #16,dv_bottom                                          dv_bottom <<= 16
	addq  #1,dv_bottom                                           dv_bottom += 1
	store dv_bottom,(dv_blitter+15) ; count                      *(dv_blitter+15) = dv_bottom

	movei #1098919681,r1                                         r1 = 1098919681
	store r1,(dv_blitter+14)                                     *(dv_blitter+14) = r1

;
; next post
;	
nextpost:
; a post record has four bytes: topdelta length pixelofs*2
	addq #4,dv_column ; advance to next post                       dv_column += 4
	btst #15,dv_oldcolread ; last column marker                    dv_oldcolread & (1<<15)?
	move dv_oldcolread,dv_col_offset                               dv_col_offset = dv_cololdread
	jr   NE,nextx                                                  if(^^^) goto nextx
	move dv_col_offset,dv_top ; harmless delay slot                  dv_top = dv_col_offset // before jump
	
	load (dv_column),dv_oldcolread ; start next read now         dv_oldcolread = *dv_column
	
	shrq #24,dv_top                                              dv_top >>= 24
	move dv_col_offset,dv_bottom                                 dv_bottom = dv_col_offset
	shlq #8,dv_bottom                                            dv_bottom <<= 8
	shrq #24,dv_bottom                                           dv_bottom >>= 24
		
	movei #L415,scratch                                          goto L415
	jump  T,(scratch)
	shlq  #16,dv_col_offset ; delay slot, leave offset <<16      dv_col_offset <<= 16 // before jump

;
; next x
;
nextx:
	addq #1,dv_x                                                 dv_x += 1
	add  dv_fracstep,dv_xfrac                                    dv_xfrac += dv_fracstep

L414:
	cmp   dv_x,dv_stopx                                          if(dv_x < dv_stopx)
	movei #L411,scratch                                             goto L411
	jump  U_LT,(scratch)
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

