#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "ff_2_163.h"

// Space allocation
#define SPACE	640000
#define NUM_ELE	(SPACE / sizeof(uint64_t))

// # of repeats
#define REPEAT	200	// Actual repeat times is SPACE * REPEAT 

// Main
int
main(int argc, char **argv)
{
	// Variables
	int		i, j;
	struct timeval	start, end;
	ff_element	*a, *b, *_a;

	// Initialize GF

	// Allocate
	if ((a = (ff_element *)malloc(sizeof(ff_element) * NUM_ELE * 2))
		== NULL) { // 
		perror("malloc");
		exit(1);
	}
	b = a + NUM_ELE; // 

	// Initialize random generator

	// Input random numbers to a, b
	for (i = 0; i < NUM_ELE; i++) {
		ff_rand(a[i]);
		ff_rand(b[i]);
	}
	_a = a;

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Repeat calc in GF
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < NUM_ELE; j++) {
			// Calculate in GF
			// To avoid elimination by cc's -O2 option, input result to a[j]
			ff_mul(*_a, b[j], a[j]); // 
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}
