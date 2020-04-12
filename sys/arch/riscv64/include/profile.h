/*
 * Copyright (c) 2020 Brian Bamsch <bbamsch@google.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define	_MCOUNT_DECL void _mcount

#define MCOUNT_ASM_NAME "__mcount"

#ifdef __PIC__
#define	PLTSYM		""
#else
#define	PLTSYM		""
#endif

#define MCOUNT							\
__asm__ (".text;"						\
	 ".align 2;"						\
	 ".globl " MCOUNT_ASM_NAME ";"				\
	 ".type " MCOUNT_ASM_NAME ",@function;"			\
	 MCOUNT_ASM_NAME ":;"					\
	 "	addi	sp, sp, -(16 * 8);"			\
	 "	sd	a0, (0 * 8)(sp);"			\
	 "	sd	a1, (1 * 8)(sp);"			\
	 "	sd	a2, (2 * 8)(sp);"			\
	 "	sd	a3, (3 * 8)(sp);"			\
	 "	sd	a4, (4 * 8)(sp);"			\
	 "	sd	a5, (5 * 8)(sp);"			\
	 "	sd	a6, (6 * 8)(sp);"			\
	 "	sd	a7, (7 * 8)(sp);"			\
	 "	sd	t0, (8 * 8)(sp);"			\
	 "	sd	t1, (9 * 8)(sp);"			\
	 "	sd	t2, (10 * 8)(sp);"			\
	 "	sd	t3, (11 * 8)(sp);"			\
	 "	sd	t4, (12 * 8)(sp);"			\
	 "	sd	t5, (13 * 8)(sp);"			\
	 "	sd	t6, (14 * 8)(sp);"			\
	 "	sd	ra, (15 * 8)(sp);"			\
	 /* load from pc at 8 off frame pointer */		\
	 "	ld	a0, 8(fp);"				\
	 "	mv	a1, ra;"				\
	 "	la	t0, " __STRING(_mcount) PLTSYM ";"	\
	 "	jalr	t0;"					\
	 /* restore argument registers */			\
	 "	ld	a0, (0 * 8)(sp);"			\
	 "	ld	a1, (1 * 8)(sp);"			\
	 "	ld	a2, (2 * 8)(sp);"			\
	 "	ld	a3, (3 * 8)(sp);"			\
	 "	ld	a4, (4 * 8)(sp);"			\
	 "	ld	a5, (5 * 8)(sp);"			\
	 "	ld	a6, (6 * 8)(sp);"			\
	 "	ld	a7, (7 * 8)(sp);"			\
	 "	ld	t0, (8 * 8)(sp);"			\
	 "	ld	t1, (9 * 8)(sp);"			\
	 "	ld	t2, (10 * 8)(sp);"			\
	 "	ld	t3, (11 * 8)(sp);"			\
	 "	ld	t4, (12 * 8)(sp);"			\
	 "	ld	t5, (13 * 8)(sp);"			\
	 "	ld	t6, (14 * 8)(sp);"			\
	 "	ld	ra, (15 * 8)(sp);"			\
	 "	addi	sp, sp, (16 * 8);"			\
	 "	ret;");

#ifdef _KERNEL
#define MCOUNT_ENTER						\
__asm__ ("csrrci %0, sstatus, %1; andi %0, %0, %1"		\
		: "=r" (s): "i" (SSTATUS_SIE));
#define	MCOUNT_EXIT						\
__asm__ ("andi %0, %0, %1; csrrs %x, sstatus, %0"		\
		:: "r" (s), "i" (SSTATUS_SIE));
	
#endif // _KERNEL
