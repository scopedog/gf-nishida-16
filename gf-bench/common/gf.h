#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>

/****************************************************************************

	Simple and fast multiplication and division functions in GF(2^16)
	based on table lookup(s).
	Memory consumption for the tables is as follows:
		GF16mul(), GF16div(): 768kB
		GF16crtRegTbl: 128kB (may fit L2 cache)
		GF16crtSpltRegTbl: 1kB (may fit L1 cache)
		GF16crt4bitRegTbl: 128B (for SIMD)

	CAUTION!! Never use b = 0 for disvision (e.g. GF16div(a, b))
	as it will output a wrong value.
	For speedup, we don't check if a, b == 0.

					Hiroshi Nishida
        
****************************************************************************/

/************************************************************
	Definitions
************************************************************/

// To achieve fast computation, we do not check if a, b == 0
// CAUTION: DO NOT USE b = 0 for GF16div(a, b). IT DOES NOT WORK CORRECTLY.
#define	GF16mul(a, b)	(GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16div(a, b)	(GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])

// Some macros for simplification 
#define GF16crtRegTblMul(a)	GF16crtRegTbl(a, 0)
#define GF16crtRegTblDivL(a)	GF16crtRegTbl(a, 1)
#define GF16crtRegTblDivR(a)	GF16crtRegTbl(a, 2)
#define GF16crtSpltRegTblMul(a)		GF16crtSpltRegTbl(a, 0)
#define GF16crtSpltRegTblDivL(a)	GF16crtSpltRegTbl(a, 1)
#define GF16crtSpltRegTblDivR(a)	GF16crtSpltRegTbl(a, 2)

#define GF16LkupRT(gf_a, x)		gf_a[(x)]
#define GF16LkupSRT(gf_a_l, gf_a_h, x)	gf_a_h[(x) >> 8] ^ gf_a_l[(x) & 0xff]

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

// 16bit
void		GF16init(void); 
uint16_t	*GF16crtRegTbl(uint16_t, int);
uint16_t	*GF16crtSpltRegTbl(uint16_t, int);
uint8_t		*GF16crt4bitRegTbl(uint16_t, int);

#endif // _GF_H_
