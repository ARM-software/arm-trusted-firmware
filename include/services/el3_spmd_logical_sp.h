/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EL3_SPMD_LOGICAL_SP_H
#define EL3_SPMD_LOGICAL_SP_H

#include <common/bl_common.h>
#include <lib/cassert.h>
#include <services/ffa_svc.h>

/*******************************************************************************
 * Structure definition, typedefs & constants for the SPMD Logical Partitions.
 ******************************************************************************/

/* Prototype for SPMD logical partition initializing function. */
typedef int32_t (*ffa_spmd_lp_init_t)(void);

/* SPMD Logical Partition Descriptor. */
struct spmd_lp_desc {
	ffa_spmd_lp_init_t init;
	uint16_t sp_id;
	uint32_t properties;
	uint32_t uuid[4];  /* Little Endian. */
	const char *debug_name;
};

/* Convenience macro to declare a SPMD logical partition descriptor. */
#define DECLARE_SPMD_LOGICAL_PARTITION(_name, _init, _sp_id, _uuid, _properties) \
	static const struct spmd_lp_desc __partition_desc_ ## _name	    \
		__section(".spmd_lp_descs") __used = {			    \
			.debug_name = #_name,				    \
			.init = (_init),				    \
			.sp_id = (_sp_id),				    \
			.uuid = _uuid,					    \
			.properties = (_properties),			    \
		}

IMPORT_SYM(uintptr_t, __SPMD_LP_DESCS_START__,	SPMD_LP_DESCS_START);
IMPORT_SYM(uintptr_t, __SPMD_LP_DESCS_END__,	SPMD_LP_DESCS_END);

#define SPMD_LP_DESCS_COUNT ((SPMD_LP_DESCS_END - SPMD_LP_DESCS_START) \
			  / sizeof(struct spmd_lp_desc))
CASSERT(sizeof(struct spmd_lp_desc) == 40, assert_spmd_lp_desc_size_mismatch);

/*
 * Reserve 63 IDs for SPMD Logical Partitions. Currently, 0xFFC0 to 0xFFFE
 * is reserved.
 */
#define SPMD_LP_ID_END		(SPMD_DIRECT_MSG_ENDPOINT_ID - 1)
#define SPMD_LP_ID_START	(SPMD_LP_ID_END - 62)

static inline bool is_spmd_lp_id(unsigned int id)
{
	return (id >= SPMD_LP_ID_START && id <= SPMD_LP_ID_END);
}

void spmd_logical_sp_set_spmc_initialized(void);
void spmc_logical_sp_set_spmc_failure(void);

int32_t spmd_logical_sp_init(void);

#endif /* EL3_SPMD_LOGICAL_SP_H */
