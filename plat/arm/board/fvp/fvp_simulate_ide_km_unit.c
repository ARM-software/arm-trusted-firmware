/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <plat/common/platform.h>

/*
 * This not a full IDE unit simulation. This mainly covers these two scenerios
 *
 * blocking mode:
 * FVP_SIMULATE_IDE_KM_UNIT == 1  && (PLAT_IDE_KM_PENDING_OPS_MAX == 0)
 * Simulate blocking IDE KM operations. In this mode, these fvp_simulate_* call
 * randomnly returns -EBUSY to simulate an access to MMIO status register is
 * BUSY or not ready. So the caller can re-try the operation after some delay
 *
 * non-blocking mode:
 * FVP_SIMULATE_IDE_KM_UNIT == 1 && (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
 * Simulate non-blocking IDE KM operations. This case is to simulate as if an IDE
 * KM request is delegated to Trusted Subsystem. When PROG/GO/STOP ABIs are
 * called, it records the target completion counter_timer by adding 50 or 100
 * milliseconds to the current counter and returns -EAGAIN.
 *
 * When the caller polls for completion, it returns -EAGAIN if the target
 * completion counter_timer is not met else returns success. This is used to
 * demonstrate a stateful long-running IDE operation through FIRME interface.
 */

#define FVP_KMU_MAX		(PLAT_PCIE_ROOT_COMPLEX_MAX)

/*
 * This timeout is a heuristic value so that the caller can retry after adding
 * delay.
 */
#define IDE_KMU_BUSY_TIMEOUT_MS	(10)

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
#define FVP_KSETS_MAX		(32)
/* heuristic timeout in MS so that the caller can poll after adding delay */
#define IDE_KMU_CMD_TIMEOUT_MS	(10)

struct ide_kset {
	bool active;
	uint64_t keyset_id;
	uint64_t poll_cntpct;
};
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

struct fvp_km_unit {
	uint64_t call_count;
	uint64_t busy_cntpct;
#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	struct ide_kset ide_ksets[FVP_KSETS_MAX];
#endif
};

struct fvp_km_unit fvp_km_units[FVP_KMU_MAX];

static inline uint64_t timeout_ms2cnt(uint32_t ms)
{
	return ((uint64_t)ms * (uint64_t)read_cntfrq_el0()) / 1000ULL;
}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
/* Simulate IDE task delegation to trusted subsystem */
static int tss_delegate_keyset_op(int rc_idx, uint64_t keyset_id,
				  uint32_t delay_ms)
{
	int i;

	for (i = 0; i < FVP_KSETS_MAX; i++) {
		if (fvp_km_units[rc_idx].ide_ksets[i].active) {
			if (fvp_km_units[rc_idx].ide_ksets[i].keyset_id ==
			    keyset_id) {
				return -EINPROGRESS;
			}
			continue;
		}

		/*
		 * Found a free slot, record the keyset_id and target
		 * completion time.
		 */
		fvp_km_units[rc_idx].ide_ksets[i].active = true;
		fvp_km_units[rc_idx].ide_ksets[i].keyset_id = keyset_id;
		fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct =
			read_cntpct_el0() + timeout_ms2cnt(delay_ms);

		VERBOSE("FVP IDE KM [%d]: kset: 0x%lx set poll_timeout: 0x%lx\n",
			rc_idx, keyset_id,
			fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct);

		return 0;
	}

	/* no free slot available */
	return -EBUSY;
}

/* Check for IDE task status from trusted subsystem for a specific keyset_id */
static int tss_delegate_keyset_get_status(int rc_idx, uint64_t keyset_id)
{
	int i;

	for (i = 0; i < FVP_KSETS_MAX; i++) {
		if (!fvp_km_units[rc_idx].ide_ksets[i].active ||
		    fvp_km_units[rc_idx].ide_ksets[i].keyset_id != keyset_id) {
			continue;
		}

		if (read_cntpct_el0() <
		    fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct) {

			/* Upon pending timeout toggle between BUSY and AGAIN */
			if (fvp_km_units[rc_idx].call_count % 2) {
				return -EBUSY;
			} else {
				return -EAGAIN;
			}
		}

		fvp_km_units[rc_idx].ide_ksets[i].active = false;
		fvp_km_units[rc_idx].ide_ksets[i].keyset_id = 0;
		fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct = 0;

		return 0;
	}

	/*
	 * There is no non-blocking operation in progress for the specified
	 * keyset ID
	 */
	return -EACCES;
}

