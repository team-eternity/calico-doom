/*
  CALICO

  Renderer phase 5 - Graphics caching
*/

#include "doomdef.h"
#include "r_local.h"

// Doom palette to CRY lookup (hardcoded for efficiency on the Jag ASIC?)
static pixel_t vgatojag[] =
{
       1, 51487, 55319, 30795, 30975, 30747, 30739, 30731, 30727, 43831, 44075, 48415, 53015, 47183, 47175, 51263, 
   38655, 38647, 42995, 42731, 42727, 42719, 46811, 46803, 46795, 46535, 46527, 46523, 46515, 50607, 50599, 50339, 
   50331, 54423, 54415, 54155, 54147, 54143, 53879, 57971, 57963, 57703, 57695, 57691, 57683, 61519, 61511, 61507, 
   34815, 34815, 39167, 38911, 38911, 43263, 43007, 43007, 47359, 47351, 47087, 47079, 47071, 51415, 51407, 51147, 
   51391, 51379, 51371, 51363, 51355, 51343, 51335, 51327, 51319, 51307, 51295, 51539, 51531, 51519, 51763, 51755, 
   30959, 30951, 30943, 30939, 30931, 30923, 30919, 30911, 30903, 30899, 30891, 30887, 30879, 30871, 30867, 30859, 
   30851, 30847, 30839, 30831, 30827, 30819, 30811, 30807, 30799, 30791, 30787, 30779, 30775, 30767, 30759, 30755, 
   36095, 36079, 36063, 36047, 36031, 36015, 35999, 35987, 35971, 35955, 35939, 35923, 35907, 40243, 35875, 40215, 
   39103, 39095, 39087, 39079, 39071, 43163, 43155, 43147, 43139, 43131, 43127, 43119, 43111, 43103, 43095, 47187, 
   43167, 43151, 43139, 47479, 47463, 47451, 47183, 51523, 39295, 39283, 39275, 35171, 43607, 39503, 39495, 39487, 
   48127, 52203, 56279, 56003, 60079, 59547, 63367, 63091, 30975, 34815, 38911, 42751, 46591, 50431, 54271, 58111, 
   61695, 61679, 61667, 61655, 61643, 61631, 61619, 61607, 61595, 61579, 61567, 61555, 61543, 61531, 61519, 61507, 
   30719, 26623, 22527, 18175, 17919, 13567,  9215,  4607,   255,   227,   203,   179,   155,   131,   107,    83, 
   30975, 34815, 39167, 43263, 47359, 51455, 55295, 59391, 59379, 63467, 59103, 63447, 63179, 63171, 63159, 63151, 
   30975, 35071, 39423, 48127, 52479, 56831, 61183, 65535, 63143, 62879, 62867, 62599, 47183, 51267, 51255, 55087,
      83,    71,    59,    47,    35,    23,    11,     1, 30975, 30975, 29951, 28927, 28879, 32927, 32879, 42663
};

