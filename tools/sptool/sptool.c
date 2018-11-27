/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sptool.h"

#define PAGE_SIZE		4096

/*
 * Linked list of entries describing entries in the secure
 * partition package.
 */
struct sp_entry_info {
	/* Location of the files in the host's RAM. */
	void *sp_data, *rd_data;

	/* Size of the files. */
	uint64_t sp_size, rd_size;

	/* Location of the binary files inside the package output file */
	uint64_t sp_offset, rd_offset;

	struct sp_entry_info *next;
};

static struct sp_entry_info *sp_info_head;

static uint64_t sp_count;

/* Align an address to a power-of-two boundary. */
static unsigned int align_to(unsigned int address, unsigned int boundary)
{
	unsigned int mask = boundary - 1U;

	if ((address & mask) != 0U)
		return (address + boundary) & ~mask;
	else
		return address;
}

/* Allocate a memory area of 'size' bytes and zero it. */
static void *xzalloc(size_t size, const char *msg)
{
	void *d;

	d = malloc(size);
	if (d == NULL) {
		fprintf(stderr, "error: malloc: %s\n", msg);
		exit(1);
	}

	memset(d, 0, size);

	return d;
}

/*
 * Write 'size' bytes from 'buf' into the specified file stream.
 * Exit the program on error.
 */
static void xfwrite(void *buf, size_t size, FILE *fp)
{
	if (fwrite(buf, 1, size, fp) != size) {
		fprintf(stderr, "error: Failed to write to output file.\n");
		exit(1);
	}
}

/*
 * Set the file position indicator for the specified file stream.
 * Exit the program on error.
 */
static void xfseek(FILE *fp, long offset, int whence)
{
	if (fseek(fp, offset, whence) != 0) {
		fprintf(stderr, "error: Failed to set file to offset 0x%lx (%d).\n",
		       offset, whence);
		perror(NULL);
		exit(1);
	}
}

static void cleanup(void)
{
	struct sp_entry_info *sp = sp_info_head;

	while (sp != NULL) {
		struct sp_entry_info *next = sp->next;

		if (sp->sp_data != NULL)
			free(sp->sp_data);

		if (sp->rd_data != NULL)
			free(sp->rd_data);

		free(sp);

		sp = next;
	}

	sp_count = 0;
	sp_info_head = NULL;
}

/*
 * Allocate a buffer big enough to store the content of the specified file and
 * load the file into it. Fill 'size' with the file size. Exit the program on
 * error.
 */
static void load_file(const char *path, void **ptr, uint64_t *size)
{
	FILE *f = fopen(path, "rb");
	if (f == NULL) {
		fprintf(stderr, "error: %s couldn't be opened.\n", path);
		exit(1);
	}

	xfseek(f, 0, SEEK_END);
	*size = ftell(f);
	if (*size == 0) {
		fprintf(stderr, "error: Size of %s is 0\n", path);
		exit(1);
	}

	rewind(f);

	*ptr = malloc(*size);
	if (*ptr == NULL) {
		fprintf(stderr, "error: Not enough memory to load %s\n", path);
		exit(1);
	}

	if (fread(*ptr, *size, 1, f) != 1) {
		fprintf(stderr, "error: Couldn't read %s\n", path);
		exit(1);
	}

	fclose(f);
}

static void load_sp_rd(char *path)
{
	char *split_mark = strstr(path, ":");

	*split_mark = '\0';

	char *sp_path = path;
	char *rd_path = split_mark + 1;

	struct sp_entry_info *sp;

	if (sp_info_head == NULL) {
		sp_info_head = xzalloc(sizeof(struct sp_entry_info),
			"Failed to allocate sp_entry_info struct");

		sp = sp_info_head;
	} else {
		sp = sp_info_head;

		while (sp->next != NULL) {
			sp = sp->next;
		}

		sp->next = xzalloc(sizeof(struct sp_entry_info),
			"Failed to allocate sp_entry_info struct");

		sp = sp->next;
	}

	load_file(sp_path, &sp->sp_data, &sp->sp_size);
	printf("Loaded image file %s (%lu bytes)\n", sp_path, sp->sp_size);

	load_file(rd_path, &sp->rd_data, &sp->rd_size);
	printf("Loaded RD file %s (%lu bytes)\n", rd_path, sp->rd_size);

	sp_count++;
}

