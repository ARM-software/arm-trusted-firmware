/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef OCOTP_H
#define OCOTP_H

#include <stdint.h>

struct otpc_map {
	/* in words. */
	uint32_t otpc_row_size;
	/* 128 bit row / 4 words support. */
	uint16_t data_r_offset[4];
	/* 128 bit row / 4 words support. */
	uint16_t data_w_offset[4];
	int word_size;
	int stride;
};

int bcm_otpc_init(struct otpc_map *map);
int bcm_otpc_read(unsigned int offset, void *val, uint32_t bytes,
		  uint32_t ecc_flag);

#endif /* OCOTP_H */