static void R_Malloc(void)
{
   // CALICO_TODO
   /*
 subq #4,FP

 move FP,r0
 addq #4,r0 ; &size
 load (r0),r1
 addq #24,r1
 store r1,(r0)
 load (r0),r1
 addq #7,r1
 movei #-8,r2
 and r2,r1
 store r1,(r0)
 movei #_refzone,r0
 load (r0),r0
 addq #4,r0
 load (r0),r0
 move r0,r16 ;(base)
 move r0,r17 ;(start)

 movei #L83,r0
 jump T,(r0)
 nop

L82:

 move r16,r0 ;(base)
 addq #4,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L85,scratch
 jump EQ,(scratch)
 nop

 move r16,r15 ;(base)(rover)

 movei #L86,r0
 jump T,(r0)
 nop

L85:

 move r16,r0 ;(base)
 addq #16,r0
 load (r0),r0
 move r0,r15 ;(rover)

L86:

 move r15,r0 ;(rover)
 moveq #0,r1
 cmp r0,r1
 movei #L87,scratch
 jump NE,(scratch)
 nop

 movei #L89,r0
 jump T,(r0)
 nop

L87:

 move r15,r0 ;(rover)
 addq #4,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L90,scratch
 jump EQ,(scratch)
 nop
 move r15,r0 ;(rover)
 addq #12,r0
 load (r0),r0
 movei #_framecount,r1
 load (r1),r1
 cmp r0,r1
 movei #L90,scratch
 jump NE,(scratch)
 nop

 move r15,r0 ;(rover)
 addq #16,r0
 load (r0),r0
 move r0,r16 ;(base)
 move r16,r0 ;(base)
 moveq #0,r1
 cmp r0,r1
 movei #L92,scratch
 jump NE,(scratch)
 nop

L89:

 movei #_refzone,r0
 load (r0),r0
 addq #8,r0
 move r0,r16 ;(base)

L92:

 move r16,r0 ;(base)
 move r17,r1 ;(start)
 cmp r0,r1
 movei #L83,scratch
 jump NE,(scratch)
 nop

 movei #_framecount,r0
 load (r0),r1
 addq #1,r1
 store r1,(r0)

 movei #L83,r0
 jump T,(r0)
 nop

L90:

 move r15,r0 ;(rover)
 addq #4,r0
 load (r0),r0
 movei #1024,r1
 cmp r0,r1
 movei #L96,scratch
 jump PL,(scratch)
 nop

 move r15,r0 ;(rover)
 addq #4,r0
 load (r0),r0
 moveq #0,r1
 store r1,(r0)

L96:

 move r15,r0 ;(rover)
 addq #10,r0
 moveq #0,r1
 storew r1,(r0)

 move r15,r0 ;(rover)
 addq #4,r0
 moveq #0,r1
 store r1,(r0)

 move r16,r0 ;(base)
 move r15,r1 ;(rover)
 cmp r0,r1
 movei #L98,scratch
 jump EQ,(scratch)
 nop

 load (r16),r0 ;(base)
 load (r15),r1 ;(rover)
 add r1,r0
 store r0,(r16) ;(base)

 moveq #16,r0
 move r16,r1 ;(base)
 add r0,r1
 move r15,r2 ;(rover)
 add r0,r2
 load (r2),r0
 store r0,(r1)

 move r15,r0 ;(rover)
 addq #16,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L100,scratch
 jump EQ,(scratch)
 nop

 move r15,r0 ;(rover)
 addq #16,r0
 load (r0),r0
 addq #20,r0
 store r16,(r0) ;(base)

L100:

L98:

L83:

 move r16,r0 ;(base)
 addq #4,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L82,scratch
 jump NE,(scratch)
 nop
 load (r16),r0 ;(base)
 load (FP+1),r1 ; local size
 cmp r0,r1
 movei #L82,scratch
 jump S_LT,(scratch)
 nop

 move FP,r0 ; &extra
 load (r16),r1 ;(base)
 load (FP+1),r2 ; local size
 sub r2,r1
 store r1,(r0)
 load (r0),r0
 movei #64,r1
 cmp r0,r1
 movei #L102,scratch
 jump PL,(scratch)
 nop

 load (FP+1),r0 ; local size
 add r16,r0 ;(base)
 move r0,r18 ;(new)
 load (FP),r0 ; local extra
 store r0,(r18) ;(new)

 move r18,r0 ;(new)
 addq #4,r0
 moveq #0,r1
 store r1,(r0)

 move r18,r0 ;(new)
 addq #12,r0
 moveq #0,r1
 store r1,(r0)
 move r18,r0 ;(new)
 addq #8,r0
 storew r1,(r0)

 move r18,r0 ;(new)
 addq #20,r0
 store r16,(r0) ;(base)

 moveq #16,r0
 move r18,r1 ;(new)
 add r0,r1
 move r16,r2 ;(base)
 add r0,r2
 load (r2),r0
 store r0,(r1)

 move r18,r0 ;(new)
 addq #16,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L104,scratch
 jump EQ,(scratch)
 nop

 move r18,r0 ;(new)
 addq #16,r0
 load (r0),r0
 addq #20,r0
 store r18,(r0) ;(new)

L104:

 move r16,r0 ;(base)
 addq #16,r0
 store r18,(r0) ;(new)

 load (FP+1),r0 ; local size
 store r0,(r16) ;(base)

L102:

 move r16,r0 ;(base)
 addq #4,r0
 load (FP+2),r1 ; local user
 store r1,(r0)

 move r16,r0 ;(base)
 addq #12,r0
 movei #_framecount,r1
 load (r1),r1
 store r1,(r0)

 move r16,r0 ;(base)
 addq #10,r0
 movei #7498,r1
 storew r1,(r0)

 move r16,r0 ;(base)
 addq #8,r0
 movei #101,r1
 storew r1,(r0)

 movei #_refzone,r0
 load (r0),r0
 addq #4,r0
 move r16,r1 ;(base)
 addq #16,r1
 load (r1),r1
 store r1,(r0)

 movei #_refzone,r0
 load (r0),r0
 addq #4,r0
 load (r0),r0
 moveq #0,r1
 cmp r0,r1
 movei #L106,scratch
 jump NE,(scratch)
 nop

 movei #_refzone,r0
 load (r0),r0
 move r0,r1
 addq #4,r1
 addq #8,r0
 store r0,(r1)

L106:

 load (FP+2),r0 ; local user
 move r16,r1 ;(base)
 addq #24,r1
 store r1,(r0)

 move r16,r0 ;(base)
 addq #24,r0
 move r0,RETURNVALUE

L81:
 jump T,(RETURNPOINT)
 addq #4,FP ; delay slot
   */
}

