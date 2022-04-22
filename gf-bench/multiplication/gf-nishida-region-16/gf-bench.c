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
	uint16_t	a, *b, *c, *d, *gf_a;
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

	// Set gf_a
	gf_a = GF16memL + GF16memIdx[a];

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Use regular region technique (faster than GF16mul)
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
	printf("Regular: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Create region table for a
	if ((gf_a = GF16crtRegionTbl(a, 0)) == NULL) {
		exit(1);
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Use special region technique not described in paper
	// Supposed to be even faster
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// One step look up
			d[j] = gf_a[b[j]];
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("Special: %ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	// Don't forget this if you called GF16crtRegionTbl()
	free(gf_a);

	// Compare c and d, they are supposed to be same
	if (memcmp(c, d, SPACE)) {
		fprintf(stderr, "Error: E-mail me (nishida at asusa.net) "
			"if this happens.\n");
		exit(1);
	}

	exit(0);
}
