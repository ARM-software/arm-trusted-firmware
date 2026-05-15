/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_PRIVATE_H
#define FIRME_PRIVATE_H

#include <stdbool.h>
#include <stdint.h>

#include <arch.h>
#include <services/firme_svc.h>

typedef int32_t (*firme_service_version_fn)(firme_instance_e instance);
typedef bool (*firme_service_is_supported_fn)(firme_instance_e instance);
typedef int32_t (*firme_service_init_fn)(void);
typedef int32_t (*firme_service_get_feature_reg_fn)(firme_instance_e instance,
						    uint8_t reg_index,
						    uint64_t *reg);

typedef u_register_t (*firme_service_call_fn)(firme_instance_e inst,
					      uint32_t smc_fid, u_register_t x1,
					      u_register_t x2, u_register_t x3,
					      u_register_t x4, void *cookie,
					      void *handle, u_register_t flags);

struct firme_service {
	uint8_t id;
	firme_service_init_fn init;
	firme_service_version_fn version;
	firme_service_is_supported_fn is_supported;
	firme_service_get_feature_reg_fn get_feature_reg;
	firme_service_call_fn call;
};

extern const struct firme_service firme_base_service;
extern const struct firme_service firme_granule_mgmt_service;
extern const struct firme_service firme_mecid_service;
#if FIRME_SUPPORT_IDE_KM
extern const struct firme_service firme_ide_km_service;
#endif

int32_t firme_service_get_version(firme_service_id_e service_id,
				  firme_instance_e instance);
int32_t firme_service_get_feature_reg(firme_service_id_e service_id,
				      firme_instance_e instance,
				      uint8_t reg_index, uint64_t *reg);
bool firme_service_is_supported(firme_service_id_e service_id,
				firme_instance_e instance);

#endif /* FIRME_PRIVATE_H */
