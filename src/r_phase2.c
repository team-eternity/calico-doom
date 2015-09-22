/*
  CALICO

  Renderer phase 2 - Wall prep
*/

#include "doomdef.h"
#include "r_local.h"

static sector_t emptysector = { 0, 0, -2, -2, -2 };

void R_WallPrep(void)
{
   viswall_t *segl = viswalls;
   seg_t     *seg;
   line_t    *line;
   side_t    *side;
   sector_t  *frontsector, *backsector;
   fixed_t    f_floorheight, f_ceilingheight;
   fixed_t    b_floorheight, b_ceilingheight;
   int        f_lightlevel, b_lightlevel;
   int        f_ceilingpic, b_ceilingpic;
   int        b_texturemid, t_texturemid;
   unsigned int actionbits;

   while(segl < lastwallcmd)
   {
      seg  = segl->seg;
      line = seg->linedef;
      side = seg->sidedef;

      line->flags |= ML_MAPPED; // mark as seen

      frontsector     = seg->frontsector;
      f_ceilingpic    = frontsector->ceilingpic;
      f_lightlevel    = frontsector->lightlevel;
      f_floorheight   = frontsector->floorheight   - viewz;
      f_ceilingheight = frontsector->ceilingheight - viewz;

      segl->floorpicnum   = flattranslation[frontsector->floorpic];
      segl->ceilingpicnum = (f_ceilingpic == -1) ? -1 : flattranslation[f_ceilingpic];

      backsector = seg->backsector;
      if(!backsector)
         backsector = &emptysector;
      b_ceilingpic    = backsector->ceilingpic;
      b_lightlevel    = backsector->lightlevel;
      b_floorheight   = backsector->floorheight   - viewz;
      b_ceilingheight = backsector->ceilingheight - viewz;

      t_texturemid = b_texturemid = 0;
      actionbits = 0;
      
      ++segl; // next viswall
   }
}

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
*/
/*
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
*/
/*
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
56:   int     validcount;                  // if == validcount, already checked
60:   mobj_t *thinglist;                   // list of mobjs in sector
64:   void   *specialdata;                 // thinker_t for reversable actions
68:   VINT    linecount;
72:   struct line_s **lines;               // [linecount] size
} sector_t;
*/

