/*
 */

#include <stdio.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "gf_complete.h"
#include "gf_rand.h"

/************************************************************
	Definitions
************************************************************/

#define NUM_OF_CALC_DATA 1000	// # of 64bit data to calculate in region
#define CALC_DATA_BYTES	 (8 * NUM_OF_CALC_DATA)	// and bytes

/************************************************************
	Global varibales
************************************************************/

gf_t	gf;

/************************************************************
	Functions
************************************************************/

// Usage
void usage(char *argv0)
{
	fprintf(stderr, "usage: %s\n", argv0);
	exit(1);
}

// Initialize 
void Init()
{
	/* Create instance for GF-Complete */
	gf_init_easy(&gf, 64);
}

int main(int argc, char **argv)
{
	uint64_t	a, b, c;
	uint64_t	*r1, *r2;
	int		i;

	if (argc != 1)
		usage(argv[0]);

	// Initialize
	Init();

	/* Get two random numbers in a and b */
	MOA_Seed(time(0));
	a = MOA_Random_64();
	b = MOA_Random_64();

	/* And multiply a and b using the galois field: */
	c = gf.multiply.w64(&gf, a, b);
	printf("%llx * %llx = %llx\n", (long long unsigned int) a, (long long unsigned int) b, (long long unsigned int) c);

	/* Divide the product by a and b */
	printf("%llx / %llx = %llx\n", (long long unsigned int) c, (long long unsigned int) a, (long long unsigned int) gf.divide.w64(&gf, c, a));
	printf("%llx / %llx = %llx\n", (long long unsigned int) c, (long long unsigned int) b, (long long unsigned int) gf.divide.w64(&gf, c, b));

	// Allocate numbers for region calculation
	r1 = (uint64_t *) malloc(CALC_DATA_BYTES); // uint64_t data array
	r2 = (uint64_t *) malloc(CALC_DATA_BYTES);

	r1[0] = b;

	for (i = 1; i < NUM_OF_CALC_DATA; i++)
		r1[i] = MOA_Random_64();

	// Calculate data array at once with multiply_region
	gf.multiply_region.w64(&gf, r1, r2, a, CALC_DATA_BYTES, 0);

	printf("\nmultiply_region by %llx\n\n", (long long unsigned int) a);
	puts("R1 (the source):  ");
	for (i = 0; i < NUM_OF_CALC_DATA; i++)
		printf(" %016llx", (long long unsigned int) r1[i]);

	puts("\nR2 (the product): ");
	for (i = 0; i < NUM_OF_CALC_DATA; i++)
		printf(" %016llx", (long long unsigned int) r2[i]);
	putchar('\n');

	exit(0);
}
