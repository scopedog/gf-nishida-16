/****************************************************************************

Copyright (c) 2015, Hiroshi Nishida and ASUSA Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

/****************************************************************************

	This library provides simple and fast arithmetic functions
	in GF(2^16) that base on memory lookups.
	The size of memory allocated for the lookup tables is 768kB.
	GF16mul() and GF16div() are defined in gf.h. 

	Please see our technical paper gf-nishida-16.pdf or
	gf-nishida-16-ja.pdf for the details.

	CAUTION!! Never use b = 0 for GF16div(a, b) because it causes
	segmentation violation.
	For speedup, the program does not check "(a == 0 || b == 0)".

					Hiroshi Nishida
					nishida at asusa.net

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
	uint16_t	t;
	uint32_t	n;
	
	// Allocate memory 
	GF16memL = (uint16_t *)malloc(sizeof(uint16_t) * GF16_SIZE * 4);
	GF16memH = GF16memL + GF16_SIZE - 1; // Second half
	GF16memIdx = (int *)malloc(sizeof(int) * GF16_SIZE);
	GF16memL[0] = t = 1;

	// Set GF16memL and GF16memIdx
	for (i = 1; i < GF16_SIZE - 1; i++) {
		n = (uint32_t)t << 1;
		GF16memL[i] = t = (uint16_t)
			((n >= GF16_SIZE) ? n ^ GF16_PRIM : n);
		GF16memIdx[t] = i;
	}
	GF16memIdx[0] = (GF16_SIZE << 1) - 1;
	GF16memIdx[1] = 0;

	// Copy first quarter of GF16memL to quarter half
	memcpy(GF16memH, GF16memL, sizeof(uint16_t) * (GF16_SIZE - 1));

	// Fill remaining space after GF16memH with zero
	memset(&GF16memL[(GF16_SIZE << 1) - 2], 0,
		sizeof(uint16_t) * (GF16_SIZE + 1));
	GF16memL[(GF16_SIZE << 2) - 2] = 0;
}

