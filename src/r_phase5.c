/*
  CALICO

  Renderer phase 5 - Graphics caching
*/

#include "doomdef.h"
#include "r_local.h"

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

#if 0
#define LENSHIFT 4 // this must be log2(LOOKAHEAD_SIZE)

unsigned char *decomp_input;
unsigned char *decomp_output;
extern int     decomp_start;

void decode(unsigned char *input, unsigned char *output)
{
   int getidbyte = 0;
   int len;
   int pos;
   int i;
   unsigned char *source;
   int idbyte = 0;

   while(1)
   {
      // get a new idbyte if necessary
      if (!getidbyte) idbyte = *input++;
      getidbyte = (getidbyte + 1) & 7;

      if (idbyte&1)
      {
         // decompress
         pos = *input++ << LENSHIFT;
         pos = pos | (*input >> LENSHIFT);
         source = output - pos - 1;
         len = (*input++ & 0xf)+1;
         if (len==1) break;
         for (i=0 ; i<len ; i++)
            *output++ = *source++;
      } else {
         *output++ = *input++;
      }

      idbyte = idbyte >> 1;
   }
}
#endif

static void R_decode(byte *input, pixel_t *out)
{
   // CALICO_TODO:
   /*
getidbyte       .equr   r4
idbyte          .equr   r5
ptr_input       .equr   r6
ptr_output      .equr   r7
pos             .equr   r8
ptr_source      .equr   r9
len             .equr   r10
lbl_main        .equr   r11
const_f			.equr	r12
palette			.equr	r13

;void R_decode(unsigned char *input, pixel_t *out)
;   decompress an lzss-compressed lump
		
;	Initialize vairables

	load	(FP),ptr_input
	load	(FP+1),ptr_output

	movei	#W_RL_main, lbl_main
	moveq	#$f,const_f                               const_f = 0xf;

; getidbyte = 0
	moveq	#1, getidbyte                             getidbyte = 1
	movei	#_vgatojag,palette                        palette   = vgatojag
	
;	Main decompression loop

	
	subq	#1,getidbyte                              getidbyte -= 1 (now 0)
W_RL_main:                                      while(1) {
	jr		EQ,w_loadidbyte	                        if(!getidbyte) goto w_loadidbyte;
	
; test low bit of idbyte
W_RL_testidbyte:
	btst	#0, idbyte		; a harmless delay slot   if(idbyte & 1)              if (idbyte&1) {...
	jr		NE, W_RL_uncompress                          idbyte >>= 1;                 
	shrq	#1, idbyte		; delay slot                  goto W_RL_uncompress       idbyte = idbyte >> 1;
	
; copy one character                                                          } else {
	loadb	(ptr_input), r0                           r0 = *ptr_input;              idbyte = idbyte >> 1; // is in delay slot above, so executes regardless of jump.
	addq	#1, ptr_input                             ++ptr_input;                  *output++ = *input++;
	shlq	#2,r0                                     r0 <<= 2;                  
	add		palette,r0                             r0 += &palette;
	load	(r0),r0			; convert to 16 bit cry    r0 = *r0;
	storew	r0, (ptr_output)                       *ptr_output = r0
	addq	#2, ptr_output                            ++ptr_output;
	jump	T, (lbl_main)                             getidbyte -= 1;               --getidbyte; continue;
	subq	#1,getidbyte	; delay slot               goto lbl_main;             }

; load idbyte and reset getidbyte                                             if (!getidbyte) idbyte = *input++;
w_loadidbyte:                                      w_loadidbyte:
	loadb	(ptr_input), idbyte                       idbyte = *ptr_input;
	addq	#1, ptr_input                             ++ptr_input;
	jr		T,W_RL_testidbyte                         getidbyte = 8;             getidbyte = (getidbyte + 1) & 7; // going down instead of up in asm.
	moveq	#8,getidbyte	; delay slot               (jump...)
		
W_RL_uncompress:

; get the position offset

	loadb	(ptr_input), r0                           r0 = *ptr_input;           pos = *input++ << LENSHIFT;
	addq	#1, ptr_input                             ++ptr_input;
	shlq	#4, r0                                    r0 <<= 4;
	loadb	(ptr_input), pos                          pos = *ptr_input;
	shrq	#4, pos                                   pos <<= 4;
	or	r0, pos                                      por |= r0;                 pos = pos | (*input >> LENSHIFT);

; add position offset to the output and store in ptr_source
	addq	#1,pos                                    pos += 1;
	move	ptr_output, ptr_source                    ptr_source = ptr_output;
	sub		pos, ptr_source                        ptr_source -= pos;
	sub		pos, ptr_source                        ptr_source -= pos;         source = output - pos - 1;

; get the length

	loadb	(ptr_input), len                          len = *ptr_input;
	addq	#1, ptr_input                             ++ptr_input;
	and		const_f, len                           len &= const_f; (0xf)      len = (*input++ & 0xf)+1;
	jump	EQ, (RETURNPOINT)		; if byte & 0xf == 0, done	                     if (len==1) break;
	loadw	(ptr_source), r0                          r0 = *ptr_source;
	
W_RL_copyloop:                                                                for (i=0 ; i<len ; i++)
	addq	#2, ptr_source                            ptr_source += 2;              *output++ = *source++;
	storew	r0, (ptr_output)                       *ptr_output = r0;
	subq	#1, len                                   len -= 1;
	addqt	#2, ptr_output                            ++ptr_output;
	jr		PL, W_RL_copyloop                         r0 = *ptr_source;
	loadw	(ptr_source), r0	; delay slot            (jump...)

	jump	T, (lbl_main)                             (loop...)
	subq	#1,getidbyte	; delay slot               getidbyte -= 1;            --getidbyte; continue;   
   */
}

