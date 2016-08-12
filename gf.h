#ifndef _GF_H_
#define _GF_H_

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

	This program provides simple and fast arithmetic functions
	in GF(2^16) based on memory lookups.
	The size of memory allocated for GF(2^16) is 768kB.
	GF16mul() and GF16div() are difined in gf.h for speedup. 

	Please see our technical paper (gf-nishida-16.pdf) for the details.

	CAUTION!! Never use b = 0 for GF16div(a, b) because it causes
	segmentation violation.
	For speedup, code like "(!a || !b) ? 0 : ..." has been removed.

                                        Hiroshi Nishida

****************************************************************************/

#include <stdint.h>

/************************************************************
	Definitions
************************************************************/

/************************************************************
	Variables
************************************************************/

#ifdef _GF_MAIN_
uint8_t		**GF8memMul = NULL;
uint8_t		**GF8memDiv = NULL;
uint16_t	*GF16memL = NULL, *GF16memH = NULL;
int		*GF16memIdx = NULL;

#else
extern uint8_t	**GF8memMul;
extern uint8_t	**GF8memDiv;
extern uint16_t	*GF16memL, *GF16memH;
extern int	*GF16memIdx;
#endif


/************************************************************
	Functions
************************************************************/

// Initialization
void	GF16init(void); 

// Multiplication and division
// CAUTION: DO NOT USE b = 0 for GF16div(a, b).  IT DOES NOT WORK.
#define	GF16mul(a, b)	(GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16div(a, b)	(GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])


#endif // _GF_H_
