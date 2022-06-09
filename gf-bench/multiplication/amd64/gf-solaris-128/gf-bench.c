#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <wmmintrin.h>
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
	__m128i		a, b; // Use Intel CLMUL instruction set
	uint64_t	*r, *r1, *r2;

	// Allocate for random numbers - Fist SPACE for r1 and second for r2
	if ((r = (uint64_t *)malloc(SPACE * 2)) == NULL) { //
		perror("malloc");
		exit(1);
	}
	r1 = r;
	r2 = r + (SPACE / sizeof(uint64_t));

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to r
	j = SPACE * 2 / sizeof(uint64_t);
	for (i = 0; i < j; i++) {
		r[i] = genrand64_int64();
	}
	a = _mm_set_epi64x(r1[0], r1[1]);

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Repeat calc in GF
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint64_t); j += 2) { // 
			// Set a, b
			b = _mm_set_epi64x(r2[j], r2[j + 1]);
			// To avoid elimination by cc's -O2 option,
			// input result into &r1[j]
			gfmul(a, b, &r1[j]);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));
}
