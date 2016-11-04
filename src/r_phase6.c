/*
  CALICO

  Renderer phase 6 - Seg Loop 
*/

#include "r_local.h"

#define OPENMARK 0xff00

static visplane_t *R_FindPlane(visplane_t *check, fixed_t height, int picnum, 
                               int lightlevel, int start, int stop)
{
   int i;

   while(check < lastvisplane)
   {
      if(height == check->height && // same plane as before?
         picnum == check->picnum &&
         lightlevel == check->lightlevel)
      {
         if(check->open[start] == OPENMARK)
         {
            // found a plane, so adjust bounds and return it
            if(start < check->minx) // in range of the plane?
               check->minx = start; // mark the new edge
            if(stop > check->maxx)
               check->maxx = stop;  // mark the new edge

            return check; // use the same one as before
         }
      }
      ++check;
   }

   // make a new plane
   check = lastvisplane;
   ++lastvisplane;

   check->height = height;
   check->picnum = picnum;
   check->lightlevel = lightlevel;
   check->minx = start;
   check->maxx = stop;

   for(i = 0; i < 80; i++)
   {
      check->open[i*2  ] = OPENMARK;
      check->open[i*2+1] = OPENMARK;
   }

   return check;
}

static void R_DrawTexture(void)
{
   // CALICO_TODO
   /*
dt_tex		.equr	r15
dt_blitter	.equr	r15

dt_top		.equr	r16
dt_bottom	.equr	r17
dt_colnum	.equr	r18
dt_frac		.equr	r5
dt_centery	.equr	r7

dt_scratch	.equr	r10
dt_scratch2	.equr	r11
dt_MATHA	.equr	r0
dt_MATHB	.equr	r1

dtb_base	.equr	r0
dtb_a1pix	.equr	r1
dtb_a1frac	.equr	r2
dtb_a1inc	.equr	r3
dtb_a1incfrac .equr r4

dtb_count	.equr	r7
dtb_command	.equr	r8

;
; common to draw texture
;
dt_floorclipx	.equr	r19
dt_ceilingclipx	.equr	r20
dt_x			.equr	r21
dt_scale		.equr	r22

dt_iscale		.equr	r23
dt_texturecol	.equr	r24
dt_texturelight	.equr	r25

  ; tex->topheight
  load  (dt_tex+3),dt_scratch                         dt_scratch = *(dt_tex+3);
  
  ; top = CENTERY - ((scale * tex->topheight) >> 15)
  imult dt_scale,dt_scratch                           dt_scratch *= dt_scale;
  sharq #15,dt_scratch                                dt_scratch >>= 15;
  movei #90,dt_centery                                dt_centery = 90; // 180/2, CENTERY
  move  dt_centery,dt_top                             dt_top = dt_centery;
  sub   dt_scratch,dt_top                             dt_top -= dt_scratch;
  
  ; if (top > ceilingclipx)
  cmp   dt_top,dt_ceilingclipx                        if(top > ceilingclipx)
  jr    S_GT,dt_topok                                   goto dt_topok;
  nop
  move  dt_ceilingclipx,dt_top                        dt_top = dt_ceilingclipx;
  addq  #1,dt_top                                     dt_top += 1;

dt_topok:
  ; tex->bottomheight
  load  (dt_tex+4),dt_scratch                         dt_scratch = *(dt_tex+4);
  
  ; bottom = CENTERY - 1 - ((scale * tex->botheight) >> 15)
  imult dt_scale,dt_scratch                           dt_scratch *= dt_scale;
  sharq #15,dt_scratch                                dt_scratch >>= 15;
  move  dt_centery,dt_bottom                          dt_bottom = dt_centery;
  subq  #1,dt_bottom                                  dt_bottom -= 1;
  sub   dt_scratch,dt_bottom                          dt_bottom -= dt_scratch;
  
  ; if (bottom < clipbottom)
  cmp   dt_bottom,dt_floorclipx                       if(dt_bottom < dt_floorclipx)
  jr    S_LT,dt_bottomok                                goto dt_bottomok;
  nop
  move  dt_floorclipx,dt_bottom                       dt_bottom = dt_floorclipx;
  subqt #1,dt_bottom                                  dt_bottom -= 1;
 
dt_bottomok:
  ; if (top>bottom) return
  cmp   dt_top,dt_bottom                              if(dt_top > dt_bottom)
  jump  S_GT,(RETURNPOINT)                              return;
  nop

;===========

  move  dt_texturecol,dt_colnum                       dt_colnum = dt_texturecol;
  
; frac = tex->texturemid - (CENTERY - top) * tx_iscale
  move  dt_centery,dt_MATHB                           MATHB = dt_centery;
  sub   dt_top,dt_MATHB                               MATHB -= dt_top;
  move  dt_MATHB,dt_scratch                           dt_scratch = MATHB;
  move  dt_iscale,dt_MATHA                            MATHA = dt_iscale;
  abs   dt_MATHB                                      MATHB = abs(MATHB);
  move  dt_MATHA,dt_scratch2                          dt_scratch2 = MATHA;
  shrq  #16,dt_scratch2                               dt_scratch2 >>= 16;
  mult  dt_MATHB,dt_MATHA                             MATHA *= MATHB;
  mult  dt_MATHB,dt_scratch2                          dt_scratch2 *= MATHB;
  shlq  #16,dt_scratch2                               dt_scratch2 <<= 16;
                                                      MATHA = dt_scratch2; // below...
  btst  #31,dt_scratch                                if(!(dt_scratch & (1<<31)))
  jr    EQ,dt_notneg                                    goto dt_notneg;
  add   dt_scratch2,dt_MATHA ; delay slot              // above..
  neg   dt_MATHA                                      MATHA = -MATHA;
  
dt_notneg:
  ; tex->texturemid
  load  (dt_tex+5),dt_frac                            dt_frac = *(dt_tex+5);
  
  sub   dt_MATHA,dt_frac                              dt_frac -= MATHA;

; while(frac < 0) { colnum--; frac += tex->height<<16; }
  jr    PL,fracpos                                    
  nop
subagain:
  ; tex->height
  load  (dt_tex+2),dt_scratch                         dt_scratch = *(dt_tex+2);
  
  shlq  #16,dt_scratch                                dt_scratch <<= 16;
  add   dt_scratch,dt_frac                            dt_frac += dt_scratch;
  jr    MI,subagain
  subq  #1,dt_colnum                                    dt_colnum -= 1;

fracpos:
; colnum = colnum - tex->width * (colnum / tex->width)
  load  (dt_tex+1),dt_scratch ; tex->width            dt_scratch = *(dt_tex+1);
  subq  #1,scratch                                    scratch -= 1;
  and   scratch,dt_colnum                             dt_colnum &= scratch;
  
; a1inc
  move  dt_iscale,dtb_a1inc                           dtb_a1inc = dt_iscale;
  shrq  #16,dtb_a1inc                                 dtb_a1inc >>= 16;
  
; a1incfrac
  movei #$ffff,dtb_a1incfrac                          dtb_a1incfrac = 0xffff;
  and   dt_iscale,dtb_a1incfrac                       dtb_a1incfrac &= dt_iscale;

; count
  move  dt_bottom,dtb_count                           dtb_count = dt_bottom;
  sub   dt_top,dtb_count                              dtb_count -= dt_top;
  addq  #1,dtb_count                                  dtb_count += 1;
  shlq  #16,dtb_count                                 dtb_count <<= 16;
  addq  #1,dtb_count                                  dtb_count += 1;
  
; a2pix
  shlq  #16,dt_top                                    dt_top <<= 16;
  or    dt_x,dt_top ; screen x                        dt_top |= dt_x;

; frac += (colnum * tex->height) << 16
  load  (dt_tex+2),dt_scratch ; tex->height           dt_scratch = *(dt_tex+2);
  mult  dt_colnum,dt_scratch                          dt_scratch *= dt_colnum;
  shlq  #16,dt_scratch                                dt_scratch <<= 16;
  add   dt_scratch,dt_frac                            dt_frac += dt_scratch;

; a1pix
  move  dt_frac,dtb_a1pix                             dtb_a1pix = dt_frac;
  shrq  #16,dtb_a1pix                                 dtb_a1pix >>= 16;
  
; a1pix frac
  movei #$ffff,dtb_a1frac                             dtb_a1frac = 0xffff;
  and   dt_frac,dtb_a1frac                            dtb_a1frac &= dt_frac;
  
;===========

; base
  load  (dt_tex),dtb_base ; dt->data                  dtb_base = *dt_tex;

  // JAG SPECIFIC STARTING FROM HERE (R_DrawColumn?)
; command
  movei #1+(1<<8)+(1<<9)+(1<<10)+(1<<11)+(1<<13)+(1<<30)+(12<<21),dtb_command // ?????
  
  movei #$f02200,dt_blitter                           dt_blitter = 0xf02200;
  movei #$f02270,dt_scratch2 ; iinc blitter register  dt_scratch2 = 0xf02270; // B_IINC
   
dt_wait: // busy loop
  load  (dt_blitter+14),dt_scratch                    dt_scratch = *(dt_blitter+14);
  btst  #0,dt_scratch                                 if(blitter is busy)
  jr    EQ,dt_wait                                      goto dt_wait;
  nop

  store dtb_base,(dt_blitter)                         *(dt_blitter+0)  = dtb_base;
  store dtb_a1pix,(dt_blitter+3)                      *(dt_blitter+3)  = dtb_a1pix; 
  store dtb_a1frac,(dt_blitter+6)                     *(dt_blitter+6)  = dtb_a1frac;
  store dtb_a1inc,(dt_blitter+4)                      *(dt_blitter+4)  = dtb_a1inc;
  store dtb_a1incfrac,(dt_blitter+5)                  *(dt_blitter+5)  = dtb_a1incfrac;
  store dt_top,(dt_blitter+12)                        *(dt_blitter+12) = dt_top;
  store dt_texturelight,(dt_scratch2)                 *(dt_scratch2)   = dt_texturelight;
  store dtb_count,(dt_blitter+15)                     *(dt_blitter+15) = dtb_count;
  jump  T,(RETURNPOINT)                               return;
  store dtb_command,(dt_blitter+14) ; delay slot        *(dt_blitter+14) = dtb_command;
  // END JAG SPECIFIC
   */
}

