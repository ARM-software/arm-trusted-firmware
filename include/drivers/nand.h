/*
 * Copyright (c) 2019-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_NAND_H
#define DRIVERS_NAND_H

#include <stddef.h>
#include <stdint.h>

#include <lib/utils_def.h>

#define PSEC_TO_MSEC(x)	div_round_up((x), 1000000000ULL)

struct ecc {
	unsigned int mode; /* ECC mode NAND_ECC_MODE_{NONE|HW|ONDIE} */
	unsigned int size; /* Data byte per ECC step */
	unsigned int bytes; /* ECC bytes per step */
	unsigned int max_bit_corr; /* Max correctible bits per ECC steps */
};

struct nand_device {
	unsigned int block_size;
	unsigned int page_size;
	unsigned long long size;
	unsigned int nb_planes;
	unsigned int buswidth;
	struct ecc ecc;
	int (*mtd_block_is_bad)(unsigned int block);
	int (*mtd_read_page)(struct nand_device *nand, unsigned int page,
			     uintptr_t buffer);
};

/*
 * Read bytes from NAND device
 *
 * @offset: Byte offset to read from in device
 * @buffer: [out] Bytes read from device
 * @length: Number of bytes to read
 * @length_read: [out] Number of bytes read from device
 * Return: 0 on success, a negative errno on failure
 */
int nand_read(unsigned int offset, uintptr_t buffer, size_t length,
	      size_t *length_read);

/*
 * Look for an extra offset to be added in case of bad blocks
 *
 * @base: Base address of the area
 * @offset: Byte offset to read from in device
 * @extra_offset: [out] Extra offset to be added if bad blocks are found
 * Return: 0 on success, a negative errno on failure
 */
int nand_seek_bb(uintptr_t base, unsigned int offset, size_t *extra_offset);

/*
 * Get NAND device instance
 *
 * Return: NAND device instance reference
 */
struct nand_device *get_nand_device(void);

#endif	/* DRIVERS_NAND_H */
