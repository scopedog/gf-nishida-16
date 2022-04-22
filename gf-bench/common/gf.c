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

	For speedup, I have removed "(!a || !b) ? 0 : ..." code from GF16div().
	Also GF16mul(a, 0) and GF16mul(0, b) do not output 0 for the same
	reason.
	However, c = GF16mul(a, b) -> a = GF16div(c, b) still holds.

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
// a: static value in regional calculation (or coefficient)
// type: 0: a * x[i]
//       1: a / x[i]
//       2: x[i] / a
// Return value: pointer to table or NULL if failed. Free it later.
// How to use:
//    For a * x[i],
//        gf_a = GF16crtRegionTbl(a, 0);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFmul(a, x[i]);
//        }
//
//    For a / x[i],
//        gf_a = GF16crtRegionTbl(a, 1);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(a, x[i]);
//        }
//
//    For x[i] / a,
//        gf_a = GF16crtRegionTbl(a, 2);
//        for (i = 0; i < N; i++) {
//            y[i] = gf_a[x[i]]; // This is equal to GFdiv(x[i], a);
//        }
//
uint16_t *
GF16crtRegionTbl(uint16_t a, int type)
{
	int		i;
	uint16_t	*table = NULL, *a_addr;

	// Allocate table
	if ((table = (uint16_t *)malloc(GF16_SIZE * sizeof(uint16_t)))
			== NULL) {
		fprintf(stderr, "Error: %s: %s\n", __func__, strerror(errno));
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