static void R_SegLoop(void)
{
   // CALICO_TODO
   /*
BIT_ADDFLOOR	.equ	0
BIT_ADDCEILING	.equ	1
BIT_TOPTEXTURE	.equ	2
BIT_BOTTOMTEXTURE .equ	3
BIT_NEWCEILING	.equ	4
BIT_NEWFLOOR	.equ	5
BIT_ADDSKY		.equ	6
BIT_CALCTEXTURE	.equ	7
BIT_TOPSIL		.equ	8
BIT_BOTTOMSIL	.equ	9
BIT_SOLIDSIL	.equ	10

; r15 stays free for subfunctions to use

sl_colnum		.equr	r16
sl_top			.equr	r17
sl_bottom		.equr	r18
sl_actionbits	.equr	r9

sl_high			.equr	r30
sl_low			.equr	r31

;
; common to draw texture
;
sl_floorclipx	.equr	r19
sl_ceilingclipx	.equr	r20
sl_x			.equr	r21		; also common to find plane
sl_scale		.equr	r22

sl_iscale		.equr	r23
sl_texturecol	.equr	r24
sl_texturelight	.equr	r25


  movei #96,scratch
  sub   scratch,FP
  
  store   RETURNPOINT,(FP+10) ; only store once       *(FP+10) = RETURNPOINT;
  movefa  VR_actionbits,sl_actionbits                 sl_actionbits = VR_actionbits;
  movefa  VR_scalefrac,r1                             r1 = VR_scalefrac;
  movefa  VR_start,sl_x                               sl_x = VR_start;
  btst    #0,r1 ; scoreboard bug
  store   r1,(FP+6) ; scalefrac                       *(FP+6) = r1; // scalefrac
  
; force R_FindPlane for both planes
  movei #_visplanes,r1                                r1 = &visplanes;
  store r1,(FP+8) ; &ceiling                          *(FP+8) = r1; // &ceiling;
  store r1,(FP+7) ; &floor                            *(FP+7) = r1; // &floor;
        
  movei #L121,r0                                      goto L121;
  jump  T,(r0)
  nop

L118:
  load   (FP+6),r1 ; scalefrac                        r1 = *(FP+6); // scalefrac
  move   r1,sl_scale                                  sl_scale = r1;
  sharq  #7,sl_scale                                  sl_scale >>= 7;
  movefa VR_scalestep,r2                              r2 = VR_scalestep;
  add    r2,r1                                        r1 += r2;
  movei  #$7fff,scratch                               scratch = 0x7fff;
  store  r1,(FP+6)                                    *(FP+6) = r1;
  ; if scale>0x7fff scale = 0x7fff
  cmp    scratch,sl_scale                             if(scratch > sl_scale)
  jr     U_GT,scaleok                                   goto scaleok;
  nop
  move   scratch,sl_scale                             sl_scale = scratch;

scaleok:
L125:
; get ceilingclipx and floorclipx from clipbounds
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  shlq  #2,r0                                         r0 <<= 2;
  movei #_clipbounds,r1                               r1 = &clipbounds;
  add   r1,r0                                         r0 += r1;
  load  (r0),sl_floorclipx                            sl_floorclipx = *r0;
  move  sl_floorclipx,sl_ceilingclipx                 sl_ceilingclipx = sl_floorclipx;
  shrq  #8,sl_ceilingclipx                            sl_ceilingclipx >>= 8;
  shlq  #24,sl_floorclipx                             sl_floorclipx <<= 24;
  subq  #1,sl_ceilingclipx                            sl_ceilingclipx -= 1;
  shrq  #24,sl_floorclipx ; mask off top 24 bits      sl_floorclipx >>= 24;

; texture only stuff
  btst  #BIT_CALCTEXTURE,sl_actionbits                if(!(sl_actionbits & BIT_CALCTEXTURE))
  movei #L129,scratch                                   goto L129;
  jump  EQ,(scratch)
  nop

; calculate texture offset
  movefa VR_centerangle,r1                            r1 = VR_centerangle;
  move   sl_x,r3 ;(x)                                 r3 = sl_x;
  shlq   #2,r3                                        r3 <<= 2;
  
  movei #_xtoviewangle,r4                             r4 = &xtoviewangle;
  add   r4,r3                                         r3 += r4;
  load  (r3),r3                                       r3 = *r3;
  add   r3,r1                                         r1 += r3;
  shrq  #19,r1                                        rl >>= ANGLETOFINESHIFT;
  shlq  #2,r1                                         r1 <<= 2;
  movei #_finetangent,r0                              r0 = &finetangent;
  add   r1,r0                                         r0 += r1;
  
  load   (r0),MATH_A                                  MATH_A = *r0;
  movefa VR_distance,MATH_B                           MATH_B = VR_distance;
;---------------------------------------
;========== FixedMul r0,
  move MATH_A,MATH_SIGN                               
  xor  MATH_B,MATH_SIGN
  abs  MATH_A
  abs  MATH_B
  move MATH_A,RETURNVALUE
  mult MATH_B,RETURNVALUE              ; al*bl
  shrq #16,RETURNVALUE
  move MATH_B,scratch2
  shrq #16,scratch2
  mult MATH_A,scratch2                 ; al*bh
  add  scratch2,RETURNVALUE
  move MATH_A,scratch2
  shrq #16,scratch2
  mult MATH_B,scratch2                 ; bl*ah
  add  scratch2, RETURNVALUE
  move MATH_A,scratch2
  shrq #16,scratch2
  move MATH_B,scratch
  shrq #16,scratch
  mult scratch,scratch2                ; bh*ah
  shlq #16,scratch2
  add  scratch2, RETURNVALUE
  btst #31,MATH_SIGN
  jr   EQ,notneg
  nop
  neg  RETURNVALUE
notneg:
;---------------------------------------
  movefa VR_offset,sl_texturecol                      sl_texturecol = VR_offset;
  sub    RETURNVALUE,sl_texturecol                    sl_texturecol -= RETURNVALUE;
  shrq   #16,sl_texturecol                            sl_texturecol >>= FRACBITS;

;
; other texture drawing info
;
  movei #33554432,sl_iscale                           sl_iscale = 33554432; // ???
  ; let this div complete in background
  div   sl_scale,sl_iscale                            sl_iscale /= sl_scale;

;
; calc light level
;
  movei #_lightcoef,r1                                r1 = &lightcoef;
  load  (r1),r1                                       r1 = *r1;
  mult  sl_scale,r1                                   r1 *= sl_scale;
  shrq  #16,r1                                        r1 >>= FRACBITS;
  movei #_lightsub,r2                                 r2 = &lightsub;
  load  (r2),r2                                       r2 = *r2;
  sub   r2,r1                                         r1 -= r2;
  
  movei #_lightmin,r0                                 r0 = &lightmin;
  load  (r0),r0                                       r0 = *r0;
  cmp   r0,r1                                         if(r0 < r1)
  jr    S_LT,lightovermin                               goto lightovermin;
  nop
  move  r0,r1                                         r1 = r0;
lightovermin:
  movei #_lightmax,r0                                 r0 = &lightmax;
  load  (r0),r0                                       r0 = *r0;
  cmp   r0,r1                                         if(r0 > r1)
  jr    S_GT,lightundermax                              goto lightundermax;
  nop
  move  r0,r1                                         r1 = r0;
lightundermax:
; convert to a hardware value
  movei #255,r0                                       r0 = 255;
  sub   r1,r0                                         r0 -= r1;
  shlq  #14,r0                                        r0 <<= 14;
  neg   r0                                            r0 = -r0;
  movei #$ffffff,sl_texturelight                      sl_texturelight = 0xffffff;
  and   r0,sl_texturelight                            sl_texturelight &= r0;

;
; draw textures
;
  btst  #BIT_TOPTEXTURE,sl_actionbits                 if(!(sl_actionbits & BIT_TOPTEXTURE) && !(sl_actionbits & BIT_BOTTOMTEXTURE)) // ????
  jr    EQ,L137                                          goto L137;
  btst  #BIT_BOTTOMTEXTURE,sl_actionbits ;delay slot

  movei #_toptex,dt_tex ; parameter                   dt_tex = &toptex;
  movei #_R_DrawTexture,r0                            r0 = R_DrawTexture;
  move  PC,RETURNPOINT                                RETURNPOINT = PC;
  jump  T,(r0)                                        call R_DrawTexture;
  addq  #6,RETURNPOINT
  movefa VR_actionbits,sl_actionbits                  sl_actionbits = VR_actionbits;

  btst  #BIT_BOTTOMTEXTURE,sl_actionbits              if(!(sl_actionbits & BIT_BOTTOMTEXTURE))
L137:                                                   goto L139;
  jr    EQ,L139
  nop

  movei #_bottomtex,dt_tex	; parameter               dt_tex = &bottomtex;
  movei #_R_DrawTexture,r0                            r0 = R_DrawTexture;
  move  PC,RETURNPOINT
  jump  T,(r0)                                        call R_DrawTexture;
  addq  #6,RETURNPOINT
  movefa VR_actionbits,sl_actionbits                  sl_actionbits = VR_actionbits;

L139:
L129:
;-----------------------
;
; floor
;
;-----------------------
  btst  #BIT_ADDFLOOR,sl_actionbits                   if(!(sl_actionbits & BIT_ADDFLOOR))
  movei #L143,scratch                                    goto L143;
  jump  EQ,(scratch)
  nop
  
  movefa VR_floorheight,r0                            r0 = VR_floorheight;
  imult  sl_scale,r0                                  r0 *= sl_scale;
  sharq  #15,r0                                       r0 >>= 15;
  movei  #90,sl_top                                   sl_top = CENTERY;
  sub    r0,sl_top                                    sl_top -= r0;
  cmp    sl_top,sl_ceilingclipx ;(top)(ceilingclipx)  if(sl_top > sl_ceilingclipx)
  jr     S_GT,nocliptop                                 goto nocliptop;
  nop
  move  sl_ceilingclipx,sl_top                        sl_top = sl_ceilingclipx;
  addq  #1,sl_top                                     sl_top += 1;
nocliptop:
  move  sl_floorclipx,sl_bottom                       sl_bottom = sl_floorclipx;
  subq  #1,sl_bottom                                  sl_bottom -= 1;
  
  cmp   sl_top,sl_bottom ;(top)(bottom)               if(sl_top > sl_bottom)
  movei #L147,scratch                                   goto L147;
  jump  S_GT,(scratch)
  nop
  
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  load  (FP+7),r3 ; local floor                       r3 = *(FP+7);
  shlq  #1,r0                                         r0 <<= 1;
  addq  #24,r0                                        r0 += &visplane_t::open;
  add   r3,r0                                         r0 += r3;
  loadw (r0),r0 ; floor->open[x]                      r0 = *r0; 
  movei #65280,r1                                     r1 = 65280; // ????
  cmp   r0,r1                                         if(r0 == r1)
  movei #L149,scratch                                   goto L149;
  jump  EQ,(scratch)
  nop

  movefa  VR_floorheight,fp_height                    fp_height = VR_floorheight;
  movefa  VR_floorpic,fp_picnum                       fp_picnum = VR_floorpic;
  movefa  VR_seglightlevel,fp_lightlevel              fp_lightlevel = VR_seglightlevel;
  movefa  VR_stop,fp_stop                             fp_stop = VR_stop;
  movei   #348,fp_check                               fp_check = sizeof(visplane_t);
  add     r3,fp_check ; fp_check = floor+1            fp_check += r3;
  
  movei #_R_FindPlane,r1                              r1 = R_FindPlane;
  move  PC,RETURNPOINT
  jump  T,(r1)                                        call R_FindPlane;
  addq  #6,RETURNPOINT
  movefa VR_actionbits,sl_actionbits                  sl_actionbits = VR_actionbits;
  
  store RETURNVALUE,(FP+7) ; floor                    *(FP+7) = RETURNVALUE; // visplane_t *floor

L149:
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  shlq  #1,r0                                         r0 <<= 1;
  load  (FP+7),r1 ; local floor                       r1 = *(FP+7); // floor
  addq  #24,r1                                        r1 += &visplane_t::open;
  add   r1,r0                                         r0 += r1;
  move  sl_top,r1 ;(top)                              r1 = sl_top;
  shlq  #8,r1                                         r1 <<= 8;
  add   sl_bottom,r1 ;(bottom)                        r1 += sl_bottom;
  storew r1,(r0) ; floor->open[x] = (top<<8)+bottom   *r0 = r1;

L147:
L143:
;-----------------------
;
; ceiling
;
;-----------------------
  btst  #BIT_ADDCEILING,sl_actionbits                 if(!(sl_actionbits & BIT_ADDCEILING))
  movei #L157,scratch                                   goto L157;
  jump  EQ,(scratch)
  nop
  
  move  sl_ceilingclipx,sl_top                        sl_top = sl_ceilingclipx;
  addq  #1,sl_top ; top = ceilingclipx+1              sl_top += 1;
  
  movei  #89,sl_bottom                                sl_bottom = 89;
  movefa VR_ceilingheight,r1                          r1 = VR_ceilingheight;
  imult  sl_scale,r1                                  r1 *= sl_scale;
  sharq  #15,r1                                       r1 >>= 15;
  sub    r1,sl_bottom                                 sl_bottom -= r1;
  
  cmp   sl_bottom,sl_floorclipx ;(bottom)(floorclipx) if(sl_bottom < sl_floorclipx)
  jr    S_LT,noclipbottom                                goto noclipbottom;
  nop
  move  sl_floorclipx,sl_bottom                       sl_bottom = sl_floorclipx;
  subq  #1,sl_bottom                                  sl_bottom -= 1;

noclipbottom:
  cmp   sl_top,sl_bottom ;(top)(bottom)               if(sl_bottom > sl_top)
  movei #L161,scratch                                   goto L161;
  jump  S_GT,(scratch)
  nop
  
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  shlq  #1,r0                                         r0 <<= 1;
  load  (FP+8),r3 ; local ceiling                     r3 = *(FP+8); // ceiling
  addq  #24,r0                                        r0 = &visplane_t::open;
  add   r3,r0                                         r0 += r3;
  loadw (r0),r0 ; ceiling->open[x]                    r0 = *r0;
  movei #65280,r1                                     r1 = 65280; // ????
  cmp   r0,r1                                         if(r0 == r1)
  movei #L163,scratch                                   goto L163;
  jump  EQ,(scratch)
  nop

  movefa VR_ceilingheight,fp_height                   fp_height = VR_ceilingheight;
  movefa VR_ceilingpic,fp_picnum                      fp_picnum = VR_ceilingpic;
  movefa VR_seglightlevel,fp_lightlevel               fp_lightlevel = VR_seglightlevel;
  movefa VR_stop,fp_stop                              fp_stop = VR_stop;
  movei  #348,fp_check                                fp_check = sizeof(visplane_t);
  add    r3,fp_check ; fp_check = ceiling+1           fp_check += r3;
  
  movei #_R_FindPlane,r1
  move  PC,RETURNPOINT
  jump  T,(r1)                                        call R_FindPlane;
  addq  #6,RETURNPOINT
  movefa VR_actionbits,sl_actionbits                  sl_actionbits = VR_actionbits;
  
  store RETURNVALUE,(FP+8) ; ceiling                  *(FP+8) = RETURNVALUE; // ceiling

L163:
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  shlq  #1,r0                                         r0 <<= 1;
  load  (FP+8),r1 ; local ceiling                     r1 = *(FP+8); // ceiling
  addq  #24,r1                                        r1 += &visplane_t::open;
  add   r1,r0                                         r0 += r1;
  move  sl_top,r1 ;(top)                              r1 = sl_top;
  shlq  #8,r1                                         r1 <<= 8;
  add   sl_bottom,r1 ;(bottom)                        r1 += sl_bottom;
  storew r1,(r0)                                      *r0 = r1;

L161:
L157:
;------------------------
;
; calc high and low
;
;------------------------
  movefa VR_floornewheight,r0                         r0 = VR_floornewheight;
  imult  sl_scale,r0                                  r0 *= sl_scale;
  sharq  #15,r0                                       r0 >>= 15;
  movei  #90,sl_low                                   sl_low = CENTERY;
; low = CENTERY-(scale*wl.floornewheight)>>15
  sub    r0,sl_low                                    sl_low -= r0;
  jr     PL,lownotneg                                 ????
  nop
  moveq  #0,sl_low                                    sl_low = 0;
lownotneg:
  cmp   sl_low,sl_floorclipx                          if(sl_low < sl_floorclipx)
  jr    S_LT,lowless                                    goto lowless;
  nop
  move  sl_floorclipx,sl_low                          sl_low = sl_floorclipx;
lowless:
  movefa VR_ceilingnewheight,r0                       r0 = VR_ceilingnewheight;
  imult  sl_scale,r0                                  r0 *= sl_scale;
  sharq  #15,r0                                       r0 >>= 15;
  movei  #89,sl_high                                  sl_high = CENTERY - 1;
; high = CENTERY-(scale*wl.ceilinewheight)>>15  
  sub    r0,sl_high                                   sl_high -= r0;
  movei  #179,r0                                      r0 = SCREENHEIGHT - 1;
  cmp    r0,sl_high                                   if(r0 > sl_high)
  jr     S_GT,highabove                                 goto highabove;
  nop
  move   r0,sl_high                                   sl_high = r0;
highabove:
  cmp    sl_high,sl_ceilingclipx                      if(sl_high > sl_ceilingclipx)
  jr     S_GT,highcheck2                                 goto highcheck2;
  nop
  move   sl_ceilingclipx,sl_high                      sl_high = sl_ceilingclipx;
highcheck2:
;
; bottom sprite clip sil
;
  btst #BIT_BOTTOMSIL,sl_actionbits                   if(!(sl_actionbits & BIT_BOTTOMSIL))
  jr   EQ,nobottomsil                                    goto nobottomsil;
  nop
  
  movefa VR_bottomsil,r1                              r1 = VR_bottomsil;
  add    sl_x,r1                                      r1 += sl_x;
  storeb sl_low,(r1)                                  *r1 = sl_low;

nobottomsil:
;
; top sprite clip sil
;
  btst  #BIT_TOPSIL,sl_actionbits                     if(!(sl_actionbits & BIT_TOPSIL))
  jr    EQ,notopsil                                     goto notopsil;
  nop
  
  movefa VR_topsil,r1                                 r1 = VR_topsil;
  add    sl_x,r1                                      r1 += sl_x;
  move   sl_high,r0                                   r0 = sl_high;
  addq   #1,r0                                        r0 += 1;
  storeb r0,(r1)                                      *r1 = r0;

notopsil:
;--------------------------------------------------------------
; sky mapping
  btst  #BIT_ADDSKY,sl_actionbits                     if(!(sl_actionbits & BIT_ADDSKY))
  movei #L190,scratch                                   goto L190;
  jump  EQ,(scratch)
  nop

L187:
  move   sl_ceilingclipx,sl_top                       sl_top = sl_ceilingclipx;
  addq   #1,sl_top                                    sl_top += 1;
  movei  #90,r0                                       r0 = CENTERY;
  movefa VR_ceilingheight,r1                          r1 = VR_ceilingheight;
  move   sl_scale,r2                                  r2 = sl_scale;
  imult  r1,r2                                        r2 *= r1;

L228:
  move   r2,r1                                        r1 = r2;
  sharq  #15,r1                                       r1 >>= 15;
  sub    r1,r0                                        r0 -= r1;
  subq   #1,r0                                        r0 -= 1;
  move   r0,sl_bottom ;(bottom)                       sl_bottom = r0;
  cmp    sl_bottom,sl_floorclipx;(bottom)(floorclipx) if(sl_bottom < sl_floorclipx)
  movei  #L192,scratch                                   goto L192;
  jump   S_LT,(scratch)
  nop

  move sl_floorclipx,r0 ;(floorclipx)                 r0 = sl_floorclipx;
  subq #1,r0                                          r0 -= 1;
  move r0,sl_bottom ;(bottom)                         sl_bottom = r0;

L192:
  cmp   sl_top,sl_bottom ;(top)(bottom)               if(sl_top ??? sl_bottom)
  movei #L194,scratch                                   goto L194;
  jump  PL,(scratch)
  nop
  
  movei #L190,r0                                      goto L190
  jump  T,(r0)
  nop

L194:
  movei  #_viewangle,r0                               r0 = &viewangle;
  load  (r0),r0                                       r0 = *r0;
  move  sl_x,r1 ;(x)                                  r1 = sl_x;
  shlq  #2,r1                                         r1 <<= 2;
  movei #_xtoviewangle,r2                             r2 = &xtoviewangle;
  add   r2,r1                                         r1 += r2;
  load  (r1),r1                                       r1 = *r1;
  add   r1,r0                                         r0 += r1;
  shrq  #22,r0                                        r0 >>= 22;
  move  r0,sl_colnum ;(colnum)                        sl_colnum = r0;
  movei #255,r0                                       r0 = 255;
  move  sl_colnum,r1 ;(colnum)                        r1 = sl_colnum;
  and   r0,r1                                         r1 += r0;
  move  r1,sl_colnum ;(colnum)                        sl_colnum = r1;


  shlq  #21,sl_colnum                                 sl_colnum <<= 21;
  
  movei #18204,r1                                     r1 = 18204; // ????
  imult sl_top,r1                                     r1 *= sl_top;
  add   r1,sl_colnum                                  sl_colnum += r1;
  
  shlq  #2,sl_colnum                                  sl_colnum <<= 2;
 
  movei #36,r0                                        r0 = 36; // ????
  add   FP,r0 ; &count                                r0 += FP; // &count ?
  move  sl_bottom,r1 ;(bottom)                        r1 = sl_bottom;
  sub   sl_top,r1 ;(top)                              r1 -= sl_top;
  moveq #1,r2                                         r2 = 1;
  add   r2,r1                                         r1 += r2;
  shlq  #16,r1                                        r1 <<= FRACBITS;
  add   r2,r1                                         r1 += r2;
  store r1,(r0)                                       *r0 = r1;

L196:
L197:
  movei #15737400,r0                                  r0 = 15737400; // 0xf02238 ????
  load  (r0),r0                                       r0 = *r0;
  moveq #1,r1                                         r1 = 1;
  and   r1,r0                                         r0 &= r1;
  moveq #0,r1                                         r1 = 0;
  cmp   r0,r1                                         if(r0 == r1)
  movei #L196,scratch                                   goto L196;
  jump  EQ,(scratch)
  nop

  movei #15737344,r0                                  r0 = 15737344; // 0xf02200 ????
  movei #_skytexturep,r1                              r1 = &skytexturep;
  load  (r1),r1                                       r1 = *r1;
  addq  #16,r1                                        r1 += 16; // &texture_t::data
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;

  movei #15737356,r0                                  r0 = 15737356; // 0xf0220c ????
  move  sl_colnum,r1 ;(colnum)                        r1 = sl_colnum;
  shrq  #16,r1                                        r1 >>= FRACBITS;
  store r1,(r0)                                       *r0 = r1;
 
  movei #15737368,r0                                  r0 = 15737368; // 0xf02218 ????
  movei #65535,r1                                     r1 = 65536;
  move  sl_colnum,r2 ;(colnum)                        r2 = sl_colnum;
  and   r1,r2                                         r2 &= r1;
  move  r2,r1                                         r1 = r2;
  store r1,(r0)                                       *r0 = r1;

  movei #15737360,r0                                  r0 = 15737360; // 0xf02210 ????
  moveq #1,r1                                         r1 = 1;
  store r1,(r0)                                       *r0 = r1;
  
  movei #15737364,r0                                  r0 = 15737364; // 0xf02214 ????
  movei #7281,r1                                      r1 = 7281; // ????
  store r1,(r0)                                       *r0 = r1;
  
  movei #15737392,r0                                  r0 = 15737392; // 0xf02230 ????
  move  sl_top,r1 ;(top)                              r1 = sl_top;
  shlq  #16,r1                                        r1 <<= FRACBITS;
  add   sl_x,r1 ;(x)                                  r1 += sl_x;
  store r1,(r0)                                       *r0 = r1;

  movei #15737456,r0                                  r0 = 15737456; // 0xf02270 ????
  moveq #0,r1                                         r1 = 0;
  store r1,(r0)                                       *r0 = r1;
  
  movei #15737404,r0                                  r0 = 15737404; // 0xf0223c ????
  load  (FP+9),r1 ; local count                       r1 = *(FP+9);  // count
  store r1,(r0)                                       *r0 = r1;
  
  movei #15737400,r0                                  r0 = 15737400;   // 0xf02238 ????
  movei #1098919681,r1                                r1 = 1098919681; // 0x41802F01 ????
  store r1,(r0)                                       *r0 = r1;

L190:
;--------------------------------------------------------------
;if (!(actionbits & (AC_NEWFLOOR|AC_NEWCEILING)))
; continue; // don't bother rewriting clipbounds[x]

  movei #48,r0                                       r0 = (AC_NEWFLOOR|AC_NEWCEILING);
  and   sl_actionbits,r0                             r0 &= sl_actionbits;
  movei #L119,scratch                                if(!^^^^)
  jump  EQ,(scratch)                                   goto L119;
  nop
  
  btst  #BIT_NEWFLOOR,sl_actionbits                  if(!(sl_actionbits & BIT_NEWFLOOR))
  jr    EQ,nonewfloor                                   goto nonewfloor;
  nop
  move  sl_low,sl_floorclipx                         sl_floorclipx = sl_low;
  
nonewfloor:
  btst  #BIT_NEWCEILING,sl_actionbits                if(!(sl_actionbits & BIT_NEWCEILING))
  jr    EQ,nonewceiling                                goto nonewceiling;
  nop
  move  sl_high,sl_ceilingclipx                      sl_ceilingclipx = sl_high;
  
nonewceiling:
;clipbounds[x] = ((ceilingclipx+1)<<8) + floorclipx;
  move  sl_x,r0 ;(x)                                  r0 = sl_x;
  shlq  #2,r0                                         r0 <<= 2;
  movei #_clipbounds,r1                               r1 = &clipbounds;
  add   r1,r0                                         r0 += r1;
  addq  #1,sl_ceilingclipx                            sl_ceilingclipx += 1;
  shlq  #8,sl_ceilingclipx                            sl_ceilingclipx <<= 8;
  add   sl_floorclipx,sl_ceilingclipx                 sl_ceilingclipx += sl_floorclipx;
  store sl_ceilingclipx,(r0)                          *r0 = sl_ceilingclipx;
  
L119:
;--------------
;
; next
;
;--------------
  addq  #1,sl_x                                       sl_x += 1;

L121:
  movefa VR_stop,r0                                   r0 = VR_stop;
  cmp    sl_x,r0 ;(x)                                 if(sl_x <= r0)
  movei  #L118,scratch                                   goto L118;
  jump   S_LE,(scratch)
  nop
  
  load  (FP+10),RETURNPOINT                           return;
  movei #96,scratch
  jump  T,(RETURNPOINT)
  add   scratch,FP ; delay slot
   */
}

