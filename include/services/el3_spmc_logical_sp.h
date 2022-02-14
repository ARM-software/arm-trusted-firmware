/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EL3_SP_H
#define EL3_SP_H

#include <common/bl_common.h>
#include <lib/cassert.h>

/*******************************************************************************
 * Structure definition, typedefs & constants for the Logical SPs.
 ******************************************************************************/

typedef uint64_t (*direct_msg_handler)(uint32_t smc_fid, bool secure_origin,
				       uint64_t x1, uint64_t x2, uint64_t x3,
				       uint64_t x4, void *cookie, void *handle,
				       uint64_t flags);

/* Prototype for logical partition initializing function. */
typedef int32_t (*ffa_partition_init_t)(void);

/* Logical Partition Descriptor. */
struct el3_lp_desc {
	ffa_partition_init_t init;
	uint16_t sp_id;
	uint32_t properties;
	uint32_t uuid[4];  /* Little Endian. */
	direct_msg_handler direct_req;
	const char *debug_name;
};

/* Convenience macro to declare a logical partition descriptor. */
#define DECLARE_LOGICAL_PARTITION(_name, _init, _sp_id, _uuid, _properties, \
				  _direct_req)				    \
	static const struct el3_lp_desc __partition_desc_ ## _name	    \
		__section("el3_lp_descs") __used = {			    \
			.debug_name = #_name,				    \
			.init = (_init),				    \
			.sp_id = (_sp_id),				    \
			.uuid = _uuid,					    \
			.properties = (_properties),			    \
			.direct_req = (_direct_req),			    \
		}


/*******************************************************************************
 * Function & variable prototypes.
 ******************************************************************************/
int el3_sp_desc_validate(void);
uintptr_t handle_el3_sp(uint32_t smc_fid, void *cookie, void *handle,
						unsigned int flags);
IMPORT_SYM(uintptr_t, __EL3_LP_DESCS_START__,	EL3_LP_DESCS_START);
IMPORT_SYM(uintptr_t, __EL3_LP_DESCS_END__,	EL3_LP_DESCS_END);

#define EL3_LP_DESCS_COUNT ((EL3_LP_DESCS_END - EL3_LP_DESCS_START) \
			  / sizeof(struct el3_lp_desc))

#endif /* EL3_SP_H */
