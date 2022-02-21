/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <services/el3_spmc_ffa_memory.h>

#include <platform_def.h>

/*
 * On the FVP platform when using the EL3 SPMC implementation allocate the
 * datastore for tracking shared memory descriptors in the TZC DRAM section
 * to ensure sufficient storage can be allocated.
 * Provide an implementation of the accessor method to allow the datastore
 * details to be retrieved by the SPMC.
 * The SPMC will take care of initializing the memory region.
 */

#define PLAT_SPMC_SHMEM_DATASTORE_SIZE 512 * 1024

__section("arm_el3_tzc_dram") static uint8_t
plat_spmc_shmem_datastore[PLAT_SPMC_SHMEM_DATASTORE_SIZE];

int plat_spmc_shmem_datastore_get(uint8_t **datastore, size_t *size)
{
	*datastore = plat_spmc_shmem_datastore;
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
