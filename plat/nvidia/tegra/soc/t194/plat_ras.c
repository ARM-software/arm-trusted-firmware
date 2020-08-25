/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/bakery_lock.h>
#include <lib/cassert.h>
#include <lib/extensions/ras.h>
#include <lib/utils_def.h>
#include <services/sdei.h>

#include <plat/common/platform.h>
#include <platform_def.h>
#include <tegra194_ras_private.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/*
 * ERR<n>FR bits[63:32], it indicates supported RAS errors which can be enabled
 * by setting corresponding bits in ERR<n>CTLR
 */
#define ERR_FR_EN_BITS_MASK	0xFFFFFFFF00000000ULL

/*
 * Number of RAS errors will be cleared per 'tegra194_ras_corrected_err_clear'
 * function call.
 */
#define RAS_ERRORS_PER_CALL	8

/*
 * the max possible RAS node index value.
 */
#define RAS_NODE_INDEX_MAX	0x1FFFFFFFU

/* bakery lock for platform RAS handler. */
static DEFINE_BAKERY_LOCK(ras_handler_lock);
#define ras_lock()		bakery_lock_get(&ras_handler_lock)
#define ras_unlock()		bakery_lock_release(&ras_handler_lock)

/*
 * Function to handle an External Abort received at EL3.
 * This function is invoked by RAS framework.
 */
static void tegra194_ea_handler(unsigned int ea_reason, uint64_t syndrome,
		void *cookie, void *handle, uint64_t flags)
{
	int32_t ret;

	ras_lock();

	ERROR("MPIDR 0x%lx: exception reason=%u syndrome=0x%" PRIx64 "\n",
		read_mpidr(), ea_reason, syndrome);

	/* Call RAS EA handler */
	ret = ras_ea_handler(ea_reason, syndrome, cookie, handle, flags);
	if (ret != 0) {
		ERROR("RAS error handled!\n");
		ret = sdei_dispatch_event(TEGRA_SDEI_EP_EVENT_0 +
				plat_my_core_pos());
		if (ret != 0)
			ERROR("sdei_dispatch_event returned %d\n", ret);
	} else {
		ERROR("Not a RAS error!\n");
	}

	ras_unlock();
}

/*
 * Function to enable all supported RAS error report.
 *
 * Uncorrected errors are set to report as External abort (SError)
 * Corrected errors are set to report as interrupt.
 */
void tegra194_ras_enable(void)
{
	VERBOSE("%s\n", __func__);

	/* skip RAS enablement if not a silicon platform. */
	if (!tegra_platform_is_silicon()) {
		return;
	}

	/*
	 * Iterate for each group(num_idx ERRSELRs starting from idx_start)
	 * use normal for loop instead of for_each_err_record_info to get rid
	 * of MISRA noise..
	 */
	for (uint32_t i = 0U; i < err_record_mappings.num_err_records; i++) {

		const struct err_record_info *info = &err_record_mappings.err_records[i];

		uint32_t idx_start = info->sysreg.idx_start;
		uint32_t num_idx = info->sysreg.num_idx;
		const struct ras_aux_data *aux_data = (const struct ras_aux_data *)info->aux_data;

		assert(aux_data != NULL);

		for (uint32_t j = 0; j < num_idx; j++) {

			/* ERR<n>CTLR register value. */
			uint64_t err_ctrl = 0ULL;
			/* all supported errors for this node. */
			uint64_t err_fr;
			/* uncorrectable errors */
			uint64_t uncorr_errs;
			/* correctable errors */
			uint64_t corr_errs;

			/*
			 * Catch error if something wrong with the RAS aux data
			 * record table.
			 */
			assert(aux_data[j].err_ctrl != NULL);

			/*
			 * Write to ERRSELR_EL1 to select the RAS error node.
			 * Always program this at first to select corresponding
			 * RAS node before any other RAS register r/w.
			 */
			ser_sys_select_record(idx_start + j);

			err_fr = read_erxfr_el1() & ERR_FR_EN_BITS_MASK;
			uncorr_errs = aux_data[j].err_ctrl();
			corr_errs = ~uncorr_errs & err_fr;

			/* enable error reporting */
			ERR_CTLR_ENABLE_FIELD(err_ctrl, ED);

			/* enable SError reporting for uncorrectable errors */
			if ((uncorr_errs & err_fr) != 0ULL) {
				ERR_CTLR_ENABLE_FIELD(err_ctrl, UE);
			}

			/* generate interrupt for corrected errors. */
			if (corr_errs != 0ULL) {
				ERR_CTLR_ENABLE_FIELD(err_ctrl, CFI);
			}

			/* enable the supported errors */
			err_ctrl |= err_fr;

			VERBOSE("errselr_el1:0x%x, erxfr:0x%" PRIx64 ", err_ctrl:0x%" PRIx64 "\n",
				idx_start + j, err_fr, err_ctrl);

			/* enable specified errors, or set to 0 if no supported error */
			write_erxctlr_el1(err_ctrl);

			/*
			 * Check if all the bit settings have been enabled to detect
			 * uncorrected/corrected errors, if not assert.
			 */
			assert(read_erxctlr_el1() == err_ctrl);
		}
	}
}