#define LENSHIFT 4 // this must be log2(LOOKAHEAD_SIZE)

//
// Decompress an lzss-compressed lump
//
static void R_decode(byte *input, pixel_t *output)
{
   int getidbyte = 0;
   int len;
   int pos;
   int i;
   pixel_t *source;
   int idbyte = 0;

   while(1)
   {
      // get a new idbyte if necessary
      if(!getidbyte)
         idbyte = *input++;
      getidbyte = (getidbyte + 1) & 7;

      if(idbyte&1)
      {
         // decompress
         pos = *input++ << LENSHIFT;
         pos = pos | (*input >> LENSHIFT);
         source = output - pos - 1;
         len = (*input++ & 0xf)+1;
         if(len == 1)
            break;
         for(i = 0; i < len; i++)
            *output++ = *source++;
      } 
      else 
         *output++ = vgatojag[*input++];

      idbyte = idbyte >> 1;
   }
}

static pixel_t *R_LoadPixels(int lumpnum)
{
   // CALICO_TODO
   return NULL;
   /*
movei #40,scratch
 sub scratch,FP

 load (FP+10),r0 ; local lumpnum          r0 = *(FP+10); // lumpnum
 shlq #2,r0                               r0 <<= 2;
 movei #_lumpcache,r1                     r1 = lumpcache;
 add r1,r0                                r0 += r1;
 load (r0),r0                             r0 = *r0;
 move r0,r15 ;(rdest)                     r15 = r0; // rdest
 move r15,r0 ;(rdest)                     r0 = r15;
 moveq #0,r1                              r1 = 0;
 cmp r0,r1                                if(r0 == r1)
 movei #L109,scratch                         goto L109;
 jump EQ,(scratch)
 nop

 move r15,r0 ;(rdest)                     r0 = r15; // rdest
 move r0,RETURNVALUE                      RETURNVALUE = r0;

 movei #L108,r0                           goto L108;
 jump T,(r0)
 nop

L109: // r0 == r1

 load (FP+10),r0 ; local lumpnum          r0 = *(FP+10); // lumpnum
 move r0,r1                               r1 = r0;
 shlq #4,r1                               r1 <<= 4;
 movei #_lumpinfo,r2                      r2 = lumpinfo;
 load (r2),r2                             r2 = *r2;
 add r2,r1                                r1 += r2;
 move r1,r16 ;(info)                      r16 = r1; // info
 move FP,r1                               r1 = FP;
 addq #8,r1 ; &count                      r1 += 8; // &count
 move r16,r2 ;(info)                      r2 = r16; // info
 addq #4,r2                               r2 += 4;
 load (r2),r2                             r2 = *r2;
 store r2,(r1)                            *r1 = r2;
 load (r1),r1                             r1 = *r1;
 shlq #1,r1                               r1 <<= 1;
 store r1,(FP) ; arg[]                    *(FP) = r1;
 shlq #2,r0                               r0 <<= 2;
 movei #_lumpcache,r1                     r1 = lumpcache;
 add r1,r0                                r0 += r1;
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]                  *(FP+1) = r0;
 movei #_R_Malloc,r0                      r0 = R_Malloc;
 store r28,(FP+4) ; push ;(RETURNPOINT)   
 store r16,(FP+5) ; push ;(info)
 movei #L111,RETURNPOINT
 jump T,(r0)                              call R_Malloc;
 store r15,(FP+6) ; delay slot ;(rdest)
L111:
 load (FP+5),r16 ; pop ;(info)
 load (FP+6),r15 ; pop ;(rdest)
 load (FP+4), RETURNPOINT ; pop
 move r29,r0 ;(RETURNVALUE)               r0 = RETURNVALUE;
 move r0,r15 ;(rdest)                     r15 = r0; // rdest
 move FP,r0                               r0 = FP;
 addq #12,r0 ; &rsrc                      r0 += 12; // rsrc
 load (r16),r1 ;(info)                    r1 = *r16; // info
 movei #_wadfileptr,r2                    r2 = wadfileptr;
 load (r2),r2                             r2 = *r2;
 add r2,r1                                r1 += r2;
 store r1,(r0)                            *r0 = r1;
 load (r0),r0                             r0 = *r0;
 store r0,(FP) ; arg[]                    *(FP) = r0;
 or r15,scratch ; scoreboard bug ;(rdest)
 store r15,(FP+1) ; arg[] ;(rdest)
 movei #_R_decode,r0                      r0 = R_decode
 store r28,(FP+4) ; push ;(RETURNPOINT)
 store r16,(FP+5) ; push ;(info)
 movei #L112,RETURNPOINT
 jump T,(r0)                              call R_decode;
 store r15,(FP+6) ; delay slot ;(rdest)
L112:
 load (FP+5),r16 ; pop ;(info)
 load (FP+6),r15 ; pop ;(rdest)
 load (FP+4), RETURNPOINT ; pop

 load (FP+10),r0 ; local lumpnum          r0 = *(FP+10);
 shlq #2,r0                               r0 <<= 2;
 movei #_lumpcache,r1                     r1 = lumpcache;
 add r1,r0                                r0 += r1;
 load (r0),r0                             r0 = *r0;
 move r0,RETURNVALUE                      RETURNVALUE = r0;

L108:
 movei #40,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
   */
}

