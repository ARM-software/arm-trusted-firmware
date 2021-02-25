/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_SCP_BL2_FORMAT_H
#define MSS_SCP_BL2_FORMAT_H

#define MAX_NR_OF_FILES	8
#define FILE_MAGIC	0xddd01ff
#define HEADER_VERSION	0x1

#define MSS_IDRAM_SIZE	0x10000 /* 64KB */
#define MSS_SRAM_SIZE	0x8000 /* 32KB */

/* Types definitions */
typedef struct file_header {
	/* Magic specific for concatenated file (used for validation) */
	uint32_t magic;
	uint32_t nr_of_imgs;	/* Number of images concatenated */
} file_header_t;

/* Types definitions */
enum cm3_t {
	MSS_AP,
	MSS_CP0,
	MSS_CP1,
	MSS_CP2,
	MSS_CP3,
	MG_CP0,
	MG_CP1,
	MG_CP2,
};

typedef struct img_header {
	uint32_t type;		/* CM3 type, can be one of cm3_t */
	uint32_t length;	/* Image length */
	uint32_t version;	/* For sanity checks and future
				 * extended functionality
				 */
} img_header_t;

#endif /* MSS_SCP_BL2_FORMAT_H */
