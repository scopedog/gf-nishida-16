#ifndef _GF_BENCH_COMMON_H_
#define _GF_BENCH_COMMON_H_

#if defined(_arm64_)
#define SPACE	1024000 // Memory allocation space
#define REPEAT	100 // # of repeats
#else
#define SPACE	5120000 // Memory allocation space
#define REPEAT	100 // # of repeats
#endif

#endif // _GF_BENCH_COMMON_H_
