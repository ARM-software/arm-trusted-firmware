/*
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include <common/debug.h>
#include <lib/bakery_lock.h>
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

	ERROR("exception reason=%u syndrome=0x%llx on 0x%lx at EL3.\n",
		ea_reason, syndrome, read_mpidr_el1());

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

/* Function to enable uncorrectable errors as External abort (SError) */
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
			uint64_t err_ctrl = 0ULL;

			/* enable SError reporting for uncorrectable error */
			ERR_CTLR_ENABLE_FIELD(err_ctrl, UE);
			ERR_CTLR_ENABLE_FIELD(err_ctrl, ED);

			/*
			 * Catch error if something wrong with the RAS aux data
			 * record table.
			 */
			assert(aux_data[j].err_ctrl != NULL);

			/* enable the specified errors */
			err_ctrl |= aux_data[j].err_ctrl();

			/* Write to ERRSELR_EL1 to select the error record */
			ser_sys_select_record(idx_start + j);

			/* enable specified errors */
			write_erxctlr_el1(err_ctrl);

			/*
			 * Check if all the bit settings have been enabled to detect
			 * uncorrected/corrected errors, if not assert.
			 */
			assert(read_erxctlr_el1() == err_ctrl);
		}
	}
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
static int32_t tegra194_ras_node_handler(const struct ras_error *errors, uint64_t status)
{
	bool found = false;
	uint32_t ierr = (uint32_t)ERR_STATUS_GET_FIELD(status, IERR);
	uint32_t serr = (uint32_t)ERR_STATUS_GET_FIELD(status, SERR);

	/* IERR to error message */
	for (uint32_t i = 0; errors[i].error_msg != NULL; i++) {
		if (ierr == errors[i].error_code) {
			ERROR("IERR = %s(0x%x)\n",
				errors[i].error_msg, errors[i].error_code);
			found = true;
			break;
		}
	}
	if (!found) {
		ERROR("unknown IERR: 0x%x\n", ierr);
	}

	ERROR("SERR = %s(0x%x)\n", ras_serr_to_str(serr), serr);

	/* Write to clear reported errors. */
	write_erxstatus_el1(status);

	return 0;
}

/* Function to handle one error node from an error record group. */
static int32_t tegra194_ras_record_handler(const struct err_record_info *info,
		int probe_data, const struct err_handler_data *const data)
{
	uint32_t num_idx = info->sysreg.num_idx;
	uint32_t idx_start = info->sysreg.idx_start;
	const struct ras_aux_data *aux_data = info->aux_data;

	uint64_t status = 0ULL;

	VERBOSE("%s\n", __func__);

	assert(probe_data >= 0);
	assert((uint32_t)probe_data < num_idx);

	uint32_t offset = (uint32_t)probe_data;
	const struct  ras_error *errors = aux_data[offset].error_records;

	assert(errors != NULL);

	/* Write to ERRSELR_EL1 to select the error record */
	ser_sys_select_record(idx_start + offset);

	/* Retrieve status register from the error record */
	status = read_erxstatus_el1();

	assert(ERR_STATUS_GET_FIELD(status, V) != 0U);
	assert(ERR_STATUS_GET_FIELD(status, UE) != 0U);

	return tegra194_ras_node_handler(errors, status);
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

static struct ras_aux_data per_cluster_ras_group[] = {
	PER_CLUSTER_RAS_GROUP_NODES
};

static struct ras_aux_data scf_l3_ras_group[] = {
	SCF_L3_BANK_RAS_GROUP_NODES
};

static struct ras_aux_data ccplex_ras_group[] = {
    CCPLEX_RAS_GROUP_NODES
};

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
	ERROR("Unhandled External Abort received on 0x%llx at EL3!\n",
			read_mpidr_el1());
	ERROR(" exception reason=%u syndrome=0x%lx\n", ea_reason, syndrome);
	panic();
#endif
}