/*
 * Function to clear RAS ERR<n>STATUS for corrected RAS error.
 *
 * This function clears number of 'RAS_ERRORS_PER_CALL' RAS errors at most.
 * 'cookie' - in/out cookie parameter to specify/store last visited RAS
 *            error record index. it is set to '0' to indicate no more RAS
 *            error record to clear.
 */
void tegra194_ras_corrected_err_clear(uint64_t *cookie)
{
	/*
	 * 'last_node' and 'last_idx' represent last visited RAS node index from
	 * previous function call. they are set to 0 when first smc call is made
	 * or all RAS error are visited by followed multipile smc calls.
	 */
	union prev_record {
		struct record {
			uint32_t last_node;
			uint32_t last_idx;
		} rec;
		uint64_t value;
	} prev;

	uint64_t clear_ce_status = 0ULL;
	int32_t nerrs_per_call = RAS_ERRORS_PER_CALL;
	uint32_t i;

	if (cookie == NULL) {
		return;
	}

	prev.value = *cookie;

	if ((prev.rec.last_node >= RAS_NODE_INDEX_MAX) ||
		(prev.rec.last_idx >= RAS_NODE_INDEX_MAX)) {
		return;
	}

	ERR_STATUS_SET_FIELD(clear_ce_status, AV, 0x1UL);
	ERR_STATUS_SET_FIELD(clear_ce_status, V, 0x1UL);
	ERR_STATUS_SET_FIELD(clear_ce_status, OF, 0x1UL);
	ERR_STATUS_SET_FIELD(clear_ce_status, MV, 0x1UL);
	ERR_STATUS_SET_FIELD(clear_ce_status, CE, 0x3UL);


	for (i = prev.rec.last_node; i < err_record_mappings.num_err_records; i++) {

		const struct err_record_info *info = &err_record_mappings.err_records[i];
		uint32_t idx_start = info->sysreg.idx_start;
		uint32_t num_idx = info->sysreg.num_idx;

		uint32_t j;

		j = (i == prev.rec.last_node && prev.value != 0UL) ?
				(prev.rec.last_idx + 1U) : 0U;

		for (; j < num_idx; j++) {

			uint64_t status;
			uint32_t err_idx = idx_start + j;

			if (err_idx >= RAS_NODE_INDEX_MAX) {
				return;
			}

			write_errselr_el1(err_idx);
			status = read_erxstatus_el1();

			if (ERR_STATUS_GET_FIELD(status, CE) != 0U) {
				write_erxstatus_el1(clear_ce_status);
			}

			--nerrs_per_call;

			/* only clear 'nerrs_per_call' errors each time. */
			if (nerrs_per_call <= 0) {
				prev.rec.last_idx = j;
				prev.rec.last_node = i;
				/* save last visited error record index
				 * into cookie.
				 */
				*cookie = prev.value;

				return;
			}
		}
	}

	/*
	 * finish if all ras error records are checked or provided index is out
	 * of range.
	 */
	*cookie = 0ULL;
	return;
}

/* Function to probe an error from error record group. */
static int32_t tegra194_ras_record_probe(const struct err_record_info *info,
		int *probe_data)
{
	/* Skip probing if not a silicon platform */
	if (!tegra_platform_is_silicon()) {
		return 0;
	}

	return ser_probe_sysreg(info->sysreg.idx_start, info->sysreg.num_idx, probe_data);
}

