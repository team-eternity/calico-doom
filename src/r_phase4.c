/*
  CALICO

  Renderer phase 4 - Late prep
*/

#include "doomdef.h"
#include "r_local.h"

static boolean cacheneeded;
static fixed_t hyp;
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
   
   dx = D_abs(x - viewx);
   dy = D_abs(y - viewy);
   
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
   fixed_t sineval, rw_offset;
   angle_t offsetangle;

   offsetangle = normalangle - wc->angle1;

   if(offsetangle > ANG180)
      offsetangle = 0 - offsetangle;

   if(offsetangle > ANG90)
      offsetangle = ANG90;

   sineval = finesine[offsetangle >> ANGLETOFINESHIFT];
   rw_offset = FixedMul(hyp, sineval);

   if(normalangle - wc->angle1 < ANG180)
      rw_offset = -rw_offset;

   wc->offset += rw_offset;
   wc->centerangle = viewangle + ANG90 - normalangle;
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
   fixed_t      sineval, rw_distance;
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
   offsetangle = D_abs(normalangle - wc->angle1);
   
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
static void R_FinishSprite(vissprite_t *vis)
{
   int   lump;
   byte *patch;

   // get column headers
   lump  = vis->patchnum;                                // CALICO: use patchnum to avoid type punning
   patch = wadfileptr + BIGLONG(lumpinfo[lump].filepos); // CALICO: requires endianness correction
   vis->patch = (patch_t *)patch;
  
   // column pixel data is in the next lump
   vis->pixels = R_CheckPixels(lump + 1);

/* 
 load (FP+14),r0 ; local vis
 load (r0),r1
 move r1,r16 ;(tx)
 addq #12,r0
 load (r0),r0
 move r0,r19 ;(xscale)
 move r17,r0 ;(patch)
 addq #4,r0
 loadw (r0),r0
 movei #$ffff8000,scratch
 add scratch,r0
 xor scratch,r0
 shlq #16,r0
 move r16,r1 ;(tx)
 sub r0,r1
 move r1,r16 ;(tx)
 store r16,(FP) ; arg[] ;(tx)
 or r19,scratch ; scoreboard bug ;(xscale)
 store r19,(FP+1) ; arg[] ;(xscale)
 movei #_G_FixedMul,r0
 store r28,(FP+2) ; push ;(RETURNPOINT)
 store r22,(FP+3) ; push ;(80)
 store r21,(FP+4) ; push ;(77)
 store r20,(FP+5) ; push ;(lump)
 store r19,(FP+6) ; push ;(xscale)
 store r18,(FP+7) ; push ;(x2)
 store r17,(FP+8) ; push ;(patch)
 store r16,(FP+9) ; push ;(tx)
 movei #L88,RETURNPOINT
 jump T,(r0)
 store r15,(FP+10) ; delay slot push ;(x1)
L88:
 load (FP+3),r22 ; pop ;(80)
 load (FP+4),r21 ; pop ;(77)
 load (FP+5),r20 ; pop ;(lump)
 load (FP+6),r19 ; pop ;(xscale)
 load (FP+7),r18 ; pop ;(x2)
 load (FP+8),r17 ; pop ;(patch)
 load (FP+9),r16 ; pop ;(tx)
 load (FP+10),r15 ; pop ;(x1)
 load (FP+2), RETURNPOINT ; pop
 movei #5242880,r0
 move r29,r1 ;(RETURNVALUE)
 add r0,r1
 move r1,r0
 sharq #16,r0
 move r0,r15 ;(x1)
 movei #160,r0
 cmp r15,r0 ;(x1)
 movei #L73,scratch
 jump PL,(scratch)
 nop

 load (FP+14),r0 ; local vis
 addq #32,r0
 moveq #0,r1
 store r1,(r0)


 movei #L72,r0
 jump T,(r0)
 nop

L73:

 loadw (r17),r0 ;(patch)
 movei #$ffff8000,scratch
 add scratch,r0
 xor scratch,r0
 shlq #16,r0
 move r16,r1 ;(tx)
 add r0,r1
 move r1,r16 ;(tx)
 store r16,(FP) ; arg[] ;(tx)
 or r19,scratch ; scoreboard bug ;(xscale)
 store r19,(FP+1) ; arg[] ;(xscale)
 movei #_G_FixedMul,r0
 store r28,(FP+2) ; push ;(RETURNPOINT)
 store r22,(FP+3) ; push ;(80)
 store r21,(FP+4) ; push ;(77)
 store r20,(FP+5) ; push ;(lump)
 store r19,(FP+6) ; push ;(xscale)
 store r18,(FP+7) ; push ;(x2)
 store r17,(FP+8) ; push ;(patch)
 store r16,(FP+9) ; push ;(tx)
 movei #L89,RETURNPOINT
 jump T,(r0)
 store r15,(FP+10) ; delay slot push ;(x1)
L89:
 load (FP+3),r22 ; pop ;(80)
 load (FP+4),r21 ; pop ;(77)
 load (FP+5),r20 ; pop ;(lump)
 load (FP+6),r19 ; pop ;(xscale)
 load (FP+7),r18 ; pop ;(x2)
 load (FP+8),r17 ; pop ;(patch)
 load (FP+9),r16 ; pop ;(tx)
 load (FP+10),r15 ; pop ;(x1)
 load (FP+2), RETURNPOINT ; pop
 movei #5242880,r0
 move r29,r1 ;(RETURNVALUE)
 add r0,r1
 move r1,r0
 sharq #16,r0
 subq #1,r0
 move r0,r18 ;(x2)
 moveq #0,r0
 cmp r18,r0 ;(x2)
 movei #L75,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 load (FP+14),r0 ; local vis
 addq #32,r0
 moveq #0,r1
 store r1,(r0)


 movei #L72,r0
 jump T,(r0)
 nop

L75:

 load (FP+14),r0 ; local vis
 movei #52,r1
 move r0,r2
 add r1,r2
 movei #48,r1
 add r1,r0
 load (r0),r0
 move r17,r1 ;(patch)
 addq #6,r1
 loadw (r1),r1
 movei #$ffff8000,scratch
 add scratch,r1
 xor scratch,r1
 shlq #16,r1
 add r1,r0
 store r0,(r2)

 load (FP+14),r0 ; local vis
 move r0,r1
 addq #28,r1
 movei #52,r2
 add r2,r0
 load (r0),r0
 movei #_viewz,r2
 load (r2),r2
 sub r2,r0
 store r0,(r1)

 moveq #0,r0
 cmp r15,r0 ;(x1)
 movei #L78,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop
 moveq #0,r0
 move r0,r21 ;(77)
 movei #L79,r0
 jump T,(r0)
 nop
L78:
 move r15,r21 ;(x1)(77)
L79:
 load (FP+14),r0 ; local vis
 store r21,(r0) ;(77)

 movei #160,r0
 cmp r18,r0 ;(x2)
 movei #L81,scratch
 jump S_LT,(scratch)
 nop
 movei #159,r0
 move r0,r22 ;(80)
 movei #L82,r0
 jump T,(r0)
 nop
L81:
 move r18,r22 ;(x2)(80)
L82:
 load (FP+14),r0 ; local vis
 addq #4,r0
 store r22,(r0) ;(80)

 load (FP+14),r0 ; local vis
 addq #16,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L83,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 load (FP+14),r0 ; local vis
 addq #8,r0
 loadw (r17),r1 ;(patch)
 movei #$ffff8000,scratch
 add scratch,r1
 xor scratch,r1
 shlq #16,r1
 subq #1,r1
 store r1,(r0)

 movei #L84,r0
 jump T,(r0)
 nop

L83:

 load (FP+14),r0 ; local vis
 addq #8,r0
 moveq #0,r1
 store r1,(r0)

L84:

 moveq #0,r0
 cmp r15,r0 ;(x1)
 movei #L85,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 load (FP+14),r0 ; local vis
 move r0,r1
 addq #8,r1
 load (r1),r2
 addq #16,r0
 load (r0),r0
 move r0,MATH_A
 movei #L90,MATH_RTS
 movei #GPU_IMUL,scratch
 jump T,(scratch)
 move r15,MATH_B ; delay slot ;(x1)
L90:
 move MATH_C,r0
 sub r0,r2
 store r2,(r1)

L85:

L72:
 movei #56,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
*/
}

