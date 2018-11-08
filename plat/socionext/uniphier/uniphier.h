/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UNIPHIER_H
#define UNIPHIER_H

#include <stdint.h>
#include <string.h>

unsigned int uniphier_get_soc_type(void);
unsigned int uniphier_get_soc_model(void);
unsigned int uniphier_get_soc_revision(void);
unsigned int uniphier_get_soc_id(void);

#define UNIPHIER_SOC_LD11		0
#define UNIPHIER_SOC_LD20		1
#define UNIPHIER_SOC_PXS3		2
#define UNIPHIER_SOC_UNKNOWN		0xffffffff

unsigned int uniphier_get_boot_device(unsigned int soc);

#define UNIPHIER_BOOT_DEVICE_EMMC	0
#define UNIPHIER_BOOT_DEVICE_NAND	1
#define UNIPHIER_BOOT_DEVICE_NOR	2
#define UNIPHIER_BOOT_DEVICE_USB	3
#define UNIPHIER_BOOT_DEVICE_RSV	0xffffffff

unsigned int uniphier_get_boot_master(unsigned int soc);

#define UNIPHIER_BOOT_MASTER_THIS	0
#define UNIPHIER_BOOT_MASTER_SCP	1
#define UNIPHIER_BOOT_MASTER_EXT	2

void uniphier_console_setup(void);

int uniphier_emmc_init(uintptr_t *block_dev_spec);
int uniphier_nand_init(uintptr_t *block_dev_spec);
int uniphier_usb_init(unsigned int soc, uintptr_t *block_dev_spec);

int uniphier_io_setup(unsigned int soc);

struct image_info;
struct image_info *uniphier_get_image_info(unsigned int image_id);

int uniphier_scp_is_running(void);
void uniphier_scp_start(void);
void uniphier_scp_open_com(void);
void uniphier_scp_system_off(void);
void uniphier_scp_system_reset(void);

struct mmap_region;
void uniphier_mmap_setup(uintptr_t total_base, size_t total_size,
			 const struct mmap_region *mmap);

void uniphier_cci_init(unsigned int soc);
void uniphier_cci_enable(void);
void uniphier_cci_disable(void);

void uniphier_gic_driver_init(unsigned int soc);
void uniphier_gic_init(void);
void uniphier_gic_cpuif_enable(void);
void uniphier_gic_cpuif_disable(void);
void uniphier_gic_pcpu_init(void);

unsigned int uniphier_calc_core_pos(u_register_t mpidr);

#define UNIPHIER_NS_DRAM_BASE		0x84000000
#define UNIPHIER_NS_DRAM_LIMIT		0x85000000
#define UNIPHIER_NS_DRAM_SIZE		((UNIPHIER_NS_DRAM_LIMIT) - \
					 (UNIPHIER_NS_DRAM_BASE))

#define UNIPHIER_BL33_BASE		(UNIPHIER_NS_DRAM_BASE)
#define UNIPHIER_BL33_MAX_SIZE		0x00100000

#define UNIPHIER_SCP_BASE		((UNIPHIER_BL33_BASE) + \
					 (UNIPHIER_BL33_MAX_SIZE))
#define UNIPHIER_SCP_MAX_SIZE		0x00020000

#define UNIPHIER_BLOCK_BUF_BASE		((UNIPHIER_SCP_BASE) + \
					 (UNIPHIER_SCP_MAX_SIZE))
#define UNIPHIER_BLOCK_BUF_SIZE		0x00100000

#define UNIPHIER_IMAGE_BUF_BASE		((UNIPHIER_BLOCK_BUF_BASE) + \
					 (UNIPHIER_BLOCK_BUF_SIZE))
#define UNIPHIER_IMAGE_BUF_SIZE		((UNIPHIER_NS_DRAM_LIMIT) - \
					 (UNIPHIER_IMAGE_BUF_BASE))

#endif /* UNIPHIER_H */
