/*
  CALICO

  Renderer phase 4 - Late prep
*/

#include "doomdef.h"
#include "r_local.h"

static boolean cacheneeded;

//
// Check if texture is loaded; return if so, flag for cache if not
//
static void *R_CheckPixels(int lumpnum)
{
   void *lumpdata = lumpcache[lumpnum];
   
   if(lumpdata)
   {
      // touch this graphic resource with the current frame number so that it 
      // will not be immediately purged again during the same frame
      memblock_t *memblock = (memblock_t *)((byte *)lumpdata - sizeof(memblock_t));
      memblock->lockframe = framecount;
   }
   else
      cacheneeded = true; // phase 5 will need to be executed to cache graphics
   
   return lumpdata;
}

//
// Late prep for viswalls
//
static void R_FinishWallPrep(viswall_t *wc)
{
   unsigned int fw_actionbits = wc->actionbits;
   texture_t   *fw_texture;
   
   if(fw_actionbits & AC_TOPTEXTURE)
   {
      fw_texture = wc->t_texture;
      fw_texture->data = R_CheckPixels(fw_texture->lumpnum);
   }
   /*
fw_offsetangle	.equr	r19

notoptex:                                                          // AFTER TOP TEXTURE CHECK
	btst	#3,fw_actionbits		; AC_BOTTOMTEXTURE                     // fw_actionbits & AC_BOTTOMTEXTURE ?
	jr		EQ,nobottex                                                 // if not, goto nobottex
	nop
	                                                               // COND: have bottom texture
	load	(fw_wc+14),fw_texture                                       // fw_wc->b_texture => fw_texture
	moveq	#20,scratch                                                 // &texture_t::lumpnum => scratch
	add		fw_texture,scratch                                       // scratch += fw_texture
	load	(scratch),r0				; texture->lumpnum                  // texture->lumpnum => r0
 
	store	r0,(FP) ; arg[]                                             // r0 => arg
	move	pc,RETURNPOINT
	jump	T,(fw_R_CheckPixels)                                        // R_CheckPixels(texture->lumpnum)
	addq	#6,RETURNPOINT
	addq	#16,fw_texture                                              // fw_texture += &texture_t::data
	store	RETURNVALUE,(fw_texture)                                    // retval => fw_texture->data

nobottex:                                                          // AFTER BOTTOM TEXTURE CHECK
	movefa	alt_firstflat,r0                                         // firstflat => r0
	load	(fw_wc+4),r1		; wc->floorpic                            // fw_wc->floorpic => r1
	add		r1,r0                                                    // r0 += r1
 
	store	r0,(FP) ; arg[]                                             // r0 => arg
	move	pc,RETURNPOINT
	jump	T,(fw_R_CheckPixels)                                        // R_CheckPixels(firstflat + fw_wc->floorpic)
	addq	#6,RETURNPOINT
	store	RETURNVALUE,(fw_wc+4)	; wc->floorpic                      // retval => fw_wc->floorpic

	load	(fw_wc+5),r1		; wc->ceilingpic                          // fw_wc->ceilingpic => r1
	btst	#31,r1                                                      // == -1 ?
	jr		EQ,notskywall                                               // if SO, notskywall.
	nop
                                                                   // SKY WALL
	movei	#_skytexturep,r0                                            // &skytexturep => r0
	load	(r0),r0                                                     // *r0 = r0
	move	r0,r1                                                       // r0 => r1
	addq	#20,r1                                                      // r1 += &texture_t::lumpnum
	load	(r1),r1                                                     // skytexturep->lumpnum => r1
 
	store	r1,(FP) ; arg[]                                             // r1 => arg
	move	pc,RETURNPOINT
	jump	T,(fw_R_CheckPixels)                                        // R_CheckPixels(skytexturep->lumpnum)
	addq	#6,RETURNPOINT
	addq	#16,r0
	jr		T,gotceiling                                                // goto gotceiling, after:
	store	RETURNVALUE,(r0)	; delay slot, skytexturep->data           // retval => skytexturep->data

notskywall:                                                        // NORMAL CEILING FLAT
	movefa	alt_firstflat,r0                                         // firstflat => r0
	add		r1,r0                                                    // r0 += r1 (fw_wc=>ceilingpic)
 
	store	r0,(FP) ; arg[]                                             // r0 => arg
	move	pc,RETURNPOINT
	jump	T,(fw_R_CheckPixels)                                        // R_CheckPixels(firstflat + fw_wc->ceilingpic)
	addq	#6,RETURNPOINT
	store	RETURNVALUE,(fw_wc+5)	; wc->ceilingpic                    // retval => fw_wc->ceilingpic

gotceiling:                                                        // AFTER CEILINGPIC LOAD
 load (FP+15),r0 ; local wc                                          // fw_wc => r0
 load (r0),r1                                                        // fw_wc->seg => r1
 move r1,r16 ;(seg)                                                  // r1 => seg (local var)
 movei #_normalangle,r1                                              // &normalangle => r1
 moveq #12,r2                                                        // 12 => r2
 move r16,r3 ;(seg)                                                  // r16 => r3 (seg)
 add r2,r3                                                           // r3 += &seg_t::angle
 load (r3),r3                                                        // seg->angle => r3
 movei #1073741824,r4                                                // ANG90 => r4
 add r4,r3                                                           // r3 += r4
 store r3,(r1)                                                       // r4 => normalangle
 load (r1),r1                                                        // normalangle => r1
 add r2,r0                                                           // r0 += &viswall_t::angle1
 load (r0),r0                                                        // fw_wc->angle1 => r0
 sub r0,r1                                                           // r1 -= r0
 move r1,fw_offsetangle ;(offsetangle)                               // r1 => fw_offsetangle
 move fw_offsetangle,r0 ;(offsetangle)                               // fw_offsetangle => r0
 moveq #0,r1                                                         // 0 => r1
 cmp r0,r1
 movei #L118,scratch
 jump EQ,(scratch)                                                   // if fw_offsetangle == 0, goto L118
 nop
 jump MI,(scratch)                                                   // if fw_offsetangle > 0, goto L118
 nop

 neg fw_offsetangle		; offsetangle                                // fw_offsetangle = -fw_offsetangle;

L118:
 movei #1073741824,r0                                                  // ANG90 => r0
 cmp fw_offsetangle,r0 ;(offsetangle)
 movei #L120,scratch
 jump CC,(scratch)                                                     // if fw_offsetangle <= ANG90, goto L120
 nop

 movei #1073741824,r0                                                  // ANG90 => r0
 move r0,fw_offsetangle ;(offsetangle)                                 // r0 => fw_offsetangle

L120:
 move FP,r0
 addq #12,r0 ; &distangle                                              // &distangle (local) => r0
 movei #1073741824,r1                                                  // ANG90 => r1
 sub fw_offsetangle,r1 ;(offsetangle)                                  // r1 -= fw_offsetangle
 store r1,(r0)                                                         // r1 => distangle
 load (r16),r0 ;(seg)                                                  // seg->v1 => r0
 load (r0),r1                                                          // v1->x => r1
 
 addq #4,r0                                                            // r0 += &vertex_t::y
 load (r0),r0                                                          // v1->y => r0
 
	store	r1,(FP) ; arg[]                                               // v1->x => arg
	store	r0,(FP+1) ; arg[]                                             // v1->y => arg
	movei	#_R_PointToDist,r0
	move	pc,RETURNPOINT
	jump	T,(r0)                                                        // R_PointToDist(seg->v1->x, seg->v1->y)
	addq	#6,RETURNPOINT
	movei	#_hyp,r0                                                      // retval => hyp
	store	RETURNVALUE,(r0) ;(RETURNVALUE)

 move FP,r0
 addq #16,r0 ; &sineval                                                // &sineval => r0 (local)
 load (FP+3),r1 ; local distangle                                      // distangle => r1
 shrq #19,r1                                                           // r1 >>= ANGLETOFINESHIFT
 shlq #2,r1                                                            // adjust to array index
 movei #_finesine,r2                                                   // &finesine => r2
 add r2,r1                                                             // r1 += r2
 load (r1),r1                                                          // finesine[distangle>>19] => r1
 store r1,(r0)                                                         // r1 => sineval
 movei #_hyp,r1                                                        // &hyp => r1

	load	(r1),r1                                                       // hyp => r1
	load	(r0),r0                                                       // sineval => r0
	store	r1,(FP) ; arg[]                                               // hyp => arg
	store	r0,(FP+1) ; arg[]                                             // sineval => arg
	movei	#_G_FixedMul,r0
	move	pc,RETURNPOINT
	jump	T,(r0)                                                        // G_FixedMul(hyp, sineval)
	addq	#6,RETURNPOINT
	store	RETURNVALUE,(FP+2) ; rw_distance                              // => rw_distance
 
 load (FP+15),r0 ; local wc                                            // fw_wc => r0
 movei #104,r1                                                         // &viswall_t::distance => r1
 add r1,r0                                                             // r0 += r1
 move r29,r1 ;(RETURNVALUE)                                            // r29 => r1
 store r1,(r0)                                                         // r1 => fw_wc->distance

 load (FP+2),r0 ; local rw_distance                                    // rw_distance => r0
 store r0,(FP) ; arg[]                                                 // r0 => arg
 movei #_viewangle,r0                                                  // &viewangle => r0
 load (r0),r0                                                          // *r0 => r0
 load (FP+15),r1 ; local wc                                            // fw_wc => r1
 move r1,r2                                                            // r1 => r2
 addq #4,r2                                                            // r2 += &viswall_t::start
 load (r2),r2                                                          // wc->start => r2
 shlq #2,r2                                                            // adjust to array index
 movei #_xtoviewangle,r3                                               // xtoviewangle => r3
 add r3,r2                                                             // r2 += r3
 load (r2),r2                                                          // xtoviewangle[r2] => r2
 add r2,r0                                                             // r0 (viewangle) += r2
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]                                               // r0 => arg
 
	movei	#_R_ScaleFromGlobalAngle,r0
	move	pc,RETURNPOINT
	jump	T,(r0)                                                        // R_ScaleFromGlobalAngle(viewangle + xtoviewangle[start])
	addq	#6,RETURNPOINT

 movei #84,r0                                                          // &viswall_t::scalefrac => r0
 add r0,r1                                                             // r1 += r0
 move r29,r0 ;(RETURNVALUE)                                            // retval => r0
 store r0,(r1)                                                         // r0 => wc->scalefrac
 move r0,r17 ;(scale2)                                                 // r0 => r17 (local scale2)
 move FP,r1
 addq #20,r1 ; &scalefrac                                              // &scalefrac => r1
 store r0,(r1)                                                         // r0 => scalefrac
 load (FP+15),r0 ; local wc                                            // wc => r0
 move r0,r1                                                            // r0 => r1
 addq #8,r1                                                            // r1 += &viswall_t::stop
 load (r1),r1                                                          // wc->stop => r1
 addq #4,r0                                                            // r0 += &viswall_t::start
 load (r0),r0                                                          // wc->start => r0
 cmp r1,r0
 movei #L122,scratch
 jump PL,(scratch)                                                     // if wc->stop <= wc->start, goto L122
 nop
                                                                     // COND: wc->stop > wc->start
 load (FP+2),r0 ; local rw_distance                                    // rw_distance => r0
 store r0,(FP) ; arg[]                                                 // r0 => arg
 movei #_viewangle,r0                                                  // &viewangle => r0
 load (r0),r0                                                          // *r0 => r0
 load (FP+15),r1 ; local wc                                            // wc => r1
 addq #8,r1                                                            // r1 += &viswall_t::stop
 load (r1),r1                                                          // wc->stop => r1
 shlq #2,r1                                                            // adjust to array index
 movei #_xtoviewangle,r2                                               // xtoviewangle => r2
 add r2,r1                                                             // r1 += r2
 load (r1),r1                                                          // xtoviewangle[wc->stop] => r1
 add r1,r0                                                             // r0 += r1
 or r0,scratch ; scoreboard bug
 
	store	r0,(FP+1) ; arg[]                                             // r0 => arg
	movei	#_R_ScaleFromGlobalAngle,r0
	move	pc,RETURNPOINT
	jump	T,(r0)                                                        // R_ScaleFromGlobalAngle(viewangle + xtoviewangle[stop])
	addq	#6,RETURNPOINT
	move	RETURNVALUE,r17 ;(RETURNVALUE)(scale2)                        // retval => r17 (scale2)

 load (FP+15),r0 ; local wc                                            // wc => r0
 movei #92,r1                                                          // &viswall_t::scalestep => r1
 move r0,r2                                                            // r0 => r2
 add r1,r2                                                             // r2 += r1
 load (FP+5),r1 ; local scalefrac                                      // scalefrac => r1
 move r17,r3 ;(scale2)                                                 // scale2 => r3
 sub r1,r3                                                             // r3 -= r1 (scale2 - rw_scale)
 move r0,r1                                                            // r0 => r1
 addq #8,r1                                                            // r1 += &viswall_t::stop
 load (r1),r1                                                          // wc->stop => r1
 addq #4,r0                                                            // r0 += &viswall_t::start
 load (r0),r0                                                          // wc->start => r0
 sub r0,r1                                                             // r1 -= r0 (stop - start)
 move r1,MATH_SIGN
 move r1,MATH_B
 xor r3,MATH_SIGN
 move r3,r0
 abs MATH_B
 abs r0
 div MATH_B,r0
 btst #31, MATH_SIGN
 jr EQ,L135
 nop
 neg r0
L135:
 store r0,(r2)                                                         // div op result => r2 (wc->scalestep)

L122:
 load (FP+15),r0 ; local wc                                            // wc => r0
 movei #88,r1                                                          // &viswall_t::scale2 => r1
 add r1,r0                                                             // r0 += r1
 move r17,r1 ;(scale2)                                                 // scale2 => r1
 store r1,(r0)                                                         // scale2 => wc->scale2

 load (FP+15),r0 ; local wc                                            // wc => r0
 addq #24,r0                                                           // &viswall_t::actionbits => r0
 load (r0),r0                                                          // wc->actionbits => r0
 moveq #12,r1                                                          // (AC_TOPTEXTURE|AC_BOTTOMTEXTURE) => r1
 and r1,r0                                                             // r0 &= r1
 moveq #0,r1                                                           // 0 => r1
 cmp r0,r1                                                             // r0 == r1 ?
 movei #L124,scratch
 jump EQ,(scratch)                                                     // if SO (NO top/bottom texture), goto L124
 nop

 load (FP+15),r0 ; local wc                                            // wc => r0
 addq #24,r0                                                           // r0 += &viswall_t::actionbits
 load (r0),r1                                                          // wc->actionbits => r1
 movei #128,r2                                                         // AC_CALCTEXTURE => r2
 or r2,r1                                                              // r1 |= r2
 store r1,(r0)                                                         // r1 => wc->actionbits

	load	(FP+15),r0 ; local wc                                         // wc => r0
	store	r0,(FP) ; arg[]                                               // r0 => arg
	movei	#_R_SetupCalc,r0
 	move	pc,RETURNPOINT
	jump	T,(r0)                                                        // R_SetupCalc(wc)
 	addq	#6,RETURNPOINT

L124:                                                                // COND: no top/bottom texture
  load (FP+6), RETURNPOINT ; pop
  movei #60,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
   */
}

//
// Late prep for vissprites
//
static void R_FinishSprite(vissprite_t *spr)
{
}

//
// Late prep for player psprites
//
static void R_FinishPSprite(vissprite_t *spr)
{
}

//
// Start late prep rendering stage
//
boolean R_LatePrep(void)
{
   viswall_t   *wall;
   vissprite_t *spr;
   
   cacheneeded = false;   
   
   // finish viswalls
   for(wall = viswalls; wall < lastwallcmd; wall++)
      R_FinishWallPrep(wall);

   // finish actor sprites   
   for(spr = vissprites; spr < lastsprite_p; spr++)
      R_FinishSprite(spr);
   
   // finish player psprites
   for(; spr < vissprite_p; spr++)
      R_FinishPSprite(spr);
   
   return cacheneeded;
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
*/

