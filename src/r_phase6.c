/*
  CALICO

  Renderer phase 6 - Seg Loop 
*/

#include "r_local.h"

#define OPENMARK 0xff00

static int clipbounds[SCREENWIDTH];
static int lightmin, lightmax, lightsub, lightcoef;

//
// Check for a matching visplane in the visplanes array, or set up a new one
// if no compatible match can be found.
//
static visplane_t *R_FindPlane(visplane_t *check, fixed_t height, pixel_t *picnum, 
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

   for(i = 0; i < SCREENWIDTH/4; i++)
   {
      check->open[i*4  ] = OPENMARK;
      check->open[i*4+1] = OPENMARK;
      check->open[i*4+2] = OPENMARK;
      check->open[i*4+3] = OPENMARK;
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
  movei #1+(1<<8)+(1<<9)+(1<<10)+(1<<11)+(1<<13)+(1<<30)+(12<<21),dtb_command // 1098919681 ?????
  
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

//
// Main seg clipping loop
//
static void R_SegLoop(viswall_t *segl)
{
   int x, scale, scalefrac, floorclipx, ceilingclipx, texturecol, iscale, 
       low, high, top, bottom;
   visplane_t *ceiling, *floor;

   x = segl->start;
   scalefrac = segl->scalefrac;

   // force R_FindPlane for both planes
   floor = ceiling = visplanes;

   do
   {
      scale = scalefrac / (1 << FIXEDTOSCALE);
      scalefrac += segl->scalestep;

      if(scale >= 0x7fff)
         scale = 0x7fff; // fix the scale to maximum

      //
      // get ceilingclipx and floorclipx from clipbounds
      //
      floorclipx   = clipbounds[x] & 0x00ff;
      ceilingclipx = ((clipbounds[x] & 0xff00) >> 8) - 1;

      //
      // texture only stuff
      //
      if(segl->actionbits & AC_CALCTEXTURE)
      {
         // calculate texture offset
         int texturelight;
         fixed_t r = FixedMul(segl->distance, 
                              finetangent[(segl->centerangle + xtoviewangle[x]) >> ANGLETOFINESHIFT]);

         // other texture drawing info
         texturecol = (segl->offset - r) / FRACUNIT;
         iscale     = 33554432 / scale;

         // calc light level
         texturelight = ((scale * lightcoef) / FRACUNIT) - lightsub;
         if(texturelight < lightmin)
            texturelight = lightmin;
         if(texturelight > lightmax)
            texturelight = lightmax;

         // convert to a hardware value
         texturelight = -((255 - texturelight) << 14) & 0xffffff;

         //
         // draw textures
         //
         if(segl->actionbits & AC_TOPTEXTURE)
            R_DrawTexture(/*toptex*/); // CALICO_TODO
         if(segl->actionbits & AC_BOTTOMTEXTURE)
            R_DrawTexture(/*bottomtex*/); // CALICO_TODO
      }

      //
      // floor
      //
      if(segl->actionbits & AC_ADDFLOOR)
      {
         int top, bottom;
         
         top = CENTERY - ((scale * segl->floorheight) >> (HEIGHTBITS + SCALEBITS));
         if(top <= ceilingclipx)
            top = ceilingclipx + 1;
         
         bottom = floorclipx - 1;
         
         if(top <= bottom)
         {
            if(floor->open[x] != OPENMARK)
            {
               floor = R_FindPlane(floor + 1, segl->floorheight, segl->floorpic, 
                                   segl->seglightlevel, x, segl->stop);
            }
            floor->open[x] = (unsigned short)((top << 8) + bottom);
         }
      }

      //
      // ceiling
      //
      if(segl->actionbits & AC_ADDCEILING)
      {
         int top, bottom;

         top = ceilingclipx + 1;

         bottom = CENTERY - 1 - ((scale * segl->ceilingheight) >> (HEIGHTBITS + SCALEBITS));
         if(bottom >= floorclipx)
            bottom = floorclipx - 1;
         
         if(top <= bottom)
         {
            if(ceiling->open[x] != OPENMARK)
            {
               ceiling = R_FindPlane(ceiling + 1, segl->ceilingheight, segl->ceilingpic, 
                                     segl->seglightlevel, x, segl->stop);
            }
            ceiling->open[x] = (unsigned short)((top << 8) + bottom);
         }
      }

      //
      // calc high and low
      //
      low = CENTERY - ((scale * segl->floornewheight) >> (HEIGHTBITS + SCALEBITS));
      if(low < 0)
         low = 0;
      if(low > floorclipx)
         low = floorclipx;

      high = CENTERY - 1 - ((scale * segl->ceilingnewheight) >> (HEIGHTBITS + SCALEBITS));
      if(high > SCREENHEIGHT - 1)
         high = SCREENHEIGHT - 1;
      if(high < ceilingclipx)
         high = ceilingclipx;

      // bottom sprite clip sil
      if(segl->actionbits & AC_BOTTOMSIL)
         segl->bottomsil[x] = low;

      // top sprite clip sil
      if(segl->actionbits & AC_TOPSIL)
         segl->topsil[x] = high + 1;

      // sky mapping
      if(segl->actionbits & AC_ADDSKY)
      {
         top = ceilingclipx + 1;
         bottom = (CENTERY - ((scale * segl->ceilingheight) >> (HEIGHTBITS + SCALEBITS))) - 1;
         if(bottom >= floorclipx)
            bottom = floorclipx - 1;
         if(top <= bottom)
         {
            int colnum = ((xtoviewangle[x] + viewangle) >> ANGLETOSKYSHIFT) & 0xff;
            // CALICO_TODO: draw sky column
            /*
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
            
              // JAG SPECIFIC
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
            */
         }
      }

      if(segl->actionbits & (AC_NEWFLOOR|AC_NEWCEILING))
      {
         // rewrite clipbounds
         if(segl->actionbits & AC_NEWFLOOR)
            floorclipx = low;
         if(segl->actionbits & AC_NEWCEILING)
            ceilingclipx = high;

         clipbounds[x] = ((ceilingclipx + 1) << 8) + floorclipx;
      }
   }
   while(++x <= segl->stop);
}

void R_SegCommands(void)
{
   int i;
   int *clip;
   viswall_t *segl;

   // initialize the clipbounds array
   clip = clipbounds;
   for(i = 0; i < SCREENWIDTH / 4; i++)
   {
      *clip++ = SCREENHEIGHT;
      *clip++ = SCREENHEIGHT;
      *clip++ = SCREENHEIGHT;
      *clip++ = SCREENHEIGHT;
   }

   // CALICO_TODO: JAG SPECIFIC
   /*
   ; setup blitter
   movei #15737348,r0   r0 = 15737348; // 0xf02204
   movei #145440,r1     r1 = 145440;   // ????
   store r1,(r0)        *r0 = r1;
                       
   movei #15737384,r0   r0 = 15737384; // 0xf02228
   movei #145952,r1     r1 = 145952;   // ????
   store r1,(r0)        *r0 = r1;
   */
  
   segl = viswalls;
   while(segl < lastwallcmd)
   {
      lightmin = segl->seglightlevel - (255 - segl->seglightlevel) * 2;
      if(lightmin < 0)
         lightmin = 0;

      lightmax = segl->seglightlevel;
      
      lightsub  = 160 * (lightmax - lightmin) / (800 - 160);
      lightcoef = ((lightmax - lightmin) << FRACBITS) / (800 - 160);

      if(segl->actionbits & AC_TOPTEXTURE)
      {
         // CALICO_TODO
      } // L71

      if(segl->actionbits & AC_BOTTOMTEXTURE)
      {
         // CALICO_TODO
      } // L83

      /*
      movei #_R_SegLoop,r0
      store r28,(FP) ; psuh ;(RETURNPOINT)
      store r17,(FP+1) ; push ;(tex)
      store r16,(FP+2) ; push ;(segl)
      movei #L99,RETURNPOINT
      jump  T,(r0)                                        call R_SegLoop;
      store r15,(FP+3) ; delay slot push ;(i)
      */
      R_SegLoop(segl); // CALICO_TODO: other params?

      ++segl;
   }

   /*
; lightmin = wl.seglightlevel - (255-wl.seglightlevel)*2;
; if (lightmin < 0)
;   lightmin = 0;
; lightmax = wl.seglightlevel;
; lightsub = 160*(lightmax-lightmin)/(800-160);
; lightcoef = ((lightmax-lightmin)<<16)/(800-160);
  
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
  btst   #3,r0                                        if(!(r0 & AC_BOTTOMTEXTURE))
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
  */
}

// EOF

/*
#define	AC_ADDFLOOR      1       000:00000001 0
#define	AC_ADDCEILING    2       000:00000010 1
#define	AC_TOPTEXTURE    4       000:00000100 2
#define	AC_BOTTOMTEXTURE 8       000:00001000 3
#define	AC_NEWCEILING    16      000:00010000 4
#define	AC_NEWFLOOR      32      000:00100000 5
#define	AC_ADDSKY        64      000:01000000 6
#define	AC_CALCTEXTURE   128     000:10000000 7
#define	AC_TOPSIL        256     001:00000000 8
#define	AC_BOTTOMSIL     512     010:00000000 9
#define	AC_SOLIDSIL      1024    100:00000000 10

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

