/*
 * Copyright (c) 2022-2026, Arm Limited and Contributors. All rights reserved.
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
 *
 * Platform implementations may use these hooks to perform ownership and
 * security-state validation of memory regions supplied through FF-A memory
 * sharing/lending operations, as well as any platform-specific access
 * control programming required by the platform security model.
 *
 * Note: These hooks must be invoked as part of the initial share request and
 * final reclaim to prevent ordering dependencies with operations that may
 * take place in the Normal World without visibility of the SPMC.
 */
int plat_spmc_shmem_begin(struct ffa_mtd *desc)
{
	return 0;
}
int plat_spmc_shmem_reclaim(struct ffa_mtd *desc)
{
	return 0;
}
