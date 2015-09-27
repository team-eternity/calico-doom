/*
  CALICO

  Renderer phase 3 - Sprite prep
*/

#include "doomdef.h"
#include "r_local.h"

//
// Project vissprite for potentially visible actor
//
static void R_PrepMobj(mobj_t *thing)
{
   fixed_t tr_x, tr_y;
   fixed_t gxt, gyt;
/*
 load (FP+24),r0 ; local thing                 // param thing => r0
 move r0,r1                                    // r0 => r1
 addq #12,r1                                   // r1 += &mobj_t::x
 load (r1),r1                                  // thing->x => r1
 movei #_viewx,r2                              // &viewx => r2
 load (r2),r2                                  // *r2 => r2 
 sub r2,r1                                     // r1 -= r2
 move r1,r21 ;(trx)                            // r1 => r21 (trx) (phase1.c line 107)

 addq #16,r0                                   // r0 += &mobj_t::y
 load (r0),r0                                  // thing->y => r0
 movei #_viewy,r1                              // &viewy => r1
 load (r1),r1                                  // *r1 => r1
 sub r1,r0                                     // r0 -= r1
 move r0,r22 ;(try)                            // r0 => r22 (try) (phase1.c line 108)

 store r21,(FP) ; arg[] ;(trx)
 movei #_viewcos,r0
 load (r0),r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_G_FixedMul,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L121,RETURNPOINT
 jump T,(r0)                                   // G_FixedMul(trx, viewcos);
 store r15,(FP+18) ; delay slot push ;(vis)    

L121:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r29,r17 ;(RETURNVALUE)(gxt)              // => gxt (phase1.c line 110)

 store r22,(FP) ; arg[] ;(try)
 movei #_viewsin,r0
 load (r0),r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_G_FixedMul,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L122,RETURNPOINT
 jump T,(r0)                                   // G_FixedMul(try, viewsin)
 store r15,(FP+18) ; delay slot push ;(vis)
L122:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r29,r0 ;(RETURNVALUE)
 neg r0                                        // r0 = -r0 
 move r0,r18 ;(gyt)                            // => gyt   (r_things.c line 489)
 move r17,r0 ;(gxt)                            // gxt => r0
 sub r18,r0 ;(gyt)                             // r0 -= gyt
 move r0,r16 ;(tz)                             // r0 => tz (r_things.c line 492)
 movei #262144,r0                              // MINZ => r0
 cmp r16,r0 ;(tz)
 movei #L103,scratch
 jump EQ,(scratch)                             // if r0 == MINZ, goto L103
 nop
 jump MI,(scratch)                             // if r0 > MINZ, goto L103
 nop


 movei #L102,r0                              // COND: r0 < MINZ (r_things.c line 495)
 jump T,(r0)                                   // goto L102
 nop

L103:

 store r21,(FP) ; arg[] ;(trx)
 movei #_viewsin,r0
 load (r0),r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_G_FixedMul,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L123,RETURNPOINT
 jump T,(r0)                                   // G_FixedMul(tr_x, viewsin) (r_things.c line 500)
 store r15,(FP+18) ; delay slot push ;(vis)
L123:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r29,r0 ;(RETURNVALUE)                    // => r0
 neg r0                                        // r0 = -r0
 move r0,r17 ;(gxt)                            // r0 => gxt

 store r22,(FP) ; arg[] ;(try)
 movei #_viewcos,r0
 load (r0),r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_G_FixedMul,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L124,RETURNPOINT
 jump T,(r0)                                   // G_FixedMul(tr_y, viewcos) (r_things.c line 501)
 store r15,(FP+18) ; delay slot push ;(vis)
L124:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r29,r18 ;(RETURNVALUE)(gyt)              // => gyt

 move r18,r0 ;(gyt)                            // gyt => r0
 add r17,r0 ;(gxt)                             // r0 += gxt
 neg r0                                        // r0 = -r0
 move r0,r19 ;(tx)                             // r0 => tx (r_things.c line 502)

 move r16,r0 ;(tz)                             // tz => r0
 shlq #2,r0                                    // r0 <<= 2
 cmp r19,r0 ;(tx)                              
 movei #L107,scratch
 jump MI,(scratch)                             // if tx > tz<<2, goto L107 (phase1.c line 120)
 nop

 neg r0                                        // r0 = -r0 (tz)
 cmp r19,r0 ;(tx)
 movei #L105,scratch
 jump EQ,(scratch)                             // if tx == r0, goto L105
 nop
 jump MI,(scratch)                             // if tx > r0, goto L105
 nop

L107:                                        // COND: sprite is off-sides
 movei #L102,r0
 jump T,(r0)                                   // goto L102 (unconditional) (phase1.c line 121)
 nop

L105:
 load (FP+24),r0 ; local thing                 // thing => r0
 movei #36,r1
 add r1,r0                                     // r0 += &mobj_t::sprite
 load (r0),r0                                  // thing->sprite => r0
 movei #91,r1                                  // NUMSPRITES => r1
 cmp r0,r1
 movei #L108,scratch
 jump U_LT,(scratch)                           // if r0 < NUMSPRITES, goto L108 (r_things.c line 510)
 nop

 movei #L102,r0                                // sprite out of range, exit
 jump T,(r0)
 nop

L108:
 move FP,r0
 addq #28,r0 ; &sprdef                         // &sprdef => r0
 load (FP+24),r1 ; local thing                 // thing => r1
 movei #36,r2                                  // &mobj_t::sprite => r2
 move r1,r3                                    // thing => r3
 add r2,r3                                     // r3 += &mobj_t::sprite
 load (r3),r2                                  // thing->sprite => r2
 shlq #3,r2                                    // adjust to array index
 movei #_sprites,r3                            // sprites => r3
 add r3,r2                                     // &sprites[r3] => r2
 store r2,(r0)                                 // r2 => *r0 (sprdef) (r_things.c line 514)

 movei #40,r2                                  // &mobj_t::frame => r2
 add r2,r1                                     // r1 += &mobj_t::frame
 load (r1),r1                                  // thing->frame => r1
 movei #32767,r2                               // FF_FRAMEMASK => r2
 and r2,r1                                     // r1 &= r2
 load (r0),r0                                  // ...
 load (r0),r0                                  // numframes => r0
 cmp r1,r0
 movei #L110,scratch
 jump S_LT,(scratch)                           // if thing->frame < sprdef->numframes, goto L110
 nop


 movei #L102,r0                                // frame out of range, exit (r_things.c line 516)
 jump T,(r0)
 nop

L110:
 move FP,r0                                    
 addq #24,r0 ; &sprframe                       // &sprframe => r0
 movei #44,r1
 load (FP+24),r2 ; local thing                 // thing => r2
 movei #40,r3                                  // &mobj_t::frame => r3
 add r3,r2                                     // r2 += &mobj_t::frame
 load (r2),r2                                  // thing->frame => r2
 movei #32767,r3                               // FF_FRAMEMASK => r3
 and r3,r2                                     // r2 &= FF_FRAMEMASK
 move r1,MATH_A
 movei #L125,MATH_RTS
 movei #GPU_IMUL,scratch
 jump T,(scratch)
 move r2,MATH_B ; delay slot
L125:
 move MATH_C,r1                                // offset => r1
 load (FP+7),r2 ; local sprdef                 // sprdef => r2
 addq #4,r2                                    // &sprdef::spriteframes => r2
 load (r2),r2                                  // *r2 => r2
 add r2,r1                                     // r1 += r2
 store r1,(r0)                                 // r1 => sprframe (r_things.c line 520)

 load (r0),r0                                  // *sprframe => r0
 load (r0),r0                                  // sprframe.rotate => r0
 moveq #0,r1                                   // 0 => r1
 cmp r0,r1
 movei #L112,scratch
 jump EQ,(scratch)                             // if sprframe->rotate == 0, goto L112 (r_things.c line 522)
 nop
                                             // COND: rotating sprite
 movei #_viewx,r0                              // viewx => r0
 load (r0),r0
 store r0,(FP) ; arg[]                         // push as arg
 movei #_viewy,r0                              // viewy => r0
 load (r0),r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]                       // push as arg
 load (FP+24),r0 ; local thing                 // thing => r0
 move r0,r1                                    // r0 => r1
 addq #12,r1                                   // &mobj_t::x => r1
 load (r1),r1                                  // thing->x => r1
 or r1,scratch ; scoreboard bug
 store r1,(FP+2) ; arg[]                       // push as arg
 addq #16,r0                                   // r0 += &mobj_t::y
 load (r0),r0                                  // thing->y => r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+3) ; arg[]                       // push as arg
 movei #_R_PointToAngle3,r0                    
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L126,RETURNPOINT
 jump T,(r0)                                   // R_PointToAngle3(viewx, viewy, thing->x, thing->y)
 store r15,(FP+18) ; delay slot push ;(vis)
L126:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 movei #36,r0
 add FP,r0 ; &ang
 store r29,(r0) ;(RETURNVALUE)                 // => ang (phase1.c line 135, r_things.c line 525)

 move FP,r0
 addq #32,r0 ; &rot                            // &rot  => r0
 load (FP+9),r1 ; local ang                    // ang   => r1
 load (FP+24),r2 ; local thing                 // thing => r2
 addq #32,r2                                   // r2 += &mobj_t::angle
 load (r2),r2                                  // thing->angle => r2
 sub r2,r1                                     // r1 -= thing->r2
 movei #-1879048192,r2                         // ANG45/2*9 => r2
 add r2,r1                                     // r1 += r2
 shrq #29,r1                                   // r1 >>= 29
 store r1,(r0)                                 // r1 => rot (r_things.c line 526)

 move FP,r1
 addq #16,r1 ; &lump                           // &lump => r1
 load (r0),r0                                  // rot => r0
 move r0,r2                                    // r0  => r2
 shlq #2,r2                                    // adjust to array index
 load (FP+6),r3 ; local sprframe               // sprframe => r3
 move r3,r4                                    // r3 => r4
 addq #4,r4                                    // r4 += &spriteframe_t::lump
 add r4,r2                                     // r2 += r4 (&sprframe->lump[rot])
 load (r2),r2                                  // *r2 => r2
 store r2,(r1)                                 // r2 => lump (r_things.c line 527)

 move FP,r1
 addq #20,r1 ; &flip                           // &flip => r1
 movei #36,r2                                  // &spriteframe_t::flip => r2
 add r2,r3                                     // r3 += r2
 add r3,r0                                     // r0 += r3 (&sprframe->flip[rot])
 loadb (r0),r0                                 // *r0 => r0 
 store r0,(r1)                                 // r0 => flip (r_things.c line 528)

 movei #L113,r0
 jump T,(r0)                                   // goto L113 (unconditional)
 nop

L112:                                        // COND: Not a rotating sprite, single-view
 move FP,r0
 addq #16,r0 ; &lump                           // &lump => r0
 load (FP+6),r1 ; local sprframe               // &sprframe => r1
 move r1,r2                                    // r1 => r2
 addq #4,r2                                    // r2 += &spriteframe_t::lump
 load (r2),r2                                  // *r2 => r2
 store r2,(r0)                                 // r2 => lump (r_things.c line 533)
 move FP,r0
 addq #20,r0 ; &flip                           // &flip => r0
 movei #36,r2                                  // &spriteframe_t::flip => r2
 add r2,r1                                     // r1 += r2 (sprframe->flip[rot])
 loadb (r1),r1                                 // *r1 => r1
 store r1,(r0)                                 // r1 => flip (r_things.c line 534)

// === THIS CODE IS IN NEITHER VANILLA NOR 3DO AS SUCH ================================================
L113:                                        // After rotated or not determination
 movei #_vissprite_p,r0                        // &vissprite_p => r0
 load (r0),r0                                  // *r0 => r0
 move r0,r15 ;(vis)                            // r0 => vis
 move r15,r0 ;(vis)                            // r15 => r0
 movei #_vissprites+7680,r1                    // vissprites + (MAXVISSPRITES*sizeof(vissprite_t) => r1
 cmp r0,r1                                     
 movei #L114,scratch                           
 jump NE,(scratch)                             // if !=, goto L114
 nop
 
 movei #L102,r0                                // Exit because too many vissprites
 jump T,(r0)
 nop

L114:
 movei #_vissprite_p,r0                        // vissprite_p => r0
 movei #60,r1                                  // sizeof(vissprite_t) => r1
 move r15,r2 ;(vis)                            // vis => r2
 add r1,r2                                     // r2 += sizeof(vissprite_t)
 store r2,(r0)                                 // r2 => vissprite_p

 move r15,r0 ;(vis)                            // vis => r0
 addq #32,r0                                   // r0 += &vissprite_t::patch
 load (FP+4),r1 ; local lump                   // lump => r1
 store r1,(r0)                                 // r1 => vis->patch

// ====================================================================================================

 store r19,(r15) ;(tx)(vis)                   // tx => vis->x1

 movei #40,r0                                 // &vissprite_t::gx => r0
 move r15,r1 ;(vis)                           // vis => r1
 add r0,r1                                    // r1 += &vissprite_t::gx
 load (FP+24),r0 ; local thing                // thing => r0
 addq #12,r0                                  // r0 += &mobj_t::x
 load (r0),r0                                 // *r0 => r0
 store r0,(r1)                                // vis->gx = thing->x; (r_things.c line 556)

 movei #44,r0                                 // &vissprite_t::gy => r0
 move r15,r1 ;(vis)                           // vis => r1
 add r0,r1                                    // r1 += &vissprite_t::gy
 load (FP+24),r0 ; local thing                // thing => r0
 addq #16,r0                                  // r0 += &mobj_t::y
 load (r0),r0                                 // *r0 => r0
 store r0,(r1)                                // vis->gy = thing->y; (r_things.c line 557)

 movei #48,r0                                 // &vissprite_t::gz => r0
 move r15,r1 ;(vis)                           // vis => r1
 add r0,r1                                    // r1 += &vissprite_t::gz
 load (FP+24),r0 ; local thing                // thing => r0
 addq #20,r0                                  // r0 += &mobj_t::gz
 load (r0),r0                                 // *r0 => r0
 store r0,(r1)                                // vis->gz = thing->z; (r_things.c line 558)

 movei #5242880,r0                            // projection (centerx<<FRACBITS / 2; 80*FRACUNIT) = r0
 store r0,(FP) ; arg[]
 or r16,scratch ; scoreboard bug ;(tz)
 store r16,(FP+1) ; arg[] ;(tz)
 movei #_G_FixedDiv,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L127,RETURNPOINT
 jump T,(r0)                                   // G_FixedDiv(projection, tz)
 store r15,(FP+18) ; delay slot push ;(vis)
L127:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r29,r20 ;(RETURNVALUE)(xscale)           // => xscale (r_things.c line 498)

 move r15,r0 ;(vis)                            // vis => r0
 addq #12,r0                                   // r0 += &vissprite_t::xscale
 store r20,(r0) ;(xscale)                      // vis->xscale = xscale; (r_things.c line 555)~

 store r20,(FP) ; arg[] ;(xscale)
 movei #144179,r0                              // STRETCH => r0 (22*FRACUNIT/10) (2.2 scale, but, why?)
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_G_FixedMul,r0
 store r28,(FP+10) ; push ;(RETURNPOINT)
 store r22,(FP+11) ; push ;(try)
 store r21,(FP+12) ; push ;(trx)
 store r20,(FP+13) ; push ;(xscale)
 store r19,(FP+14) ; push ;(tx)
 store r18,(FP+15) ; push ;(gyt)
 store r17,(FP+16) ; push ;(gxt)
 store r16,(FP+17) ; push ;(tz)
 movei #L128,RETURNPOINT
 jump T,(r0)                                   // G_FixedMul(xscale, STRETCH)
 store r15,(FP+18) ; delay slot push ;(vis)
L128:
 load (FP+11),r22 ; pop ;(try)
 load (FP+12),r21 ; pop ;(trx)
 load (FP+13),r20 ; pop ;(xscale)
 load (FP+14),r19 ; pop ;(tx)
 load (FP+15),r18 ; pop ;(gyt)
 load (FP+16),r17 ; pop ;(gxt)
 load (FP+17),r16 ; pop ;(tz)
 load (FP+18),r15 ; pop ;(vis)
 load (FP+10), RETURNPOINT ; pop
 move r15,r0 ;(vis)                            // vis => r0
 addq #20,r0                                   // r0 += &vissprite_t::yscale
 store r29,(r0) ;(RETURNVALUE)                 // r29 => vis->yscale (phase1.c line 165)

 move r15,r0 ;(vis)                            // vis => r0
 addq #24,r0                                   // r0 += &vissprite_t::yiscale
 movei #-1,r1                                  // -1 => r1
 move r15,r2 ;(vis)                            // vis => r2
 addq #20,r2                                   // r2 += &vissprite_t::yscale
 load (r2),r2                                  // *r2 => r2
 div r2,r1                                     // r1 /= r2
 store r1,(r0)                                 // r1 => vis->yiscale

 load (FP+5),r0 ; local flip                   // flip => r0
 moveq #0,r1                                   // 0 => r1
 cmp r0,r1
 movei #L117,scratch
 jump EQ,(scratch)                             // if flip == 0, goto L117 (r_things.c line 565)
 nop
                                             // COND: flip != 0
 move r15,r0 ;(vis)                            // vis => r0
 addq #16,r0                                   // r0 += &vissprite_t::xiscale
 movei #-1,r1                                  // -1 => r1
 move r20,r2 ;(xscale)                         // xscale => r2
 div r2,r1                                     // r1 /= r2
 neg r1                                        // r1 = -r1
 store r1,(r0)                                 // r1 => vis->xiscale (r_things.c line 568)

 movei #L118,r0
 jump T,(r0)                                   // goto L118 (unconditional)
 nop

L117:                                        // COND: !flip
 move r15,r0 ;(vis)                            // vis => r0
 addq #16,r0                                   // r0 += &vissprite_t::xiscale
 movei #-1,r1                                  // -1 => r1
 move r20,r2 ;(xscale)                         // xscale => r2
 div r2,r1                                     // r1 /= r2
 store r1,(r0)                                 // r1 => vis->xiscale (r_things.c line 573)

L118:                                        // After if(flip)
 load (FP+24),r0 ; local thing                 // thing => r0
 movei #40,r1                                  // &mobj_t::frame => r1
 add r1,r0                                     // r0 += r1
 load (r0),r0                                  // thing->frame => r0
 movei #32768,r1                               // FF_FULLBRIGHT => r1
 and r1,r0                                     // r0 &= r1
 moveq #0,r1                                   // 0 => r1
 cmp r0,r1
 movei #L119,scratch
 jump EQ,(scratch)                             // if !(thing->frame & FF_FULLBRIGHT), goto L119
 nop
                                             // COND: fullbright thing
 movei #36,r0                                  // &vissprite_t::colormap => r0
 move r15,r1 ;(vis)                            // vis => r1
 add r0,r1                                     // r1 += r0
 movei #255,r0                                 // 255 => r0
 store r0,(r1)                                 // r0 => vis->colormap (phase1.c line 175)

 movei #L120,r0
 jump T,(r0)                                   // goto L120 (unconditional)
 nop

L119:                                        // thing is not fullbright
 move FP,r0
 addq #16,r0 ; &lump                           // &lump => r0
 load (FP+24),r1 ; local thing                 // thing => r1
 movei #52,r2                                  // &mobj_t::subsector => r2
 add r2,r1                                     // r1 += r2
 load (r1),r1                                  // thing->subsector => r1
 load (r1),r1                                  // r1->sector => r1
 addq #16,r1                                   // r1 += &sector_t::lightlevel
 load (r1),r1                                  // r1->lightlevel => r1
 store r1,(r0)                                 // r1 => lump (???, reuse?)
 movei #36,r1                                  // &vissprite_t::colormap => r1
 move r15,r2 ;(vis)                            // vis => r2
 add r1,r2                                     // r2 += r1
 load (r0),r0                                  // *r0 => r0
 store r0,(r2)                                 // r0 => vis->colormap

L120:                                        // RETURN FROM FUNCTION

L102:                                        // RETURN FROM FUNCTION
 movei #96,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
*/
}

//
// Process actors in all visible subsectors
//
void R_SpritePrep(void)
{
   subsector_t **ssp = vissubsectors;

   while(ssp < lastvissubsector)
   {
      subsector_t *ss = *ssp;
      sector_t    *se = ss->sector;

      if(se->validcount != validcount) // not already processed?
      {
         mobj_t *thing = se->thinglist;
         se->validcount = validcount;  // mark it as processed

         while(thing) // walk sector thing list
         {
            R_PrepMobj(thing);
            thing = thing->snext;
         }
      }
      ++ssp;
   }

   // TODO: player vissprite after L55
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
*/

