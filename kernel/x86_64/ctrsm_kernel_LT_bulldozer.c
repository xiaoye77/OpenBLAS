/*********************************************************************/
/* Copyright 2009, 2010 The University of Texas at Austin.           */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include "common.h"

static FLOAT dm1 = -1.;

#ifdef CONJ
#define GEMM_KERNEL   GEMM_KERNEL_L
#else
#define GEMM_KERNEL   GEMM_KERNEL_N
#endif

#if GEMM_DEFAULT_UNROLL_M == 1
#define GEMM_UNROLL_M_SHIFT 0
#endif

#if GEMM_DEFAULT_UNROLL_M == 2
#define GEMM_UNROLL_M_SHIFT 1
#endif

#if GEMM_DEFAULT_UNROLL_M == 4
#define GEMM_UNROLL_M_SHIFT 2
#endif

#if GEMM_DEFAULT_UNROLL_M == 6
#define GEMM_UNROLL_M_SHIFT 2
#endif

#if GEMM_DEFAULT_UNROLL_M == 8
#define GEMM_UNROLL_M_SHIFT 3
#endif

#if GEMM_DEFAULT_UNROLL_M == 16
#define GEMM_UNROLL_M_SHIFT 4
#endif

#if GEMM_DEFAULT_UNROLL_N == 1
#define GEMM_UNROLL_N_SHIFT 0
#endif

#if GEMM_DEFAULT_UNROLL_N == 2
#define GEMM_UNROLL_N_SHIFT 1
#endif

#if GEMM_DEFAULT_UNROLL_N == 4
#define GEMM_UNROLL_N_SHIFT 2
#endif

#if GEMM_DEFAULT_UNROLL_N == 8
#define GEMM_UNROLL_N_SHIFT 3
#endif

#if GEMM_DEFAULT_UNROLL_N == 16
#define GEMM_UNROLL_N_SHIFT 4
#endif



#ifndef CONJ

static void ctrsm_LT_solve_opt(BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc, FLOAT *as, FLOAT *bs)  __attribute__ ((noinline));

