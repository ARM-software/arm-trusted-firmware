/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stdint.h>

#include <services/drtm_svc.h>

/*
 * This file contains DRTM platform functions which don't really do anything on
 * FVP but are needed for DRTM to function.
 */

uint64_t plat_drtm_get_min_size_normal_world_dce(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_imp_def_dlme_region_size(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_tcb_hash_features(void)
{
	return 0ULL;
}

uint64_t plat_drtm_get_tcb_hash_table_size(void)
{
	return 0ULL;
}