static void R_LoadPixels(void)
{
   // CALICO_TODO
   /*
movei #40,scratch
 sub scratch,FP

 load (FP+10),r0 ; local lumpnum
 shlq #2,r0
 movei #_lumpcache,r1
 add r1,r0
 load (r0),r0
 move r0,r15 ;(rdest)
 move r15,r0 ;(rdest)
 moveq #0,r1
 cmp r0,r1
 movei #L109,scratch
 jump EQ,(scratch)
 nop

 move r15,r0 ;(rdest)
 move r0,RETURNVALUE

 movei #L108,r0
 jump T,(r0)
 nop

L109:

 load (FP+10),r0 ; local lumpnum
 move r0,r1
 shlq #4,r1
 movei #_lumpinfo,r2
 load (r2),r2
 add r2,r1
 move r1,r16 ;(info)
 move FP,r1
 addq #8,r1 ; &count
 move r16,r2 ;(info)
 addq #4,r2
 load (r2),r2
 store r2,(r1)
 load (r1),r1
 shlq #1,r1
 store r1,(FP) ; arg[]
 shlq #2,r0
 movei #_lumpcache,r1
 add r1,r0
 or r0,scratch ; scoreboard bug
 store r0,(FP+1) ; arg[]
 movei #_R_Malloc,r0
 store r28,(FP+4) ; push ;(RETURNPOINT)
 store r16,(FP+5) ; push ;(info)
 movei #L111,RETURNPOINT
 jump T,(r0)
 store r15,(FP+6) ; delay slot push ;(rdest)
L111:
 load (FP+5),r16 ; pop ;(info)
 load (FP+6),r15 ; pop ;(rdest)
 load (FP+4), RETURNPOINT ; pop
 move r29,r0 ;(RETURNVALUE)
 move r0,r15 ;(rdest)
 move FP,r0
 addq #12,r0 ; &rsrc
 load (r16),r1 ;(info)
 movei #_wadfileptr,r2
 load (r2),r2
 add r2,r1
 store r1,(r0)
 load (r0),r0
 store r0,(FP) ; arg[]
 or r15,scratch ; scoreboard bug ;(rdest)
 store r15,(FP+1) ; arg[] ;(rdest)
 movei #_R_decode,r0
 store r28,(FP+4) ; push ;(RETURNPOINT)
 store r16,(FP+5) ; push ;(info)
 movei #L112,RETURNPOINT
 jump T,(r0)
 store r15,(FP+6) ; delay slot push ;(rdest)
L112:
 load (FP+5),r16 ; pop ;(info)
 load (FP+6),r15 ; pop ;(rdest)
 load (FP+4), RETURNPOINT ; pop

 load (FP+10),r0 ; local lumpnum
 shlq #2,r0
 movei #_lumpcache,r1
 add r1,r0
 load (r0),r0
 move r0,RETURNVALUE

L108:
 movei #40,scratch
 jump T,(RETURNPOINT)
 add scratch,FP ; delay slot
   */
}