static void ctrsm_LT_solve_opt(BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc, FLOAT *as, FLOAT *bs)
{

	FLOAT *c1 = c + ldc*2 ;
	BLASLONG n1 = n * 4;
	BLASLONG i=0;

        __asm__  __volatile__
        (
	"	vzeroupper							\n\t"
	"	prefetcht0	(%4)						\n\t"
	"	prefetcht0	(%5)						\n\t"
	"	vxorps	%%xmm8 , %%xmm8 , %%xmm8				\n\t"
	"	vxorps	%%xmm9 , %%xmm9 , %%xmm9				\n\t"
	"	vxorps	%%xmm10, %%xmm10, %%xmm10				\n\t"
	"	vxorps	%%xmm11, %%xmm11, %%xmm11				\n\t"
	"	vxorps	%%xmm12, %%xmm12, %%xmm12				\n\t"
	"	vxorps	%%xmm13, %%xmm13, %%xmm13				\n\t"
	"	vxorps	%%xmm14, %%xmm14, %%xmm14				\n\t"
	"	vxorps	%%xmm15, %%xmm15, %%xmm15				\n\t"

	"	cmpq	       $0, %0						\n\t"
	"	je	       3f						\n\t"

	"	.align 16							\n\t"
	"1:									\n\t"

	"	vbroadcastss        (%3,%1,4), %%xmm0				\n\t"	// b0 real, b0 real
	"	vbroadcastss       4(%3,%1,4), %%xmm1				\n\t"	// b0 imag, b0 imag
	"	vbroadcastss       8(%3,%1,4), %%xmm2				\n\t"	// b1 real, b1 real
	"	vbroadcastss      12(%3,%1,4), %%xmm3				\n\t"	// b1 imag, b1 imag

	"	vmovups         (%2,%1,8), %%xmm4				\n\t"	// a0 real , a0 imag
	"	vmovups       16(%2,%1,8), %%xmm5				\n\t"	// a1 real , a1 imag

	"	vfnmaddps	%%xmm8 , %%xmm0 , %%xmm4 , %%xmm8		\n\t"   // a_real * b_real , a_imag * b_real
	"	vfnmaddps	%%xmm9 , %%xmm1 , %%xmm4 , %%xmm9		\n\t"   // a_real * b_imag , a_imag * b_imag

	"	vfnmaddps	%%xmm10, %%xmm0 , %%xmm5 , %%xmm10		\n\t"   // a_real * b_real , a_imag * b_real
	"	vfnmaddps	%%xmm11, %%xmm1 , %%xmm5 , %%xmm11		\n\t"   // a_real * b_imag , a_imag * b_imag

	"	vfnmaddps	%%xmm12, %%xmm2 , %%xmm4 , %%xmm12		\n\t"   // a_real * b_real , a_imag * b_real
	"	vfnmaddps	%%xmm13, %%xmm3 , %%xmm4 , %%xmm13		\n\t"   // a_real * b_imag , a_imag * b_imag

	"	vfnmaddps	%%xmm14, %%xmm2 , %%xmm5 , %%xmm14		\n\t"   // a_real * b_real , a_imag * b_real
	"	vfnmaddps	%%xmm15, %%xmm3 , %%xmm5 , %%xmm15		\n\t"   // a_real * b_imag , a_imag * b_imag

	"	addq		$4, %1						\n\t"
	"	cmpq		%1, %0						\n\t"

	"	jnz		1b						\n\t"


	"2:									\n\t"

	"	vshufps		$0xb1 , %%xmm9  , %%xmm9, %%xmm9		\n\t"
	"	vshufps		$0xb1 , %%xmm11 , %%xmm11 , %%xmm11		\n\t"
	"	vshufps		$0xb1 , %%xmm13 , %%xmm13 , %%xmm13		\n\t"
	"	vshufps		$0xb1 , %%xmm15 , %%xmm15 , %%xmm15		\n\t"

	"	vaddsubps		%%xmm8 , %%xmm9 , %%xmm8		\n\t"
	"	vaddsubps		%%xmm10, %%xmm11, %%xmm10		\n\t"
	"	vaddsubps		%%xmm12, %%xmm13, %%xmm12		\n\t"
	"	vaddsubps		%%xmm14, %%xmm15, %%xmm14		\n\t"

	"	vxorps		%%xmm7 , %%xmm7 , %%xmm7			\n\t"

	"	vaddsubps		%%xmm8 , %%xmm7 , %%xmm8		\n\t"
	"	vaddsubps		%%xmm10, %%xmm7 , %%xmm10		\n\t"
	"	vaddsubps		%%xmm12, %%xmm7 , %%xmm12		\n\t"
	"	vaddsubps		%%xmm14, %%xmm7 , %%xmm14		\n\t"

	"	vmovups		  (%4) , %%xmm0					\n\t"
	"	vmovups		16(%4) , %%xmm1					\n\t"

	"	vmovups		  (%5) , %%xmm4					\n\t"
	"	vmovups		16(%5) , %%xmm5					\n\t"

	"	vaddps		%%xmm0 , %%xmm8 , %%xmm8			\n\t"
	"	vaddps		%%xmm1 , %%xmm10, %%xmm10			\n\t"
	"	vaddps		%%xmm4 , %%xmm12, %%xmm12			\n\t"
	"	vaddps		%%xmm5 , %%xmm14, %%xmm14			\n\t"

	"	vmovups		%%xmm8  ,  (%4)					\n\t"
	"	vmovups		%%xmm10 ,16(%4)					\n\t"

	"	vmovups		%%xmm12 ,  (%5)					\n\t"
	"	vmovups		%%xmm14 ,16(%5)					\n\t"

	"3:									\n\t"	

	"	vzeroupper							\n\t"

        :
        :
          "r" (n1),     // 0    
          "a" (i),      // 1    
          "r" (a),      // 2
          "r" (b),      // 3
          "r" (c),      // 4
          "r" (c1),     // 5
          "r" (as),     // 6
          "r" (bs)      // 7
        : "cc",
          "%xmm0", "%xmm1", "%xmm2", "%xmm3",
          "%xmm4", "%xmm5", "%xmm6", "%xmm7",
          "%xmm8", "%xmm9", "%xmm10", "%xmm11",
          "%xmm12", "%xmm13", "%xmm14", "%xmm15",
          "memory"
        );

}

