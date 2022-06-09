#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
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
	uint16_t	a, *b, *c, *d, *gf_a;
	uint64_t	*r;

	// Initialize GF
	GF16init();

	// Allocate b and c
	if ((b = (uint16_t *)aligned_alloc(64, SPACE * 3)) == NULL) {
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

#if 0
	/*** Two step table lookup technique ***/

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
	printf("Two step table lookup        : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));
#endif

	/*** One step table lookup technique
	     This will run in L2 cache as gf_a is 128kB ***/

	// Create region table for a
	if ((gf_a = GF16crtRegTbl(a, 2)) == NULL) {
		exit(1);
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// c = a * b = gf_a[b]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// One step look up
			c[j] = gf_a[b[j]];
			// Or use:
			//c[j] = GF16LkupRT(gf_a, b[j]);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

#if 0
	// Print result
	printf("One step table lookup        : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));
#endif

	// Don't forget this if you called GF16crtRegTbl()
	free(gf_a);

#if defined(__SSSE3__) || defined(__AVX2__)
	// From now on, we need this
	uint8_t	*gf_tb;

	/*** 4bit multi table region technique by SSSE3 ***/

	uint8_t	*_b, *_d;
	__m128i	tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h;
	__m128i	tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h;

	// Reset d
	memset(d, 0, SPACE); 

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Create 4 * 16 byte region tables for a
	if ((gf_tb = GF16crt4bitRegTbl(a, 2)) == NULL) {
		exit(1);
	}
#endif

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = (uint8_t *)b;
		_d = (uint8_t *)d;

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// This is only for benchmarking purpose
		// Create 4 * 16 byte region tables for a
		if ((gf_tb = GF16crt4bitRegTbl(a, 2)) == NULL) {
			exit(1);
		}
#endif

		// Load tables -- you can do this outside loop
		tb_a_0_l = _mm_loadu_si128((__m128i *)(gf_tb + 0));
		tb_a_0_h = _mm_loadu_si128((__m128i *)(gf_tb + 16));
		tb_a_1_l = _mm_loadu_si128((__m128i *)(gf_tb + 32));
		tb_a_1_h = _mm_loadu_si128((__m128i *)(gf_tb + 48));
		tb_a_2_l = _mm_loadu_si128((__m128i *)(gf_tb + 64));
		tb_a_2_h = _mm_loadu_si128((__m128i *)(gf_tb + 80));
		tb_a_3_l = _mm_loadu_si128((__m128i *)(gf_tb + 96));
		tb_a_3_h = _mm_loadu_si128((__m128i *)(gf_tb + 112));

		for (j = 0; j < SPACE; j += 32) { // Do every 128 * 2bit
			// Use SIMD lookup
			GF16lkupSIMD128(tb_a_0_l, tb_a_0_h, tb_a_1_l, tb_a_1_h,
					tb_a_2_l, tb_a_2_h, tb_a_3_l, tb_a_3_h,
					_b, _d);

			_b += 32;
			_d += 32;
		}

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// This is only for benchmarking purpose
		// Don't forget this if you called GF16crt4bitRegTbl()
		free(gf_tb);
#endif
	}

	// Get end time
	gettimeofday(&end, NULL);

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);
#endif

	// Print result
	printf("One step lookup by SSE       : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

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
#endif
}
