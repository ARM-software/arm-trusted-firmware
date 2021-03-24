/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_IO_STORAGE_H
#define DDR_IO_STORAGE_H

#include <drivers/io/io_driver.h>

#ifndef PLAT_DDR_FIP_OFFSET
#define PLAT_DDR_FIP_OFFSET	0x800000
#endif

#ifndef PLAT_DDR_FIP_MAX_SIZE
#define PLAT_DDR_FIP_MAX_SIZE	0x32000
#endif

int ddr_fip_setup(const io_dev_connector_t *fip_dev_con, unsigned int boot_dev);
int plat_get_ddr_fip_image_source(unsigned int image_id, uintptr_t *dev_handle,
				  uintptr_t *image_spec,
				  int (*check)(const uintptr_t spec));

#endif	/*	DDR_IO_STORAGE_H	*/
