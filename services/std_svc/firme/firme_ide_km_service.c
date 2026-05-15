/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "firme_private.h"

#include <arch.h>
#include <arch_features.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <services/firme/firme_ide_km.h>
#include <services/firme_svc.h>
#include <smccc_helpers.h>

/* IDE KM service feature register definitions. */
#define FIRME_IDE_KM_FR0_KEYSET_PROG			BIT(0)
#define FIRME_IDE_KM_FR0_KEYSET_GO			BIT(1)
#define FIRME_IDE_KM_FR0_KEYSET_STOP			BIT(2)

/* IDE KM only has feature register 0. */
#define FIRME_IDE_KM_FEATURE_REG_COUNT			U(1)
#define FIRME_IDE_KM_FR0_DEFAULT	(FIRME_IDE_KM_FR0_KEYSET_PROG	| \
					 FIRME_IDE_KM_FR0_KEYSET_GO	| \
					 FIRME_IDE_KM_FR0_KEYSET_STOP)

/* Max count of root complex count limited by firme */
#define FIRME_IDE_KM_UNITS_MAX				U(8)

/* IDE type used in Input flags */
#define FIRME_IDE_TYPE_SHIFT				U(0)
#define FIRME_IDE_TYPE_WIDTH				U(2)
#define FIRME_IDE_TYPE_PCIE_SELECTIVE_STREAM		U(0)
#define FIRME_IDE_TYPE_PCIE_LINK_STREAM			U(1)
#define FIRME_IDE_TYPE_CXL_CACHEMEM_LINK_STREAM		U(3)

/* KeySet ID */
#define FIRME_KEYSET_ID_KEYSET				BIT(0)
#define FIRME_KEYSET_ID_DIRECTION			BIT(1)
#define FIRME_KEYSET_ID_SUBSTREAM_ID_SHIFT		U(2)
#define FIRME_KEYSET_ID_SUBSTREAM_ID_WIDTH		U(4)
#define FIRME_KEYSET_ID_STREAM_ID_SHIFT			U(6)
#define FIRME_KEYSET_ID_STREAM_ID_WIDTH			U(8)
#define FIRME_KEYSET_ID_ROOTPORT_ID_SHIFT		U(14)
#define FIRME_KEYSET_ID_ROOTPORT_ID_WIDTH		U(16)
#define FIRME_KEYSET_ID_SEGMENT_NUMBER_SHIFT		U(30)
#define FIRME_KEYSET_ID_SEGMENT_NUMBER_WIDTH		U(8)

static uint64_t registers[FIRME_IDE_KM_FEATURE_REG_COUNT] = {
	FIRME_IDE_KM_FR0_DEFAULT
};

#if ENABLE_RMM
#define IDE_KM_INSTANCE_SUPPORT	BIT(FIRME_REALM)
#else
#define IDE_KM_INSTANCE_SUPPORT	BIT(FIRME_NONSECURE)
#endif

struct ide_km_unit {
	spinlock_t lock;
};

CASSERT(((PLAT_PCIE_ROOT_COMPLEX_MAX >= 1) &&
	 (PLAT_PCIE_ROOT_COMPLEX_MAX <= FIRME_IDE_KM_UNITS_MAX)),
	assert_pcie_root_complex_max);
static struct ide_km_unit ide_km_units[PLAT_PCIE_ROOT_COMPLEX_MAX];

static int validate_and_get_rc_idx(uint64_t ecam_address, uint64_t flags,
				   uint64_t keyset_id, int *rc_idx_ret)
{
	unsigned int ide_type;
	int idx;

	(void)keyset_id;

	ide_type = EXTRACT(FIRME_IDE_TYPE, flags);
	if ((ide_type != FIRME_IDE_TYPE_PCIE_SELECTIVE_STREAM) &&
	    (ide_type != FIRME_IDE_TYPE_PCIE_LINK_STREAM)) {
		return FIRME_INVALID_PARAMETERS;
	}

	idx = plat_get_root_complex_index(ecam_address);
	if ((idx < 0) || (idx >= PLAT_PCIE_ROOT_COMPLEX_MAX)) {
		return FIRME_INVALID_PARAMETERS;
	}

	*rc_idx_ret = idx;

	return FIRME_SUCCESS;
}

static int firme_ide_km_keyset_prog(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id, uint64_t keyqw0,
				    uint64_t keyqw1, uint64_t keyqw2,
				    uint64_t keyqw3, uint64_t handle)
{
	int idx;
	int firme_rc, err;

	(void)handle;

	firme_rc = validate_and_get_rc_idx(ecam_address, flags, keyset_id, &idx);
	if (firme_rc != FIRME_SUCCESS) {
		return firme_rc;
	}

	if (!spin_trylock(&ide_km_units[idx].lock)) {
		return FIRME_BUSY;
	}

	/* Call platform layer to program the key in Root Port */
	err = plat_ide_km_keyset_prog(ecam_address, flags, keyset_id, keyqw0,
				      keyqw1, keyqw2, keyqw3);
	firme_rc = firme_errno_from_generic_errno(err);

	assert(firme_rc == FIRME_SUCCESS || firme_rc == FIRME_NOT_SUPPORTED ||
	       firme_rc == FIRME_INVALID_PARAMETERS || firme_rc == FIRME_BUSY ||
	       firme_rc == FIRME_OP_CONFLICT);

	spin_unlock(&ide_km_units[idx].lock);

	return firme_rc;
}

