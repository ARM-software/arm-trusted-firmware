/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <services/el3_spmc_ffa_memory.h>

#include <platform_def.h>

IMPORT_SYM(uintptr_t, __PLAT_SPMC_SHMEM_DATASTORE_START__, DATASTORE_BASE);

__section(".arm_el3_tzc_dram") __unused static uint8_t
plat_spmc_shmem_datastore[PLAT_SPMC_SHMEM_DATASTORE_SIZE];

int plat_spmc_shmem_datastore_get(uint8_t **datastore, size_t *size)
{
	*datastore = (uint8_t *)DATASTORE_BASE;
	*size = PLAT_SPMC_SHMEM_DATASTORE_SIZE;
	return 0;
}

/*
 * Add dummy implementations of memory management related platform hooks.
 * These can be used to implement platform specific functionality to support
 * a memory sharing/lending operation.
 *
 * Note: The hooks must be located as part of the initial share request and
 * final reclaim to prevent order dependencies with operations that may take
 * place in the normal world without visibility of the SPMC.
 */
int plat_spmc_shmem_begin(struct ffa_mtd *desc)
{
	return 0;
}
int plat_spmc_shmem_reclaim(struct ffa_mtd *desc)
{
	return 0;
}
