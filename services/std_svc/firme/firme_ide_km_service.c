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
#define FIRME_IDE_KM_FR0_KEYSET_POLL			BIT(3)

/* IDE KM only has feature register 0. */
#define FIRME_IDE_KM_FEATURE_REG_COUNT			U(1)

#if (PLAT_IDE_KM_PENDING_OPS_MAX == 0)
#define FIRME_IDE_KM_FR0_DEFAULT	(FIRME_IDE_KM_FR0_KEYSET_PROG	| \
					 FIRME_IDE_KM_FR0_KEYSET_GO	| \
					 FIRME_IDE_KM_FR0_KEYSET_STOP)
#else
#define FIRME_IDE_KM_FR0_DEFAULT	(FIRME_IDE_KM_FR0_KEYSET_PROG	| \
					 FIRME_IDE_KM_FR0_KEYSET_GO	| \
					 FIRME_IDE_KM_FR0_KEYSET_STOP	| \
					 FIRME_IDE_KM_FR0_KEYSET_POLL)
#endif

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

/*
 * Note:
 * FVP currently does not implement IDE key management at the Root Port, and
 * there is no available platform to validate the implementation details of IDE
 * key management.
 *
 * The code under (PLAT_IDE_KM_PENDING_OPS_MAX != 0) will be refactored once a
 * platform with this support becomes available.
 */
#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)

/* Poll timeout 5 ms */
#define FIRME_KEYSET_POLL_TIMEOUT_US			U(5000)

CASSERT(((PLAT_IDE_KM_PENDING_OPS_MAX >= 1) &&
	 (PLAT_IDE_KM_PENDING_OPS_MAX <= 16)), assert_ide_km_pending_ops_max);

struct ide_km_lro {
	/*
	 * Set to represent the unique keyset operation in progress. Used in
	 * case of non-blocking IDE operation
	 */
	unsigned long keyset_id;

	/*
	 * Implementation defined value that was specified by the caller to
	 * track a non-blocking IDE key configuration operation.
	 */
	unsigned long handle;
};
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

struct ide_km_unit {
	spinlock_t lock;

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	uint64_t lro_bitmap;
	struct ide_km_lro lro[PLAT_IDE_KM_PENDING_OPS_MAX];
#endif
};

CASSERT(((PLAT_PCIE_ROOT_COMPLEX_MAX >= 1) &&
	 (PLAT_PCIE_ROOT_COMPLEX_MAX <= FIRME_IDE_KM_UNITS_MAX)),
	assert_pcie_root_complex_max);
static struct ide_km_unit ide_km_units[PLAT_PCIE_ROOT_COMPLEX_MAX];

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
static int ide_km_lro_reserve(struct ide_km_unit *km)
{
	const uint64_t mask = (1UL << PLAT_IDE_KM_PENDING_OPS_MAX) - 1;
	uint64_t inverted;

	inverted = (~km->lro_bitmap) & mask;

	if (inverted != 0) {
		int lro_idx = (int)__builtin_ctzll(inverted);

		km->lro_bitmap |= (1UL << lro_idx);
		return lro_idx;
	}

	return -1;
}

static void ide_km_lro_release(struct ide_km_unit *km, int lro_idx)
{
	assert((km->lro_bitmap & (1UL << lro_idx)) != 0);

	km->lro_bitmap &= ~(1UL << lro_idx);
	km->lro[lro_idx].handle = 0UL;
	km->lro[lro_idx].keyset_id = 0UL;
}

static void ide_km_lro_set(struct ide_km_unit *km, int lro_idx, uint64_t handle,
			   uint64_t keyset_id)
{
	assert((km->lro_bitmap & (1UL << lro_idx)) != 0);

	km->lro[lro_idx].handle = handle;
	km->lro[lro_idx].keyset_id = keyset_id;
}

