#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"
#include "gf.h"
#include "simd.h"
#include "mt64.h"

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	uint16_t	a, *b, *c, *d, *gf_a, *gf_a_l, *gf_a_h;
	uint64_t	*r;

	// Initialize GF
	GF16init();

	// Allocate b and c
	if ((b = (uint16_t *)malloc(SPACE * 3)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + (SPACE / sizeof(uint16_t));
	d = c + (SPACE / sizeof(uint16_t));

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = (uint16_t)(genrand64_int64() & 0xffff);
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	/*** Regular region technique (faster than GF16mul) ***/
	// Set gf_a
	gf_a = GF16memL + GF16memIdx[a];

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// c = a * b = gf_a[GF16memIdx(b)]]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// Two step look up
			// To avoid elimination by cc's -O2 option,
			// input result into c[j]
			c[j] = gf_a[GF16memIdx[b[j]]];
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("gf_a[GF16memIdx(b)]]             : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	/*** Special region technique not written in paper
	     This will run in L2 cache as gf_a is 128kB ***/
	// Create region table for a
	if ((gf_a = GF16crtRegTbl(a, 0)) == NULL) {
		exit(1);
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// d = a * b = gf_a[b]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// One step look up
			d[j] = gf_a[b[j]];
			// Or use:
			//d[j] = GF16LkupRT(gf_a, b[j]);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("gf_a[b]                          : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crtRegTbl()
	free(gf_a);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error: E-mail me (nishida at asusa.net) "
			"if this happened.\n");
		exit(1);
	}

	/*** Extreme region technique not written in paper
	     This will run in L1 cache as gf_a_l and gf_a_h are 512B each ***/
	// Create 2 * 512 byte region tables for a
	if ((gf_a_l = GF16crtSpltRegTbl(a, 0)) == NULL) {
		exit(1);
	}
	gf_a_h = gf_a_l + 256; // Don't forget this

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// d = a * b = gf_a_h[b >> 8] ^ gf_a_l[b & 0xff]
	uint16_t bj;
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			bj = b[j];
			d[j] = gf_a_h[bj >> 8] ^ gf_a_l[bj & 0xff];
			// Or use:
			//d[j] = GF16LkupSRT(gf_a_l, gf_a_h, bj);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("gf_a_h[b >> 8] ^ gf_a_l[b & 0xff]: %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16GF16crtSpltRegTbl()
	// No need to free gf_a_h because gf_a_h = gf_a_l + 256
	free(gf_a_l);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error: E-mail me (nishida at asusa.net) "
			"if this happened.\n");
		exit(1);
	}

	// From now on, we need this
	uint8_t	*gf_tb;

