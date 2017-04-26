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
	Pseuedo 64bit functions
************************************************************/

// Pseuedo 64bit multiplication using 16bit -- result is output to 'out'
void
GF16_64mul(uint16_t a, uint64_t *b, uint64_t *out)
{
        uint16_t	*_gf16mem_ = GF16memL + GF16memIdx[a];
        uint16_t	*b16 = (uint16_t*)b, *out16 = (uint16_t *)out;

	/* Since GF16mul(a, b) is equal to
	   GF16memL[GF16memIdx[a] + GF16memIdx[b]], we first get
	   &GF16memL[GF16memIdx[a]], i.e,. _gf16mem_.
	   Then we obtain GF16mul(a, b) by _gf16mem_[GF16memIdx[b]]. */

	// Regard b as 16bit * 4 and get output
	*out16 = _gf16mem_[GF16memIdx[*b16]]; // Equal to GF16mul(a, *b16)
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
}

// Pseuedo 64bit division (*b / a) using 16bit -- result is output to 'out'
void
GF16_64div(uint64_t *b, uint16_t a, uint64_t *out)
{
        uint16_t	*_gf16mem_ = GF16memH - GF16memIdx[a];
        uint16_t	*b16 = (uint16_t*)b, *out16 = (uint16_t *)out;

	/* Since GF16mul(b, a) is equal to
	   GF16memH[GF16memIdx[b] - GF16memIdx[a]], we first get
	   &GF16memH[-GF16memIdx[a]], i.e,. _gf16mem_.
	   Then we obtain GF16mul(b, a) by _gf16mem_[GF16memIdx[b]]. */

	// Regard b as 16bit * 4 and get output
	*out16 = _gf16mem_[GF16memIdx[*b16]]; // Equal to GF16mul(a, *b16)
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
	out16++;	b16++;
	*out16 = _gf16mem_[GF16memIdx[*b16]];
}

