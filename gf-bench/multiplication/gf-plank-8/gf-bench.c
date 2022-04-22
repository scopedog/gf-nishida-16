#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "galois.h"
#include "common.h"
#include "mt64.h"

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	uint8_t		a, *b, *c;
	uint64_t	*r;

	// Allocate b and c
	if ((b = (uint8_t *)malloc(SPACE * 2)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + (SPACE / sizeof(uint8_t)); // 8bit;

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = (uint8_t)(genrand64_int64() & 0xff);
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Calculate c[j] = a * b[j]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint8_t); j++) { // 8bit
			// Calculate in GF
			// To avoid elimination by cc's -O2 option,
			// input result into c[j]
			c[j] = galois_single_multiply(a, b[j], 8); // 8bit
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	exit(0);
}
