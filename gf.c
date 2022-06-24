/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2016, 2022
 *      ASUSA Corporation.  All rights reserved.
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
 */

/****************************************************************************

	This program provides simple and fast calculation functions/macros
	in GF(2^16) based on table lookup.
	GF16mul() and GF16div() are difined in gf.h for speedup. 

	CAUTION!! Never use b = 0 for all division functions div(a, b)
	because it outputs a non-zero random value.

					Hiroshi Nishida

****************************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <string.h> 
#include <errno.h> 
#include <time.h> 
#define	_GF_MAIN_
#include "gf.h"
#undef	_GF_MAIN_

/************************************************************
	Definitions
************************************************************/

// Definitions for GF16
#define	GF16_PRIM	69643	// Prim poly: 0x1100b = x^16 + x^12 + x^3 + x +1
#define	GF16_SIZE	65536	// = 16bit 

/*
	For more primitive polynomials, see:
	http://web.eecs.utk.edu/~plank/plank/papers/CS-07-593/primitive-polynomial-table.txt
*/


/************************************************************
	Functions
************************************************************/

// Initialize 16bit GF
void
GF16init(void)
{
	int		i;
	uint32_t	n;
	
	// Allocate memory 
	GF16memL = (uint16_t *)malloc(sizeof(uint16_t) * GF16_SIZE * 4);
	GF16memH = GF16memL + GF16_SIZE - 1; // Second half
	GF16memIdx = (int *)malloc(sizeof(int) * GF16_SIZE);
	GF16memL[0] = 1;

	// Set GF16memL and GF16memIdx
	for (i = 0; i < GF16_SIZE - 1;) {
		n = (uint32_t)GF16memL[i] << 1;
		i++;
		GF16memL[i] = (uint16_t)
			((n >= GF16_SIZE) ? n ^ GF16_PRIM : n);
		GF16memIdx[GF16memL[i]] = i;
	}
	GF16memIdx[0] = (GF16_SIZE << 1) - 1;
	GF16memIdx[1] = 0;

	// Copy first half of GF16memL to second half
	memcpy(GF16memH, GF16memL, sizeof(uint16_t) * (GF16_SIZE - 1));

	// Fill remaining space after GF16memH with zero
	memset(&GF16memL[(GF16_SIZE << 1) - 2], 0,
		sizeof(uint16_t) * ((GF16_SIZE << 1) + 2));
}

/************************************************************
	For regional calculation
************************************************************/

// Create table for regional calculation such as:
//     a * x[i]
//     a / x[i]
//     x[i] / a
// This method may fit L2 cache and will be fast.
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: a / x[i]
//           2: x[i] / a
//
// Return value:
//     pointer to table or NULL if failed. Free it later.
//
// How to use:
//    For a * x[i],
//        uint16_t *gf_a = GF16crtRegTbl(a, 0);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFmul(a, x[i]);
//            // Or use y[i] = GF16LkupRT(gf_a, x[i]);
//        }
//        free(gf_a);
//
//    For a / x[i],
//        uint16_t *gf_a = GF16crtRegTbl(a, 1);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(a, x[i]);
//            // Or use y[i] = GF16LkupRT(gf_a, x[i]);
//        }
//        free(gf_a);
//
//    For x[i] / a,
//        uint16_t *gf_a = GF16crtRegTbl(a, 2);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(x[i], a);
//            // Or use y[i] = GF16LkupRT(gf_a, x[i]);
//        }
//        free(gf_a);
//
uint16_t *
GF16crtRegTbl(uint16_t a, int type)
{
	int		i;
	uint16_t	*table = NULL, *a_addr;

	// Allocate table
	if ((table = (uint16_t *)
			aligned_alloc(64, GF16_SIZE * sizeof(uint16_t)))
				== NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;	
	}

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF16memL + GF16memIdx[a];

		// Input values
		for (i = 0; i < GF16_SIZE; i++) {
			table[i] = a_addr[GF16memIdx[i]];
		}
		break;

	case 1: // a / x[i]
		a_addr = GF16memH + GF16memIdx[a];

		// Input values
		for (i = 0; i < GF16_SIZE; i++) {
			table[i] = *(a_addr - GF16memIdx[i]);
		}
		break;

	case 2: // x[i] / a
		a_addr = GF16memH - GF16memIdx[a];

		// Input values
		for (i = 0; i < GF16_SIZE; i++) {
			table[i] = a_addr[GF16memIdx[i]];
		}
		break;

	default:
		fprintf(stderr, "Error: %s: Illegal second argument value: %d "
			"(value must be 0, 1, or 2)\n",
			__func__, type);
		free(table);
		return NULL;
	}

	return table;
}