void R_SegCommands(void)
{
   // CALICO_TODO: R_SegCommands
   /*
  subq  #32,FP

  movei #_clipbounds,r0                               r0 = &clipbounds;
  movei #180,r1		; SCREENHEIGHT                      r1 = SCREENHEIGHT;
  movei #160,r2		; SCREENWIDTH                       r2 = SCREENWIDTH;
  movei #clearclipbounds,r3                           r3 = &clearclipbounds;
  
clearclipbounds:
  store r1,(r0)                                       *r0 = r1;
  addq  #4,r0                                         r0 += 4;
  store r1,(r0)                                       *r0 = r1;
  addq  #4,r0                                         r0 += 4;
  store r1,(r0)                                       *r0 = r1;
  addq  #4,r0                                         r0 += 4;
  subq  #4,r2                                         r2 -= 4;
  store r1,(r0)                                       *r0 = r1;
  jump  NE,(r3)
  addq  #4,r0

;
; setup blitter
;
  movei #15737348,r0                                  r0 = 15737348; // 0xf02204
  movei #145440,r1                                    r1 = 145440; // ????
  store r1,(r0)                                       *r0 = r1;
  
  movei #15737384,r0                                  r0 = 15737384; // 0xf02228
  movei #145952,r1                                    r1 = 145952; // ????
  store r1,(r0)                                       *r0 = r1;
  
  movei #_viswalls,r0                                 r0 = &viswalls;
  move  r0,r16 ;(segl)                                r16 = r0; // segl
  
  movei #L61,r0                                       goto L61;
  jump  T,(r0)
  nop

L58: // loop start
;
; copy viswall to local memory
;
  move  r16,r15                                       r15 = r16;
  
  load   (r15+VS_start),r0                            r0 = *(r15 + VS_start);
  moveta r0,VR_start                                  VR_start = r0;
  load   (r15+VS_stop),r0                             r0 = *(r15+VS_stop);
  moveta r0,VR_stop                                   VR_stop = r0;
  load   (r15+VS_floorpic),r0                         r0 = *(r15+VS_floorpic);
  moveta r0,VR_floorpic                               VR_floorpic = r0;
  load   (r15+VS_ceilingpic),r0                       r0 = *(r15+VS_ceilingpic);
  moveta r0,VR_ceilingpic                             VR_ceilingpic = r0;
  load   (r15+VS_actionbits),r0                       r0 = *(r15+VS_actionbits);
  moveta r0,VR_actionbits                             VR_actionbits = r0;
  load   (r15+VS_floorheight),r0                      r0 = *(r15+VS_floorheight);
  moveta r0,VR_floorheight                            VR_floorheight = r0;
  load   (r15+VS_floornewheight),r0                   r0 = *(r15+VS_floornewheight);
  moveta r0,VR_floornewheight                         VR_floornewheight = r0;
  load   (r15+VS_ceilingheight),r0                    r0 = *(r15+VS_ceilingheight);
  moveta r0,VR_ceilingheight                          VR_ceilingheight = r0;
  load   (r15+VS_ceilingnewheight),r0                 r0 = *(r15+VS_ceilingnewheight);
  moveta r0,VR_ceilingnewheight                       VR_ceilingnewheight = r0;
  load   (r15+VS_topsil),r0                           r0 = *(r15+VS_topsil);
  moveta r0,VR_topsil                                 VR_topsil = r0;
  load   (r15+VS_bottomsil),r0                        r0 = *(r15+VS_bottomsil);
  moveta r0,VR_bottomsil                              VR_bottomsil = r0;
  load   (r15+VS_scalefrac),r0                        r0 = *(r15+VS_scalefrac);
  moveta r0,VR_scalefrac                              VR_scalefrac = r0;
  load   (r15+VS_scalestep),r0                        r0 = *(r15+VS_scalestep);
  moveta r0,VR_scalestep                              VR_scalestep = r0;
  load   (r15+VS_centerangle),r0                      r0 = *(r15 + VS_centerangle);
  moveta r0,VR_centerangle                            VR_centerangle = r0;
  load   (r15+VS_offset),r0                           r0 = *(r15 + VS_offset);
  moveta r0,VR_offset                                 VR_offset = r0;
  load   (r15+VS_distance),r0                         r0 = *(r15+VS_distance);
  moveta r0,VR_distance                               VR_distance = r0;
  load   (r15+VS_seglightlevel),r0                    r0 = *(r15+VS_seglightlevel);
  moveta r0,VR_seglightlevel                          VR_seglightlevel = r0;

; lightmin = wl.seglightlevel - (255-wl.seglightlevel)*2;
; if (lightmin < 0)
;   lightmin = 0;
sc_lightmin .equr	r5
sc_lightmax .equr	r6

  movefa VR_seglightlevel,sc_lightmin                 sc_lightmin = VR_seglightlevel;
  movei  #255,r2                                      r2 = 255;
  movefa VR_seglightlevel,r3                          r3 = VR_seglightlevel;
  sub    r3,r2                                        r2 -= r3;
  shlq   #1,r2                                        r2 <<= 1;
  sub    r2,sc_lightmin                               sc_lightmin -= r2;
  jr     PL,minnotneg                                 ????
  nop
  moveq  #0,sc_lightmin                               sc_lightmin = 0;
minnotneg:
  movei  #_lightmin,r0                                r0 = &lightmin;
  store  sc_lightmin,(r0)                             *r0 = sc_lightmin;
  
; lightmax = wl.seglightlevel;
  movei  #_lightmax,r0                                r0 = &lightmax;
  movefa VR_seglightlevel,sc_lightmax                 sc_lightmax = VR_seglightlevel;
  store  sc_lightmax,(r0)                             *r0 = sc_lightmax;

; lightsub = 160*(lightmax-lightmin)/(800-160);
; lightcoef = ((lightmax-lightmin)<<16)/(800-160);
  sub   sc_lightmin,sc_lightmax                       sc_lightmax -= sc_lightmin;
  move  sc_lightmax,r1                                r1 = sc_lightmax;
  movei #160,r2                                       r2 = 160;
  mult  r1,r2                                         r2 *= r1;
  movei #640,r4                                       r4 = 640;
  div   r4,r2                                         r2 /= r4;
  movei #_lightsub,r0                                 r0 = &lightsub;

  move  sc_lightmax,r3                                r3 = sc_lightmax;
  shlq  #16,r3                                        r3 <<= 16;
  store r2,(r0) ; div hit                             *r0 = r2;
  div   r4,r3                                         r3 /= r4;
  
  movei #_lightcoef,r0                                r0 = &lightcoef;
  store r3,(r0) ; div hit                             *r0 = r3;
  
  movefa VR_actionbits,r0                             r0 = VR_actionbits;
  btst   #2,r0                                        if(!(r0 & 2))
  movei  #L71,scratch                                   goto L71;
  jump   EQ,(scratch)
  nop

  movei #_toptex+12,r0                                r0 = &toptex + 12;
  load  (r15+7),r1                                    r1 = *(r15+7);
  store r1,(r0)                                       *r0 = r1;
  
  movei #_toptex+16,r0                                r0 = &toptex + 16;
  load  (r15+8),r1                                    r1 = *(r15+8);
  store r1,(r0)                                       *r0 = r1;
  
  movei #_toptex+20,r0                                r0 = &toptex + 20;
  load  (r15+9),r1                                    r1 = *(r15+9);
  store r1,(r0)                                       *r0 = r1;
  
  load  (r15+10),r0                                   r0 = *(r15+10);
  move  r0,r17 ;(tex)                                 r17 = r0; // tex
  movei #_toptex+4,r0                                 r0 = &toptex + 4;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #8,r1                                         r1 += 8;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;

  movei #_toptex+8,r0                                 r0 = &toptex + 8;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #12,r1                                        r1 += 12;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;
 
  movei #_toptex,r0                                   r0 = &toptex;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #16,r1                                        r1 += 16;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;

L71:
  movefa VR_actionbits,r0                             r0 = VR_actionbits;
  btst   #3,r0                                        if(!(r0 & 3))
  movei  #L83,scratch                                    goto L83;
  jump   EQ,(scratch)
  nop

  movei #_bottomtex+12,r0                             r0 = &bottomtex + 12;
  load  (r15+11),r1                                   r1 = *(r15+11)
  store r1,(r0)                                       *r0 = r1;
  
  movei #_bottomtex+16,r0                             r0 = &bottomtex + 16;
  load  (r15+12),r1                                   r1 = *(r15+12);
  store r1,(r0)                                       *r0 = r1;
  
  movei #_bottomtex+20,r0                             r0 = &bottomtex + 20;
  load  (r15+13),r1                                   r1 = *(r15+13);
  store r1,(r0)                                       *r0 = r1;
  
  load  (r15+14),r0                                   r0 = *(r15+14);
  move  r0,r17 ;(tex)                                 r17 = r0; // tex
  movei #_bottomtex+4,r0                              r0 = &bottomtex + 4;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #8,r1                                         r1 += 8;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;

  movei #_bottomtex+8,r0                              r0 = &bottomtex + 8;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #12,r1                                        r1 += 12;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;
 
  movei #_bottomtex,r0                                r0 = &bottomtex;
  move  r17,r1 ;(tex)                                 r1 = r17; // tex
  addq  #16,r1                                        r1 += 16;
  load  (r1),r1                                       r1 = *r1;
  store r1,(r0)                                       *r0 = r1;

L83:
  movei #_R_SegLoop,r0
  store r28,(FP) ; psuh ;(RETURNPOINT)
  store r17,(FP+1) ; push ;(tex)
  store r16,(FP+2) ; push ;(segl)
  movei #L99,RETURNPOINT
  jump  T,(r0)                                        call R_SegLoop;
  store r15,(FP+3) ; delay slot push ;(i)
L99:
  load (FP+1),r17 ; pop ;(tex)
  load (FP+2),r16 ; pop ;(segl)
  load (FP+3),r15 ; pop ;(i)
  load (FP),RETURNPOINT ; pop

L59:
  movei #112,r0                                       r0 = sizeof(viswall_t);
  move  r16,r1 ;(segl)                                r1 = r16; // segl
  add   r0,r1                                         r1 += r0;
  move  r1,r16 ;(segl)                                r16 = r1; // segl

L61: // loop end
  move  r16,r0 ;(segl)                                r0 = r16; // segl
  movei #_lastwallcmd,r1                              r1 = &lastwallcmd;
  load  (r1),r1                                       r1 = *r1;
  cmp   r0,r1                                         if(r0 < r1)
  movei #L58,scratch                                    goto L58;
  jump  U_LT,(scratch)
  nop

  movei #_phasetime+24,r0
  movei #_samplecount,r1
  load  (r1),r1
  store r1,(r0)
  
  movei #_gpucodestart,r0
  movei #_ref7_start,r1
  store r1,(r0)

L53:
  jump T,(RETURNPOINT)
  addq #32,FP ; delay slot
   */
}

