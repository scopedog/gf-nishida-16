#include <stdint.h>
#include <wmmintrin.h>

struct aes_block {
	uint64_t	a;
	uint64_t	b;
};

// This func uses Intel CLMUL instruction set which is specialized for Galois calculation
void
gfmul(__m128i x_in_m, __m128i y_m, uint64_t * res)
{
	uint64_t	R = {0xe100000000000000ULL};
	struct aes_block z = {0, 0};
	struct aes_block v;
	uint64_t	x;
	int		i         , j;
	uint64_t       *x_in = (uint64_t *) & x_in_m;
	uint64_t       *y = (uint64_t *) & y_m;
	v.a = y[1];
	v.b = y[0];
	for (j = 1; j >= 0; j--) {
		x = x_in[j];
		for (i = 0; i < 64; i++, x <<= 1) {
			if (x & 0x8000000000000000ULL) {
				z.a ^= v.a;
				z.b ^= v.b;
			}
			if (v.b & 1ULL) {
				v.b = (v.a << 63) | (v.b >> 1);
				v.a = (v.a >> 1) ^ R;
			} else {
				v.b = (v.a << 63) | (v.b >> 1);
				v.a = v.a >> 1;
			}
		}
	}
	res[0] = z.b;
	res[1] = z.a;
}
