This library provides simple and fast arithmetic functions in GF(2^16) based on memory lookups.
The size of memory allocated for the lookup tables is 768kB.
GF16mul() and GF16div() are defined in gf.h. 

Please see our technical paper gf-nishida-16.pdf or gf-nishida-16-ja.pdf
(Japanese) for the details.

CAUTION!! Never use b = 0 for GF16div(a, b) because it causes segmentation
violation.
The program does not check "(a == 0 || b == 0)" for speedup.

The only programs you need are gf.c and gf.h.
Just copy them to your directory and compile.

				Hiroshi Nishida
				nishida at asusa.net