//
// Late prep for player psprites
//
static void R_FinishPSprite(vissprite_t *vis)
{
   fixed_t  topoffset;
   int      x1, x2;
   int      lump;
   byte    *patch;

   // get column headers
   lump  = vis->patchnum;                                // CALICO: use patchnum to avoid type punning
   patch = wadfileptr + BIGLONG(lumpinfo[lump].filepos); // CALICO: requires endianness correction throughout
   vis->patch = (patch_t *)patch;

   // column pixel data is in the next lump
   vis->pixels = R_CheckPixels(lump + 1);

   topoffset = (fixed_t)BIGSHORT(vis->patch->topoffset) << FRACBITS;
   vis->texturemid = 100*FRACUNIT - (vis->texturemid - topoffset);

   x1 = vis->x1 - BIGSHORT(vis->patch->leftoffset);

   // off the right side
   if(x1 > 160)
      return;

   x2 = (x1 + BIGSHORT(vis->patch->width)) - 1;

   // off the left side
   if(x2 < 0)
      return;

   // store information in vissprite
   vis->x1        = x1 < 0 ? 0 : x1;
   vis->x2        = x2 >= 160 ? 160 - 1 : x2;
   vis->xscale    = FRACUNIT;
   vis->yscale    = FRACUNIT;
   vis->yiscale   = FRACUNIT;
   vis->xiscale   = FRACUNIT;
   vis->startfrac = 0;
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
*/