/* Function to handle error from one given node */
static int32_t tegra194_ras_node_handler(uint32_t errselr, const char *name,
		const struct ras_error *errors, uint64_t status)
{
	bool found = false;
	uint32_t ierr = (uint32_t)ERR_STATUS_GET_FIELD(status, IERR);
	uint32_t serr = (uint32_t)ERR_STATUS_GET_FIELD(status, SERR);
	uint64_t val = 0;

	/* not a valid error. */
	if (ERR_STATUS_GET_FIELD(status, V) == 0U) {
		return 0;
	}

	ERR_STATUS_SET_FIELD(val, V, 1);

	/* keep the log print same as linux arm64_ras driver. */
	ERROR("**************************************\n");
	ERROR("RAS Error in %s, ERRSELR_EL1=0x%x:\n", name, errselr);
	ERROR("\tStatus = 0x%" PRIx64 "\n", status);

	/* Print uncorrectable errror information. */
	if (ERR_STATUS_GET_FIELD(status, UE) != 0U) {

		ERR_STATUS_SET_FIELD(val, UE, 1);
		ERR_STATUS_SET_FIELD(val, UET, 1);

		/* IERR to error message */
		for (uint32_t i = 0; errors[i].error_msg != NULL; i++) {
			if (ierr == errors[i].error_code) {
				ERROR("\tIERR = %s: 0x%x\n",
					errors[i].error_msg, ierr);

				found = true;
				break;
			}
		}

		if (!found) {
			ERROR("\tUnknown IERR: 0x%x\n", ierr);
		}

		ERROR("SERR = %s: 0x%x\n", ras_serr_to_str(serr), serr);

		/* Overflow, multiple errors have been detected. */
		if (ERR_STATUS_GET_FIELD(status, OF) != 0U) {
			ERROR("\tOverflow (there may be more errors) - "
				"Uncorrectable\n");
			ERR_STATUS_SET_FIELD(val, OF, 1);
		}

		ERROR("\tUncorrectable (this is fatal)\n");

		/* Miscellaneous Register Valid. */
		if (ERR_STATUS_GET_FIELD(status, MV) != 0U) {
			ERROR("\tMISC0 = 0x%lx\n", read_erxmisc0_el1());
			ERROR("\tMISC1 = 0x%lx\n", read_erxmisc1_el1());
			ERR_STATUS_SET_FIELD(val, MV, 1);
		}

		/* Address Valid. */
		if (ERR_STATUS_GET_FIELD(status, AV) != 0U) {
			ERROR("\tADDR = 0x%lx\n", read_erxaddr_el1());
			ERR_STATUS_SET_FIELD(val, AV, 1);
		}

		/* Deferred error */
		if (ERR_STATUS_GET_FIELD(status, DE) != 0U) {
			ERROR("\tDeferred error\n");
			ERR_STATUS_SET_FIELD(val, DE, 1);
		}

	} else {
		/* For corrected error, simply clear it. */
		VERBOSE("corrected RAS error is cleared: ERRSELR_EL1:0x%x, "
			"IERR:0x%x, SERR:0x%x\n", errselr, ierr, serr);
		ERR_STATUS_SET_FIELD(val, CE, 1);
	}

	ERROR("**************************************\n");

	/* Write to clear reported errors. */
	write_erxstatus_el1(val);

	/* error handled */
	return 0;
}

/* Function to handle one error node from an error record group. */
static int32_t tegra194_ras_record_handler(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data __unused)
{
	uint32_t num_idx = info->sysreg.num_idx;
	uint32_t idx_start = info->sysreg.idx_start;
	const struct ras_aux_data *aux_data = info->aux_data;
	const struct ras_error *errors;
	uint32_t offset;
	const char *node_name;

	uint64_t status = 0ULL;

	VERBOSE("%s\n", __func__);

	assert(probe_data >= 0);
	assert((uint32_t)probe_data < num_idx);

	offset = (uint32_t)probe_data;
	errors = aux_data[offset].error_records;
	node_name = aux_data[offset].name;

	assert(errors != NULL);

	/* Write to ERRSELR_EL1 to select the error record */
	ser_sys_select_record(idx_start + offset);

	/* Retrieve status register from the error record */
	status = read_erxstatus_el1();

	return tegra194_ras_node_handler(idx_start + offset, node_name,
			errors, status);
}


