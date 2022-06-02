This library provides simple and fast arithmetic functions (macros) in GF(2^16)
based on table lookup.
The only programs you need are gf.c and gf.h; copy them to your directory and
compile your programs with them.

Initially, call
```
    GF16init();
```
and thereafter call other functions (macros) such as GF16mul(), GF16div().

Some techniques to accelerate region calculation such as:
```
    for (i = 0; i < N; i++)
        c[i] = a * b[i]; // Static coefficient * region of data (array)
```
are also included.
Please see gf-bench/multiplication/gf-nishida-region-16/gf-bench.c for
the code examples.

All the deitais and benchmark results are described in our technical papers
gf-nishida-16.pdf (English) and gf-nishida-16-ja.pdf (Japanese).

CAUTION!! Never use b = 0 for GF16div(a, b) because it causes segmentation
violation.
The program does not check "(a == 0 || b == 0)" for speedup.


				Hiroshi Nishida
				nishida at asusa.net
