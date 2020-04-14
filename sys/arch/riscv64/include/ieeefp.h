/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Brian Bamsch <bbamsch@google.com>
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
 * 3. Neither the name of the copyright holder nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTOR(S) ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTOR(S) BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _MACHINE_IEEEFP_H_
#define _MACHINE_IEEEFP_H_

#include <sys/types.h>

/* Exception type (used by fpsetmask() et al.) */
typedef uint64_t fp_except;

/* Bit defines for fp_except */
#define	FP_X_INV	(1 << 4)	/* invalid operation */
#define	FP_X_DZ		(1 << 3)	/* division by zero */
#define	FP_X_OFL	(1 << 2)	/* overflow */
#define	FP_X_UFL	(1 << 1)	/* underflow */
#define	FP_X_UMP	(1 << 0)	/* imprecision (inexact) */
#define	FP_X_MASK	(0x1f)

/* Rounding modes */
typedef enum {
  FP_RN=0, /* round to nearest, ties to even */
  FP_RZ=1, /* round towards zero */
  FP_RM=3, /* round down (towards -infinity) */
  FP_RP=3, /* round up (towards +infinity) */
} fp_rnd;

#endif /* _MACHINE_IEEEFP_H_ */
