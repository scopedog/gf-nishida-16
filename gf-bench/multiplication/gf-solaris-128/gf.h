#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>
#include <wmmintrin.h>


/************************************************************
	Definitions
************************************************************/

/************************************************************
	Variables
************************************************************/

/************************************************************
	Functions
************************************************************/

void	gfmul(__m128i, __m128i, uint64_t *);

#endif // _GF_H_
