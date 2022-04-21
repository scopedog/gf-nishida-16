#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "gf.h"
#include "mt64.h"

// Space allocation
#define SPACE	640000

// # of repeats
#define REPEAT	200	// Actual repeat times is SPACE * REPEAT 

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	uint16_t	*a, *b, *gf_a; // 16bit
	uint64_t	*r;
	uint64_t	rand_init[4] = {UINT64_C(0xfd308), UINT64_C(0x65ab8),
				UINT64_C(0x931cd54), UINT64_C(0x9475ea2)};

	// Initialize GF
	GF16init(); // 16bit

	// Allocate
	if ((a = (uint16_t *)malloc(SPACE * 2)) == NULL) { // 16bit
		perror("malloc");
		exit(1);
	}
	b = a + (SPACE / sizeof(uint16_t)); // 16bit

	// Initialize random generator
	init_by_array64(rand_init, 4);

	// Input random numbers to a, b
	j = SPACE * 2 / sizeof(uint64_t);
	r = (uint64_t *)a;
	for (i = 0; i < j; i++) {
		r[i] = genrand64_int64();
	}

	// Set gf_a
	gf_a = GF16memL + GF16memIdx[a[0]];

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Repeat calc in GF
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// Calculate in GF
			// To avoid elimination by cc's -O2 option, input result into a[j]
			a[j] = gf_a[GF16memIdx[b[j]]]; // 16bit
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}