#endif



#ifndef COMPLEX

static inline void solve(BLASLONG m, BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc) {

  FLOAT aa, bb;

  int i, j, k;

  for (i = 0; i < m; i++) {

    aa = *(a + i);

    for (j = 0; j < n; j ++) {
      bb = *(c + i + j * ldc);
      bb *= aa;
      *b             = bb;
      *(c + i + j * ldc) = bb;
      b ++;

      for (k = i + 1; k < m; k ++){
	*(c + k + j * ldc) -= bb * *(a + k);
      }

    }
    a += m;
  }
}

#else

static inline void solve(BLASLONG m, BLASLONG n, FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc) {

  FLOAT aa1, aa2;
  FLOAT bb1, bb2;
  FLOAT cc1, cc2;

  int i, j, k;

  ldc *= 2;

  for (i = 0; i < m; i++) {

    aa1 = *(a + i * 2 + 0);
    aa2 = *(a + i * 2 + 1);

    for (j = 0; j < n; j ++) {
      bb1 = *(c + i * 2 + 0 + j * ldc);
      bb2 = *(c + i * 2 + 1 + j * ldc);

#ifndef CONJ
      cc1 = aa1 * bb1 - aa2 * bb2;
      cc2 = aa1 * bb2 + aa2 * bb1;
#else
      cc1 = aa1 * bb1 + aa2 * bb2;
      cc2 = aa1 * bb2 - aa2 * bb1;
#endif

      *(b + 0) = cc1;
      *(b + 1) = cc2;
      *(c + i * 2 + 0 + j * ldc) = cc1;
      *(c + i * 2 + 1 + j * ldc) = cc2;
      b += 2;

      for (k = i + 1; k < m; k ++){
#ifndef CONJ
	*(c + k * 2 + 0 + j * ldc) -= cc1 * *(a + k * 2 + 0) - cc2 * *(a + k * 2 + 1);
	*(c + k * 2 + 1 + j * ldc) -= cc1 * *(a + k * 2 + 1) + cc2 * *(a + k * 2 + 0);
#else
	*(c + k * 2 + 0 + j * ldc) -= cc1 * *(a + k * 2 + 0) + cc2 * *(a + k * 2 + 1);
	*(c + k * 2 + 1 + j * ldc) -= -cc1 * *(a + k * 2 + 1) + cc2 * *(a + k * 2 + 0);
#endif
      }

    }
    a += m * 2;
  }
}

#endif


