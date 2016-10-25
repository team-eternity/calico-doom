/*
  CALICO

  Renderer phase 4 - Late prep
*/

#include "doomdef.h"
#include "r_local.h"

static boolean cacheneeded;
angle_t normalangle;

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
// Get distance to point in 3D projection
//
static fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
   int     angle;
   fixed_t dx, dy, temp;
   
   dx = abs(x - viewx);
   dy = abs(y - viewy);
   
   if(dy > dx)
   {
      temp = dx;
      dx = dy;
      dy = temp;
   }
   
   angle = (tantoangle[FixedDiv(dy, dx)>>DBITS] + ANG90) >> ANGLETOFINESHIFT;
   
   // use as cosine
   return FixedDiv(dx, finesine[angle]);
}

//
// Convert angle and distance within view frustum to texture scale factor.
//
static fixed_t R_ScaleFromGlobalAngle(fixed_t rw_distance, angle_t visangle)
{
   angle_t anglea, angleb;
   fixed_t num, den;
   int     sinea, sineb;
   
   anglea = ANG90 + (visangle - viewangle);
   sinea  = finesine[anglea >> ANGLETOFINESHIFT];
   angleb = ANG90 + (visangle - normalangle);
   sineb  = finesine[angleb >> ANGLETOFINESHIFT];
   
   num = sineb * 22 * 8; // CALICO_TODO: This value makes no sense...
   den = FixedMul(rw_distance, sinea);

   return FixedDiv(num, den);
}

//
// Setup texture calculations for lines with upper and lower textures
//
static void R_SetupCalc(viswall_t *wc)
{
/*
 movei #36,scratch
 sub scratch,FP

 movei #_normalangle,r0       r0 = &normalangle
 load (r0),r0                 r0 = *r0
 load (FP+9),r1 ; local wc    r1 = &wc
 addq #12,r1                  r1 += 12
 load (r1),r1                 r1 = *r1
 sub r1,r0                    r0 -= r1
 move r0,r15 ;(offsetangle)   offsetangle = r0
 movei #-2147483648,r0        r0 = ANG180
 cmp r15,r0 ;(offsetangle)    offsetangle <= r0?
 movei #L104,scratch
 jump CC,(scratch)               goto L104
 nop

 move r15,r0 ;(offsetangle)   r0 = offsetangle
 neg r0                       r0 = -r0
 move r0,r15 ;(offsetangle)   offsetangle = r0

L104:

 movei #1073741824,r0         r0 = ANG90
 cmp r15,r0 ;(offsetangle)    offsetangle <= r0?
 movei #L106,scratch
 jump CC,(scratch)               goto L106
 nop

 movei #1073741824,r0         r0 = ANG90
 move r0,r15 ;(offsetangle)   offsetangle = r0

L106:

 move FP,r0                                           r0 = FP
 addq #8,r0 ; &sineval                                r0 += 8
 move r15,r1 ;(offsetangle)                           r1 = offsetangle
 shrq #19,r1                                          r1 >>= ANGLETOFINESHIFT
 shlq #2,r1                                           r1 <<= 2
 movei #_finesine,r2                                  r2 = finesine
 add r2,r1                                            r1 += r2
 load (r1),r1                                         r1 = *r1
 store r1,(r0)                                        *r0 = r1
 movei #_hyp,r1                                       r1 = _hyp
 load (r1),r1                                         r1 = *r1
 store r1,(FP) ; arg[]                                *FP = r1
 load (r0),r0                                         r0 = *r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]                              *(FP+1) = r0
 movei #_G_FixedMul,r0                                r0 = G_FixedMul
 store r28,(FP+3) ; push ;(RETURNPOINT)               *(FP+3) = r28
 store r16,(FP+4) ; push ;(rw_offset)                 *(FP+4) = rw_offset
 movei #L110,RETURNPOINT
 jump T,(r0)                                          call G_FixedMul
 store r15,(FP+5) ; delay slot push ;(offsetangle)    *(FP+5) = offsetangle
L110:
 load (FP+4),r16 ; pop ;(rw_offset)                   rw_offset = *(FP+4)
 load (FP+5),r15 ; pop ;(offsetangle)                 offsetangle = *(FP+5)
 load (FP+3), RETURNPOINT ; pop
 move r29,r16 ;(RETURNVALUE)(rw_offset)               rw_offset = r29

 movei #_normalangle,r0          r0 = &normalangle
 load (r0),r0                    r0 = *r0
 load (FP+9),r1 ; local wc       wc = *(FP+9)
 addq #12,r1                     wc += 12
 load (r1),r1                    wc = *(wc)
 sub r1,r0                       r0 -= r1
 movei #-2147483648,r1           r1 = ANG180
 cmp r0,r1                       r0 >= r1?
 movei #L108,scratch
 jump EQ,(scratch)                  goto L108
 nop
 jump CS,(scratch)                  goto L108
 nop  

 move r16,r0 ;(rw_offset)        r0 = rw_offset
 neg r0                          r0 = -r0
 move r0,r16 ;(rw_offset)        rw_offset = r0

L108:

 load (FP+9),r0 ; local wc       r0 = *(FP+9)
 movei #100,r1                   r1 = 100
 add r1,r0                       r0 += r1
 load (r0),r1                    r1 = *r0
 move r16,r2 ;(rw_offset)        r2 = rw_offset
 add r2,r1                       r1 += r2
 store r1,(r0)                   *r0 = r1

 load (FP+9),r0 ; local wc       r0 = *(FP+9)
 movei #96,r1                    r1 = 96
 add r1,r0                       r0 += r1
 movei #_viewangle,r1            r1 = &viewangle
 load (r1),r1                    r1 = *r1
 movei #1073741824,r2            r2 = ANG90
 add r2,r1                       r1 += r2
 movei #_normalangle,r2          r2 = &normalangle
 load (r2),r2                    r2 = *r2
 sub r2,r1                       r1 -= r2
 store r1,(r0)                   *r0 = r1


L103:
 movei #36,scratch               // pop stack
 jump T,(RETURNPOINT)            return;
 add scratch,FP ; delay slot
*/
}

