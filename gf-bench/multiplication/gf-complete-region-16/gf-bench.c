#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <malloc_np.h>
#include <sys/time.h>
#include "gf_complete.h"
#include "common.h"
#include "mt64.h"

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i;
	struct timeval	start, end;
	uint16_t	a, *b, *c;
	uint64_t	*r;
	gf_t		gf;

	// Initialize GF
	gf_init_easy(&gf, 16); // 16bit

	// Allocate b and c
	if ((b = (uint16_t *)aligned_alloc(64, SPACE * 2)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + (SPACE / sizeof(uint16_t)); // 16bit;

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = (uint16_t)(genrand64_int64() & 0xffff);
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Calculate c[j] = a * b[j]
	for (i = 0; i < REPEAT; i++) {
		gf.multiply_region.w32(&gf, b, c, a, SPACE, 0);
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	exit(0);
}
