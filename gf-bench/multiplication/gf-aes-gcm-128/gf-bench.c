#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "aes-gcm.h"
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
	uint8_t		*a, *b, _a; // 128bit (8bit * 16)
	uint64_t	*r;
	uint64_t	rand_init[4] = {UINT64_C(0xfd308), UINT64_C(0x65ab8),
				UINT64_C(0x931cd54), UINT64_C(0x9475ea2)};

	// Allocate first SPACE bytes for a and second SPACE bytes for b
	if ((a = (uint8_t *)malloc(SPACE * 2)) == NULL) { //
		perror("malloc");
		exit(1);
	}
	b = a + SPACE; //

	// Initialize random generator
	init_by_array64(rand_init, 4);

	// Input random numbers to a, b - Do not change this
	j = SPACE * 2 / sizeof(uint64_t);
	r = (uint64_t *)a;
	for (i = 0; i < j; i++) {
		r[i] = genrand64_int64();
	}
	_a = a[0];

	// Start measuring elapsed time
	gettimeofday(&start, NULL); // Get start time

	// Repeat calc in GF
	for (i = 0; i < REPEAT; i++) {
		for (j = 0; j < SPACE; j += 16) { // 128bit
			// Calculate in GF
			// To avoid elimination by cc's -O2 option, input result into &a[j]
			gf_mult(&_a, &b[j], &a[j]); // 
		}
	}

	// Get end time
	gettimeofday(&end, NULL);

	// Print result
	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)));
}
