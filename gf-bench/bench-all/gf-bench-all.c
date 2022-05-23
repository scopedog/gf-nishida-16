#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include "common.h"

/************************************************************
	Definitions
************************************************************/

#define DEFAULT_REPEAT	10

/************************************************************
	Global variables
************************************************************/

int	num_repeat = DEFAULT_REPEAT;

/************************************************************
	Main
************************************************************/

// Show usage and exit
static void
UsageExit(const char *program, int argc, char **argv, int exit_stat)
{
	fprintf(stderr, "Usage: %s [num_of_repeats]\n", program);
	exit(exit_stat);
}

// Trim string
char *
TrimString(char *str)
{
	unsigned char	*p; /* There was a reason why I had to use 'unsigned'
			       here, but I forgot it. Probably, it was
			       necessary for UTF-8 strings. */

	/* Change '\n' to '\0' because some OSs like FreeBSD don't add '\0'
	   at the end of string retrieved by fgets() */
	if ((p = (unsigned char *) strrchr(str, '\n')) != NULL)
		*p = '\0';

	p = (unsigned char *) strrchr(str, '\0');
	for (--p; isspace((int) *p) && p >= (unsigned char *) str; p--)
		*p = '\0';

	for (p = (unsigned char *) str; isspace((int) *p) && *p != '\0'; p++);

	/* Return the pointer to trimmed string */
	return (char *) p;
}

// Benchmark gf-nishida-region-16
static int
BenchGfNishidaRegion16()
{
	char		buf[BUFSIZ], *p, *q;
	int		n, idx, err = 0;
	uint64_t	bench_res[4];
	FILE		*fp = NULL;

	// Iniitialize
	memset(bench_res, 0, sizeof(bench_res));

	// Start benchmark 
	for (n = 0; n < num_repeat; n++) {
		idx = 0;
		if ((fp = popen("make bench 2> /dev/null", "r")) == NULL) {
			fprintf(stderr, "Error: popen: make bench: %s\n",
				strerror(errno));
			err = errno;
			goto END;
		}

		// Scan result
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			// Trim bug
			p = TrimString(buf);

			// Check if buf contains ':'
			if ((q = strrchr(p, ':')) == NULL) {
				continue;
			}

			// Retrieve result
			q++;
			while (isspace(*q)) {
				q++;
			}
			bench_res[idx] += atol(q);
			idx++;
		}

		fclose(fp);
		fp = NULL;
	}

	// Print results (MB/s)
	for (idx = 0; idx < 4; idx++) {
		//printf("%ld\n", bench_res[idx] / num_repeat);
		printf("gf-nishida-region-16-%d, %f\n", idx + 1,
			(double)(SPACE * REPEAT) / ((double)bench_res[idx] /
				(double)num_repeat));
	}

END:	// Finalize
	if (fp != NULL) {
		fclose(fp);
	}

	return err ? -1 : 0;
}

// Benchmark other gf algorithm
static int
BenchGfOther(const char *name)
{
	char		buf[BUFSIZ], *p;
	int		n, err = 0;
	uint64_t	bench_res = 0;
	FILE		*fp = NULL;

	// Start benchmark 
	for (n = 0; n < num_repeat; n++) {
		if ((fp = popen("make bench 2> /dev/null", "r")) == NULL) {
			fprintf(stderr, "Error: popen: make bench: %s\n",
				strerror(errno));
			err = errno;
			goto END;
		}

		// Scan result
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			// Trim bug
			p = TrimString(buf);

			// Ritrieve only number
			if (isdigit(*p)) {
				bench_res += atol(p);
			}
		}

		fclose(fp);
		fp = NULL;
	}

	// Print results (MB/s)
	//printf("%ld\n", bench_res[idx] / num_repeat);
	printf("%s, %f\n", name,
		(double)(SPACE * REPEAT)/ ((double)bench_res /
			(double)num_repeat));

END:	// Finalize
	if (fp != NULL) {
		fclose(fp);
	}

	return err ? -1 : 0;
}

// Main
int
main(int argc, char **argv)
{
	const char	*program, *name;
	int		err = 0;
	DIR		*dp = NULL;
	struct dirent	*de;

	// Get program name
	if ((program = strrchr(argv[0], '/')) == NULL) {
		program = argv[0];
	}

	// Check args
	if (argc == 1) {
		num_repeat = DEFAULT_REPEAT;
	}
	else if (argc == 2) {
		errno = 0;
		num_repeat = atoi(argv[1]);
		if (errno) { // Error with atoi
			UsageExit(program, argc, argv, EXIT_FAILURE);
		}
	}
	else {
		UsageExit(program, argc, argv, EXIT_FAILURE);
	}

	// Go to multiplication dir and scan dir
	chdir("../multiplication");
	if ((dp = opendir("./")) == NULL) {
		fprintf(stderr, "Error: opendir: ../multiplication: %s\n",
			strerror(errno));
		err = errno;
		goto END;
	}

	puts("Multiplication\nAlgorithm, Speed (MB/s)");

	// Scan all dirs and benchmark multiplication
	while ((de = readdir(dp)) != NULL) {
		name = de->d_name;

		// Skip if name starts with . or entry is not dir
		if (name[0] == '.' || de->d_type != DT_DIR) {
			continue;
		}

		// Benchmark
		if (chdir(name) == -1) {
			fprintf(stderr, "Error: chdir %s: %s\n",
				name, strerror(errno));
			err = errno;
			goto END;
		}

		// Benchmark gf-nishida-region-16
		if (strcmp(name, "gf-nishida-region-16") == 0) {
			BenchGfNishidaRegion16();
		}
		else { // Benchmark other
			BenchGfOther(name);
		}

		chdir("../");
	}

	// End multiplication benchmark
	closedir(dp);
	dp = NULL;

	// Go to division dir and scan dir
	chdir("../division");
	if ((dp = opendir("./")) == NULL) {
		fprintf(stderr, "Error: opendir: ../division: %s\n",
			strerror(errno));
		err = errno;
		goto END;
	}

	puts("\nDivision\nAlgorithm, Speed (MB/s)");

	// Scan all dirs and benchmark division
	while ((de = readdir(dp)) != NULL) {
		name = de->d_name;

		// Skip if name starts with . or entry is not dir
		if (name[0] == '.' || de->d_type != DT_DIR) {
			continue;
		}

		// Benchmark
		if (chdir(name) == -1) {
			fprintf(stderr, "Error: chdir %s: %s\n",
				name, strerror(errno));
			err = errno;
			goto END;
		}

		// Benchmark gf-nishida-region-16
		if (strcmp(name, "gf-nishida-region-16") == 0) {
			BenchGfNishidaRegion16();
		}
		else { // Benchmark other
			BenchGfOther(name);
		}

		chdir("../");
	}

END:	// Finalize
	if (dp != NULL) {
		closedir(dp);
	}

	exit(err ? EXIT_FAILURE : EXIT_SUCCESS);
}