/* Query based on keyset_id from active long running operation list */
static int ide_km_lro_query(struct ide_km_unit *km, uint64_t keyset_id)
{
	uint64_t active_lro;

	active_lro = km->lro_bitmap;

	/* Iterate through set bits in the bitmap */
	while (active_lro) {
		int lro_idx = __builtin_ctzll(active_lro);

		if (km->lro[lro_idx].keyset_id == keyset_id) {
			return lro_idx;
		}

		active_lro &= (active_lro - 1);
	}

	return -1;
}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

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
	int lro_idx __unused;
	int firme_rc, err;

	(void)handle;

	firme_rc = validate_and_get_rc_idx(ecam_address, flags, keyset_id, &idx);
	if (firme_rc != FIRME_SUCCESS) {
		return firme_rc;
	}

	if (!spin_trylock(&ide_km_units[idx].lock)) {
		return FIRME_BUSY;
	}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	lro_idx = ide_km_lro_query(&ide_km_units[idx], keyset_id);
	if (lro_idx != -1) {
		firme_rc = FIRME_OP_CONFLICT;
		goto unlock_err_out;
	}

	lro_idx = ide_km_lro_reserve(&ide_km_units[idx]);
	if (lro_idx == -1) {
		firme_rc = FIRME_BUSY;
		goto unlock_err_out;
	}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

	/* Call platform layer to program the key in Root Port */
	err = plat_ide_km_keyset_prog(ecam_address, flags, keyset_id, keyqw0,
				      keyqw1, keyqw2, keyqw3);
	firme_rc = firme_errno_from_generic_errno(err);


	switch (firme_rc) {
	case FIRME_SUCCESS:
	case FIRME_NOT_SUPPORTED:
	case FIRME_INVALID_PARAMETERS:
	case FIRME_BUSY:
	case FIRME_OP_CONFLICT:
#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	case FIRME_INCOMPLETE:
#endif
		break;
	default:
		assert(false);
	}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	if (firme_rc == FIRME_INCOMPLETE) {
		ide_km_lro_set(&ide_km_units[idx], lro_idx, handle, keyset_id);
	} else {
		ide_km_lro_release(&ide_km_units[idx], lro_idx);
	}
unlock_err_out:
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

	spin_unlock(&ide_km_units[idx].lock);

	return firme_rc;
}

