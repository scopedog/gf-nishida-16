#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "ff_2_32.h"
#include "common.h"

#define NUM_ELE	(SPACE / sizeof(uint32_t))

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	ff_element	a, *b, *c;

	// Allocate b and c
	if ((b = (ff_element *)malloc(sizeof(ff_element) * NUM_ELE * 2))
			== NULL) {
		perror("malloc");
		exit(1);
	}
	c = b + NUM_ELE;

	// Input random numbers to a, b
	ff_rand(a);
	for (i = 0; i < NUM_ELE; i++) {
		ff_rand(b[i]);
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Calculate c[j] = a * b[j]
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < NUM_ELE; j++) {
			// Calculate in GF
			// To avoid elimination by cc's -O2 option,
			// input result into c[j]
			ff_mul(a, b[j], c[j]);
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec)));

	exit(0);
}