void R_Cache(void)
{
   // CALICO_TODO: R_Cache
   /*
subq #24,FP

 movei #_viswalls,r0
 move r0,r15 ;(wall)

 movei #L55,r0
 jump T,(r0)
 nop

L52:

 move r15,r0 ;(wall)
 addq #24,r0
 load (r0),r0
 moveq #4,r1
 and r1,r0
 moveq #0,r1
 cmp r0,r1
 movei #L56,scratch
 jump EQ,(scratch)
 nop
 movei #40,r0
 move r15,r1 ;(wall)
 add r0,r1
 load (r1),r0
 addq #16,r0
 load (r0),r0
 movei #4096,r1
 cmp r0,r1
 movei #L56,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 movei #40,r0
 move r15,r1 ;(wall)
 add r0,r1
 load (r1),r0
 addq #16,r0
 load (r0),r1
 store r1,(FP) ; arg[]
 movei #_R_LoadPixels,r1
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L75,RETURNPOINT
 jump T,(r1)
 store r0,(FP+4) ; delay slot push
L75:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r0 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r1 ;(RETURNVALUE)
 store r1,(r0)

L56:

 move r15,r0 ;(wall)
 addq #24,r0
 load (r0),r0
 moveq #8,r1
 and r1,r0
 moveq #0,r1
 cmp r0,r1
 movei #L58,scratch
 jump EQ,(scratch)
 nop
 movei #56,r0
 move r15,r1 ;(wall)
 add r0,r1
 load (r1),r0
 addq #16,r0
 load (r0),r0
 movei #4096,r1
 cmp r0,r1
 movei #L58,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 movei #56,r0
 move r15,r1 ;(wall)
 add r0,r1
 load (r1),r0
 addq #16,r0
 load (r0),r1
 store r1,(FP) ; arg[]
 movei #_R_LoadPixels,r1
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L76,RETURNPOINT
 jump T,(r1)
 store r0,(FP+4) ; delay slot push
L76:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r0 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r1 ;(RETURNVALUE)
 store r1,(r0)

L58:

 move r15,r0 ;(wall)
 addq #16,r0
 load (r0),r0
 movei #4096,r1
 cmp r0,r1
 movei #L60,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 move r15,r0 ;(wall)
 addq #16,r0
 load (r0),r1
 store r1,(FP) ; arg[]
 movei #_R_LoadPixels,r1
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L77,RETURNPOINT
 jump T,(r1)
 store r0,(FP+4) ; delay slot push
L77:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r0 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r1 ;(RETURNVALUE)
 store r1,(r0)

L60:

 move r15,r0 ;(wall)
 addq #20,r0
 load (r0),r0
 movei #-1,r1
 cmp r0,r1
 movei #L62,scratch
 jump NE,(scratch)
 nop

 movei #_skytexturep,r0
 load (r0),r0
 addq #16,r0
 load (r0),r0
 movei #4096,r1
 cmp r0,r1
 movei #L63,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 movei #_skytexturep,r0
 load (r0),r0
 addq #16,r0
 load (r0),r1
 store r1,(FP) ; arg[]
 movei #_R_LoadPixels,r1
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L78,RETURNPOINT
 jump T,(r1)
 store r0,(FP+4) ; delay slot push
L78:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r0 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r1 ;(RETURNVALUE)
 store r1,(r0)

 movei #L63,r0
 jump T,(r0)
 nop

L62:

 move r15,r0 ;(wall)
 addq #20,r0
 load (r0),r0
 movei #4096,r1
 cmp r0,r1
 movei #L66,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 move r15,r0 ;(wall)
 addq #20,r0
 load (r0),r1
 store r1,(FP) ; arg[]
 movei #_R_LoadPixels,r1
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L79,RETURNPOINT
 jump T,(r1)
 store r0,(FP+4) ; delay slot push
L79:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r0 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r1 ;(RETURNVALUE)
 store r1,(r0)

L66:

L63:

L53:

 movei #112,r0
 move r15,r1 ;(wall)
 add r0,r1
 move r1,r15 ;(wall)

L55:

 move r15,r0 ;(wall)
 movei #_lastwallcmd,r1
 load (r1),r1
 cmp r0,r1
 movei #L52,scratch
 jump U_LT,(scratch)
 nop

 movei #_vissprites,r0
 move r0,r16 ;(spr)

 movei #L71,r0
 jump T,(r0)
 nop

L68:

 movei #56,r0
 move r16,r1 ;(spr)
 add r0,r1
 load (r1),r0
 movei #4096,r1
 cmp r0,r1
 movei #L72,scratch
 jump EQ,(scratch)
 nop
 jump MI,(scratch)
 nop

 movei #56,r0
 move r16,r1 ;(spr)
 add r0,r1
 load (r1),r0
 store r0,(FP) ; arg[]
 movei #_R_LoadPixels,r0
 store r28,(FP+1) ; push ;(RETURNPOINT)
 store r16,(FP+2) ; push ;(spr)
 store r15,(FP+3) ; push ;(wall)
 movei #L80,RETURNPOINT
 jump T,(r0)
 store r1,(FP+4) ; delay slot push
L80:
 load (FP+2),r16 ; pop ;(spr)
 load (FP+3),r15 ; pop ;(wall)
 load (FP+4),r1 ; pop
 load (FP+1), RETURNPOINT ; pop
 move r29,r0 ;(RETURNVALUE)
 store r0,(r1)

L72:

L69:

 movei #60,r0
 move r16,r1 ;(spr)
 add r0,r1
 move r1,r16 ;(spr)

L71:

 move r16,r0 ;(spr)
 movei #_vissprite_p,r1
 load (r1),r1
 cmp r0,r1
 movei #L68,scratch
 jump U_LT,(scratch)
 nop

 movei #_phasetime+20,r0
 movei #_samplecount,r1
 load (r1),r1
 store r1,(r0)

 movei #_gpucodestart,r0
 movei #_ref6_start,r1
 store r1,(r0)


L51:
 jump T,(RETURNPOINT)
 addq #24,FP ; delay slot
   */
}

// EOF