//
// Late prep for viswalls
//
static void R_FinishWallPrep(viswall_t *wc)
{
   unsigned int fw_actionbits = wc->actionbits;
   texture_t   *fw_texture;
   angle_t      distangle, offsetangle;
   seg_t       *seg = wc->seg;
   fixed_t      hyp, sineval, rw_distance;
   fixed_t      scalefrac, scale2;
   
   // has top or middle texture?
   if(fw_actionbits & AC_TOPTEXTURE)
   {
      fw_texture = wc->t_texture;
      fw_texture->data = R_CheckPixels(fw_texture->lumpnum);
   }
   
   // has bottom texture?
   if(fw_actionbits & AC_BOTTOMTEXTURE)
   {
      fw_texture = wc->b_texture;
      fw_texture->data = R_CheckPixels(fw_texture->lumpnum);
   }
   
   // get floor texture
   wc->floorpic = R_CheckPixels(firstflat + wc->floorpicnum); // CALICO: use floorpicnum field here
   
   // is there sky at this wall?
   if(wc->ceilingpicnum == -1) // CALICO: likewise for ceilingpicnum
   {
      // cache skytexture if needed
      skytexturep->data = R_CheckPixels(skytexturep->lumpnum);
   }
   else
   {
      // normal ceilingpic
      wc->ceilingpic = R_CheckPixels(firstflat + wc->ceilingpicnum);
   }
   
   // this is essentially R_StoreWallRange
   // calculate rw_distance for scale calculation
   normalangle = seg->angle + ANG90;
   offsetangle = abs(normalangle - wc->angle1);
   
   if(offsetangle > ANG90)
      offsetangle = ANG90;
   
   distangle = ANG90 - offsetangle;
   hyp = R_PointToDist(seg->v1->x, seg->v1->y);
   sineval = finesine[distangle >> ANGLETOFINESHIFT];
   wc->distance = rw_distance = FixedMul(hyp, sineval);
   
   scalefrac = scale2 = wc->scalefrac =
      R_ScaleFromGlobalAngle(rw_distance, viewangle + xtoviewangle[wc->start]);

   if(wc->stop > wc->start)
   {
      scale2 = R_ScaleFromGlobalAngle(rw_distance, viewangle + xtoviewangle[wc->stop]);
      wc->scalestep = (scale2 - scalefrac) / (wc->stop - wc->start);
   }

   wc->scale2 = scale2;

   // does line have top or bottom textures?
   if(wc->actionbits & (AC_TOPTEXTURE|AC_BOTTOMTEXTURE))
   {
      wc->actionbits |= AC_CALCTEXTURE; // set to calculate texture info
      R_SetupCalc(wc);                  // do calc setup
   }
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

