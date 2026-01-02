/* SPDX-License-Identifier: GPL-2.0 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void help(const char *app)
{
	printf("Usage: %s <options> <file>\n", app);
	printf("Options:\n");
	printf("  -a <address>     The address at which the code should be loaded\n");
	printf("  -h               Show this help\n");
	printf("  -n <name>        The name of the code file\n");
	printf("  -o <output>      The output file\n");
	printf("  -t <type>        The input type (0: BASIC program, 1: number array, 2: character array, 3: bytes (default))\n");
}

static ssize_t __write(int fd, const void *buf, size_t n, unsigned char *checksum)
{
	const unsigned char *__buf = buf;

	if (checksum) {
		for (size_t i = 0; i < n; i++)
			*checksum ^= __buf[i];
	}

	if (write(fd, buf, n) != (ssize_t)n)
		return -1;

	return (ssize_t)n;
}

static ssize_t __write1(int fd, unsigned char data, unsigned char *checksum)
{
	return __write(fd, &data, sizeof(data), checksum);
}

#define INPUT_MAX_SIZE 49152 /* 48KiB */

static int mktap(const char *input, const char *output, const char *name, int address, int type)
{
	struct stat st = {};
	char ibuf[INPUT_MAX_SIZE] = {};
	char __name[11] = {};
	unsigned char checksum = 0;
	int ifd = open(input, O_RDONLY);
	int ofd = -1;

	if (ifd < 0) {
		fprintf(stderr, "Error: cannot open '%s': %s\n", input, strerror(errno));
		return EXIT_FAILURE;
	}

	if (fstat(ifd, &st) < 0) {
		perror("stat");
		close(ifd);
		return EXIT_FAILURE;
	}

	if (!st.st_size || st.st_size > INPUT_MAX_SIZE || (address + st.st_size) > 65535) {
		fprintf(stderr, "Error: the input file size is invalid\n");
		close(ifd);
		return EXIT_FAILURE;
	}

	if (read(ifd, ibuf, (size_t)st.st_size) < (ssize_t)st.st_size) {
		perror("read");
		close(ifd);
		return EXIT_FAILURE;
	}

	close(ifd);
	ifd = -1;

	ofd = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (ofd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}

#define WRITE(__fd, __buf, __size, __checksum)                      \
	if (__write(__fd, __buf, (size_t)__size, __checksum) < 0) { \
		close(ofd);                                         \
		return EXIT_FAILURE;                                \
	}

#define WRITE1(__fd, __data, __checksum)                             \
	if (__write1(__fd, (unsigned char)__data, __checksum) < 0) { \
		close(ofd);                                          \
		return EXIT_FAILURE;                                 \
	}

	/* Length */
	WRITE1(ofd, 19, NULL);
	WRITE1(ofd, 0, NULL);

	/* Flag */
	WRITE1(ofd, 0, &checksum);

	/* Type */
	WRITE1(ofd, type, &checksum);

	/* Filename */
	snprintf(__name, sizeof(__name), "%-10s", name);
	for (size_t i = 0; i < (sizeof(__name) - 1); i++)
		__name[i] = (char)toupper(__name[i]);
	WRITE(ofd, __name, sizeof(__name) - 1, &checksum);

	/* Length of data block */
	WRITE1(ofd, st.st_size & 0xff, &checksum);
	WRITE1(ofd, (st.st_size & 0xff00) >> 8, &checksum);

	/* Parameter 1 */
	if (type == 0) {
		/* Program: no auto-run */
		WRITE1(ofd, 0, &checksum);
		WRITE1(ofd, 128, &checksum);
	} else if (type == 3) {
		/* Bytes: start address */
		WRITE1(ofd, address & 0xff, &checksum);
		WRITE1(ofd, (address & 0xff00) >> 8, &checksum);
	} else {
		WRITE1(ofd, 0, &checksum);
		WRITE1(ofd, 128, &checksum);
	}

	/* Parameter 2 */
	WRITE1(ofd, 0, &checksum);
	WRITE1(ofd, 128, &checksum);

	/* Checksum */
	WRITE1(ofd, checksum, NULL);

	checksum = 0;

	/* Length */
	WRITE1(ofd, (st.st_size + 2) & 0xff, NULL);
	WRITE1(ofd, ((st.st_size + 2) & 0xff00) >> 8, NULL);

	/* Flag */
	WRITE1(ofd, 255, &checksum);

	/* Data */
	WRITE(ofd, ibuf, st.st_size, &checksum);

	/* Checksum */
	WRITE1(ofd, checksum, NULL);

#undef WRITE1
#undef WRITE

	close(ofd);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	int opt = -1;
	char *name = NULL;
	char *output = NULL;
	int address = 0;
	int type = 3;

	while ((opt = getopt(argc, argv, "a:hn:o:s:t:")) != -1) {
		switch (opt) {
		case 'a':
			sscanf(optarg, "%i", &address);
			break;
		case 'h':
			help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'n':
			name = optarg;
			break;
		case 'o':
			output = optarg;
			break;
		case 't':
			sscanf(optarg, "%d", &type);
			break;
		default:
			help(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (address < 16384 || address > 65535) {
		fprintf(stderr, "Error: address %d is out of range ([16384, 65536))\n", address);
		exit(EXIT_FAILURE);
	}

	if (!name) {
		fprintf(stderr, "Error: missing -n <name>\n");
		exit(EXIT_FAILURE);
	}

	if (!output) {
		fprintf(stderr, "Error: missing -o <output>\n");
		exit(EXIT_FAILURE);
	}

	if (optind >= argc) {
		fprintf(stderr, "Error: missing input file\n");
		exit(EXIT_FAILURE);
	}

	if (type < 0 || type > 3) {
		fprintf(stderr, "Error: invalid type\n");
		exit(EXIT_FAILURE);
	}

	return mktap(argv[optind], output, name, address, type);
}
