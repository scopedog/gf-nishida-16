#ifndef _GF_H_
#define _GF_H_

#include <stdint.h>
#if defined(__SSSE3__) || defined(__AVX2__)
#include <immintrin.h>
#elif defined(_arm64_)
//#elif defined(__ARM_NEON__) // Clang for arm64 does not support -march=native
#include <arm_neon.h>
#endif

/****************************************************************************

	Simple and fast multiplication and division functions in GF(2^16)
	based on table lookup(s).
	Memory consumption for the tables is as follows:
		GF16mul(), GF16div(): 768kB
		GF16crtRegTbl: 128kB (may fit L2 cache)
		GF16crtSpltRegTbl: 1kB (may fit L1 cache)
		GF16crt4bitRegTbl: 128B (for 128bit SIMD (SSE))
		GF16crt4bitRegTbl256: 256B (for 256bit SIMD (AVX))

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
#define GF16LkupSRT(gf_a_l, gf_a_h, x)	(gf_a_h[(x) >> 8] ^ gf_a_l[(x) & 0xff])

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
uint8_t		*GF16crt4bitRegTbl256(uint16_t, int);

/************************************************************
	Inline functions
************************************************************/
#if defined(__SSSE3__) || defined(__AVX2__)

// Definitions 
typedef __m128i	v128_t;

// Get GF(2^16) result by lookup by SSE -- call every 32bytes 
static inline void
GF16lkupSIMD128(const __m128i tb_a_0_l, const __m128i tb_a_0_h,
		const __m128i tb_a_1_l, const __m128i tb_a_1_h,
		const __m128i tb_a_2_l, const __m128i tb_a_2_h,
		const __m128i tb_a_3_l, const __m128i tb_a_3_h,
		const uint8_t *input, uint8_t *output)
{
	/*** 4bit table lookup region technique with SSSE3 ***/
	__m128i	v_0, v_1, input_0, input_1, input_l, input_h;
	__m128i	input_l_l, input_l_h, input_h_l, input_h_h;
	__m128i	output_l, output_h, tmp;

	// Load inputs
	input_0 = _mm_loadu_si128((__m128i *)input);
	input_1 = _mm_loadu_si128((__m128i *)(input + 16));

#if 0
	// Pack low and high bytes of inputs to input_l and input_h
	v_0 = _mm_shuffle_epi8(input_0,
		_mm_set_epi32(0x0f0d0b09, 0x07050301, 0x0e0c0a08, 0x06040200));
	v_1 = _mm_shuffle_epi8(input_1,
		_mm_set_epi32(0x0f0d0b09, 0x07050301, 0x0e0c0a08, 0x06040200));
	input_l = _mm_unpacklo_epi64(v_0, v_1);
	input_h = _mm_unpackhi_epi64(v_0, v_1);
#else
	// Pack low bytes of inputs to input_l
	tmp = _mm_set1_epi16(0x00ff);
	v_0 = _mm_and_si128(input_0, tmp);
	v_1 = _mm_and_si128(input_1, tmp);
	input_l = _mm_packus_epi16(v_0, v_1);

	// Pack high bytes of inputs to input_h
	v_0 = _mm_srli_epi16(input_0, 8);
	v_1 = _mm_srli_epi16(input_1, 8);
	input_h = _mm_packus_epi16(v_0, v_1);
#endif

	// Retrieve low 4bit of each byte from input_l
	tmp = _mm_set1_epi8(0x0f);
	input_l_l = _mm_and_si128(input_l, tmp);

	// Retrieve high 4bit of each byte from input_l
	//v_0 = ShiftR4_128(input_l);
	v_0 = _mm_srli_epi16(input_l, 4);
	input_l_h = _mm_and_si128(v_0, tmp);

	// Retrieve low 4bit of each byte from input_h
	input_h_l = _mm_and_si128(input_h, tmp);

	// Retrieve high 4bit of each byte from input_h
	//v_0 = ShiftR4_128(input_h);
	v_0 = _mm_srli_epi16(input_h, 4);
	input_h_h = _mm_and_si128(v_0, tmp);

	// Get GF calc results for low bytes
	v_0 = _mm_shuffle_epi8(tb_a_0_l, input_l_l);
	v_0 = _mm_xor_si128(v_0, _mm_shuffle_epi8(tb_a_1_l, input_l_h));
	v_0 = _mm_xor_si128(v_0, _mm_shuffle_epi8(tb_a_2_l, input_h_l));
	v_0 = _mm_xor_si128(v_0, _mm_shuffle_epi8(tb_a_3_l, input_h_h));

	// Get GF calc results for high bytes
	v_1 = _mm_shuffle_epi8(tb_a_0_h, input_l_l);
	v_1 = _mm_xor_si128(v_1, _mm_shuffle_epi8(tb_a_1_h, input_l_h));
	v_1 = _mm_xor_si128(v_1, _mm_shuffle_epi8(tb_a_2_h, input_h_l));
	v_1 = _mm_xor_si128(v_1, _mm_shuffle_epi8(tb_a_3_h, input_h_h));

	// Unpack low bytes
	output_l = _mm_unpacklo_epi8(v_0, v_1);

	// Unpack high bytes
	output_h = _mm_unpackhi_epi8(v_0, v_1);

	// Save results
	_mm_storeu_si128((__m128i *)output, output_l);
	_mm_storeu_si128((__m128i *)(output + 16), output_h);
}