int CNAME(BLASLONG m, BLASLONG n, BLASLONG k, FLOAT dummy1,
#ifdef COMPLEX
	   FLOAT dummy2,
#endif
	   FLOAT *a, FLOAT *b, FLOAT *c, BLASLONG ldc, BLASLONG offset){

  FLOAT *aa, *cc;
  BLASLONG  kk;
  BLASLONG i, j, jj;

#if 0
  fprintf(stderr, "TRSM KERNEL LT : m = %3ld  n = %3ld  k = %3ld offset = %3ld\n",
	  m, n, k, offset);
#endif

  jj = 0;

  j = (n >> GEMM_UNROLL_N_SHIFT);

  while (j > 0) {

    kk = offset;
    aa = a;
    cc = c;

    i = (m >> GEMM_UNROLL_M_SHIFT);

    while (i > 0) {

#ifdef CONJ

	if (kk > 0) {
	  GEMM_KERNEL(GEMM_UNROLL_M, GEMM_UNROLL_N, kk, dm1,
#ifdef COMPLEX
		      ZERO,
#endif
		      aa, b, cc, ldc);
	}

	solve(GEMM_UNROLL_M, GEMM_UNROLL_N,
	      aa + kk * GEMM_UNROLL_M * COMPSIZE,
	      b  + kk * GEMM_UNROLL_N * COMPSIZE,
	      cc, ldc);
#else

	ctrsm_LT_solve_opt(kk, aa, b, cc, ldc, aa + kk * GEMM_UNROLL_M * COMPSIZE, b  + kk * GEMM_UNROLL_N * COMPSIZE);

	solve(GEMM_UNROLL_M, GEMM_UNROLL_N,
	      aa + kk * GEMM_UNROLL_M * COMPSIZE,
	      b  + kk * GEMM_UNROLL_N * COMPSIZE,
	      cc, ldc);
#endif

      aa += GEMM_UNROLL_M * k * COMPSIZE;
      cc += GEMM_UNROLL_M     * COMPSIZE;
      kk += GEMM_UNROLL_M;
      i --;
    }

    if (m & (GEMM_UNROLL_M - 1)) {
      i = (GEMM_UNROLL_M >> 1);
      while (i > 0) {
	if (m & i) {
	    if (kk > 0) {
	      GEMM_KERNEL(i, GEMM_UNROLL_N, kk, dm1,
#ifdef COMPLEX
			  ZERO,
#endif
			  aa, b, cc, ldc);
	    }
	  solve(i, GEMM_UNROLL_N,
		aa + kk * i             * COMPSIZE,
		b  + kk * GEMM_UNROLL_N * COMPSIZE,
		cc, ldc);

	  aa += i * k * COMPSIZE;
	  cc += i     * COMPSIZE;
	  kk += i;
	}
	i >>= 1;
      }
    }

    b += GEMM_UNROLL_N * k   * COMPSIZE;
    c += GEMM_UNROLL_N * ldc * COMPSIZE;
    j --;
    jj += GEMM_UNROLL_M;
  }

  if (n & (GEMM_UNROLL_N - 1)) {

    j = (GEMM_UNROLL_N >> 1);
    while (j > 0) {
      if (n & j) {

	kk = offset;
	aa = a;
	cc = c;

	i = (m >> GEMM_UNROLL_M_SHIFT);

	while (i > 0) {
	  if (kk > 0) {
	    GEMM_KERNEL(GEMM_UNROLL_M, j, kk, dm1,
#ifdef COMPLEX
			ZERO,
#endif
			aa,
			b,
			cc,
			ldc);
	  }

	  solve(GEMM_UNROLL_M, j,
		aa + kk * GEMM_UNROLL_M * COMPSIZE,
		b  + kk * j             * COMPSIZE, cc, ldc);

	  aa += GEMM_UNROLL_M * k * COMPSIZE;
	  cc += GEMM_UNROLL_M     * COMPSIZE;
	  kk += GEMM_UNROLL_M;
	  i --;
	}

	if (m & (GEMM_UNROLL_M - 1)) {
	  i = (GEMM_UNROLL_M >> 1);
	  while (i > 0) {
	    if (m & i) {
	      if (kk > 0) {
		GEMM_KERNEL(i, j, kk, dm1,
#ifdef COMPLEX
			    ZERO,
#endif
			    aa,
			    b,
			    cc,
			    ldc);
	      }

	      solve(i, j,
		    aa + kk * i * COMPSIZE,
		    b  + kk * j * COMPSIZE, cc, ldc);

	      aa += i * k * COMPSIZE;
	      cc += i     * COMPSIZE;
	      kk += i;
	      }
	    i >>= 1;
	  }
	}

	b += j * k   * COMPSIZE;
	c += j * ldc * COMPSIZE;
      }
      j >>= 1;
    }
  }

  return 0;
}
