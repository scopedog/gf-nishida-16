#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>

/****************************************************************************

	Simple (and fast?) calculation functions in GF(2^8) and GF(2^16)
	that use memory lookup.
	GF(2^8) uses 64kB and GF(2^16) uses 256kB.

	CAUTION!! Never use b = 0 for GF16div(a, b).
	GFdiv16(a, 0) outputs a wrong value.
	This is because I have eliminated "(!a || !b) ? 0 : ...." code
	in order to achieve faster computation.

					Hiroshi Nishida
        
****************************************************************************/

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


// 16bit
void	GF16init(void); 

#if 1	// To achieve fast computation, we do not check if a, b == 0
	// CAUTION: DO NOT USE b = 0 for GF16div(a, b).
	// IT DOES NOT WORK CORRECTLY.
#define	GF16mul(a, b)	(GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16div(a, b)	(GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])
#else	// If you'd like to get correct GF values, use below
#define	GF16mul(a, b)	((!a || !b) ? 0 : GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define	GF16div(a, b)	((!a || !b) ? 0 : GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])
#endif

// 16-64bit
void	GF16_64mul(uint16_t, uint64_t *, uint64_t *);
void	GF16_64div(uint64_t *, uint16_t, uint64_t *);


#endif // _GF_H_