// Get GF(2^16) result by lookup by AVX -- call every 64bytes 
static inline void
GF16lkupSIMD256(const __m256i tb_a_0_l, const __m256i tb_a_0_h,
		const __m256i tb_a_1_l, const __m256i tb_a_1_h,
		const __m256i tb_a_2_l, const __m256i tb_a_2_h,
		const __m256i tb_a_3_l, const __m256i tb_a_3_h,
		const uint8_t *input, uint8_t *output)
{
	/*** 4bit multi table region technique by AVX ***/
	__m256i	v_0, v_1, input_0, input_1, input_l, input_h;
	__m256i	input_l_l, input_l_h, input_h_l, input_h_h;
	__m256i	output_l, output_h, tmp;

	input_0 = _mm256_loadu_si256((__m256i *)input);
	input_1 = _mm256_loadu_si256((__m256i *)(input + 32));

	// Pack low bytes of inputs to input_l
	tmp = _mm256_set1_epi16(0x00ff);
	v_0 = _mm256_and_si256(input_0, tmp);
	v_1 = _mm256_and_si256(input_1, tmp);
	input_l = _mm256_packus_epi16(v_0, v_1);

	// Pack high bytes of inputs to input_h
	v_0 = _mm256_srli_epi16(input_0, 8);
	v_1 = _mm256_srli_epi16(input_1, 8);
	input_h = _mm256_packus_epi16(v_0, v_1);

	// Retrieve low 4bit of each byte from input_l
	tmp = _mm256_set1_epi8(0x0f);
	input_l_l = _mm256_and_si256(input_l, tmp);

	// Retrieve high 4bit of each byte from input_l
	v_0 = _mm256_srli_epi16(input_l, 4);
	input_l_h = _mm256_and_si256(v_0, tmp);

	// Retrieve low 4bit of each byte from input_h
	input_h_l = _mm256_and_si256(input_h, tmp);

	// Retrieve high 4bit of each byte from input_h
	v_0 = _mm256_srli_epi16(input_h, 4);
	input_h_h = _mm256_and_si256(v_0, tmp);

	// Get GF calc results for low bytes
	v_0 = _mm256_shuffle_epi8(tb_a_0_l, input_l_l);
	v_0 = _mm256_xor_si256(v_0, _mm256_shuffle_epi8(tb_a_1_l, input_l_h));
	v_0 = _mm256_xor_si256(v_0, _mm256_shuffle_epi8(tb_a_2_l, input_h_l));
	v_0 = _mm256_xor_si256(v_0, _mm256_shuffle_epi8(tb_a_3_l, input_h_h));

	// Get GF calc results for high bytes
	v_1 = _mm256_shuffle_epi8(tb_a_0_h, input_l_l);
	v_1 = _mm256_xor_si256(v_1, _mm256_shuffle_epi8(tb_a_1_h, input_l_h));
	v_1 = _mm256_xor_si256(v_1, _mm256_shuffle_epi8(tb_a_2_h, input_h_l));
	v_1 = _mm256_xor_si256(v_1, _mm256_shuffle_epi8(tb_a_3_h, input_h_h));

	// Unpack low bytes
	output_l = _mm256_unpacklo_epi8(v_0, v_1);

	// Unpack high bytes
	output_h = _mm256_unpackhi_epi8(v_0, v_1);

	// Save results
	_mm256_storeu_si256((__m256i *)output, output_l);
	_mm256_storeu_si256((__m256i *)(output + 32), output_h);
}