// Create 8bit split tables for regional calculation such as:
//     a * x[i]
//     a / x[i]
//     x[i] / a
// This method may fit L1 cache and will be even faster.
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: a / x[i]
//           2: x[i] / a
//
// Return value:
//     pointer to lowest byte table or NULL if failed. Free it later.
//
// How to use:
//    For a * x[i],
//        uint16_t *gf_a_l = GF16crtSpltRegTbl(a, 0);
//        uint16_t *gf_a_h = gf_a_l + 256;
//        uint16_t xi;
//        for (i = 0; i < N; i++) {
//            xi = x[i];
//            y[i] = gf_a_h[xi >> 8] ^ gf_a_l[xi & 0xff]; // = GFmul(a, x[i]);
//            // Or use y[i] = GF16LkupSRT(gf_a_l, gf_a_h, xi);
//        }
//        free(gf_a_l);
//
//    For a / x[i],
//        uint16_t *gf_a_l = GF16crtSpltRegTbl(a, 1);
//        uint16_t *gf_a_h = gf_a_l + 256;
//        uint16_t xi;
//        for (i = 0; i < N; i++) {
//            xi = x[i];
//            y[i] = gf_a_h[xi >> 8] ^ gf_a_l[xi & 0xff]; // = GFdiv(a, x[i]);
//            // Or use y[i] = GF16LkupSRT(gf_a_l, gf_a_h, xi);
//        }
//        free(gf_a_l);
//
//    For x[i] / a,
//        uint16_t *gf_a_l = GF16crtSpltRegTbl(a, 1);
//        uint16_t *gf_a_h = gf_a_l + 256;
//        uint16_t xi;
//        for (i = 0; i < N; i++) {
//            xi = x[i];
//            y[i] = gf_a_h[xi >> 8] ^ gf_a_l[xi & 0xff]; // = GFdiv(x[i], a);
//            // Or use y[i] = GF16LkupSRT(gf_a_l, gf_a_h, xi);
//        }
//        free(gf_a_l);
//
uint16_t *
GF16crtSpltRegTbl(uint16_t a, int type)
{
	int		i;
	uint16_t	*tb_h, *tb_l, *a_addr;

	// Initialize
	tb_l = NULL;

	// Allocate table
	if ((tb_l = (uint16_t *)
			aligned_alloc(64, 256 * sizeof(uint16_t) * 2))
				== NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;
	}
	tb_h = tb_l + 256;
	memset(tb_l, 0, 256 * sizeof(uint16_t) * 2);

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF16memL + GF16memIdx[a];

		// Input values
		for (i = 0; i < 256; i++) {
			tb_l[i] = a_addr[GF16memIdx[i]];
			tb_h[i] = a_addr[GF16memIdx[i << 8]];
		}
		break;

	case 1: // a / x[i]
		a_addr = GF16memH + GF16memIdx[a];

		// Input values
		for (i = 0; i < 256; i++) {
			tb_l[i] = *(a_addr - GF16memIdx[i]);
			tb_h[i] = *(a_addr - GF16memIdx[i << 8]);
		}
		break;

	case 2: // x[i] / a
		a_addr = GF16memH - GF16memIdx[a];

		// Input values
		for (i = 0; i < 256; i++) {
			tb_l[i] = a_addr[GF16memIdx[i]];
			tb_h[i] = a_addr[GF16memIdx[i << 8]];
		}
		break;

	default:
		fprintf(stderr, "Error: %s: Illegal second argument value: %d "
			"(value must be 0, 1, or 2)\n",
			__func__, type);
		free(tb_l);
		return NULL;
	}

	return tb_l;
}

