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
   line_t    *li;
   side_t    *si;
   sector_t  *front_sector, *back_sector;
   fixed_t    f_floorheight, f_ceilingheight;
   fixed_t    b_floorheight, b_ceilingheight;
   int        f_lightlevel, b_lightlevel;
   int        f_ceilingpic, b_ceilingpic;
   int        b_texturemid, t_texturemid;
   int        rw_x, rw_stopx;
   boolean    skyhack;
   unsigned int actionbits;

   while(segl < lastwallcmd)
   {
      seg = segl->seg;
      li  = seg->linedef;
      si  = seg->sidedef;

      li->flags |= ML_MAPPED; // mark as seen

      front_sector    = seg->frontsector;
      f_ceilingpic    = front_sector->ceilingpic;
      f_lightlevel    = front_sector->lightlevel;
      f_floorheight   = front_sector->floorheight   - viewz;
      f_ceilingheight = front_sector->ceilingheight - viewz;

      segl->floorpicnum   = flattranslation[front_sector->floorpic];
      segl->ceilingpicnum = (f_ceilingpic == -1) ? -1 : flattranslation[f_ceilingpic];

      back_sector = seg->backsector;
      if(!back_sector)
         back_sector = &emptysector;
      b_ceilingpic    = back_sector->ceilingpic;
      b_lightlevel    = back_sector->lightlevel;
      b_floorheight   = back_sector->floorheight   - viewz;
      b_ceilingheight = back_sector->ceilingheight - viewz;

      t_texturemid = b_texturemid = 0;
      actionbits = 0;

      // NB: this code is missing in 3DO (function uses parameters instead)
      rw_x     = segl->start;
      rw_stopx = segl->stop + 1;

      // deal with sky ceilings (also missing in 3DO)
      if(f_ceilingpic == -1 && b_ceilingpic == -1)
         skyhack = true;
      else 
         skyhack = false;

      // add floors and ceilings if the wall needs them
      if(f_floorheight < 0 &&                                // is the camera above the floor?
         (front_sector->floorpic != back_sector->floorpic || // floor texture changes across line?
          f_floorheight   != b_floorheight                || // height changes across line?
          f_lightlevel    != b_lightlevel                 || // light level changes across line?
          b_ceilingheight == b_floorheight))                 // backsector is closed?
      {
         segl->floorheight = segl->floornewheight = f_floorheight >> FIXEDTOHEIGHT;
         actionbits |= (AC_ADDFLOOR|AC_NEWFLOOR);
      }

      if(!skyhack                                         && // not a sky hack wall
         (f_ceilingheight > 0 || f_ceilingpic == -1)      && // ceiling below camera, or sky
         (f_ceilingpic    != b_ceilingpic                 || // ceiling texture changes across line?
          f_ceilingheight != b_ceilingheight              || // height changes across line?              
          f_lightlevel    != b_lightlevel                 || // light level changes across line?
          b_ceilingheight == b_floorheight))                 // backsector is closed?
      {
         segl->ceilingheight = segl->ceilingnewheight = f_ceilingheight >> FIXEDTOHEIGHT;
         if(f_ceilingpic == -1)
            actionbits |= (AC_ADDSKY|AC_NEWCEILING);
         else
            actionbits |= (AC_ADDCEILING|AC_NEWCEILING);
      }

      segl->t_topheight = f_ceilingheight >> FIXEDTOHEIGHT; // top of texturemap

      if(back_sector == &emptysector)
      {
         // single-sided line
         segl->t_texture = &textures[texturetranslation[si->midtexture]];

         // handle unpegging (bottom of texture at bottom, or top of texture at top)
         if(li->flags & ML_DONTPEGBOTTOM)
            t_texturemid = f_floorheight + (segl->t_texture->height << FRACBITS);
         else
            t_texturemid = f_ceilingheight;

         t_texturemid += si->rowoffset;                         // add in sidedef texture offset
         segl->t_bottomheight = f_floorheight >> FIXEDTOHEIGHT; // set bottom height
         actionbits |= (AC_SOLIDSIL|AC_TOPTEXTURE);             // solid line; draw middle texture only
      }
      else
      {
         // two-sided line
         
         // is bottom texture visible?
         if(b_floorheight > f_floorheight)
         {
            segl->b_texture = &textures[texturetranslation[si->bottomtexture]];
            if(li->flags & ML_DONTPEGBOTTOM)
               b_texturemid = f_ceilingheight;
            else
               b_texturemid = b_floorheight;

            b_texturemid += si->rowoffset; // add in sidedef texture offset

            segl->b_topheight    = segl->floornewheight = b_floorheight >> FIXEDTOHEIGHT;
            segl->b_bottomheight = f_floorheight >> FIXEDTOHEIGHT;
            actionbits |= (AC_BOTTOMTEXTURE|AC_NEWFLOOR); // generate bottom wall and floor
         }

         // is top texture visible?
         if(b_ceilingheight < f_ceilingheight && !skyhack)
         {
            segl->t_texture = &textures[texturetranslation[si->toptexture]];
            if(li->flags & ML_DONTPEGTOP)
               t_texturemid = f_ceilingheight;
            else
               t_texturemid = b_ceilingheight + (segl->t_texture->height << FRACBITS);

            t_texturemid += si->rowoffset; // add in sidedef texture offset

            segl->t_bottomheight = segl->ceilingnewheight = b_ceilingheight >> FIXEDTOHEIGHT;
            actionbits |= (AC_NEWCEILING|AC_TOPTEXTURE); // draw top texture and ceiling
         }

         // check if this wall is solid, for sprite clipping
         if(b_floorheight >= f_ceilingheight || b_ceilingheight <= f_floorheight)
            actionbits |= AC_SOLIDSIL;
         else
         {
            int width;

            // get width of opening
            // note this is halved because openings are treated like bytes 
            // everywhere despite using short storage
            width = (rw_stopx - rw_x + 1) / 2; 

            if((b_floorheight > 0 && b_floorheight > f_floorheight) ||
               (f_floorheight < 0 && f_floorheight > b_floorheight))
            {
               actionbits |= AC_BOTTOMSIL; // set bottom mask
               segl->bottomsil = (byte *)lastopening - rw_x;
               lastopening += width;
            }

            if(!skyhack)
            {
               if((b_ceilingheight <= 0 && b_ceilingheight < f_ceilingheight) ||
                  (f_ceilingheight >  0 && b_ceilingheight > f_ceilingheight))
               {
                  actionbits |= AC_TOPSIL; // set top mask
                  segl->topsil = (byte *)lastopening - rw_x;
                  lastopening += width;
               }
            }
         }
      }

      // save local data to the viswall structure
      segl->actionbits    = actionbits;
      segl->t_texturemid  = t_texturemid;
      segl->b_texturemid  = b_texturemid;
      segl->seglightlevel = f_lightlevel;
      segl->offset        = si->textureoffset + seg->offset;
       
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
56:   int     validcount;                  // if == validcount, already checked
60:   mobj_t *thinglist;                   // list of mobjs in sector
64:   void   *specialdata;                 // thinker_t for reversable actions
68:   VINT    linecount;
72:   struct line_s **lines;               // [linecount] size
} sector_t;

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

