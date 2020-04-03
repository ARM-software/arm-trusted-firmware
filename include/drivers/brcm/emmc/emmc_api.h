/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMMC_API_H
#define EMMC_API_H

#include "bcm_emmc.h"
#include "emmc_pboot_hal_memory_drv.h"

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
/*
 * The erasable unit of the eMMC is the Erase Group
 * Erase group is measured in write blocks which
 * are the basic writable units of the Device
 * EMMC_ERASE_GROUP_SIZE is the number of writeable
 * units (each unit is 512 bytes)
 */

/* Start address (sector) */
#define EMMC_ERASE_START_BLOCK		0x0
/* Number of blocks to be erased */
#define EMMC_ERASE_BLOCK_COUNT		0x1

#define EMMC_ERASE_USER_AREA		0
#define EMMC_ERASE_BOOT_PARTITION1	1
#define EMMC_ERASE_BOOT_PARTITION2	2

/* eMMC partition to be erased */
#define EMMC_ERASE_PARTITION EMMC_ERASE_USER_AREA
#endif

uint32_t bcm_emmc_init(bool card_rdy_only);
void emmc_deinit(void);

#ifdef INCLUDE_EMMC_DRIVER_ERASE_CODE
int emmc_erase(uintptr_t mem_addr, size_t num_of_blocks, uint32_t partition);
#endif

uint32_t emmc_partition_select(uint32_t partition);
uint32_t emmc_read(uintptr_t mem_addr, uintptr_t storage_addr,
		   size_t storage_size, size_t bytes_to_read);
uint32_t emmc_write(uintptr_t mem_addr, uintptr_t data_addr,
		    size_t bytes_to_write);
#endif /* EMMC_API_H */
