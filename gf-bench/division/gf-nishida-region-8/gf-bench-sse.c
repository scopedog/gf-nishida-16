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
	uint8_t		a, *b, *c, *d, *_b, *_d, *gf_a, *gf_tb;
	uint64_t	*r;
	__m128i		tb_a_l, tb_a_h;

#if !defined(__SSSE3__)
	fputs("Error: This program is only for SSE (SSSE3)\n", stderr);
	exit(1);
#endif

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

	// Create region table for a
	if ((gf_a = GF8crtRegTbl(a, 1)) == NULL) {
		exit(1);
	}

	// c = a * b = gf_a[b]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE; j++) {
			// One step look up
			c[j] = gf_a[b[j]];
			// Or use:
			//c[j] = GF8LkupRT(gf_a, b[j]);
		}
	}

	// Don't forget this if you called GF8crtRegTbl()
	free(gf_a);

	/*** 4bit multi table region technique by SSSE3 ***/

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

		// Load tables -- you can do this outside loop
		tb_a_l = _mm_loadu_si128((__m128i *)(gf_tb + 0));
		tb_a_h = _mm_loadu_si128((__m128i *)(gf_tb + 16));

		for (j = 0; j < SPACE; j += 16) { // Do every 128 * 2bit
			// Use SIMD lookup
			GF8lkupSIMD128(tb_a_l, tb_a_h, _b, _d);
			_b += 16;
			_d += 16;
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
	// Don't forget this if you called GF1166crt4bitRegTbl()
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
}
