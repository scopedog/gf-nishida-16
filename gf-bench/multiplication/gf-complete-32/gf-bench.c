#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "gf_complete.h"
#include "common.h"
#include "mt64.h"

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	uint32_t	a, *b, *c;
	uint64_t	*r;
	gf_t		gf;

	// Initialize GF
	gf_init_easy(&gf, 32); // 32bit

	// Allocate b and c
	if ((b = (uint32_t *)malloc(SPACE * 2)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + (SPACE / sizeof(uint32_t)); // 32bit;

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = (uint32_t)(genrand64_int64() & 0xffffffff);
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Calculate c[j] = a * b[j]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint32_t); j++) { // 32bit
			// Calculate in GF
			// To avoid elimination by cc's -O2 option,
			// input result into c[j]
			c[j] = gf.multiply.w32(&gf, a, b[j]); // 32bit
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	exit(0);
}