static void output_write(const char *path)
{
	struct sp_entry_info *sp;

	if (sp_count == 0) {
		fprintf(stderr, "error: At least one SP must be provided.\n");
		exit(1);
	}

	/* The layout of the structs is specified in the header file sptool.h */

	printf("Writing %lu partitions to output file.\n", sp_count);

	unsigned int header_size = (sizeof(struct sp_pkg_header) * 8)
				 + (sizeof(struct sp_pkg_entry) * 8 * sp_count);

	FILE *f = fopen(path, "wb");
	if (f == NULL) {
		fprintf(stderr, "error: Failed to open %s\n", path);
		exit(1);
	}

	unsigned int file_ptr = align_to(header_size, PAGE_SIZE);

	/* First, save all partition images aligned to page boundaries */

	sp = sp_info_head;

	for (uint64_t i = 0; i < sp_count; i++) {
		xfseek(f, file_ptr, SEEK_SET);

		printf("Writing image %lu to offset 0x%x (0x%lx bytes)\n",
		       i, file_ptr, sp->sp_size);

		sp->sp_offset = file_ptr;
		xfwrite(sp->sp_data, sp->sp_size, f);
		file_ptr = align_to(file_ptr + sp->sp_size, PAGE_SIZE);
		sp = sp->next;
	}

	/* Now, save resource description blobs aligned to 8 bytes */

	sp = sp_info_head;

	for (uint64_t i = 0; i < sp_count; i++) {
		xfseek(f, file_ptr, SEEK_SET);

		printf("Writing RD blob %lu to offset 0x%x (0x%lx bytes)\n",
		       i, file_ptr, sp->rd_size);

		sp->rd_offset = file_ptr;
		xfwrite(sp->rd_data, sp->rd_size, f);
		file_ptr = align_to(file_ptr + sp->rd_size, 8);
		sp = sp->next;
	}

	/* Finally, write header */

	uint64_t version = 0x1;
	uint64_t sp_num = sp_count;

	xfseek(f, 0, SEEK_SET);

	xfwrite(&version, sizeof(uint64_t), f);
	xfwrite(&sp_num, sizeof(uint64_t), f);

	sp = sp_info_head;

	for (unsigned int i = 0; i < sp_count; i++) {

		uint64_t sp_offset, sp_size, rd_offset, rd_size;

		sp_offset = sp->sp_offset;
		sp_size = align_to(sp->sp_size, PAGE_SIZE);
		rd_offset = sp->rd_offset;
		rd_size = sp->rd_size;

		xfwrite(&sp_offset, sizeof(uint64_t), f);
		xfwrite(&sp_size, sizeof(uint64_t), f);
		xfwrite(&rd_offset, sizeof(uint64_t), f);
		xfwrite(&rd_size, sizeof(uint64_t), f);

		sp = sp->next;
	}

	/* All information has been written now */

	fclose(f);
}

static void usage(void)
{
	printf("usage: sptool ");
#ifdef VERSION
	printf(VERSION);
#else
	/* If built from sptool directory, VERSION is not set. */
	printf("version unknown");
#endif
	printf(" [<args>]\n\n");

	printf("This tool takes as inputs several image binary files and the\n"
	       "resource description blobs as input and generates a package\n"
	       "file that contains them.\n\n");
	printf("Commands supported:\n");
	printf("  -o <path>            Set output file path.\n");
	printf("  -i <sp_path:rd_path> Add Secure Partition image and Resource\n"
	       "                       Description blob (specified in two paths\n"
	       "                       separated by a colon).\n");
	printf("  -h                   Show this message.\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int ch;
	const char *outname = NULL;

	while ((ch = getopt(argc, argv, "hi:o:")) != -1) {
		switch (ch) {
		case 'i':
			load_sp_rd(optarg);
			break;
		case 'o':
			outname = optarg;
			break;
		case 'h':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (outname == NULL) {
		fprintf(stderr, "error: An output file path must be provided.\n\n");
		usage();
		return 1;
	}

	output_write(outname);

	cleanup();

	return 0;
}