/* Check for IDE task status from trusted subsystem for any keyset_id */
static int tss_delegate_keyset_any_get_status(int rc_idx, uint64_t *keyset_id_ret)
{
	int i;
	bool pending_kset_exists = false;

	/* Pick any keyset that has timedout */
	for (i = 0; i < FVP_KSETS_MAX; i++) {
		if (!fvp_km_units[rc_idx].ide_ksets[i].active) {
			continue;
		}

		if (read_cntpct_el0() <
		    fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct) {
			pending_kset_exists = true;
			continue;
		}

		*keyset_id_ret = fvp_km_units[rc_idx].ide_ksets[i].keyset_id;

		fvp_km_units[rc_idx].ide_ksets[i].active = false;
		fvp_km_units[rc_idx].ide_ksets[i].keyset_id = 0;
		fvp_km_units[rc_idx].ide_ksets[i].poll_cntpct = 0;

		return 0;
	}

	/* todo: need spec clarification for any_keyset poll */
	if (pending_kset_exists) {
		return -EAGAIN;
	}

	/* There is no active keyset that has timedout */
	return -EACCES;
}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */

static bool simulate_km_unit_busy(int rc_idx)
{
	if (read_cntpct_el0() < fvp_km_units[rc_idx].busy_cntpct) {
		return true;
	}

	/* update busy timeout by 50 ms for every 10th iteration */
	if ((fvp_km_units[rc_idx].call_count % 10) == 0) {
		fvp_km_units[rc_idx].busy_cntpct = read_cntpct_el0() +
			timeout_ms2cnt(IDE_KMU_BUSY_TIMEOUT_MS);

		VERBOSE("FVP IDE KM [%d]: set busy_timeout: 0x%lx\n",
			rc_idx, fvp_km_units[rc_idx].busy_cntpct);
	}

	return false;
}

static int fvp_simulate_ide_km_keyset_action(uint64_t ecam_address,
					     uint64_t flags, uint64_t keyset_id)
{
	int rc_idx;
	int rc = 0;

	rc_idx = plat_get_root_complex_index(ecam_address);
	if (rc_idx < 0) {
		return -EINVAL;
	}

	fvp_km_units[rc_idx].call_count++;

	if (simulate_km_unit_busy(rc_idx)) {
		return -EBUSY;
	}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
	/* Simulate IDE task delegation to trusted subsystem */
	rc = tss_delegate_keyset_op(rc_idx, keyset_id, IDE_KMU_CMD_TIMEOUT_MS);
	if (rc == 0) {
		/* once successfully delegated return for caller to poll */
		rc = -EAGAIN;
	}
#endif

	return rc;
}

int fvp_simulate_ide_km_keyset_prog(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id, uint64_t keyqw0,
				    uint64_t keyqw1, uint64_t keyqw2,
				    uint64_t keyqw3)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

int fvp_simulate_ide_km_keyset_go(uint64_t ecam_address, uint64_t flags,
				  uint64_t keyset_id)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

int fvp_simulate_ide_km_keyset_stop(uint64_t ecam_address, uint64_t flags,
				    uint64_t keyset_id)
{
	return fvp_simulate_ide_km_keyset_action(ecam_address, flags,
						 keyset_id);
}

#if (PLAT_IDE_KM_PENDING_OPS_MAX != 0)
/* Poll for specific keyset_id in the ECAM space */
int fvp_simulate_ide_km_keyset_poll(uint64_t ecam_address, uint64_t keyset_id)
{
	int rc_idx;

	rc_idx = plat_get_root_complex_index(ecam_address);
	if (rc_idx < 0) {
		return -EINVAL;
	}

	fvp_km_units[rc_idx].call_count++;

	return tss_delegate_keyset_get_status(rc_idx, keyset_id);
}

/* Poll for any pending keyset_id in the ECAM space and return it on success */
int fvp_simulate_ide_km_poll(uint64_t ecam_address, uint64_t *keyset_id_ret)
{
	int rc_idx;

	rc_idx = plat_get_root_complex_index(ecam_address);
	if (rc_idx < 0) {
		return -EINVAL;
	}

	fvp_km_units[rc_idx].call_count++;

	return tss_delegate_keyset_any_get_status(rc_idx, keyset_id_ret);
}
#endif /* (PLAT_IDE_KM_PENDING_OPS_MAX != 0) */
