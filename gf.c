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


/**************************************************************************
	8bit
**************************************************************************/

// Definitions for GF8
#define GF8_PRIM	487	// Prim poly (others are 285, 299, 301, 333, ...
#define GF8_SIZE	256	// = 8bit

// Initialize 8bit GF
void
GF8init(void)
{
	int		i, j, idx_i, idx_j;
	uint8_t		*GF8memL, *GF8memH;
	int		*GF8memIdx;
	uint8_t		t, *p;
	uint32_t	n;

	// Allocate memory
	GF8memL = (uint8_t *)malloc(sizeof(uint8_t) * GF8_SIZE * 4);
	GF8memH = GF8memL + GF8_SIZE - 1; // Second half
	GF8memIdx = (int *)malloc(sizeof(int) * GF8_SIZE);
	GF8memMul = (uint8_t **)malloc(sizeof(uint8_t*) * GF8_SIZE * 2);
	GF8memDiv = GF8memMul + GF8_SIZE;
	p = (uint8_t *)malloc(sizeof(uint8_t) * GF8_SIZE * GF8_SIZE * 2);
	for (i = 0; i < GF8_SIZE; i++) {
		GF8memMul[i] = p;
		p += GF8_SIZE;
		GF8memDiv[i] = p;
		p += GF8_SIZE;
	}
	GF8memL[0] = t = 1;

	// Set GF8mem and GF8memIdx
	for (i = 0; i < GF8_SIZE - 1;) {
		n = (uint32_t)t << 1;
		i++;
		GF8memL[i] = t = (uint8_t)((n >= GF8_SIZE) ? n ^ GF8_PRIM : n);
		GF8memIdx[t] = i;
	}
	GF8memIdx[0] = (GF8_SIZE << 1) - 1;
	GF8memIdx[1] = 0;

	// Copy GF8memL to GF8memH
	memcpy(GF8memH, GF8memL, sizeof(uint8_t) * (GF8_SIZE - 1));

	// Fill remaining after GF8memH with zero
	memset(&GF8memL[(GF8_SIZE << 1) - 2], 0,
		sizeof(uint8_t) * ((GF8_SIZE << 1) + 2));

	// Set GF8memMul and GF8memDiv
	for (i = 0; i < GF8_SIZE; i++) {
		idx_i = GF8memIdx[i];
		for (j = 0; j < GF8_SIZE; j++) {
			idx_j = GF8memIdx[j];
			GF8memMul[i][j] = GF8memL[idx_i + idx_j];
			GF8memDiv[i][j] = GF8memH[idx_i - idx_j];
		}
	}

	// Free temp space
	free(GF8memL);
	free(GF8memIdx);
}

/******************** For regional calculation ********************/ 