static int firme_ide_km_keyset_action(uint64_t ecam_address, uint64_t flags,
				      uint64_t keyset_id, uint64_t handle,
				      uint64_t action_type)
{
	int idx;
	int lro_idx __unused;
	int firme_rc, err;

	(void)handle;

	firme_rc = validate_and_get_rc_idx(ecam_address, flags, keyset_id, &idx);
	if (firme_rc != FIRME_SUCCESS) {
		return firme_rc;
	}

	if (!spin_trylock(&ide_km_units[idx].lock)) {
		return FIRME_BUSY;
	}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	lro_idx = ide_km_lro_query(&ide_km_units[idx], keyset_id);
	if (lro_idx != -1) {
		firme_rc = FIRME_OP_CONFLICT;
		goto unlock_err_out;
	}

	lro_idx = ide_km_lro_reserve(&ide_km_units[idx]);
	if (lro_idx == -1) {
		firme_rc = FIRME_BUSY;
		goto unlock_err_out;
	}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

	/* Call platform layer to program the key_go in Root Port */
	if (action_type == FIRME_IDE_KEYSET_GO_FID) {
		err = plat_ide_km_keyset_go(ecam_address, flags, keyset_id);
	} else {
		err = plat_ide_km_keyset_stop(ecam_address, flags, keyset_id);
	}

	firme_rc = firme_errno_from_generic_errno(err);

	switch (firme_rc) {
	case FIRME_SUCCESS:
	case FIRME_NOT_SUPPORTED:
	case FIRME_INVALID_PARAMETERS:
	case FIRME_BUSY:
	case FIRME_OP_CONFLICT:
	case FIRME_DENIED:
#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	case FIRME_INCOMPLETE:
#endif
		break;
	default:
		assert(false);
	}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	if (firme_rc == FIRME_INCOMPLETE) {
		ide_km_lro_set(&ide_km_units[idx], lro_idx, handle, keyset_id);
	} else {
		ide_km_lro_release(&ide_km_units[idx], lro_idx);
	}
unlock_err_out:
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

	spin_unlock(&ide_km_units[idx].lock);

	return firme_rc;
}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
static u_register_t firme_ide_km_keyset_poll(uint64_t ecam_address,
					     uint64_t flags, uint64_t keyset_id,
					     void *smc_handle)
{
	int idx;
	int lro_idx;
	int firme_rc, err;
	bool match_keyset_id;
	uint64_t timeout;
	uint64_t keyset_id_any = ULONG_MAX;

	idx = plat_get_root_complex_index(ecam_address);
	if ((idx < 0) || (idx >= PLAT_PCIE_ROOT_COMPLEX_MAX)) {
		SMC_RET1(smc_handle, FIRME_INVALID_PARAMETERS);
	}

	if (!spin_trylock(&ide_km_units[idx].lock)) {
		SMC_RET1(smc_handle, FIRME_BUSY);
	}

	if ((flags & BIT(0)) == 0U) {
		lro_idx = ide_km_lro_query(&ide_km_units[idx], keyset_id);
		if (lro_idx == -1) {
			firme_rc = FIRME_DENIED;
			goto unlock_err_out;
		}

		match_keyset_id = true;
	} else {
		match_keyset_id = false;
	}

	/* Set 5 ms timeout value */
	timeout = timeout_init_us(FIRME_KEYSET_POLL_TIMEOUT_US);
	do {
		if (match_keyset_id) {
			err = plat_ide_km_keyset_poll(ecam_address, keyset_id);
		} else {
			err = plat_ide_km_poll(ecam_address, &keyset_id_any);
		}

		firme_rc = firme_errno_from_generic_errno(err);

		if ((firme_rc != FIRME_INCOMPLETE) &&
		    (firme_rc != FIRME_BUSY)) {
			break;
		}
	} while (!timeout_elapsed(timeout));

	assert(firme_rc == FIRME_SUCCESS || firme_rc == FIRME_NOT_SUPPORTED ||
	       firme_rc == FIRME_INVALID_PARAMETERS || firme_rc == FIRME_BUSY ||
	       firme_rc == FIRME_ABORTED || firme_rc == FIRME_DENIED ||
	       firme_rc == FIRME_INCOMPLETE);

	if ((firme_rc == FIRME_SUCCESS) || (firme_rc == FIRME_ABORTED)) {
		if (!match_keyset_id) {
			int lro_idx_any;

			/*
			 * For poll any keyset_id case, if the platform returns
			 * ABORTED a valid keyset_id must be returned.
			 */
			lro_idx_any = ide_km_lro_query(&ide_km_units[idx],
						       keyset_id_any);

			/*
			 * There must be an entry if poll is for any keyset_id
			 * and platform layer successfully polled a response
			 * for any pending keyset in the ECAM address.
			 */
			assert(lro_idx_any != -1);
			if (lro_idx_any == -1) {
				firme_rc = FIRME_DENIED;
				goto unlock_err_out;
			}

			if (firme_rc == FIRME_SUCCESS) {
				SMC_SET_GP(smc_handle, CTX_GPREG_X1,
					   ide_km_units[idx].lro[lro_idx_any].handle);
				SMC_SET_GP(smc_handle, CTX_GPREG_X2,
					   keyset_id_any);
			}

			/* Release the matched LRO for any keyset ID */
			ide_km_lro_release(&ide_km_units[idx], lro_idx_any);
		} else {
			if (firme_rc == FIRME_SUCCESS) {
				SMC_SET_GP(smc_handle, CTX_GPREG_X1,
					   ide_km_units[idx].lro[lro_idx].handle);
			}

			/* Release the matched LRO */
			ide_km_lro_release(&ide_km_units[idx], lro_idx);
		}
	}

unlock_err_out:
	spin_unlock(&ide_km_units[idx].lock);
	SMC_RET1(smc_handle, firme_rc);
}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

int firme_ide_km_service_init(void)
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
#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	case FIRME_IDE_KEYSET_POLL_FID:
		return firme_ide_km_keyset_poll(x1, x2, x3, handle);
		break;
#endif
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
