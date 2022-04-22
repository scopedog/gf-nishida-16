#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "aes-gcm.h"
#include "common.h"
#include "mt64.h"

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	uint8_t		*a, *b, *c;
	uint64_t	*r;

	// Allocate b and c
	if ((b = (uint8_t *)malloc(SPACE * 2)) == NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + SPACE;

	// Initialize random generator
	init_genrand64(time(NULL));

	// Input random numbers to a, b
	a = b; // Use first 128bit of b as a
	r = (uint64_t *)b;
	for (i = 0; i < SPACE / sizeof(uint64_t); i++) {
		r[i] = genrand64_int64();
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Use aes-gcm's gf_mult to calculate a * b[j]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE; j += 16) { // 128bit
			// Calculate in GF
			// To avoid elimination by cc's -O2 option,
			// input result into c[j]
			gf_mult(a, &b[j], &c[j]);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	exit(0);
}