/*
;====================
_R_WallPrep::
;4 dag registers  8 register variables
;localoffset:0  regoffset:56  argoffset:56
;====================

L52:
 load (r15),r0 ;(segl)    // segl->seg => r0
 move r0,r22 ;(seg)       // r0 => r22
 move FP,r0               // LOCAL: line_t *li
 addq #20,r0 ; &li        // &seg_t::linedef (offset)
 move r22,r1 ;(seg)       // r22 => r1
 addq #20,r1              // r1 += &seg_t::linedef
 load (r1),r1             // seg->linedef => r1
 store r1,(r0)            // r1 => (r0)                *** li = seg->linedef (phase2.c line 148)
 move FP,r1               // 
 addq #8,r1 ; &si         // LOCAL: sidedef_t *si
 moveq #16,r2             // 16 => r2
 move r22,r3 ;(seg)       // seg => r3
 add r2,r3                // seg+16 => r3 (16 == &seg_t::sidedef)
 load (r3),r3             // seg->sidedef => r3
 store r3,(r1)            // r3 => (r1)                *** si = seg->sidedef (phase2.c line 153)
 load (r0),r0             // seg->linedef => r0
 add r2,r0                // += 16 (&line_t::flags)
 load (r0),r1             // &line->flags => r1 (phase2.c line 149)
 movei #256,r2            // ML_MAPPED => r2 
 or r2,r1                 // line->flags | ML_MAPPED => r1
 store r1,(r0)            // r1 => *(&line->flags) (phase2.c line 150)

 move r22,r0 ;(seg)
 addq #24,r0                  // &seg_t::frontsector
 load (r0),r0
 move r0,r21 ;(front_sector)  // r0 => front_sector (phase2.c line 154)
 move FP,r0 ; &f_ceilingpic
 move r21,r1 ;(front_sector)
 addq #12,r1                  // &sector_t::ceilingpic
 load (r1),r1                 // frontsector->ceilingpic => r1
 store r1,(r0)                // r1 => f_ceilingpic (phase2.c line 155)
 move FP,r0
 addq #16,r0 ; &f_lightlevel
 moveq #16,r1                 // &sector_t::lightlevel
 move r21,r2 ;(front_sector)
 add r1,r2                    
 load (r2),r2                   // frontsector->lightlevel => r2
 store r2,(r0)                  // r2 => f_lightlevel (phase2.c line 156)
 load (r21),r0 ;(front_sector)
 movei #_viewz,r2              
 load (r2),r2                   // viewz => r2
 sub r2,r0                      // frontsector->floorheight - viewz
 move r0,r18 ;(f_floorheight)   //  => f_floorheight (phase2.c line 157)
 move r21,r0 ;(front_sector)
 addq #4,r0                     // &sector_t::ceilingheight
 load (r0),r0
 sub r2,r0                      // frontsector->ceilingheight - viewz
 move r0,r16 ;(f_ceilingheight) //  => f_ceilingheight (phase2.c line 158)
 move r15,r0 ;(segl)            // segl => r0
 add r1,r0                      // += r1 (16) == &viswall_t::floorpic => r0
 move r21,r1 ;(front_sector)
 addq #8,r1                     // &sector_t::floorpic
 load (r1),r1
 shlq #2,r1
 movei #_flattranslation,r2     // flattranslation => r2
 load (r2),r2
 add r2,r1                      // flattranslation[frontsector->floorpic]
 load (r1),r1
 store r1,(r0)                  // r1 => segl->floorpic (but, not a pointer??) (phase2.c line 162, minus burger mods)

 load (FP),r0 ; local f_ceilingpic
 movei #-1,r1
 cmp r0,r1                      // f_ceilingpic == -1?
 movei #L56,scratch
 jump NE,(scratch)              // if NOT, jump to L56.
 nop                            // SO: f_ceilingpic == -1, this is SKY (phase2.c line 163)

 move r15,r0 ;(segl)            // f_ceilingpic is -1, so... segl => r0
 addq #20,r0                    // segl += 20 (&viswall_t::ceilingpic)
 movei #-1,r1                   // -1 => r1
 store r1,(r0)                  // r1 => segl->ceilingpic (but not a pointer!!) (phase2.c line 164)

 movei #L57,r0                  // goto L57
 jump T,(r0)
 nop

L56:                            // L56 -- f_ceilingpic is NOT -1, this is not sky. (phase2.c line 165)
 move r15,r0 ;(segl)               // segl => r0
 addq #20,r0                       // &viswall_t::ceilingpic
 load (FP),r1 ; local f_ceilingpic
 shlq #2,r1                        // adjust f_ceilingpic to array index
 movei #_flattranslation,r2        // flattranslation => r2
 load (r2),r2                      
 add r2,r1                         // flattranslation[f_ceilingpic] => r1
 load (r1),r1
 store r1,(r0)                     // r1 => segl->ceilingpic (but not a pointer!) (phase2.c line 166)

//============================================================================

L57:                           // L57 - after ceilingpic assign (phase2.c line 168)
 move r22,r0 ;(seg)               // seg => r0
 addq #28,r0                      // &seg_t::backsector => r0
 load (r0),r0                     // seg->backsector => r0
 move r0,r17 ;(back_sector)       // back_sector => r17
 move r17,r0 ;(back_sector)       // r17 => r0 (phase2.c line 169)
 moveq #0,r1                      // 0 => r1
 cmp r0,r1                        // back_sector == 0?
 movei #L58,scratch
 jump NE,(scratch)                // if backsector != NULL, goto L58
 nop                              // SO: there is not a backsector (phase2.c line 170)

 movei #_emptysector,r0
 move r0,r17 ;(back_sector)       // backsector = &emptysector (phase2.c line 171)

L58:                            // NB: after seg->backsector NULL check.
 move FP,r0
 addq #32,r0 ; &b_ceilingpic      // &b_ceilingpic => r0
 move r17,r1 ;(back_sector)       // back_sector => r1
 addq #12,r1                      // r1 += &sector_t::ceilingpic
 load (r1),r1                     
 store r1,(r0)                    // backsector->ceilingpic => b_ceilingpic (phase2.c line 173)
 movei #36,r0
 add FP,r0 ; &b_lightlevel        // &b_lightlevel => r0
 move r17,r1 ;(back_sector)       // back_sector => r1
 addq #16,r1                      // r1 += &sector_t::lightlevel
 load (r1),r1
 store r1,(r0)                    // backsector->lightlevel => b_lightlevel (phase2.c line 174)
 load (r17),r0 ;(back_sector)     // back_sector => r0
 movei #_viewz,r1                 // viewz => r1
 load (r1),r1
 sub r1,r0                        // r0 = backsector->floorheight - viewz
 move r0,r19 ;(b_floorheight)     // r0 => b_floorheight (phase2.c line 175)
 moveq #4,r0                                // r0 == 4 *************************
 move r17,r2 ;(back_sector)       // back_sector => r2
 add r0,r2                        // r2 += &sector_t::ceilingheight
 load (r2),r2
 sub r1,r2                        // r2 = backsector->ceilingheight - viewz
 move r2,r20 ;(b_ceilingheight)   // r2 => b_ceilingheight (phase2.c line 176)
 
 //============================================================================
 // NB: this code is not in 3DO:
 
 move FP,r1
 addq #28,r1 ; &b_texturemid
 moveq #0,r2                                // r2 == 0 *************************
 store r2,(r1)                              // 0 => b_texturemid
 move FP,r1
 addq #12,r1 ; &t_texturemid
 store r2,(r1)                              // 0 => t_texturemid

 //============================================================================

 move FP,r1
 addq #4,r1 ; &actionbits
 store r2,(r1)                              // 0 => actionbits (phase2.c line 178)

 //=============================================================================================
 // NB: this code is missing in 3DO because of burger changes (function uses parameters instead)
 
 movei #40,r1                               // r0 = 40
 add FP,r1 ; &rw_x                          // LOCAL: rw_x
 move r15,r2 ;(segl)                        // segl => r2
 add r0,r2                                  // &viswall_t::start (r0 == 4 ^^^^) => r2
 load (r2),r0                               // segl->start => r0
 store r0,(r1)                              // segl->start => rw_x

 movei #44,r0                               // r0 == 44
 add FP,r0 ; &rw_stopx                      // LOCAL: rw_stopx
 move r15,r1 ;(segl)                        // segl => r1
 addq #8,r1                                 // &viswall_t::stop
 load (r1),r1                               // segl->stop => r1
 addq #1,r1                                 // r1 += 1
 store r1,(r0)                              // r1 => rw_stopx
 
 //=============================================================================================
 // NB: MOST of the code below is missing in 3DO.
 
 load (FP),r0 ; local f_ceilingpic          // f_ceilingpic => r0 (phase2.c line 163)
 movei #-1,r1                               // -1 => r1
 cmp r0,r1
 movei #L61,scratch
 jump NE,(scratch)                          // if f_ceilingpic != -1, goto L61
 nop                                        // SO: f_ceilingpic == -1
 
 load (FP+8),r0 ; local b_ceilingpic        // b_ceilingpic => r0 (NB: this check is not in 3DO!)
 cmp r0,r1                                  // r0 == -1?
 movei #L61,scratch
 jump NE,(scratch)                          // if b_ceilingpic != -1, goto L61
 nop                                        // SO: b_ceilingpic == -1
                                         // we are dealing with sky.
 movei #48,r0                               
 add FP,r0 ; &60                            // temp_skyhack => r0???
 moveq #1,r1                                // 1 => r1
 store r1,(r0)                              // r1 => temp_skyhack???
 
 movei #L62,r0
 jump T,(r0)                                // goto L62
 nop

L61:                                    // COND: f_ceilingpic != -1 && b_ceilingpic != -1
 movei #48,r0
 add FP,r0 ; &60                          // temp_skyhack => r0???
 moveq #0,r1                              // 0 => r1
 store r1,(r0)                            // r1 => temp_skyhack???

L62:
 move FP,r0
 addq #24,r0 ; &skyhack                   // &skyhack => r0
 load (FP+12),r1 ; local 60               // temp_skyhack => r1 ???
 store r1,(r0)                            // temp_skyhack => skyhack

 // ==== BACK ON TRACK WITH 3DO =============================================================
 moveq #0,r0                              // 0 => r0
 cmp r18,r0 ;(f_floorheight)              // f_floorheight == 0 ?

 movei #L63,scratch
 jump EQ,(scratch)                        // if f_floorheight == 0, goto L63
 nop                                      // SO: f_floorheight != 0 

 jump MI,(scratch)                        // if f_floorheight > 0, goto L63
 nop                                      // SO: floor is below viewz. (phase2.c line 182)

 moveq #8,r0
 move r21,r1 ;(front_sector)              // front_sector => r1
 add r0,r1                                // r1 += &sector_t::floorpic
 load (r1),r1                             // front_sector->floorpic => r1
 move r17,r2 ;(back_sector)               // back_sector => r2
 add r0,r2                                // r2 += &sector_t::floorpic
 load (r2),r0                             // back_sector->floorpic => r0

 cmp r1,r0
 movei #L67,scratch
 jump NE,(scratch)                        // if front_sector->floorpic != back_sector->floorpic, goto L67
 nop                                      // SO: sectors have same floorpic. (phase2.c line 183)

 cmp r18,r19 ;(f_floorheight)(b_floorheight)
 movei #L67,scratch
 jump NE,(scratch)                        // if f_floorheight != b_floorheight, goto L67
 nop                                      // SO: floor heights are equal (phase2.c line 184)

 load (FP+4),r0 ; local f_lightlevel      // f_lightlevel => r0
 load (FP+9),r1 ; local b_lightlevel      // b_lightlevel => r1
 cmp r0,r1                              
 movei #L67,scratch
 jump NE,(scratch)                        // if f_lightlevel != b_lightlevel, goto L67
 nop                                      // SO: sectors have same light levels (phase2.c line 185)

 cmp r20,r19 ;(b_ceilingheight)(b_floorheight) 
 movei #L63,scratch
 jump NE,(scratch)                      // if b_ceilingheight != f_floorheight, goto L63
 nop                                    // SO: b_ceilingheight == f_floorheight (phase2.c line 186)

L67:                                  // COND: Differences exist across this line on the floor.
 movei #64,r0                           
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::floornewheight
 move r18,r0 ;(f_floorheight)           // f_floorheight => r0
 sharq #10,r0                           // r0 >>= FIXEDTOHEIGHT
 store r0,(r1)                          // r0 => segl->floornewheight
 movei #60,r1                           
 move r15,r2 ;(segl)                    // segl => r2
 add r1,r2                              // r2 += &viswall_t::floorheight
 store r0,(r2)                          // r0 => segl->floorheight (phase2.c line 187)

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // *r0 => r1
 movei #33,r2                           // (AC_ADDFLOOR|AC_NEWFLOOR) => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 188)

L63:                                  // NB: After check for floor differences across line.
 load (FP+6),r0 ; local skyhack
 moveq #0,r1                            // 0 => r1
 cmp r0,r1                              // skyhack == 0 ?
 movei #L68,scratch
 jump NE,(scratch)                      // if skyhack != 0, goto L68 (phase2.c line 191)
 nop

 cmp r16,r1 ;(f_ceilingheight)
 movei #L70,scratch
 jump MI,(scratch)                      // if (f_ceilingheight > 0), goto L70 ? (camera above ceiling) (phase2.c line 192)
 nop

 load (FP),r0 ; local f_ceilingpic
 movei #-1,r1                           // -1 => r1
 cmp r0,r1                              // f_ceilingpic == -1 ?
 movei #L68,scratch
 jump NE,(scratch)                      // if f_ceilingpic != -1, goto L68 (phase2.c line 192)
 nop

L70:                                  // COND: f_ceilingheight > 0 && f_ceilingpic == -1
 load (FP),r0 ; local f_ceilingpic      // f_ceilingpic => r0
 load (FP+8),r1 ; local b_ceilingpic    // b_ceilingpic => r1
 cmp r0,r1
 movei #L73,scratch
 jump NE,(scratch)                      // if f_ceilingpic != b_ceilingpic, goto L73
 nop                                    // SO: ceiling pics are same. (phase2.c line 193)

 cmp r16,r20 ;(f_ceilingheight)(b_ceilingheight)
 movei #L73,scratch
 jump NE,(scratch)                      // if f_ceilingheight != b_ceilingheight, goto L73
 nop                                    // SO: heights are the same. (phase2.c line 194)

 load (FP+4),r0 ; local f_lightlevel    // f_lightlevel => r0
 load (FP+9),r1 ; local b_lightlevel    // b_lightlevel => r1
 cmp r0,r1
 movei #L73,scratch
 jump NE,(scratch)                      // if f_lightlevel != b_lightlevel, goto L73
 nop                                    // SO: light levels are same. (phase2.c line 195)

 cmp r20,r19 ;(b_ceilingheight)(b_floorheight)
 movei #L68,scratch
 jump NE,(scratch)                      // if b_ceilingheight != b_floorheight, goto L68 (phase2.c line 196)
 nop

L73:                                  // COND: A ceiling difference exists across this line.
 movei #72,r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::ceilingnewheight
 move r16,r0 ;(f_ceilingheight)         // f_ceilingheight => r0
 sharq #10,r0                           // r0 >>= FIXEDTOHEIGHT
 store r0,(r1)                          // r0 => segl->ceilingnewheight
 movei #68,r1                           // &viswall_t::ceilingheight => r1
 move r15,r2 ;(segl)                    // segl => r2
 add r1,r2                              // r2 += r1
 store r0,(r2)                          // r0 => segl->ceilingheight (phase2.c line 197)

 load (FP),r0 ; local f_ceilingpic      // f_ceilingpic => r0
 movei #-1,r1                           // -1 => r1
 cmp r0,r1                              // f_ceilingpic == -1 ?
 movei #L74,scratch
 jump NE,(scratch)                      // if NOT, goto L74 (phase2.c line 198)
 nop

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // actionbits => r1
 movei #80,r2                           // 80 == (AC_ADDSKY|AC_NEWCEILING) => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 199)
 movei #L75,r0
 jump T,(r0)                            // goto L75 (unconditional)
 nop

L74:                                  // COND: f_ceilingpic != -1
 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // actionbits => r1
 moveq #18,r2                           // 18 == (AC_ADDCEILING|AC_NEWCEILING) => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 201)

L75:                               // NB: after actionbits |= (AC_ADDSKY|AC_NEWCEILING)

L68:                               // COND: skyhack != 0
 move r15,r0 ;(segl)                    // segl => r0
 addq #28,r0                            // r0 += &viswall_t::t_topheight
 move r16,r1 ;(f_ceilingheight)         // f_ceilingheight => r1
 sharq #10,r1                           // r1 >>= FIXEDTOHEIGHT
 store r1,(r0)                          // r1 => segl->t_topheight; (phase2.c line 205)

 move r17,r0 ;(back_sector)
 movei #_emptysector,r1
 cmp r0,r1                              // backsector == &emptysector ?
 movei #L76,scratch
 jump NE,(scratch)                      // if NOT, goto L76 (phase2.c line 209)
 nop

 movei #40,r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::t_texture
 load (FP+2),r0 ; local si              // si => r0
 addq #16,r0                            // r0 += &side_t::midtexture
 load (r0),r0                           // si->midtexture => r0
 shlq #2,r0                             // adjust r0 to DWORD offset
 movei #_texturetranslation,r2          // texturetranslation => r2
 load (r2),r2                           // *texturetranslation => r2
 add r2,r0                              // r0 += r2 (texturetranslation[si->midtexture])
 load (r0),r0                           // texturetranslation[si->midtexture] => r0
 shlq #5,r0                             // adjust texture # * sizeof(texture_t) [32]
 movei #_textures,r2                    // textures => r2
 add r2,r0                              // &textures[r0] => r0
 store r0,(r1)                          // segl->t_texture = r0; (phase2.c line 211)

 load (FP+5),r0 ; local li              // li => r0
 addq #16,r0                            // &line_t::flags => r0
 load (r0),r0                           // li->flags => r0
 moveq #16,r1                           // ML_DONTPEGBOTTOM => r1
 and r1,r0                              // r0 &= ML_DONTPEGBOTTOM
 moveq #0,r1                            // 0 => r1
 cmp r0,r1                              // r0 == r1 ? (flag NOT set?)
 movei #L78,scratch
 jump EQ,(scratch)                      // if SO (flag NOT set), goto L78
 nop

 move FP,r0
 addq #12,r0 ; &t_texturemid            // &t_texturemid => r0
 movei #40,r1                           // &viswall_t::t_texture => r1
 move r15,r2 ;(segl)                    // segl => r2
 add r1,r2                              // r2 += r1
 load (r2),r1                           // segl->t_texture => r1
 addq #12,r1                            // r1 += &texture_t::height
 load (r1),r1                           // segl->t_texture->height => r1
 shlq #16,r1                            // r1 <<= FRACBITS
 move r18,r2 ;(f_floorheight)           // f_floorheight => r2
 add r1,r2                              // r2 += r1
 store r2,(r0)                          // r2 => t_texturemid (phase2.c line 213)
 movei #L79,r0
 jump T,(r0)                            // goto L79 (unconditional)
 nop

L78:                                  // COND: !(li->flags & ML_DONTPEGBOTTOM)
 move FP,r0
 addq #12,r0 ; &t_texturemid
 store r16,(r0) ;(f_ceilingheight)      // t_texturemid = f_ceilingheight; (phase2.c line 215)

L79:                                  // NB: jump dest from end of L68 seq
 move FP,r0
 addq #12,r0 ; &t_texturemid            // &t_texturemid => r0
 load (r0),r1                           // t_texturemid => r1
 load (FP+2),r2 ; local si              // si => r2
 addq #4,r2                             // &side_t::rowoffset => r2
 load (r2),r2                           // si->rowoffset => r2
 add r2,r1                              // r1 += r2 (t_texturemid += si->rowoffset)
 store r1,(r0)                          // r1 => t_texturemid (phase2.c line 217)

 move r15,r0 ;(segl)                    // segl => r0
 addq #32,r0                            // &viswall_t::t_bottomheight => r0
 move r18,r1 ;(f_floorheight)           // f_floorheight => r1
 sharq #10,r1                           // r1 >>= FIXEDTOHEIGHT
 store r1,(r0)                          // r1 => segl->t_bottomheight (phase2.c line 219)

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // actionbits => r1
 movei #1028,r2                         // r2 = (AC_SOLIDSIL|AC_TOPTEXTURE)
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 220)
 movei #L80,r0
 jump T,(r0)                            // goto L80 (unconditional)
 nop

L76:                                  // COND: backsector != &emptysector (2S line)
 cmp r19,r18 ;(b_floorheight)(f_floorheight)  // b_floorheight > f_floorheight?
 movei #L81,scratch
 jump PL,(scratch)                      // goto L81 if NOT (LE test) (phase2.c line 225)
 nop

 movei #56,r0                           // &viswall_t::b_texture => r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::b_texture
 load (FP+2),r0 ; local si              // si => r0
 addq #12,r0                            // r0 += &side_t::bottomtexture
 load (r0),r0                           // si->bottomtexture => r0
 shlq #2,r0                             // adjust to DWORD offset
 movei #_texturetranslation,r2          // texturetranslation => r2
 load (r2),r2                           // *texturetransation => r2
 add r2,r0                              // r0 += r2 (texturetranslation[si->bottomtexture])
 load (r0),r0                           // texturetranslation[si->bottomtexture] => r0
 shlq #5,r0                             // adjust to textures offset (sizeof(texture_t) == 32)
 movei #_textures,r2                    // textures => r2
 add r2,r0                              // r0 += r2 (textures[r0])
 store r0,(r1)                          // textures[r0] => segl->b_texture (phase2.c line 230)

 load (FP+5),r0 ; local li              // li => r0
 addq #16,r0                            // &line_t::flags => r0
 load (r0),r0                           // li->flags => r0
 moveq #16,r1                           // ML_DONTPEGBOTTOM => r1
 and r1,r0                              // r0 &= r1
 moveq #0,r1                            // 0 => r1
 cmp r0,r1                              // r0 == r1? [!(line->flags & ML_DONTPEGBOTTOM)]
 movei #L83,scratch
 jump EQ,(scratch)                      // if SO (flag NOT set), goto L83 (phase2.c line 231)
 nop

 move FP,r0
 addq #28,r0 ; &b_texturemid            // b_texturemid => r0
 store r16,(r0) ;(f_ceilingheight)      // f_ceilingheight => b_texturemid (phase2.c line 232)

 movei #L84,r0
 jump T,(r0)                            // goto L84 (unconditional)
 nop

L83:                                  // COND: !(li->flags & ML_DONTPEGBOTTOM)

 move FP,r0
 addq #28,r0 ; &b_texturemid            // &b_texturemid => r0
 store r19,(r0) ;(b_floorheight)        // b_floorheight => b_texturemid (phase2.c line 234)

L84:                                  // NB: dest after (li->flags & ML_DONTPEGBOTTOM)
 move FP,r0
 addq #28,r0 ; &b_texturemid            // &b_texturemid => r0
 load (r0),r1                           // b_texturemid => r1
 load (FP+2),r2 ; local si              // si => r2
 addq #4,r2                             // &side_t::rowoffset => r2
 load (r2),r2                           // si->rowoffset => r2
 add r2,r1                              // r1 += r2 (b_texturemid += si->rowoffset)
 store r1,(r0)                          // r1 => b_texturemid (phase2.c line 236)

 movei #64,r0                           // &viswall_t::floornewheight => r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::floornewheight
 move r19,r0 ;(b_floorheight)           // b_floorheight => r0
 sharq #10,r0                           // r0 >>= FIXEDTOHEIGHT
 store r0,(r1)                          // r0 => segl->floornewheight
 movei #44,r1                           // &viswall_t::b_topheight => r1
 move r15,r2 ;(segl)                    // segl => r2
 add r1,r2                              // r2 += r1
 store r0,(r2)                          // r0 => segl->b_topheight (phase2.c line 239)

 movei #48,r0                           // &viswall_t::b_bottomheight => r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += r0
 move r18,r0 ;(f_floorheight)           // f_floorheight => r0
 sharq #10,r0                           // >>= FIXEDTOHEIGHT
 store r0,(r1)                          // r0 => segl->b_bottomheight (phase2.c line 240)

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // actionbits => r1
 movei #40,r2                           // 40 == (AC_BOTTOMTEXTURE|AC_NEWFLOOR)
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 241)

L81:                                  // COND: b_floorheight <= f_floorheight
 cmp r20,r16 ;(b_ceilingheight)(f_ceilingheight)
 movei #L85,scratch
 jump EQ,(scratch)                      // if b_ceilingheight == f_ceilingheight, goto L85
 nop
 jump MI,(scratch)                      // if b_ceilingheight > f_ceilingheight, goto L85
 nop                                 // SO: b_ceilingheight is less than f_ceilingheight (there is an upper texture or sky) (phase2.c line 245)
 load (FP+6),r0 ; local skyhack         // skyhack => r0
 moveq #0,r1                            // 0 => r1
 cmp r0,r1                              // skyhack == 0?
 movei #L85,scratch
 jump NE,(scratch)                      // if NOT (there IS sky), goto L85
 nop

 movei #40,r0                        // COND: there is NOT sky; there should be an upper texture. (phase2.c line 250)
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::t_texture
 load (FP+2),r0 ; local si              // si => r0
 addq #8,r0                             // r0 += &sidedef_t::toptexture
 load (r0),r0                           // si->toptexture => r0
 shlq #2,r0                             // adjust to index
 movei #_texturetranslation,r2          // texturetransation => r2
 load (r2),r2                           
 add r2,r0
 load (r0),r0                           // texturetransation[si->toptexture] => r0
 shlq #5,r0                             // adjust by sizeof(texture_t)
 movei #_textures,r2                    // textures => r2
 add r2,r0                              // &textures[r0] => r0
 store r0,(r1)                          // segl->t_texture = r0 (phase2.c line 250)

 load (FP+5),r0 ; local li              // li => r0
 addq #16,r0                            // r0 += &line_t::flags
 load (r0),r0                           // li->flags => r0
 moveq #8,r1                            // ML_DONTPEGTOP => r1
 and r1,r0                              
 moveq #0,r1
 cmp r0,r1                              // li->flags & ML_DONTPEGTOP ?
 movei #L87,scratch
 jump EQ,(scratch)                      // if NOT, goto L87 (phase2.c line 251)
 nop

 move FP,r0
 addq #12,r0 ; &t_texturemid
 store r16,(r0) ;(f_ceilingheight)      // t_texturemid = f_ceilingheight (phase2.c line 252)

 movei #L88,r0
 jump T,(r0)                            // goto L88 (unconditional)
 nop

L87:                                  // COND: !(li->flags & ML_DONTPEGTOP)
 move FP,r0
 addq #12,r0 ; &t_texturemid            // &t_texturemid => r0
 movei #40,r1
 move r15,r2 ;(segl)                    // segl => r2
 add r1,r2                              // r2 += &viswall_t::t_texture
 load (r2),r1                           // segl->t_texture => r1
 addq #12,r1                            // r1 += &texture_t::height
 load (r1),r1                           // segl->t_texture->height => r1
 shlq #16,r1                            // r1 <<= FRACBITS
 move r20,r2 ;(b_ceilingheight)         // b_ceilingheight => r2
 add r1,r2                              // r2 += r1
 store r2,(r0)                          // r2 => t_texturemid (phase2.c line 254)

L88:                                  // After ML_DONTPEGTOP check (phase2.c line 256)
 move FP,r0
 addq #12,r0 ; &t_texturemid            // &t_texturemid => r0
 load (r0),r1                           // *r0 => r1
 load (FP+2),r2 ; local si              // si => r2
 addq #4,r2                             // r2 += &side_t::rowoffset
 load (r2),r2                           // si->rowoffset => r2
 add r2,r1                              
 store r1,(r0)                          // r1 += r2 (phase2.c line 256)
 movei #72,r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::ceilingnewheight
 move r20,r0 ;(b_ceilingheight)         // b_ceilingheight => r0
 sharq #10,r0                           // r0 >>= FIXEDTOHEIGHT
 store r0,(r1)                          // r0 => segl->ceilingnewheight
 move r15,r1 ;(segl)                    // segl => r1
 addq #32,r1                            // r1 += &viswall_t::t_bottomheight
 store r0,(r1)                          // r0 => segl->t_bottomheight (phase2.c line 258)

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // *r0 => r1
 moveq #20,r2                           // AC_NEWCEILING|AC_TOPTEXTURE => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 259)

L85:                                  // COND: skyhack != 0
 cmp r19,r16 ;(b_floorheight)(f_ceilingheight)
 movei #L91,scratch
 jump EQ,(scratch)                      // if b_floorheight == f_ceilingheight, goto L91
 nop
 jump MI,(scratch)                      // if b_floorheight > f_ceilingheight, goto L91
 nop                                    // SO: b_floorheight < f_ceilingheight
 cmp r20,r18 ;(b_ceilingheight)(f_floorheight)
 movei #L89,scratch
 jump MI,(scratch)                      // if b_ceilingheight > f_floorheight, goto L89
 nop                                    // SO: b_ceilingheight <= f_floorheight (ie, this line is closed)
 
L91:                                  // COND: b_floorheight >= f_ceilingheight || b_ceilingheight <= f_floorheight (phase2.c line 263)

 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // *r0 = > r1
 movei #1024,r2                         // AC_SOLIDSIL => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 264)

 movei #L90,r0
 jump T,(r0)                            // goto L90 (unconditional)
 nop

L89:                                  // COND: b_ceilingheight > f_floorheight (phase2.c line 265)
 movei #52,r0
 add FP,r0 ; &width                     // &width   => r0
 load (FP+11),r1 ; local rw_stopx       // rw_stopx => r1
 load (FP+10),r2 ; local rw_x           // rw_x     => r2
 sub r2,r1                              // r1 -= r2
 addq #1,r1                             // r1 += 1  (phase2.c line 268)
 shrq #1,r1                             // r1 /= 2 (WHY? Because Jag has half horiz resolution (160)! NB: NOT in 3DO code!!!)
 store r1,(r0)                          // r1 => width
 moveq #0,r0                            // 0 => r0
 cmp r19,r0 ;(b_floorheight)
 movei #L95,scratch
 jump PL,(scratch)                      // if b_floorheight <= 0, goto L95
 nop
 cmp r19,r18 ;(b_floorheight)(f_floorheight)
 movei #L94,scratch
 jump MI,(scratch)                      // if b_floorheight > f_floorheight, goto L94
 nop                                    // SO: b_floorheight > 0, but b_floorheight <= f_floorheight (phase2.c line 269)
 
L95:
 moveq #0,r0                            // 0 => r0
 cmp r18,r0 ;(f_floorheight)            
 movei #L92,scratch
 jump EQ,(scratch)                      // if f_floorheight == 0, goto L92
 nop
 jump MI,(scratch)                      // if f_floorheight > 0, goto L92
 nop                                    // SO: f_floorheight < 0
 cmp r18,r19 ;(f_floorheight)(b_floorheight)
 movei #L92,scratch
 jump PL,(scratch)                      // if f_floorheight <= b_floorheight, goto L92
 nop                                    // SO: f_floorheight < 0 && f_floorheight > b_floorheight (phase2.c line 270)
 
L94:                                 // COND: (b_floorheight > 0 && b_floorheight > f_floorheight) || (f_floorheight < 0 && f_floorheight > b_floorheight) 
 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // *r0 => r1
 movei #512,r2                          // AC_BOTTOMSIL => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 271)
 movei #80,r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::bottomsil
 movei #_lastopening,r0                 // lastopening => r0
 load (r0),r0
 load (FP+10),r2 ; local rw_x           // rw_x => r2
 sub r2,r0                              // r0 -= r2
 store r0,(r1)                          // r0 => segl->bottomsil (phase2.c line 272)

 movei #_lastopening,r0                 // lastopening => r0
 load (FP+13),r1 ; local width          // width => r1
 shlq #1,r1                             // r1 *= 2 (removing half-width, NB: not in 3DO)
 load (r0),r2                           // *r0 => r2
 add r2,r1                              // r1 += r2
 store r1,(r0)                          // r1 => lastopening (phase2.c line 273)

L92:                                  // AFTER "get width of opening" block (phase2.c line 275)
 load (FP+6),r0 ; local skyhack         // skyhack => r0
 moveq #0,r1                            // 0 => r1
 cmp r0,r1                              
 movei #L96,scratch
 jump EQ,(scratch)                      // if skyhack == 0, goto L96
 nop                                    // THUS: skyhack!

 movei #L98,r0
 jump T,(r0)                            // goto L98 (unconditional) (phase2.c line 276 "only if no sky")
 nop

L96:                                 // COND: !skyhack
 moveq #0,r0                            // 0 => r0
 cmp r20,r0 ;(b_ceilingheight)
 movei #L102,scratch
 jump MI,(scratch)                      // if b_ceilingheight > 0, goto L102
 nop                                    // SO: b_ceilingheight <= 0.
 cmp r20,r16 ;(b_ceilingheight)(f_ceilingheight) 
 movei #L101,scratch
 jump S_LT,(scratch)                    // if b_ceilingheight >= f_ceilingheight, goto L101
 nop                                    // SO: b_ceilingheight < f_ceilingheight (phase2.c line 277)
 
L102:                                 // COND: (b_ceilingheight <= 0 && b_ceilingheight < f_ceilingheight)
 moveq #0,r0                            // 0 => r0
 cmp r16,r0 ;(f_ceilingheight)
 movei #L99,scratch
 jump PL,(scratch)                      // if f_ceilingheight <= 0, goto L99
 nop                                    // SO: f_ceilingheight > 0
 cmp r20,r16 ;(b_ceilingheight)(f_ceilingheight)
 movei #L99,scratch
 jump PL,(scratch)                      // if b_ceilingheight <= f_ceilingheight, goto L99
 nop                                    // SO: b_ceilingheight > f_ceilingheight
 
L101:                                 // COND: (f_ceilingheight > 0 && b_ceilingheight > f_ceilingheight) (phase2.c line 278)
 move FP,r0
 addq #4,r0 ; &actionbits               // &actionbits => r0
 load (r0),r1                           // *r0 => r1
 movei #256,r2                          // AC_TOPSIL => r2
 or r2,r1                               // r1 |= r2
 store r1,(r0)                          // r1 => actionbits (phase2.c line 279)
 movei #76,r0
 move r15,r1 ;(segl)                    // segl => r1
 add r0,r1                              // r1 += &viswall_t::topsil
 movei #_lastopening,r0                 // lastopening => r0
 load (r0),r0
 load (FP+10),r2 ; local rw_x           // rw_x => r2
 sub r2,r0                              // r0 -= r2
 store r0,(r1)                          // r0 => segl->topsil (phase2.c line 280)

 movei #_lastopening,r0                 // lastopening => r0
 load (FP+13),r1 ; local width          // width => r1
 shlq #1,r1                             // removing half-width (NB: not in 3DO)
 load (r0),r2
 add r2,r1                              // lastopening += width (phase2.c line 281)
 store r1,(r0)

 // COPY LOCALS INTO VISWALL ==================================================
L99: // COND: f_ceilingheight <= 0 (below viewz) || b_ceilingheight <= f_ceilingheight (no topsil)
L98: // COND: skyhack after bottomsil checks
L90: // COND: After AC_SOLIDSIL set due to closed line
L80:

 move r15,r0 ;(segl)
 addq #24,r0                          // &viswall_t::actionbits
 load (FP+1),r1 ; local actionbits
 store r1,(r0)                        // segl->actionbits = actionbits;

 movei #36,r0                         // &viswall_t::t_texturemid
 move r15,r1 ;(segl)
 add r0,r1
 load (FP+3),r0 ; local t_texturemid
 store r0,(r1)                        // segl->t_texturemid = t_texturemid;

 movei #52,r0                         // &viswall_t::b_texturemid
 move r15,r1 ;(segl)
 add r0,r1
 load (FP+7),r0 ; local b_texturemid
 store r0,(r1)                        // segl->b_texturemid = b_texturemid;

 movei #108,r0                        // &viswall_t::seglightlevel
 move r15,r1 ;(segl)
 add r0,r1
 load (FP+4),r0 ; local f_lightlevel
 store r0,(r1)                        // segl->seglightlevel = f_lightlevel;

 movei #100,r0                        // &viswall_t::offset
 move r15,r1 ;(segl)
 add r0,r1                            // r1 == &segl->offset
 load (FP+2),r0 ; local si
 load (r0),r0                         // si->offset => r0
 move r22,r2 ;(seg)
 addq #8,r2                           // r2 = &seg->offset
 load (r2),r2                         // r2 = seg->offset
 add r2,r0                            // r0 += seg->offset
 store r0,(r1)                        // r0 => segl->offset

 // increment segl ============================================================
L53:

 movei #112,r0           // sizeof(viswall_t)
 move r15,r1 ;(segl)     // segl => r1
 add r0,r1               // r1 += 112
 move r1,r15 ;(segl)     // r1 => segl

 // loop condition ============================================================
L55:

 move r15,r0 ;(segl)
 movei #_lastwallcmd,r1
 load (r1),r1
 cmp r0,r1
 movei #L52,scratch
 jump U_LT,(scratch)      // loop if segl < lastwallcmd (goto L52)
 nop

 movei #_phasetime+8,r0   // outside loop; store render time; GPU state
 movei #_samplecount,r1
 load (r1),r1
 store r1,(r0)

 movei #_gpucodestart,r0
 movei #_ref3_start,r1
 store r1,(r0)

 // RETURN FROM FUNCTION ======================================================
L51:
 movei #56,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
*/

/*
 24:   unsigned int  actionbits;       // STORE ACCOUNTED
 28:   int           t_topheight;
 32:   int           t_bottomheight;
 36:   int           t_texturemid;     // STORE ACCOUNTED
 40:   texture_t    *t_texture;
 44:   int           b_topheight;
 48:   int           b_bottomheight;
 52:   int           b_texturemid;     // STORE ACCOUNTED
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
100:   unsigned int  offset;           // STORE ACCOUNTED
104:   unsigned int  distance;
108:   unsigned int  seglightlevel;    // STORE ACCOUNTED
*/

/*
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
*/

// EOF

