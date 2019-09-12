/*
 * Copyright (c) 2019, Remi Pommarel <repk@triplefau.lt>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <endian.h>

#define DEFAULT_PROGNAME "doimage"
#define PROGNAME(argc, argv) (((argc) >= 1) ? ((argv)[0]) : DEFAULT_PROGNAME)

#define BL31_MAGIC 0x12348765
#define BL31_LOADADDR 0x05100000
#define BUFLEN 512

static inline void usage(char const *prog)
{
	fprintf(stderr, "Usage: %s <bl31.bin> <bl31.img>\n", prog);
}

static inline int fdwrite(int fd, uint8_t *data, size_t len)
{
	ssize_t nr;
	size_t l;
	int ret = -1;

	for (l = 0; l < len; l += nr) {
		nr = write(fd, data + l, len - l);
		if (nr < 0) {
			perror("Cannot write to bl31.img");
			goto out;
		}
	}

	ret = 0;
out:
	return ret;
}

int main(int argc, char **argv)
{
	int fin, fout, ret = -1;
	ssize_t len;
	uint32_t data;
	uint8_t buf[BUFLEN];

	if (argc != 3) {
		usage(PROGNAME(argc, argv));
		goto out;
	}

	fin = open(argv[1], O_RDONLY);
	if (fin < 0) {
		perror("Cannot open bl31.bin");
		goto out;
	}

	fout = open(argv[2], O_WRONLY | O_CREAT, 0660);
	if (fout < 0) {
		perror("Cannot open bl31.img");
		goto closefin;
	}

	data = htole32(BL31_MAGIC);
	if (fdwrite(fout, (uint8_t *)&data, sizeof(data)) < 0)
		goto closefout;

	lseek(fout, 8, SEEK_SET);
	data = htole32(BL31_LOADADDR);
	if (fdwrite(fout, (uint8_t *)&data, sizeof(data)) < 0)
		goto closefout;

	lseek(fout, 0x200, SEEK_SET);
	while ((len = read(fin, buf, sizeof(buf))) > 0)
		if (fdwrite(fout, buf, len) < 0)
			goto closefout;
	if (len < 0) {
		perror("Cannot read bl31.bin");
		goto closefout;
	}

	ret = 0;

closefout:
	close(fout);
closefin:
	close(fin);
out:
	return ret;
}