static int firme_ide_km_keyset_action(uint64_t ecam_address, uint64_t flags,
				      uint64_t keyset_id, uint64_t handle,
				      uint64_t action_type)
{
	int idx;
	int firme_rc, err;

	(void)handle;

	firme_rc = validate_and_get_rc_idx(ecam_address, flags, keyset_id, &idx);
	if (firme_rc != FIRME_SUCCESS) {
		return firme_rc;
	}

	if (!spin_trylock(&ide_km_units[idx].lock)) {
		return FIRME_BUSY;
	}

	/* Call platform layer to program the key_go in Root Port */
	if (action_type == FIRME_IDE_KEYSET_GO_FID) {
		err = plat_ide_km_keyset_go(ecam_address, flags, keyset_id);
	} else {
		err = plat_ide_km_keyset_stop(ecam_address, flags, keyset_id);
	}

	firme_rc = firme_errno_from_generic_errno(err);
	assert(firme_rc == FIRME_SUCCESS || firme_rc == FIRME_NOT_SUPPORTED ||
	       firme_rc == FIRME_INVALID_PARAMETERS || firme_rc == FIRME_BUSY ||
	       firme_rc == FIRME_OP_CONFLICT || firme_rc == FIRME_DENIED);

	spin_unlock(&ide_km_units[idx].lock);

	return firme_rc;
}

static int32_t firme_ide_km_service_init(void)
{
	registers[0] = FIRME_IDE_KM_FR0_DEFAULT;

	return FIRME_SUCCESS;
}

static int32_t firme_ide_km_service_version(firme_instance_e instance __unused)
{
	return FIRME_VERSION(FIRME_IDE_KEY_MGMT_VERSION_MAJOR,
			     FIRME_IDE_KEY_MGMT_VERSION_MINOR);
}

static bool firme_ide_km_service_is_supported(firme_instance_e instance)
{
	/*
	 * If RMM is enabled, the NS host uses the IDE KM service exposed by RMM
	 * to establish IDE streams between Root Ports and Endpoints.
	 */
	return (IDE_KM_INSTANCE_SUPPORT & BIT(instance)) != 0U;
}

static int32_t
firme_ide_km_service_get_feature_reg(firme_instance_e instance __unused,
				     uint8_t reg_index, uint64_t *reg)
{
	if (reg == NULL) {
		return FIRME_INVALID_PARAMETERS;
	}

	if (reg_index >= FIRME_IDE_KM_FEATURE_REG_COUNT) {
		return FIRME_NOT_SUPPORTED;
	}

	*reg = registers[reg_index];
	return FIRME_SUCCESS;
}

static u_register_t firme_ide_km_service_handler(firme_instance_e instance,
						 uint32_t smc_fid, uint64_t x1,
						 uint64_t x2, uint64_t x3,
						 uint64_t x4, void *cookie,
						 void *handle, uint64_t flags)
{
	int ret;

	(void)cookie;
	(void)flags;

	switch (smc_fid) {
	case FIRME_IDE_KEYSET_PROG_FID:
		ret = firme_ide_km_keyset_prog(x1, x2, x3, x4,
					       SMC_GET_GP(handle, CTX_GPREG_X5),
					       SMC_GET_GP(handle, CTX_GPREG_X6),
					       SMC_GET_GP(handle, CTX_GPREG_X7),
					       SMC_GET_GP(handle, CTX_GPREG_X8));
		SMC_RET1(handle, ret);
		break;
	case FIRME_IDE_KEYSET_GO_FID:
	case FIRME_IDE_KEYSET_STOP_FID:
		ret = firme_ide_km_keyset_action(x1, x2, x3, x4, smc_fid);
		SMC_RET1(handle, ret);
		break;
	default:
		VERBOSE("FIRME IDE KM FID 0x%X not implemented\n", smc_fid);
		SMC_RET1(handle, FIRME_NOT_SUPPORTED);
	}
}

const struct firme_service firme_ide_km_service = {
	.id = FIRME_IDE_KEY_MGMT_ID,
	.init = firme_ide_km_service_init,
	.version = firme_ide_km_service_version,
	.is_supported = firme_ide_km_service_is_supported,
	.get_feature_reg = firme_ide_km_service_get_feature_reg,
	.call = firme_ide_km_service_handler,
};
