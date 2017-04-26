#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "galois.h"
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
	uint16_t	*a, *b, _a; // 16bit
	uint64_t	*r;
	uint64_t	rand_init[4] =
			{UINT64_C(0xfd308), UINT64_C(0x65ab8),
			UINT64_C(0x931cd54), UINT64_C(0x9475ea2)};

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
	_a = a[0];

	// Initialize gf table
/*
	if (galois_create_mult_tables(16) < 0) { // This fails
		fputs("Error: galois_create_mult_tables failed\n", stderr);
		exit(1);
	}
*/
	if (galois_create_log_tables(16) < 0) {
		fputs("Error: galois_create_log_tables failed\n", stderr);
		exit(1);
	}

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Repeat calc in GF
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE / sizeof(uint16_t); j++) {
			// Calculate in GF
			// To avoid elimination by cc's -O2 option, input result into a[j]
			a[j] = (uint16_t)galois_logtable_divide(b[j], _a, 16); // 16bit
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}
