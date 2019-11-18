/*
 * Copyright (c) 2017-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm/byteorder.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Magic = 'S' 'T' 'M' 0x32 */
#define HEADER_MAGIC		__be32_to_cpu(0x53544D32)
#define VER_MAJOR		2
#define VER_MINOR		1
#define VER_VARIANT		0
#define HEADER_VERSION_V1	0x1
#define TF_BINARY_TYPE		0x10

/* Default option : bit0 => no signature */
#define HEADER_DEFAULT_OPTION	(__cpu_to_le32(0x00000001))

struct stm32_header {
	uint32_t magic_number;
	uint8_t image_signature[64];
	uint32_t image_checksum;
	uint8_t  header_version[4];
	uint32_t image_length;
	uint32_t image_entry_point;
	uint32_t reserved1;
	uint32_t load_address;
	uint32_t reserved2;
	uint32_t version_number;
	uint32_t option_flags;
	uint32_t ecdsa_algorithm;
	uint8_t ecdsa_public_key[64];
	uint8_t padding[83];
	uint8_t binary_type;
};

static void stm32image_default_header(struct stm32_header *ptr)
{
	if (!ptr) {
		return;
	}

	ptr->magic_number = HEADER_MAGIC;
	ptr->option_flags = HEADER_DEFAULT_OPTION;
	ptr->ecdsa_algorithm = __cpu_to_le32(1);
	ptr->version_number = __cpu_to_le32(0);
	ptr->binary_type = TF_BINARY_TYPE;
}

static uint32_t stm32image_checksum(void *start, uint32_t len)
{
	uint32_t csum = 0;
	uint32_t hdr_len = sizeof(struct stm32_header);
	uint8_t *p;

	if (len < hdr_len) {
		return 0;
	}

	p = (unsigned char *)start + hdr_len;
	len -= hdr_len;

	while (len > 0) {
		csum += *p;
		p++;
		len--;
	}

	return csum;
}

static void stm32image_print_header(const void *ptr)
{
	struct stm32_header *stm32hdr = (struct stm32_header *)ptr;

	printf("Image Type   : ST Microelectronics STM32 V%d.%d\n",
	       stm32hdr->header_version[VER_MAJOR],
	       stm32hdr->header_version[VER_MINOR]);
	printf("Image Size   : %lu bytes\n",
	       (unsigned long)__le32_to_cpu(stm32hdr->image_length));
	printf("Image Load   : 0x%08x\n",
	       __le32_to_cpu(stm32hdr->load_address));
	printf("Entry Point  : 0x%08x\n",
	       __le32_to_cpu(stm32hdr->image_entry_point));
	printf("Checksum     : 0x%08x\n",
	       __le32_to_cpu(stm32hdr->image_checksum));
	printf("Option     : 0x%08x\n",
	       __le32_to_cpu(stm32hdr->option_flags));
	printf("Version	   : 0x%08x\n",
	       __le32_to_cpu(stm32hdr->version_number));
}

static void stm32image_set_header(void *ptr, struct stat *sbuf, int ifd,
				  uint32_t loadaddr, uint32_t ep, uint32_t ver,
				  uint32_t major, uint32_t minor)
{
	struct stm32_header *stm32hdr = (struct stm32_header *)ptr;

	stm32image_default_header(stm32hdr);

	stm32hdr->header_version[VER_MAJOR] = major;
	stm32hdr->header_version[VER_MINOR] = minor;
	stm32hdr->load_address = __cpu_to_le32(loadaddr);
	stm32hdr->image_entry_point = __cpu_to_le32(ep);
	stm32hdr->image_length = __cpu_to_le32((uint32_t)sbuf->st_size -
					     sizeof(struct stm32_header));
	stm32hdr->image_checksum =
		__cpu_to_le32(stm32image_checksum(ptr, sbuf->st_size));
	stm32hdr->version_number = __cpu_to_le32(ver);
}

static int stm32image_create_header_file(char *srcname, char *destname,
					 uint32_t loadaddr, uint32_t entry,
					 uint32_t version, uint32_t major,
					 uint32_t minor)
{
	int src_fd, dest_fd;
	struct stat sbuf;
	unsigned char *ptr;
	struct stm32_header stm32image_header;

	dest_fd = open(destname, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0666);
	if (dest_fd == -1) {
		fprintf(stderr, "Can't open %s: %s\n", destname,
			strerror(errno));
		return -1;
	}

	src_fd = open(srcname, O_RDONLY);
	if (src_fd == -1) {
		fprintf(stderr, "Can't open %s: %s\n", srcname,
			strerror(errno));
		return -1;
	}

	if (fstat(src_fd, &sbuf) < 0) {
		return -1;
	}

	ptr = mmap(NULL, sbuf.st_size, PROT_READ, MAP_SHARED, src_fd, 0);
	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't read %s\n", srcname);
		return -1;
	}

	memset(&stm32image_header, 0, sizeof(struct stm32_header));

	if (write(dest_fd, &stm32image_header, sizeof(struct stm32_header)) !=
	    sizeof(struct stm32_header)) {
		fprintf(stderr, "Write error %s: %s\n", destname,
			strerror(errno));
		return -1;
	}

	if (write(dest_fd, ptr, sbuf.st_size) != sbuf.st_size) {
		fprintf(stderr, "Write error on %s: %s\n", destname,
			strerror(errno));
		return -1;
	}

	munmap((void *)ptr, sbuf.st_size);
	close(src_fd);

	if (fstat(dest_fd, &sbuf) < 0) {
		return -1;
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   dest_fd, 0);

	if (ptr == MAP_FAILED) {
		fprintf(stderr, "Can't write %s\n", destname);
		return -1;
	}

	stm32image_set_header(ptr, &sbuf, dest_fd, loadaddr, entry, version,
			      major, minor);

	stm32image_print_header(ptr);

	munmap((void *)ptr, sbuf.st_size);
	close(dest_fd);
	return 0;
}

int main(int argc, char *argv[])
{
	int opt, loadaddr = -1, entry = -1, err = 0, version = 0;
	int major = HEADER_VERSION_V1;
	int minor = 0;
	char *dest = NULL, *src = NULL;

	while ((opt = getopt(argc, argv, ":s:d:l:e:v:m:n:")) != -1) {
		switch (opt) {
		case 's':
			src = optarg;
			break;
		case 'd':
			dest = optarg;
			break;
		case 'l':
			loadaddr = strtol(optarg, NULL, 0);
			break;
		case 'e':
			entry = strtol(optarg, NULL, 0);
			break;
		case 'v':
			version = strtol(optarg, NULL, 0);
			break;
		case 'm':
			major = strtol(optarg, NULL, 0);
			break;
		case 'n':
			minor = strtol(optarg, NULL, 0);
			break;
		default:
			fprintf(stderr,
				"Usage : %s [-s srcfile] [-d destfile] [-l loadaddr] [-e entry_point] [-m major] [-n minor]\n",
					argv[0]);
			return -1;
		}
	}

	if (!src) {
		fprintf(stderr, "Missing -s option\n");
		return -1;
	}

	if (!dest) {
		fprintf(stderr, "Missing -d option\n");
		return -1;
	}

	if (loadaddr == -1) {
		fprintf(stderr, "Missing -l option\n");
		return -1;
	}

	if (entry == -1) {
		fprintf(stderr, "Missing -e option\n");
		return -1;
	}

	err = stm32image_create_header_file(src, dest, loadaddr,
					    entry, version, major, minor);

	return err;
}