#if 0
	/*** Non-SIMD region technique similar to SIMD one below ***/
	uint8_t	*gf_a_0_l, *gf_a_0_h, *gf_a_1_l, *gf_a_1_h;
	uint8_t	*gf_a_2_l, *gf_a_2_h, *gf_a_3_l, *gf_a_3_h;
	uint16_t tmp, low, high;

	// Reset d
	memset(d, 0, SPACE); 

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 0)) == NULL) {
		exit(1);
	}
	
	gf_a_0_l = gf_tb;
	gf_a_0_h = gf_tb + 16;
	gf_a_1_l = gf_tb + 32;
	gf_a_1_h = gf_tb + 48;
	gf_a_2_l = gf_tb + 64;
	gf_a_2_h = gf_tb + 80;
	gf_a_3_l = gf_tb + 96;
	gf_a_3_h = gf_tb + 112;

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			bj = b[j];
/*			// Basic idea
			d[j] = (gf_a_0_l[bj & 0x000f]) ^
			       (gf_a_0_h[bj & 0x00f] << 8) ^
			       (gf_a_1_l[(bj >> 4) & 0x00f]) ^
			       (gf_a_1_h[(bj >> 4) & 0x00f] << 8) ^
			       (gf_a_2_l[(bj >> 8) & 0x00f]) ^
			       (gf_a_2_h[(bj >> 8) & 0x00f] << 8) ^
			       (gf_a_3_l[(bj >> 12) & 0x00f]) ^
			       (gf_a_3_h[(bj >> 12) & 0x00f] << 8);
*/
			tmp = bj & 0x000f;
			low = gf_a_0_l[tmp];
			high = gf_a_0_h[tmp];
			tmp = (bj & 0x00f0) >> 4;
			low ^= gf_a_1_l[tmp];
			high ^= gf_a_1_h[tmp];
			tmp = (bj & 0x0f00) >> 8;
			low ^= gf_a_2_l[tmp];
			high ^= gf_a_2_h[tmp];
			tmp = (bj & 0xf000) >> 12;
			low ^= gf_a_3_l[tmp];
			high ^= gf_a_3_h[tmp];
			d[j] = low | (high << 8);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("4 x 16byte lookup tables         : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);
#endif

#if 0
	// For debugging
#undef REPEAT
#undef SPACE
#define REPEAT	1
#define SPACE	32
	a = 0x3f29;
	b[0] = 0x0001;
	b[1] = 0x0203;
	b[2] = 0x0405;
	b[3] = 0x0607;
	b[4] = 0x0809;
	b[5] = 0x0a0b;
	b[6] = 0x0c0d;
	b[7] = 0x0e0f;
	b[8] = 0xfde1;
	b[9] = 0xd2c3;
	b[10] = 0xb4a5;
	b[11] = 0x9687;
	b[12] = 0x7869;
	b[13] = 0x5a4b;
	b[14] = 0x3c2d;
	b[15] = 0x1e0f;
#endif


#if defined(_amd64_) || defined(_x86_64_)
	/*** 4bit multi table region technique with SSE3 ***/
	uint8_t	*_b, *_d;
	__m128i	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	__m128i	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;
	__m128i	v_0, v_1, input_0, input_1, input_l, input_h;
	__m128i	input_l_l, input_l_h, input_h_l, input_h_h;
	__m128i	out_l, out_h, tmp;

	// Reset d
	memset(d, 0, SPACE); 

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 0)) == NULL) {
		exit(1);
	}

	// Assign other addresses
	tb_a_0_l = _mm_loadu_si128((__m128i *)(gf_tb + 0));
	tb_a_0_h = _mm_loadu_si128((__m128i *)(gf_tb + 16));
	tb_a_1_l = _mm_loadu_si128((__m128i *)(gf_tb + 32));
	tb_a_1_h = _mm_loadu_si128((__m128i *)(gf_tb + 48));
	tb_a_2_l = _mm_loadu_si128((__m128i *)(gf_tb + 64));
	tb_a_2_h = _mm_loadu_si128((__m128i *)(gf_tb + 80));
	tb_a_3_l = _mm_loadu_si128((__m128i *)(gf_tb + 96));
	tb_a_3_h = _mm_loadu_si128((__m128i *)(gf_tb + 112));

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = (uint8_t *)b;
		_d = (uint8_t *)d;
		for (j = 0; j < SPACE; j += 32) { // Do every 128 * 2bit
			// Load inputs
			input_0 = _mm_loadu_si128((__m128i *)_b);
			input_1 = _mm_loadu_si128((__m128i *)(_b + 16));

			// Pack low bytes of inputs to input_l
			tmp = _mm_set1_epi16(0x00ff);
			v_0 = _mm_and_si128(input_0, tmp);
			v_1 = _mm_and_si128(input_1, tmp);
			input_l = _mm_packus_epi16(v_0, v_1);

			// Pack high bytes of inputs to input_h
			v_0 = _mm_srli_epi16(input_0, 8);
			v_1 = _mm_srli_epi16(input_1, 8);
			input_h = _mm_packus_epi16(v_0, v_1);

			// Retrieve low 4bit of each byte from input_l
			tmp = _mm_set1_epi8(0x0f);
			input_l_l = _mm_and_si128(input_l, tmp);

			// Retrieve high 4bit of each byte from input_l
			v_0 = ShiftR4_128(input_l);
			input_l_h = _mm_and_si128(v_0, tmp);

			// Retrieve low 4bit of each byte from input_h
			input_h_l = _mm_and_si128(input_h, tmp);

			// Retrieve high 4bit of each byte from input_h
			v_0 = ShiftR4_128(input_h);
			input_h_h = _mm_and_si128(v_0, tmp);

			// Get GF calc results for low bytes
			v_0 = _mm_shuffle_epi8(tb_a_0_l, input_l_l);
			v_0 = _mm_xor_si128(v_0,
				_mm_shuffle_epi8(tb_a_1_l, input_l_h));
			v_0 = _mm_xor_si128(v_0,
				_mm_shuffle_epi8(tb_a_2_l, input_h_l));
			v_0 = _mm_xor_si128(v_0,
				_mm_shuffle_epi8(tb_a_3_l, input_h_h));

			// Get GF calc results for high bytes
			v_1 = _mm_shuffle_epi8(tb_a_0_h, input_l_l);
			v_1 = _mm_xor_si128(v_1,
				_mm_shuffle_epi8(tb_a_1_h, input_l_h));
			v_1 = _mm_xor_si128(v_1,
				_mm_shuffle_epi8(tb_a_2_h, input_h_l));
			v_1 = _mm_xor_si128(v_1,
				_mm_shuffle_epi8(tb_a_3_h, input_h_h));

			// Unpack low bytes
			out_l = _mm_unpacklo_epi8(v_0, v_1);

			// Unpack high bytes
			out_h = _mm_unpackhi_epi8(v_0, v_1);

			// Save results
			_mm_storeu_si128((__m128i *)_d, out_l);
			_mm_storeu_si128((__m128i *)(_d + 16), out_h);

			_b += 32;
			_d += 32;
		}
	}

