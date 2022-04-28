/*
 *	All rights revserved by ASUSA Corporation and ASJ Inc.
 *	Copying any part of this program is strictly prohibited.
 *	Author: Hiroshi Nishida
 */

#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <string.h> 
#include <time.h> 
#define	_GF_MAIN_
#include "gf.h"
#undef	_GF_MAIN_

/************************************************************
	Definitions
************************************************************/

// Definitions for GF8
#define	GF8_PRIM	487	// Prim poly: Others are 285, 299, 301, 333, 351, 355, 357, 361, 391..
#define	GF8_SIZE	256	// = 8bit 

// Definitions for GF16
#define	GF16_PRIM	69643	// Prim poly: 0x1100b = x^16 + x^12 + x^3 + x +1
#define	GF16_SIZE	65536	// = 16bit 

/*
	For more primitive polynomials, see:
	../gf-prim/primitive-polynomial-table.txt

	or

	http://web.eecs.utk.edu/~plank/plank/papers/CS-07-593/primitive-polynomial-table.txt
*/


/************************************************************
	Variables
************************************************************/

static GF8type	*GF8memOrg;
static GF8type	*GF8memIdx;
#if 0
static GF16type	*GF16memOrg, *GF16memOrgH;
static int	*GF16memIdx;
#endif

/************************************************************
	Functions for GF8 (8bit)
************************************************************/

// Return a * b 
static GF8type
GF8mulVal(GF8type a, GF8type b)
{
	if (a == 0 || b == 0)
		return 0;

	return GF8memOrg[(GF8memIdx[a] + GF8memIdx[b]) % (GF8_SIZE - 1)];
}

// Return a / b
static GF8type
GF8divVal(GF8type a, GF8type b)
{
	if (a == 0 || b == 0)
		return 0;

	return GF8memOrg[(GF8memIdx[a] - GF8memIdx[b] + (GF8_SIZE - 1)) %
			(GF8_SIZE - 1)];
}

// Initialize 8 bit GF
void
GF8init(void)
{
	int		i, j;
	uint32_t	n;
	
	// Allocate memory
	GF8memOrg = (GF8type *)malloc(sizeof(GF8type) * GF8_SIZE);
	GF8memIdx = (GF8type *)malloc(sizeof(GF8type) * GF8_SIZE);
	GF8memMul = (GF8type **)malloc(sizeof(GF8type*) * GF8_SIZE);
	GF8memDiv = (GF8type **)malloc(sizeof(GF8type*) * GF8_SIZE);
 
	for (i = 0; i < GF8_SIZE; i++) {
		GF8memMul[i] = (GF8type *)malloc(sizeof(GF8type) * GF8_SIZE);
		GF8memDiv[i] = (GF8type *)malloc(sizeof(GF8type) * GF8_SIZE);
	}

	GF8memOrg[0] = 1;
	GF8memIdx[0] = -1;

	// Set GF8memOrg and GF8memIdx
	for (i = 0; i < GF8_SIZE - 1;) {
		n = (uint32_t)GF8memOrg[i] << 1;
		i++;
		GF8memOrg[i] = (GF8type)
			((n >= GF8_SIZE) ? n ^ GF8_PRIM : n);
		GF8memIdx[GF8memOrg[i]] = i;
	}
	GF8memIdx[1] = 0;

	// Set GF8memMul and GF8memDiv
	for (i = 0; i < GF8_SIZE; i++) {
		for (j = 0; j < GF8_SIZE; j++) {
			GF8memMul[i][j] = GF8mulVal((GF8type)i, (GF8type)j);
			GF8memDiv[i][j] = GF8divVal((GF8type)i, (GF8type)j);
		}
	}
}

#if 1
/************************************************************
	Functions for GF16 (16bit) -- Improved
************************************************************/

// Initialize 16bit GF
void
GF16init(void)
{
	int		i;
	uint32_t	n;
	
	// Allocate memory
	GF16memOrg = (GF16type *)malloc(sizeof(GF16type) * GF16_SIZE * 4);
	GF16memIdx = (int *)malloc(sizeof(int) * GF16_SIZE);

	GF16memOrgH = GF16memOrg + GF16_SIZE - 1;
	GF16memOrg[0] = 1;
	GF16memIdx[0] = -1;

	// Set GF16memOrg and GF16memIdx
	for (i = 0; i < GF16_SIZE - 1;) {
		n = (uint32_t)GF16memOrg[i] << 1;
		i++;
		GF16memOrg[i] = (GF16type)
			((n >= GF16_SIZE) ? n ^ GF16_PRIM : n);
		GF16memIdx[GF16memOrg[i]] = i;
	}
	GF16memIdx[0] = (GF16_SIZE << 1) - 1;
	GF16memIdx[1] = 0;

	// Copy first half of GF16memOrg to second half
	memcpy(&GF16memOrg[GF16_SIZE - 1], GF16memOrg,
		sizeof(GF16type) * (GF16_SIZE - 1));

	// Fill remaining of GF16memOrg with zero
	memset(&GF16memOrg[(GF16_SIZE << 1) - 2], 0,
		sizeof(GF16type) * ((GF16_SIZE << 1) + 2));

	//DebugMsg("GF16init: Done\n");
}

