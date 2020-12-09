/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef FUSE_IO_H
#define FUSE_IO_H

#include <drivers/io/io_driver.h>

/* Can be overridden from platform_def.h file.
 */
#ifndef PLAT_FUSE_FIP_OFFSET
#define PLAT_FUSE_FIP_OFFSET	0x880000
#endif
#ifndef PLAT_FUSE_FIP_MAX_SIZE
#define PLAT_FUSE_FIP_MAX_SIZE	0x80000
#endif

int fip_fuse_provisioning(uintptr_t image_buf, uint32_t size);
int fuse_fip_setup(const io_dev_connector_t *fip_dev_con, unsigned int boot_dev);
int plat_get_fuse_image_source(unsigned int image_id,
			       uintptr_t *dev_handle,
			       uintptr_t *image_spec,
			       int (*check)(const uintptr_t spec));
#endif	/*	FUSE_IO_H	*/