// Create table for regional calculation such as:
//     a * x[i]
//     x[i] / a
// This method may fit L2 cache and will be fast.
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
//
// Return value:
//     pointer to table or NULL if failed. Free it later.
//
// How to use:
//    For a * x[i],
//        uint8_t *gf_a = GF8crtRegTbl(a, 0);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFmul(a, x[i]);
//            // Or use y[i] = GF8LkupRT(gf_a, x[i]);
//        }
//        free(gf_a);
//
//    For x[i] / a,
//        uint8_t *gf_a = GF8crtRegTbl(a, 1);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(x[i], a);
//            // Or use y[i] = GF8LkupRT(gf_a, x[i]);
//        }
//        free(gf_a);
//
uint8_t *
GF8crtRegTbl(uint8_t a, int type)
{
	uint8_t	*table = NULL;

	// Allocate table
	if ((table = (uint8_t *)
			aligned_alloc(64, GF8_SIZE)) == NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;	
	}

	// Input values
	switch (type) {
	case 0: // a * x[i]
		memcpy(table, GF8memMul[a], GF8_SIZE);
		break;

	case 1: // x[i] / a
		memcpy(table, GF8memMul[GF8div(1, a)], GF8_SIZE);
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

// Create 4bit split tables for regional calculation such as:
//     a * x[i]
//     x[i] / a
// This is basically used with SIMD (SSE/NEON).
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
// Usage:
//     use with GF8lkupSIMD128 in gf.h.
//     See gf-bench/multiplication/gf-nishida-region-8/gf-bench*.c
//     for more details.
//
uint8_t *
GF8crt4bitRegTbl(uint8_t a, int type)
{
	int	i;
	uint8_t	*tb_l, *tb_h, *a_addr;

	// Initialize
	tb_l = NULL;

	// Allocate table
	if ((tb_l = (uint8_t *)aligned_alloc(64, 16 * 2)) == NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;
	}
	tb_h = tb_l + 16;

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF8memMul[a];

		// Input values
		for (i = 0; i < 16; i++) {
			tb_l[i] = a_addr[i];
			tb_h[i] = a_addr[i << 4];
		}
		break;

	case 1: // x[i] / a
		a_addr = GF8memMul[GF8div(1, a)];

		// Input values
		for (i = 0; i < 16; i++) {
			tb_l[i] = a_addr[i];
			tb_h[i] = a_addr[i << 4];
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

// Same as GF8crt4bitRegTbl() but for 256bit SIMD like AVX
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
// Usage:
//     use with GF8lkupSIMD128 in gf.h.
//     See gf-bench/multiplication/gf-nishida-region-8/gf-bench*.c
//     for more details.
//
uint8_t *
GF8crt4bitRegTbl256(uint8_t a, int type)
{
	int	i, i_16;
	uint8_t	*tb_l, *tb_h, *a_addr;

	// Initialize
	tb_l = NULL;

	// Allocate table
	if ((tb_l = (uint8_t *)aligned_alloc(64, 32 * 2)) == NULL) {
		fprintf(stderr, "Error: %s: aligned_alloc: %s\n",
			__func__, strerror(errno));
		return NULL;
	}
	tb_h = tb_l + 32;

	// Input values
	switch (type) {
	case 0: // a * x[i]
		a_addr = GF8memMul[a];

		// Input values
		for (i = 0, i_16 = 16; i < 16; i++, i_16++) {
			tb_l[i] = tb_l[i_16] = a_addr[i];
			tb_h[i] = tb_h[i_16] = a_addr[i << 4];
		}
		break;

	case 1: // x[i] / a
		a_addr = GF8memMul[GF8div(1, a)];

		// Input values
		for (i = 0, i_16 = 16; i < 16; i++, i_16++) {
			tb_l[i] = tb_l[i_16] = a_addr[i];
			tb_h[i] = tb_h[i_16] = a_addr[i << 4];
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

// Test GF8
void
GF8test(void)
{
	int		i, j;
	uint8_t		a, b, c, d, *tbl_0, *tbl_1;

	for (i = 0; i < 256; i++) {
		a = (uint8_t)i;
		for (j = 1; j < 256; j++) {
			b = (uint8_t)j;
			c = GF8mul(a, b);
			d = GF8div(c, b);

			if (d != i) {
				printf("GF8test: d (%d) != i (%d)\n",
					d, i);
				printf("i = %d, j = %d, c = %d, d = %d\n",
					i, j, c, d);
				exit(1);
			}
		}
	}

	a = 0;
	do {
		// Test one step lookup region multiplication
		tbl_0 = GF8crtRegTbl(a, 0);
		for (i = 0; i < 256; i++) {
			d = tbl_0[i];
			if (d != GF8mul(a, i)) {
				printf("GF8test: %d != GF8mul(%d, %d)\n",
					d, a, i);
				exit(1);
			}
		}
		free(tbl_0);

		// Test 4bit split table lookup region multiplication
		tbl_0 = GF8crt4bitRegTbl(a, 0);
		tbl_1 = tbl_0 + 16;
		for (i = 0; i < 256; i++) {
			d = tbl_1[i >> 4] ^ tbl_0[i & 0xf];
			if (d != GF8mul(a, i)) {
				printf("GF8test: %d != GF8mul(%d, %d)\n",
					d, a, i);
				exit(1);
			}
		}
		free(tbl_0);

		// Division
		if (a) {
			// Test one step lookup region division
			tbl_0 = GF8crtRegTbl(a, 1);
			for (i = 0; i < 256; i++) {
				d = tbl_0[i];
				if (d != GF8div(i, a)) {
					printf("GF8test: %d != GF8div(%d, %d)"
					       "\n", d, i, a);
					exit(1);
				}
			}
			free(tbl_0);

			// Test 4bit split table lookup region division
			tbl_0 = GF8crt4bitRegTbl(a, 1);
			tbl_1 = tbl_0 + 16;
			for (i = 0; i < 256; i++) {
				d = tbl_1[i >> 4] ^ tbl_0[i & 0xf];
				if (d != GF8div(i, a)) {
					printf("GF8test: %d != GF8div(%d, %d)"
					       "\n", d, i, a);
					exit(1);
				}
			}
			free(tbl_0);
		}

		a++;
	} while (a);

	puts("GF8test: Passed");
}

/**************************************************************************
	16bit
**************************************************************************/

// Definitions for GF16
#define	GF16_PRIM	69643	// Prim poly: 0x1100b = x^16 + x^12 + x^3 + x +1
#define	GF16_SIZE	65536	// = 16bit 
/*
	For more primitive polynomials, see:
	http://web.eecs.utk.edu/~plank/plank/papers/CS-07-593/primitive-polynomial-table.txt
*/

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
	GF16memL[0] = n = 1;

	// Set GF16memL and GF16memIdx
	for (i = 0; i < GF16_SIZE - 1;) {
#if 1
		n <<= 1;
		i++;
		if (n >= GF16_SIZE) {
			n ^= GF16_PRIM;
		}
		GF16memL[i] = (uint16_t)n;
		GF16memIdx[n] = i;
#else
		n = (uint32_t)GF16memL[i] << 1;
		i++;
		GF16memL[i] = (uint16_t)
			((n >= GF16_SIZE) ? n ^ GF16_PRIM : n);
		GF16memIdx[GF16memL[i]] = i;
#endif
	}
	GF16memIdx[0] = (GF16_SIZE << 1) - 1;
	GF16memIdx[1] = 0;

	// Copy first half of GF16memL to second half
	memcpy(GF16memH, GF16memL, sizeof(uint16_t) * (GF16_SIZE - 1));

	// Fill remaining space after GF16memH with zero
	memset(&GF16memL[(GF16_SIZE << 1) - 2], 0,
		sizeof(uint16_t) * ((GF16_SIZE << 1) + 2));
}

/******************** For regional calculation ********************/ 

// Create table for regional calculation such as:
//     a * x[i]
//     x[i] / a
// This method may fit L2 cache and will be fast.
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
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
//    For x[i] / a,
//        uint16_t *gf_a = GF16crtRegTbl(a, 1);
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

	case 1: // x[i] / a
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
//     x[i] / a
// This method may fit L1 cache and will be even faster.
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
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

	case 1: // x[i] / a
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
//     x[i] / a
// This is basically used with SIMD (SSE/NEON).
//
// Args:
//     a: static value in regional calculation (or coefficient)
//     type: 0: a * x[i]
//           1: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
// Usage:
//     use with GF16lkupSIMD128x2 in gf.h.
//     See gf-bench/multiplication/gf-nishida-region-16/gf-bench*.c
//     for more details.
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

	case 1: // x[i] / a
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
//           1: x[i] / a
//
// Return value:
//     pointer to lowest table or NULL if failed. Free it later.
//
// Usage:
//     use with GF16lkupSIMD256x2 in gf.h.
//     See gf-bench/multiplication/gf-nishida-region-16/gf-bench*.c
//     for more details.
//
uint8_t *
GF16crt4bitRegTbl256(uint16_t a, int type)
{
	int		i, i_16;
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
		for (i = 0, i_16 = 16; i < 16; i++, i_16++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tb_0_l[i_16] = tmp & 0xff;
			tb_0_h[i] = tb_0_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tb_1_l[i_16] = tmp & 0xff;
			tb_1_h[i] = tb_1_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tb_2_l[i_16] = tmp & 0xff;
			tb_2_h[i] = tb_2_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tb_3_l[i_16] = tmp & 0xff;
			tb_3_h[i] = tb_3_h[i_16] = tmp >> 8;
		}
		break;

	case 1: // x[i] / a
		a_addr = GF16memH - GF16memIdx[a];

		// Input values
		for (i = 0, i_16 = 16; i < 16; i++, i_16++) {
			tmp = a_addr[GF16memIdx[i]];
			tb_0_l[i] = tb_0_l[i_16] = tmp & 0xff;
			tb_0_h[i] = tb_0_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 4]];
			tb_1_l[i] = tb_1_l[i_16] = tmp & 0xff;
			tb_1_h[i] = tb_1_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 8]];
			tb_2_l[i] = tb_2_l[i_16] = tmp & 0xff;
			tb_2_h[i] = tb_2_h[i_16] = tmp >> 8;
			tmp = a_addr[GF16memIdx[i << 12]];
			tb_3_l[i] = tb_3_l[i_16] = tmp & 0xff;
			tb_3_h[i] = tb_3_h[i_16] = tmp >> 8;
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
