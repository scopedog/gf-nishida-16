/*
 *	All rights revserved by ASUSA Corporation and ASJ Inc.
 *	Copying any part of this program is strictly prohibited.
 *	Author: Hiroshi Nishida
 */

/****************************************************************************

	This program provides simple (and fast?) calculation functions
	in GF(2^16) that use memory lookup.
	The size of memory allocated for GF(2^16) is 512kB.
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
//        }
//        free(gf_a);
//
//    For a / x[i],
//        uint16_t *gf_a = GF16crtRegTbl(a, 1);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(a, x[i]);
//        }
//        free(gf_a);
//
//    For x[i] / a,
//        uint16_t *gf_a = GF16crtRegTbl(a, 2);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(x[i], a);
//        }
//        free(gf_a);
//
uint16_t *
GF16crtRegTbl(uint16_t a, int type)
{
	int		i;
	uint16_t	*table = NULL, *a_addr;

	// Allocate table
	if ((table = (uint16_t *)malloc(GF16_SIZE * sizeof(uint16_t)))
			== NULL) {
		fprintf(stderr, "Error: %s: malloc: %s\n",
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
//            // Or use y[i] = GF16SRT(gf_a_l, gf_a_h, xi);
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
//            // Or use y[i] = GF16SRT(gf_a_l, gf_a_h, xi);
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
//            // Or use y[i] = GF16SRT(gf_a_l, gf_a_h, xi);
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
	if ((tb_l = (uint16_t *)malloc(256 * sizeof(uint16_t) * 2)) == NULL) {
		fprintf(stderr, "Error: %s: malloc: %s\n",
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