// Show each byte of v256_t
static inline void
mm_print256_8(const char *str, __m256i var)
{
	uint8_t val[32];
	size_t	len = strlen(str);

	memcpy(val, &var, sizeof(val));

	printf("%s%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	       "%02x %02x %02x %02x %02x\n",  str,
		val[15], val[14], val[13], val[12], val[11], val[10], val[9],
		val[8], val[7], val[6], val[5], val[4], val[3], val[2],
		val[1], val[0]);
	for (; len; len--) {
		putchar(' ');
	}
	printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x "
	       "%02x %02x %02x %02x %02x\n",
		val[31], val[30], val[29], val[28], val[27], val[26], val[25],
		val[24], val[23], val[22], val[21], val[20], val[19], val[18],
		val[17], val[16]);
}

#elif defined(_arm64_) // NEON

// Definitions 
typedef uint8x16_t	v128_t;

// Get GF(2^16) result by lookup with NEON -- call every 32bytes 
static inline void
GF16lkupSIMD128(const uint8x16_t tb_a_0_l, const uint8x16_t tb_a_0_h,
		const uint8x16_t tb_a_1_l, const uint8x16_t tb_a_1_h,
		const uint8x16_t tb_a_2_l, const uint8x16_t tb_a_2_h,
		const uint8x16_t tb_a_3_l, const uint8x16_t tb_a_3_h,
		const uint8_t *input, uint8_t *output)
{
	/*** 4bit table lookup region technique with NEON ***/
	uint8x16x2_t	input_v, output_v;
	uint8x16_t	input_l, input_h, v_0, v_1;
	uint8x16_t	input_l_l, input_l_h, input_h_l, input_h_h, tmp;

	// Load interleaved inputs
	input_v = vld2q_u8(input);
	input_l = input_v.val[0];
	input_h = input_v.val[1];

	// Retrieve low 4bit of each byte from input_l
	tmp = vdupq_n_u8(0x0f);
	input_l_l = vandq_u8(input_l, tmp);

	// Retrieve high 4bit of each byte from input_l
	input_l_h = vshrq_n_u8(input_l, 4);

	// Retrieve low 4bit of each byte from input_h
	input_h_l = vandq_u8(input_h, tmp);

	// Retrieve high 4bit of each byte from input_h
	input_h_h = vshrq_n_u8(input_h, 4);

	// Get GF calc results for low bytes
	v_0 = vqtbl1q_u8(tb_a_0_l, input_l_l);
	v_0 = veorq_s64(v_0, vqtbl1q_u8(tb_a_1_l, input_l_h));
	v_0 = veorq_s64(v_0, vqtbl1q_u8(tb_a_2_l, input_h_l));
	v_0 = veorq_s64(v_0, vqtbl1q_u8(tb_a_3_l, input_h_h));

	// Get GF calc results for high bytes
	v_1 = vqtbl1q_u8(tb_a_0_h, input_l_l);
	v_1 = veorq_s64(v_1, vqtbl1q_u8(tb_a_1_h, input_l_h));
	v_1 = veorq_s64(v_1, vqtbl1q_u8(tb_a_2_h, input_h_l));
	v_1 = veorq_s64(v_1, vqtbl1q_u8(tb_a_3_h, input_h_h));

	// Save interleaved results
	output_v.val[0] = v_0;
	output_v.val[1] = v_1;
	vst2q_u8(output, output_v);
}
#endif

#if defined(__SSSE3__) || defined(__AVX2__) || defined(_arm64_)
// Show each byte of v128_t
static inline void
mm_print128_8(const char *str, v128_t var)
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


#endif // _GF_H_
