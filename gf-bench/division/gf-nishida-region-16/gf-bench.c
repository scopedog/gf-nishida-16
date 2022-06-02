#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"
#include "gf.h"
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

	/*** Two step table lookup technique ***/

	// Set gf_a for c = b / a 
	gf_a = GF16memH - GF16memIdx[a];

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// c = b / a = gf_a[GF16memIdx(b)]]
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
	printf("Two step table lookup        : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	/*** One step table lookup technique
	     This will run in L2 cache as gf_a is 128kB ***/

	// Create region table for a
	if ((gf_a = GF16crtRegTbl(a, 2)) == NULL) {
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
	printf("One step table lookup        : %ld\n",
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

	/*** One step table lookup with two small tables
	     This will run in L1 cache as gf_a_l and gf_a_h are 512B each ***/
	// Create 2 * 512 byte region tables for a
	if ((gf_a_l = GF16crtSpltRegTbl(a, 2)) == NULL) {
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
	printf("One step lookup /w small tbls: %ld\n",
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

	// Reset d
	memset(d, 0, SPACE); 

#if defined(_amd64_) || defined(_x86_64_) // SSE/AVX
{
	/*** 4bit multi table region technique with SSSE3 ***/

	uint8_t	*_b, *_d;
	__m128i	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	__m128i	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 2)) == NULL) {
		exit(1);
	}

	// Load tables
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
			// Use SIMD lookup
			GF16lkupSIMD128(tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h,
					tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h,
					_b, _d);

			_b += 32;
			_d += 32;
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step lookup by SSE       : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error at SSE: E-mail me "
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
}

{
	/*** 4bit multi table region technique by AVX2 ***/

	uint8_t	*_b, *_d;
	__m256i	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	__m256i	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;

	// Reset d
	memset(d, 0, SPACE); 

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl256(a, 2)) == NULL) {
		exit(1);
	}

	// Load tables
	tb_a_0_l = _mm256_loadu_si256((__m256i *)(gf_tb + 0));
	tb_a_0_h = _mm256_loadu_si256((__m256i *)(gf_tb + 32));
	tb_a_1_l = _mm256_loadu_si256((__m256i *)(gf_tb + 64));
	tb_a_1_h = _mm256_loadu_si256((__m256i *)(gf_tb + 96));
	tb_a_2_l = _mm256_loadu_si256((__m256i *)(gf_tb + 128));
	tb_a_2_h = _mm256_loadu_si256((__m256i *)(gf_tb + 160));
	tb_a_3_l = _mm256_loadu_si256((__m256i *)(gf_tb + 192));
	tb_a_3_h = _mm256_loadu_si256((__m256i *)(gf_tb + 224));

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = (uint8_t *)b;
		_d = (uint8_t *)d;
		for (j = 0; j < SPACE; j += 64) { // Do every 256 * 2bit
			// Use SIMD lookup
			GF16lkupSIMD256(tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h,
					tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h,
					_b, _d);

			_b += 64;
			_d += 64;
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step lookup by AVX       : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error at AVX: E-mail me "
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
}
#elif defined(_arm64_) // NEON
	/*** 4bit multi table region technique by NEON ***/

	uint8_t		*_b, *_d;
	uint8x16_t	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	uint8x16_t	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;

	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 2)) == NULL) {
		exit(1);
	}

	// Load tables
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
			// Use SIMD lookup
			GF16lkupSIMD(tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h,
				tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h,
				_b, _d);

			_b += 32;
			_d += 32;
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step lookup by NEON      : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error at NEON: E-mail me "
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
#endif

#if 0	// Set non 0 if you want to compare SIMD and non-SIMD
	/*** Non-SIMD region technique similar to SIMD one above ***/
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
	printf("One step lookup /w tiny tbls : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);
#endif

	exit(0);
}