#elif defined(_arm64_)
	/*** 4bit multi table region technique with NEON ***/
	uint8_t		*_b, *_d;
	uint8x16_t	input_l, input_h, v_0, v_1;
	uint8x16_t	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	uint8x16_t	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;
	uint8x16_t	input_l_l, input_l_h, input_h_l, input_h_h, tmp;
	uint8x16x2_t	input, output;

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 0)) == NULL) {
		exit(1);
	}

	// Assign other addresses
	tb_a_0_l = vld1q_u8(gf_tb + 0);
	tb_a_0_h = vld1q_u8(gf_tb + 16);
	tb_a_1_l = vld1q_u8(gf_tb + 32);
	tb_a_1_h = vld1q_u8(gf_tb + 48);
	tb_a_2_l = vld1q_u8(gf_tb + 64);
	tb_a_2_h = vld1q_u8(gf_tb + 80);
	tb_a_3_l = vld1q_u8(gf_tb + 96);
	tb_a_3_h = vld1q_u8(gf_tb + 112);

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = (uint8_t *)b;
		_d = (uint8_t *)d;
		for (j = 0; j < SPACE; j += 32) { // Do every 128 * 2bit
			// Load interleaved inputs
			input = vld2q_u8(_b);
			input_l = input.val[0];
			input_h = input.val[1];

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
			output.val[0] = v_0;
			output.val[1] = v_1;
			vst2q_u8(_d, output);

			_b += 32;
			_d += 32;
		}
	}
#endif

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("4 x 16byte lookup tables + SIMD  : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error at SIMD: E-mail me "
			"(nishida at asusa.net) if this happened.\n");
		for (i = 0; i < 16; i++) {
			printf("%04x ", c[i]);
		}
		putchar('\n');
		for (i = 0; i < 16; i++) {
			printf("%04x ", d[i]);
		}
		putchar('\n');
		exit(1);
	}

	exit(0);
}
