This library provides simple and fast arithmetic functions in GF(2^16)
based on two step memory lookup.
The size of memory allocated for the lookup tables is only 768kB.
Note GF16mul() and GF16div() are not functions but are defined in gf.h.
The only programs you need are gf.c and gf.h.
Just copy them to your directory and compile your programs with them.

Please see our technical paper gf-nishida-16.pdf (English) or 
gf-nishida-16-ja.pdf (Japanese) for the further details.

CAUTION!! Never use b = 0 for GF16div(a, b) because it causes segmentation
violation.
The program does not check "(a == 0 || b == 0)" for speedup.


				Hiroshi Nishida
				nishida at asusa.net