/* Instantiate RAS nodes */
PER_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
PER_CLUSTER_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
SCF_L3_BANK_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)
CCPLEX_RAS_NODE_LIST(DEFINE_ONE_RAS_NODE)

/* Instantiate RAS node groups */
static struct ras_aux_data per_core_ras_group[] = {
	PER_CORE_RAS_GROUP_NODES
};
CASSERT(ARRAY_SIZE(per_core_ras_group) < RAS_NODE_INDEX_MAX,
	assert_max_per_core_ras_group_size);

static struct ras_aux_data per_cluster_ras_group[] = {
	PER_CLUSTER_RAS_GROUP_NODES
};
CASSERT(ARRAY_SIZE(per_cluster_ras_group) < RAS_NODE_INDEX_MAX,
	assert_max_per_cluster_ras_group_size);

static struct ras_aux_data scf_l3_ras_group[] = {
	SCF_L3_BANK_RAS_GROUP_NODES
};
CASSERT(ARRAY_SIZE(scf_l3_ras_group) < RAS_NODE_INDEX_MAX,
	assert_max_scf_l3_ras_group_size);

static struct ras_aux_data ccplex_ras_group[] = {
    CCPLEX_RAS_GROUP_NODES
};
CASSERT(ARRAY_SIZE(ccplex_ras_group) < RAS_NODE_INDEX_MAX,
	assert_max_ccplex_ras_group_size);

/*
 * We have same probe and handler for each error record group, use a macro to
 * simply the record definition.
 */
#define ADD_ONE_ERR_GROUP(errselr_start, group) \
	ERR_RECORD_SYSREG_V1((errselr_start), (uint32_t)ARRAY_SIZE((group)), \
			&tegra194_ras_record_probe, \
			&tegra194_ras_record_handler, (group))

/* RAS error record group information */
static struct err_record_info carmel_ras_records[] = {
	/*
	 * Per core ras error records
	 * ERRSELR starts from 0*256 + Logical_CPU_ID*16 + 0 to
	 * 0*256 + Logical_CPU_ID*16 + 5 for each group.
	 * 8 cores/groups, 6 * 8 nodes in total.
	 */
	ADD_ONE_ERR_GROUP(0x000, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x010, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x020, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x030, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x040, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x050, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x060, per_core_ras_group),
	ADD_ONE_ERR_GROUP(0x070, per_core_ras_group),

	/*
	 * Per cluster ras error records
	 * ERRSELR starts from 2*256 + Logical_Cluster_ID*16 + 0 to
	 * 2*256 + Logical_Cluster_ID*16 + 3.
	 * 4 clusters/groups, 3 * 4 nodes in total.
	 */
	ADD_ONE_ERR_GROUP(0x200, per_cluster_ras_group),
	ADD_ONE_ERR_GROUP(0x210, per_cluster_ras_group),
	ADD_ONE_ERR_GROUP(0x220, per_cluster_ras_group),
	ADD_ONE_ERR_GROUP(0x230, per_cluster_ras_group),

	/*
	 * SCF L3_Bank ras error records
	 * ERRSELR: 3*256 + L3_Bank_ID, L3_Bank_ID: 0-3
	 * 1 groups, 4 nodes in total.
	 */
	ADD_ONE_ERR_GROUP(0x300, scf_l3_ras_group),

	/*
	 * CCPLEX ras error records
	 * ERRSELR: 4*256 + Unit_ID, Unit_ID: 0 - 4
	 * 1 groups, 5 nodes in total.
	 */
	ADD_ONE_ERR_GROUP(0x400, ccplex_ras_group),
};

CASSERT(ARRAY_SIZE(carmel_ras_records) < RAS_NODE_INDEX_MAX,
	assert_max_carmel_ras_records_size);

REGISTER_ERR_RECORD_INFO(carmel_ras_records);

/* dummy RAS interrupt */
static struct ras_interrupt carmel_ras_interrupts[] = {};
REGISTER_RAS_INTERRUPTS(carmel_ras_interrupts);

/*******************************************************************************
 * RAS handler for the platform
 ******************************************************************************/
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if RAS_EXTENSION
	tegra194_ea_handler(ea_reason, syndrome, cookie, handle, flags);
#else
	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
#endif
}
