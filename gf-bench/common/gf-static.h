#ifndef _GF_STATIC_H_
#define _GF_STATIC_H_

#include <stdint.h>

/***************************************************************************

    This program provides simple (and fast?) calculation functions in
    GF(2^8) and GF(2^16) that use memory lookup.
    GF(2^8) uses 64kB and GF(2^16) uses 256kB memory space.

    CAUTION!! Never use b = 0 for GF16div(a, b).
    GFdiv16(a, 0) outputs a wrong value.
    This is because I have eliminated "(!a || !b) ? 0 : ...." code
    in order to achieve faster computation.

                                        Hiroshi Nishida

***************************************************************************/

/************************************************************
	Definitions
************************************************************/

#define	GF16_SIZE	65536	// = 16bit
#define GF16_PRIM	0x18bb7 // Primitive polynomial used for this

/************************************************************
	Variables
************************************************************/

#ifdef _GF_MAIN_
uint16_t	GF16memL[GF16_SIZE * 4] =

int	GF16memIdx[GF16_SIZE] =

uint16_t	*GF16memH = GF16memL + GF16_SIZE - 1;
#else
extern uint16_t	GF16memL[GF16_SIZE * 4], *GF16memH;
extern int	GF16memIdx[GF16_SIZE];
#endif


/************************************************************
	Functions
************************************************************/

// 16bit
#if 1   // To achieve fast computation, we do not check if a, b == 0
        // CAUTION: DO NOT USE b = 0 for GF16div(a, b). IT DOES NOT WORK CORRECTLY
#define GF16mul(a, b)	(GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define GF16div(a, b)	(GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])
#else   // If you'd like to get correct GF values, use below
#define GF16mul(a, b)	((!a || !b) ? 0 : GF16memL[GF16memIdx[(a)] + GF16memIdx[(b)]])
#define GF16div(a, b)	((!a || !b) ? 0 : GF16memH[GF16memIdx[(a)] - GF16memIdx[(b)]])
#endif

// 16-64bit
void	GF16_64mul(uint16_t , uint64_t *, uint64_t *);
void	GF16_64div(uint64_t *, uint16_t, uint64_t *);

// Others
void	GF16test(void);
void	GF16_64test(void);

#endif // _GF_STATIC_H_