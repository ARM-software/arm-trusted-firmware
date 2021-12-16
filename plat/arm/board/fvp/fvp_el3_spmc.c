/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
