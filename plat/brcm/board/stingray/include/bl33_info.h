/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BL33_INFO_H
#define BL33_INFO_H

/* Increase version number each time this file is modified */
#define BL33_INFO_VERSION	4

struct chip_info {
	unsigned int chip_id;
	unsigned int rev_id;
};

struct boot_time_info {
	unsigned int bl1_start;
	unsigned int bl1_end;
	unsigned int bl2_start;
	unsigned int bl2_end;
	unsigned int bl31_start;
	unsigned int bl31_end;
	unsigned int bl32_start;
	unsigned int bl32_end;
	unsigned int bl33_start;
	unsigned int bl33_prompt;
	unsigned int bl33_end;
};

struct bl33_info {
	unsigned int version;
	struct chip_info chip;
	struct boot_time_info boot_time_info;
};

#endif
