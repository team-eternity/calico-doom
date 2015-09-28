/*
  CALICO

  Renderer phase 4 - Late prep
*/

#include "doomdef.h"
#include "r_local.h"

//
// Start late prep rendering stage
//
boolean R_LatePrep(void)
{
   viswall_t   *wall;
   vissprite_t *spr;
/*
 subq #24,FP

 movei #_cacheneeded,r0                        // &cacheneeded => r0
 moveq #0,r1                                   // 0 => r1
 store r1,(r0)                                 // r1 => cacheneeded
 movei #_viswalls,r0                           // viswalls => r0
 move r0,r16 ;(wall)                           // r0 => r16 (wall)

//=== VISWALLS LOOP =====================================================================
 movei #L55,r0
 jump T,(r0)                                   // goto L55 (unconditional - loop start)
 nop

L52:                                         // LOOP START
 store r16,(FP) ; arg[] ;(wall)                // r16 => arg
 movei #_R_FinishWallPrep,r0
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(wall)
 movei #L69,RETURNPOINT
 jump T,(r0)                                   // R_FinishWallPrep(wall)
 store r15,(FP+3) ; delay slot push ;(spr)
L69:
 load (FP+2),r16 ; pop ;(wall)
 load (FP+3),r15 ; pop ;(spr)
 load (FP+1), RETURNPOINT ; pop

L53:
 movei #112,r0                                 // sizeof(viswall_t) => r0
 move r16,r1 ;(wall)                           // wall => r1
 add r0,r1                                     // r1 += r0
 move r1,r16 ;(wall)                           // r1 => r16

L55:                                         // VISWALLS LOOP CONDITION
 move r16,r0 ;(wall)                           // wall => r0
 movei #_lastwallcmd,r1                        // &lastwallcmd => r1
 load (r1),r1                                  // *r1 => r1
 cmp r0,r1
 movei #L52,scratch
 jump U_LT,(scratch)                           // if wall < lastwallcmd, goto L52 (loop)
 nop

//=== END OF VISWALLS LOOP ==============================================================
//=== VISSPRITES LOOP =================================================================== 
 movei #_vissprites,r0                         // &vissprites => r0
 move r0,r15 ;(spr)                            // r0 => spr

 movei #L59,r0                                 // goto L59 (loop condition)
 jump T,(r0)
 nop

L56:

 store r15,(FP) ; arg[] ;(spr)
 movei #_R_FinishSprite,r0
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(wall)
 movei #L70,RETURNPOINT
 jump T,(r0)
 store r15,(FP+3) ; delay slot push ;(spr)
L70:
 load (FP+2),r16 ; pop ;(wall)
 load (FP+3),r15 ; pop ;(spr)
 load (FP+1), RETURNPOINT ; pop

L57:

 movei #60,r0
 move r15,r1 ;(spr)
 add r0,r1
 move r1,r15 ;(spr)

L59:                                         // VISSPRITES LOOP CONDITION
 move r15,r0 ;(spr)                            // r15 => r0 (spr)
 movei #_lastsprite_p,r1                       // &lastsprite_p => r1
 load (r1),r1                                  // *r1 => r1
 cmp r0,r1
 movei #L56,scratch
 jump U_LT,(scratch)                           // if spr < lastsprite_p, loop
 nop
//=== END VISSPRITES LOOP ===============================================================
 
 movei #L63,r0
 jump T,(r0)                                   // goto L63 (unconditional)
 nop

//=== PSPRITES LOOP =====================================================================
L60:
 store r15,(FP) ; arg[] ;(spr)                 // r15 (spr) => arg
 movei #_R_FinishPSprite,r0
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(wall)
 movei #L71,RETURNPOINT
 jump T,(r0)                                   // R_FinishPSprite(spr)
 store r15,(FP+3) ; delay slot push ;(spr)
L71:
 load (FP+2),r16 ; pop ;(wall)
 load (FP+3),r15 ; pop ;(spr)
 load (FP+1), RETURNPOINT ; pop

L61:
 movei #60,r0
 move r15,r1 ;(spr)
 add r0,r1
 move r1,r15 ;(spr)

L63:                                         // PSPRITES LOOP CONDITION
 move r15,r0 ;(spr)                            // r15 (spr) => r0
 movei #_vissprite_p,r1                        // &vissprite_p => r1
 load (r1),r1                                  // *r1 => r1
 cmp r0,r1
 movei #L60,scratch
 jump U_LT,(scratch)                           // if spr < vissprite_p, loop
 nop
//=== END PSPRITES LOOP =================================================================
 
 movei #_phasetime+16,r0                       // Jag-specific timing crap
 movei #_samplecount,r1
 load (r1),r1
 store r1,(r0)

 movei #_cacheneeded,r0                        // &cacheneeded => r0
 load (r0),r0                                  // *r0 => r0
 moveq #0,r1                                   // 0 => r1
 cmp r0,r1
 movei #L65,scratch
 jump EQ,(scratch)                             // if !cacheneeded, goto L65
 nop

 movei #_gpucodestart,r0                       // COND: cacheneeded.
 movei #_ref5_start,r1                         // ref5_start => r1
 store r1,(r0)                                 // r1 => gpucodestart

 movei #L66,r0                                 
 jump T,(r0)                                   // goto L66 (unconditional)
 nop

L65:                                         // COND: !cacheneeded
 movei #_phasetime+20,r0                       // Jag-specific timing crap
 movei #_phasetime+16,r1
 load (r1),r1
 store r1,(r0)

 movei #_gpucodestart,r0                       // &gpucodestart => r0
 movei #_ref6_start,r1                         // _ref6_start => r1
 store r1,(r0)                                 // r1 => gpucodestart

L66:
 movei #_cacheneeded,r0                        // &cacheneeded => r0
 load (r0),r0                                  // *r0 => r0
 move r0,RETURNVALUE                           // return r0;

L51:
 jump T,(RETURNPOINT)
 addq #24,FP ; delay slot
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

