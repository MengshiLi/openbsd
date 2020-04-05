/*	$OpenBSD: brk.S,v 1.5 2018/08/12 17:15:10 mortimer Exp $	*/
/*	$NetBSD: brk.S,v 1.7 2003/12/26 11:23:44 martin Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)brk.s	5.2 (Berkeley) 12/17/90
 */

#include "SYS.h"

	.hidden	_C_LABEL(__minbrk)
	.hidden	_C_LABEL(__curbrk)
	.globl	_C_LABEL(__minbrk)
	.globl	_C_LABEL(__curbrk)
	.globl	_C_LABEL(_end)

	.data
	.align	3
_C_LABEL(__minbrk):
	.dword	_C_LABEL(_end)
END(__minbrk):
_C_LABEL(__curbrk):
	.dword	_C_LABEL(_end)
END(__curbrk)

/*
 * Change the data segment size
 */
ENTRY(brk)
	RETGUARD_SETUP(brk, t6)
	lla	t1, _C_LABEL(__minbrk)
	ld	t5, 0(t1)
	bgeu	a0, t5, 1f
	mv	a0, t5
1:
	SYSTRAP(break)
				/* error */
	sd	a0, 8(t1)	/* success */
	mv	a0, x0
	RETGUARD_CHECK(brk, t6)
	ret
END(brk)
	.weak	brk
