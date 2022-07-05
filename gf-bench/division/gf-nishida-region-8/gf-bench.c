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
	uint8_t		a, *b, *c, *d, *gf_a;
	uint64_t	*r;

	// Initialize GF
	GF8init();

	// Allocate b and c
	if ((b = (uint8_t *)aligned_alloc(64, SPACE * 3)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + SPACE;
	d = c + SPACE;

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = (uint8_t)(genrand64_int64() & 0xff);
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	/*** One step table lookup technique
	     This will run in L1 cache as gf_a is 256B ***/

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Create region table for a
	if ((gf_a = GF8crtRegTbl(a, 1)) == NULL) {
		exit(1);
	}
#endif

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// c = a * b = gf_a[b]
	for (i = 0; i < REPEAT; i++) {
#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// This is only for benchmarking purpose
		// Create region table for a
		if ((gf_a = GF8crtRegTbl(a, 1)) == NULL) {
			exit(1);
		}
#endif

		for (j = 0; j < SPACE; j++) {
			// One step look up
			c[j] = gf_a[b[j]];
			// Or use:
			//c[j] = GF8LkupRT(gf_a, b[j]);
		}

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// Don't forget this if you called GF16crtRegTbl()
		free(gf_a);
#endif
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step table lookup        : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Don't forget this if you called GF8crtRegTbl()
	free(gf_a);
#endif


	// From now on, we need this
	uint8_t	*gf_tb;

	// AVX2
#if defined(__AVX2__)
{
	/*** 4bit multi table region technique by AVX2 ***/

	uint8_t	*_b, *_d;
	__m256i	tb_a_l, tb_a_h;

	// Reset d
	memset(d, 0, SPACE); 

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Create 2 * 16 byte region tables for a
	if ((gf_tb = GF8crt4bitRegTbl256(a, 1)) == NULL) {
		exit(1);
	}
#endif

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = b;
		_d = d;

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// This is only for benchmarking purpose
		// Create 2 * 16 byte region tables for a
		if ((gf_tb = GF8crt4bitRegTbl256(a, 1)) == NULL) {
			exit(1);
		}
#endif

		// Load tables
		tb_a_l = _mm256_loadu_si256((__m256i *)(gf_tb + 0));
		tb_a_h = _mm256_loadu_si256((__m256i *)(gf_tb + 32));

		for (j = 0; j < SPACE; j += 32) { // Do every 256bit
			// Use SIMD lookup
			GF8lkupSIMD256(tb_a_l, tb_a_h, _b, _d);
			_b += 32;
			_d += 32;
		}

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// Don't forget this if you called GF16crt4bitRegTbl()
		free(gf_tb);
#endif
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step lookup by AVX       : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Don't forget this if you called GF16crt4bitRegTbl()
	free(gf_tb);
#endif

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
{
	/*** 4bit multi table region technique by NEON ***/

	uint8_t		*_b, *_d;
	uint8x16_t	tb_a_l, tb_a_h;

	// Reset d
	memset(d, 0, SPACE); 

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Create 2 * 16 byte region tables for a
	if ((gf_tb = GF8crt4bitRegTbl(a, 1)) == NULL) {
		exit(1);
	}
#endif

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// This is a little complicated...
	for (i = 0; i < REPEAT; i++) {
		_b = b;
		_d = d;

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// This is only for benchmarking purpose
		// Create 2 * 16 byte region tables for a
		if ((gf_tb = GF8crt4bitRegTbl(a, 1)) == NULL) {
			exit(1);
		}
#endif

		// Load tables
		tb_a_l = vld1q_u8(gf_tb + 0);
		tb_a_h = vld1q_u8(gf_tb + 16);

		for (j = 0; j < SPACE; j += 16) { // Do every 128bit
			// Use SIMD lookup
			GF8lkupSIMD128(tb_a_l, tb_a_h, _b, _d);
			_b += 16;
			_d += 16;
		}

#if !defined(_REAL_USE_) // For real use, do this outside loop, not here
		// Don't forget this if you called GF16crt4bitRegTbl()
		free(gf_tb);
#endif
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("One step lookup by NEON      : %ld\n",
		((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

#if defined(_REAL_USE_) // For real use, do this here, not inside loop
	// Don't forget this if you called GF8crt4bitRegTbl()
	free(gf_tb);
#endif

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
}
#endif // _arm64_

	exit(0);
}