// Create 4bit split tables for regional calculation such as:
//     a * x[i]
//     a / x[i]
//     x[i] / a
// This is basically used with SIMD (SSE/NEON).
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: a / x[i]
//           2: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
uint8_t *
GF16crt4bitRegTbl(uint16_t a, int type)
{
	int		i;
	uint8_t		*tb_0_l, *tb_0_h, *tb_1_l, *tb_1_h;
	uint8_t		*tb_2_l, *tb_2_h, *tb_3_l, *tb_3_h;
	uint16_t	 *a_addr, tmp;

	// Initialize
	tb_0_l = NULL;

	// Allocate table
	if ((tb_0_l = (uint8_t *)aligned_alloc(64, 16 * 8)) == NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;
	}
	tb_0_h = tb_0_l + 16;
	tb_1_l = tb_0_h + 16;
	tb_1_h = tb_1_l + 16;
	tb_2_l = tb_1_h + 16;
	tb_2_h = tb_2_l + 16;
	tb_3_l = tb_2_h + 16;
	tb_3_h = tb_3_l + 16;

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF16memL + GF16memIdx[a];

		// Input values
		for (i = 0; i < 16; i++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tmp & 0xff;
			tb_0_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tmp & 0xff;
			tb_1_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tmp & 0xff;
			tb_2_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tmp & 0xff;
			tb_3_h[i] = tmp >> 8;
		}
		break;

	case 1: // a / x[i]
		a_addr = GF16memH + GF16memIdx[a];

		// Input values
		for (i = 0; i < 16; i++) {
			tmp = *(a_addr - GF16memIdx[i]);
			tb_0_l[i] = tmp & 0xff;
			tb_0_h[i] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 4]);
			tb_1_l[i] = tmp & 0xff;
			tb_1_h[i] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 8]);
			tb_2_l[i] = tmp & 0xff;
			tb_2_h[i] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 12]);
			tb_3_l[i] = tmp & 0xff;
			tb_3_h[i] = tmp >> 8;
		}
		break;

	case 2: // x[i] / a
		a_addr = GF16memH - GF16memIdx[a];

		// Input values
		for (i = 0; i < 16; i++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tmp & 0xff;
			tb_0_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tmp & 0xff;
			tb_1_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tmp & 0xff;
			tb_2_h[i] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tmp & 0xff;
			tb_3_h[i] = tmp >> 8;
		}
		break;

	default:
		fprintf(stderr, "Error: %s: Illegal second argument value: %d "
			"(value must be 0, 1, or 2)\n",
			__func__, type);
		free(tb_0_l);
		return NULL;
	}

	return tb_0_l;
}

// Same as GF16crt4bitRegTbl() but for 256bit SIMD like AVX
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: a / x[i]
//           2: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
uint8_t *
GF16crt4bitRegTbl256(uint16_t a, int type)
{
	int		i, i16;
	uint8_t		*tb_0_l, *tb_0_h, *tb_1_l, *tb_1_h;
	uint8_t		*tb_2_l, *tb_2_h, *tb_3_l, *tb_3_h;
	uint16_t	 *a_addr, tmp;

	// Initialize
	tb_0_l = NULL;

	// Allocate table
	if ((tb_0_l = (uint8_t *)aligned_alloc(64, 256)) == NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;
	}
	tb_0_h = tb_0_l + 32;
	tb_1_l = tb_0_h + 32;
	tb_1_h = tb_1_l + 32;
	tb_2_l = tb_1_h + 32;
	tb_2_h = tb_2_l + 32;
	tb_3_l = tb_2_h + 32;
	tb_3_h = tb_3_l + 32;

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF16memL + GF16memIdx[a];

		// Input values
		for (i = 0, i16 = 16; i < 16; i++, i16++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tb_0_l[i16] = tmp & 0xff;
			tb_0_h[i] = tb_0_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tb_1_l[i16] = tmp & 0xff;
			tb_1_h[i] = tb_1_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tb_2_l[i16] = tmp & 0xff;
			tb_2_h[i] = tb_2_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tb_3_l[i16] = tmp & 0xff;
			tb_3_h[i] = tb_3_h[i16] = tmp >> 8;
		}
		break;

	case 1: // a / x[i]
		a_addr = GF16memH + GF16memIdx[a];

		// Input values
		for (i = 0, i16 = 16; i < 16; i++, i16++) {
			tmp = *(a_addr - GF16memIdx[i]);
			tb_0_l[i] = tb_0_l[i16] = tmp & 0xff;
			tb_0_h[i] = tb_0_h[i16] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 4]);
			tb_1_l[i] = tb_1_l[i16] = tmp & 0xff;
			tb_1_h[i] = tb_1_h[i16] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 8]);
			tb_2_l[i] = tb_2_l[i16] = tmp & 0xff;
			tb_2_h[i] = tb_2_h[i16] = tmp >> 8;
			tmp = *(a_addr - GF16memIdx[i << 12]);
			tb_3_l[i] = tb_3_l[i16] = tmp & 0xff;
			tb_3_h[i] = tb_3_h[i16] = tmp >> 8;
		}
		break;

	case 2: // x[i] / a
		a_addr = GF16memH - GF16memIdx[a];

		// Input values
		for (i = 0, i16 = 16; i < 16; i++, i16++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tb_0_l[i16] = tmp & 0xff;
			tb_0_h[i] = tb_0_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tb_1_l[i16] = tmp & 0xff;
			tb_1_h[i] = tb_1_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tb_2_l[i16] = tmp & 0xff;
			tb_2_h[i] = tb_2_h[i16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tb_3_l[i16] = tmp & 0xff;
			tb_3_h[i] = tb_3_h[i16] = tmp >> 8;
		}
		break;

	default:
		fprintf(stderr, "Error: %s: Illegal second argument value: %d "
			"(value must be 0, 1, or 2)\n",
			__func__, type);
		free(tb_0_l);
		return NULL;
	}

	return tb_0_l;
}