#if 0	// Moved to gf.h 
// Return a * b 
GF16type
GF16mul(GF16type a, GF16type b)
{
	if (!a || !b)
		return 0;

	return GF16memOrg[GF16memIdx[a] + GF16memIdx[b]];
}

// Return a / b
GF16type
GF16div(GF16type a, GF16type b)
{
	if (!a || !b)
		return 0;

	//return GF16memOrg[GF16memIdx[a] - GF16memIdx[b] + 65535];
	return GF16memOrgH[GF16memIdx[a] - GF16memIdx[b]];
}
#endif

#else
/************************************************************
	Functions for GF16 (16bit) -- Legacy
************************************************************/

// Initialize 16bit GF
void
GF16init(void)
{
	int		i;
	uint32_t	n;
	
	// Allocate memory
	GF16memOrg = (GF16type *)malloc(sizeof(GF16type) * GF16_SIZE);
	GF16memIdx = (GF16type *)malloc(sizeof(GF16type) * GF16_SIZE);

	GF16memOrg[0] = 1;
	GF16memIdx[0] = -1;

	// Set GF16memOrg and GF16memIdx
	for (i = 0; i < GF16_SIZE - 1;) {
		n = (uint32_t)GF16memOrg[i] << 1;
		i++;
		GF16memOrg[i] = (GF16type)
			((n >= GF16_SIZE) ? n ^ GF16_PRIM : n);
		GF16memIdx[GF16memOrg[i]] = i;
	}
	GF16memIdx[1] = 0;
	//DebugMsg("GF16init: Done\n");
}

#if 1
// Return a * b 
GF16type
GF16mul(GF16type a, GF16type b)
{
	uint32_t	idx;

	if (a == 0 || b == 0)
		return 0;

	idx = GF16memIdx[a] + GF16memIdx[b];
	idx %= GF16_SIZE - 1;

	return GF16memOrg[idx];
}
#else
// Return a * b 
GF16type
GF16mul(GF16type a, GF16type b)
{
	if (a == 0 || b == 0)
		return 0;

	return GF16memOrg[(GF16memIdx[a] + GF16memIdx[b]) % (GF16_SIZE - 1)];
}
#endif

#if 1
// Return a / b
GF16type
GF16div(GF16type a, GF16type b)
{
	int32_t	idx;

	if (a == 0 || b == 0)
		return 0;

	idx = (int)GF16memIdx[a] - (int)GF16memIdx[b];
	if (idx == GF16_SIZE - 1) {
		idx = 0;
	}
	else if (idx & 0x80000000) {
		idx += GF16_SIZE - 1;
	}

	return GF16memOrg[idx];
}
#else
// Return a / b
GF16type
GF16div(GF16type a, GF16type b)
{
	if (a == 0 || b == 0)
		return 0;

	return GF16memOrg[(GF16memIdx[a] - GF16memIdx[b] + (GF16_SIZE - 1)) %
			(GF16_SIZE - 1)];
}
#endif
#endif

/************************************************************
	Functions for testing GF
************************************************************/

#if 1
// Test GF8
void
GF8test(void)
{
	int64_t		i;
	GF8type		a, b, c, d;
	time_t		time_s, time_e;

	time_s = time(NULL);
	for (i = 0; i < 2000000000; i++) {
		do {
			a = (GF8type)(random() & 0x00ff);
		} while (a == 0);
		do {
			b = (GF8type)(random() & 0x00ff);
		} while (b == 0);

		c = GF8mul(a, b);
		d = GF8div(c, b);

		if (d != a) {
			printf("GF8test: d (%d) != a (%d)\n", d, a);
			printf("a = %d, b = %d, c = %d, d = %d\n", a, b, c, d);
			exit(1);
		}
	}
	time_e = time(NULL);
	printf("Elapsed time: %ld\n", time_e - time_s);
	puts("GF8test: Passed");
}

// Test GF16
void
GF16test(void)
{
	int		i, j;
	GF16type	a, b, c, d;
	time_t		time_s, time_e;

	time_s = time(NULL);
	for (i = 1; i < 65536; i++) {
		a = (GF16type)i;
		for (j = 1; j < 65536; j++) {
			b = (GF16type)j;
			c = GF16mul(a, b);
			d = GF16div(c, b);

			if (d != a) {
				printf("GF16test: d (%d) != a (%d)\n", d, a);
				printf("a = %d, b = %d, c = %d, d = %d\n",
					a, b, c, d);
				exit(1);
			}
		}
	}
	time_e = time(NULL);
	printf("Elapsed time: %ld\n", time_e - time_s);
	puts("GF16test: Passed");
}
#endif