// EOF

/*
#define	AC_ADDFLOOR      1       000:00000001
#define	AC_ADDCEILING    2       000:00000010
#define	AC_TOPTEXTURE    4       000:00000100
#define	AC_BOTTOMTEXTURE 8       000:00001000
#define	AC_NEWCEILING    16      000:00010000
#define	AC_NEWFLOOR      32      000:00100000
#define	AC_ADDSKY        64      000:01000000
#define	AC_CALCTEXTURE   128     000:10000000
#define	AC_TOPSIL        256     001:00000000
#define	AC_BOTTOMSIL     512     010:00000000
#define	AC_SOLIDSIL      1024    100:00000000

typedef struct
{
0:  int  filepos; // also texture_t * for comp lumps
4:  int  size;
8:  char name[8];
} lumpinfo_t;

typedef struct
{
  0: fixed_t        height;
  4: pixel_t       *picnum;
  8: int            lightlevel;
 12: int            minx;
 16: int            maxx;
 20: int            pad1;              // leave pads for [minx-1]/[maxx+1]
 24: unsigned short open[SCREENWIDTH]; // top<<8 | bottom 
344: int            pad2;
} visplane_t; // sizeof() = 348

typedef struct
{
   // filled in by bsp
  0:   seg_t        *seg;
  4:   int           start;
  8:   int           stop;   // inclusive x coordinates
 12:   int           angle1; // polar angle to start

   // filled in by late prep
 16:   pixel_t      *floorpic;
 20:   pixel_t      *ceilingpic;

   // filled in by early prep
 24:   unsigned int  actionbits;
 28:   int           t_topheight;
 32:   int           t_bottomheight;
 36:   int           t_texturemid;
 40:   texture_t    *t_texture;
 44:   int           b_topheight;
 48:   int           b_bottomheight;
 52:   int           b_texturemid;
 56:   texture_t    *b_texture;
 60:   int           floorheight;
 64:   int           floornewheight;
 68:   int           ceilingheight;
 72:   int           ceilingnewheight;
 76:   byte         *topsil;
 80:   byte         *bottomsil;
 84:   unsigned int  scalefrac;
 88:   unsigned int  scale2;
 92:   int           scalestep;
 96:   unsigned int  centerangle;
100:   unsigned int  offset;
104:   unsigned int  distance;
108:   unsigned int  seglightlevel;
} viswall_t;

typedef struct seg_s
{
 0:   vertex_t *v1;
 4:   vertex_t *v2;
 8:   fixed_t   offset;
12:   angle_t   angle;       // this is not used (keep for padding)
16:   side_t   *sidedef;
20:   line_t   *linedef;
24:   sector_t *frontsector;
28:   sector_t *backsector;  // NULL for one sided lines
} seg_t;

typedef struct line_s
{
 0: vertex_t     *v1;
 4: vertex_t     *v2;
 8: fixed_t      dx;
12: fixed_t      dy;                    // v2 - v1 for side checking
16: VINT         flags;
20: VINT         special;
24: VINT         tag;
28: VINT         sidenum[2];               // sidenum[1] will be -1 if one sided
36: fixed_t      bbox[4];
52: slopetype_t  slopetype;                // to aid move clipping
56: sector_t    *frontsector;
60: sector_t    *backsector;
64: int          validcount;               // if == validcount, already checked
68: void        *specialdata;              // thinker_t for reversable actions
72: int          fineangle;                // to get sine / cosine for sliding
} line_t;

typedef struct
{
 0:  fixed_t   textureoffset; // add this to the calculated texture col
 4:  fixed_t   rowoffset;     // add this to the calculated texture top
 8:  VINT      toptexture;
12:  VINT      bottomtexture;
16:  VINT      midtexture;
20:  sector_t *sector;
} side_t;

typedef	struct
{
 0:   fixed_t floorheight;
 4:   fixed_t ceilingheight;
 8:   VINT    floorpic;
12:   VINT    ceilingpic;        // if ceilingpic == -1,draw sky
16:   VINT    lightlevel;
20:   VINT    special;
24:   VINT    tag;
28:   VINT    soundtraversed;              // 0 = untraversed, 1,2 = sndlines -1
32:   mobj_t *soundtarget;                 // thing that made a sound (or null)
36:   VINT        blockbox[4];             // mapblock bounding box for height changes
52:   degenmobj_t soundorg;                // for any sounds played by the sector
76:   int     validcount;                  // if == validcount, already checked
80:   mobj_t *thinglist;                   // list of mobjs in sector
84:   void   *specialdata;                 // thinker_t for reversable actions
88:   VINT    linecount;
92:   struct line_s **lines;               // [linecount] size
} sector_t;

typedef struct subsector_s
{
0:   sector_t *sector;
4:   VINT      numlines;
8:   VINT      firstline;
} subsector_t;

typedef struct
{
 0:  char     name[8];  // for switch changing, etc
 8:  int      width;
12:  int      height;
16:  pixel_t *data;     // cached data to draw from
20:  int      lumpnum;
24:  int      usecount; // for precaching
28:  int      pad;
} texture_t;

typedef struct mobj_s
{
  0: struct mobj_s *prev;
  4: struct mobj_s *next;
  8: latecall_t     latecall;
 12: fixed_t        x;
 16: fixed_t        y;
 20: fixed_t        z;
 24: struct mobj_s *snext;
 28: struct mobj_s *sprev;
 32: angle_t        angle;
 36: VINT           sprite;
 40: VINT           frame;
 44: struct mobj_s *bnext;
 48: struct mobj_s *bprev;
 52: struct subsector_s *subsector;
 56: fixed_t        floorz;
 60: fixed_t        ceilingz;
 64: fixed_t        radius;
 68: fixed_t        height;
 72: fixed_t        momx;
 76: fixed_t        momy;
 80: fixed_t        momz;
 84: mobjtype_t     type;
 88: mobjinfo_t    *info;
 92: VINT           tics;
 96: state_t       *state;
100: int            flags;
104: VINT           health;
108: VINT           movedir;
112: VINT           movecount;
116: struct mobj_s *target;
120: VINT           reactiontime;
124: VINT           threshold;
132: struct player_s *player;
136: struct line_s *extradata;
140: short spawnx;
142: short spawny;
144: short spawntype;
146: short spawnangle;
} mobj_t;

typedef struct vissprite_s
{
 0:  int     x1;
 4:  int     x2;
 8:  fixed_t startfrac;
12:  fixed_t xscale;
16:  fixed_t xiscale;
20:  fixed_t yscale;
24:  fixed_t yiscale;
28:  fixed_t texturemid;
32:  patch_t *patch;
36:  int     colormap;
40:  fixed_t gx;
44:  fixed_t gy;
48:  fixed_t gz;
52:  fixed_t gzt;
56:  pixel_t *pixels;
} vissprite_t;

typedef struct 
{ 
0:  short width;
2:  short height;
4:  short leftoffset;
6:  short topoffset;
8:  unsigned short columnofs[8];
} patch_t; 

typedef struct
{
 0:  state_t *state;
 4:  int      tics;
 8:  fixed_t  sx;
12:  fixed_t  sy;
} pspdef_t;

typedef struct
{
 0:  spritenum_t sprite;
 4:  long        frame;
 8:  long        tics;
12:  void        (*action)();
16:  statenum_t  nextstate;
20:  long        misc1;
24:  long        misc2;
} state_t;

typedef struct
{
 0:  boolean rotate;  // if false use 0 for any position
 4:  int     lump[8]; // lump to use for view angles 0-7
36:  byte    flip[8]; // flip (1 = flip) to use for view angles 0-7
} spriteframe_t;

typedef struct
{
0: int            numframes;
4: spriteframe_t *spriteframes;
} spritedef_t;

typedef struct memblock_s
{
 0: int     size; // including the header and possibly tiny fragments
 4: void  **user; // NULL if a free block
 8: short   tag;  // purgelevel
10: short   id;   // should be ZONEID
12: int     lockframe; // don't purge on the same frame
16: struct memblock_s *next;
20: struct memblock_s *prev;
} memblock_t; // sizeof() == 24

typedef struct
{
 0:  int         size;      // total bytes malloced, including header
 4:  memblock_t *rover;
 8:  memblock_t  blocklist; // start / end cap for linked list
      0: int     size; // including the header and possibly tiny fragments
12:   4: void  **user; // NULL if a free block
16:   8: short   tag;  // purgelevel
18:  10: short   id;   // should be ZONEID
20:  12: int     lockframe; // don't purge on the same frame
24:  16: struct memblock_s *next;
28:  20: struct memblock_s *prev;
} memzone_t; // sizeof() == 32

*/

