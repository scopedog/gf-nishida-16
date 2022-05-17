#ifndef _GF_SMID_H_
#define _GF_SMID_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*********************************************************************
	Macros/inlines
*********************************************************************/

#if defined(_amd64_) || defined(_x86_64_)
/*********************************************************************
	SSE (SSSE3)
*********************************************************************/
#include <immintrin.h>

// Shift left n bits
inline __m128i
ShiftL128(__m128i v, int n)
{
	__m128i v1, v2;

	if ((n) >= 64) {
		v1 = _mm_slli_si128(v, 8);
		v1 = _mm_slli_epi64(v1, (n) - 64);
	}
	else {
		v1 = _mm_slli_epi64(v, n);
		v2 = _mm_slli_si128(v, 8);
		v2 = _mm_srli_epi64(v2, 64 - (n));
		v1 = _mm_or_si128(v1, v2);
	}

	return v1;
}

// Shift right n bits
inline __m128i
ShiftR128(__m128i v, int n)
{
	__m128i v1, v2;

	if ((n) >= 64) {
		v1 = _mm_srli_si128(v, 8);
		v1 = _mm_srli_epi64(v1, (n) - 64);
	}
	else {
		v1 = _mm_srli_epi64(v, n);
		v2 = _mm_srli_si128(v, 8);
		v2 = _mm_slli_epi64(v2, 64 - (n));
		v1 = _mm_or_si128(v1, v2);
	}

	return v1;
}

// Shift right 4bits
inline __m128i
ShiftR4_128(__m128i v)
{
	__m128i v1, v2;

	v1 = _mm_srli_epi64(v, 4);
	v2 = _mm_srli_si128(v, 8);
	v2 = _mm_slli_epi64(v2, 60);
	v1 = _mm_or_si128(v1, v2);

	return v1;
}

// Show each byte of __m128i
inline void
mm_print128_8(const char *str, __m128i var)
{
	uint8_t val[16];

	memcpy(val, &var, sizeof(val));

	printf("%s%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	       "%02x %02x %02x %02x %02x\n",  str,
#if 1   // Left to right
		val[15], val[14], val[13], val[12], val[11], val[10], val[9],
		val[8], val[7], val[6], val[5], val[4], val[3], val[2],
		val[1], val[0]);
#else   // Right to left
		val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
		val[8], val[9], val[10], val[11], val[12], val[13],
		val[14], val[15]);
#endif
}

#elif defined(_arm64_)
/*********************************************************************
	NEON
*********************************************************************/
#include <arm_neon.h>
#include "SSE2NEON.h"

// Shift right 4bits
#define ShiftR4_128(v)	vshrq_n_u8((v), 4)

// Show each byte of uint8x16_t
inline void
mm_print128_8(const char *str, uint8x16_t var)
{
	uint8_t val[16];

	memcpy(val, &var, sizeof(val));

	printf("%s%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	       "%02x %02x %02x %02x %02x\n",  str,
#if 1   // Left to right
		val[15], val[14], val[13], val[12], val[11], val[10], val[9],
		val[8], val[7], val[6], val[5], val[4], val[3], val[2],
		val[1], val[0]);
#else   // Right to left
		val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
		val[8], val[9], val[10], val[11], val[12], val[13],
		val[14], val[15]);
#endif
}
#endif
#endif // _GF_SMID_H_