void R_Cache(void)
{
   viswall_t   *wall;
   vissprite_t *spr;

   wall = viswalls;
   while(wall < lastwallcmd)
   {
      // load upper or middle texture if needed
      if(wall->actionbits & AC_TOPTEXTURE)
      {
         if(wall->t_texture->data == NULL)
            wall->t_texture->data = R_LoadPixels(wall->t_texture->lumpnum);
      }

      // load lower texture if needed
      if(wall->actionbits & AC_BOTTOMTEXTURE)
      {
         if(wall->b_texture->data == NULL)
            wall->b_texture->data = R_LoadPixels(wall->b_texture->lumpnum);
      }

      // load floorpic
      // CALICO: use floorpicnum to avoid type punning
      if(wall->floorpic == NULL)
         wall->floorpic = R_LoadPixels(firstflat + wall->floorpicnum);

      // load sky or normal ceilingpic
      // CALICO: use ceilingpicnum to avoid type punning
      if(wall->ceilingpicnum == -1) // sky 
      {
         if(skytexturep->data == NULL)
            skytexturep->data = R_LoadPixels(skytexturep->lumpnum);
      }
      else if(wall->ceilingpic == NULL)
         wall->ceilingpic = R_LoadPixels(firstflat + wall->ceilingpicnum);

      ++wall;
   }

   spr = vissprites;
   while(spr < vissprite_p)
   {
      if(spr->pixels == NULL)
         spr->pixels = R_LoadPixels(spr->patchnum + 1);

      ++spr;
   }
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

