/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include "spmc.h"

#include <platform_def.h>

/*******************************************************************************
 * spmc_build_pm_message
 *
 * Builds an SPMC to SP direct message request.
 ******************************************************************************/
static void spmc_build_pm_message(gp_regs_t *gpregs,
				  unsigned long long message,
				  uint8_t pm_msg_type,
				  uint16_t sp_id)
{
	write_ctx_reg(gpregs, CTX_GPREG_X0, FFA_MSG_SEND_DIRECT_REQ_SMC32);
	write_ctx_reg(gpregs, CTX_GPREG_X1,
		      (FFA_SPMC_ID << FFA_DIRECT_MSG_SOURCE_SHIFT) |
		      sp_id);
	write_ctx_reg(gpregs, CTX_GPREG_X2, FFA_FWK_MSG_BIT |
		      (pm_msg_type & FFA_FWK_MSG_MASK));
	write_ctx_reg(gpregs, CTX_GPREG_X3, message);
}

/*******************************************************************************
 * This CPU has been turned on. Enter the SP to initialise S-EL1.
 ******************************************************************************/
static void spmc_cpu_on_finish_handler(u_register_t unused)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	struct sp_exec_ctx *ec;
	unsigned int linear_id = plat_my_core_pos();
	entry_point_info_t sec_ec_ep_info = {0};
	uint64_t rc;

	/* Sanity check for a NULL pointer dereference. */
	assert(sp != NULL);

	/* Initialize entry point information for the SP. */
	SET_PARAM_HEAD(&sec_ec_ep_info, PARAM_EP, VERSION_1,
		       SECURE | EP_ST_ENABLE);

	/*
	 * Check if the primary execution context registered an entry point else
	 * bail out early.
	 * TODO: Add support for boot reason in manifest to allow jumping to
	 * entrypoint into the primary execution context.
	 */
	if (sp->secondary_ep == 0) {
		WARN("%s: No secondary ep on core%u\n", __func__, linear_id);
		return;
	}

	sec_ec_ep_info.pc = sp->secondary_ep;

	/*
	 * Setup and initialise the SP execution context on this physical cpu.
	 */
	spmc_el1_sp_setup(sp, &sec_ec_ep_info);
	spmc_sp_common_ep_commit(sp, &sec_ec_ep_info);

	/* Obtain a reference to the SP execution context. */
	ec = spmc_get_sp_ec(sp);

	/*
	 * TODO: Should we do some PM related state tracking of the SP execution
	 * context here?
	 */

	/* Update the runtime model and state of the partition. */
	ec->rt_model = RT_MODEL_INIT;
	ec->rt_state = RT_STATE_RUNNING;

	INFO("SP (0x%x) init start on core%u.\n", sp->sp_id, linear_id);

	rc = spmc_sp_synchronous_entry(ec);
	if (rc != 0ULL) {
		ERROR("%s failed (%lu) on CPU%u\n", __func__, rc, linear_id);
	}

	/* Update the runtime state of the partition. */
	ec->rt_state = RT_STATE_WAITING;

	VERBOSE("CPU %u on!\n", linear_id);
}
/*******************************************************************************
 * Helper function to send a FF-A power management message to an SP.
 ******************************************************************************/
static int32_t spmc_send_pm_msg(uint8_t pm_msg_type,
				unsigned long long psci_event)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	struct sp_exec_ctx *ec;
	gp_regs_t *gpregs_ctx;
	unsigned int linear_id = plat_my_core_pos();
	u_register_t resp;
	uint64_t rc;

	/* Obtain a reference to the SP execution context. */
	ec = spmc_get_sp_ec(sp);

	/*
	 * TODO: Should we do some PM related state tracking of the SP execution
	 * context here?
	 */

	/*
	 * Build an SPMC to SP direct message request.
	 * Note that x4-x6 should be populated with the original PSCI arguments.
	 */
	spmc_build_pm_message(get_gpregs_ctx(&ec->cpu_ctx),
			      psci_event,
			      pm_msg_type,
			      sp->sp_id);

	/* Sanity check partition state. */
	assert(ec->rt_state == RT_STATE_WAITING);

	/* Update the runtime model and state of the partition. */
	ec->rt_model = RT_MODEL_DIR_REQ;
	ec->rt_state = RT_STATE_RUNNING;

	rc = spmc_sp_synchronous_entry(ec);
	if (rc != 0ULL) {
		ERROR("%s failed (%lu) on CPU%u.\n", __func__, rc, linear_id);
		assert(false);
		return -EINVAL;
	}

	/*
	 * Validate we receive an expected response from the SP.
	 * TODO: We don't currently support aborting an SP in the scenario
	 * where it is misbehaving so assert these conditions are not
	 * met for now.
	 */
	gpregs_ctx = get_gpregs_ctx(&ec->cpu_ctx);

	/* Expect a direct message response from the SP. */
	resp = read_ctx_reg(gpregs_ctx, CTX_GPREG_X0);
	if (resp != FFA_MSG_SEND_DIRECT_RESP_SMC32) {
		ERROR("%s invalid SP response (%lx).\n", __func__, resp);
		assert(false);
		return -EINVAL;
	}

	/* Ensure the sender and receiver are populated correctly. */
	resp = read_ctx_reg(gpregs_ctx, CTX_GPREG_X1);
	if (!(ffa_endpoint_source(resp) == sp->sp_id &&
	      ffa_endpoint_destination(resp) == FFA_SPMC_ID)) {
		ERROR("%s invalid src/dst response (%lx).\n", __func__, resp);
		assert(false);
		return -EINVAL;
	}

	/* Expect a PM message response from the SP. */
	resp = read_ctx_reg(gpregs_ctx, CTX_GPREG_X2);
	if ((resp & FFA_FWK_MSG_BIT) == 0U ||
	    ((resp & FFA_FWK_MSG_MASK) != FFA_PM_MSG_PM_RESP)) {
		ERROR("%s invalid PM response (%lx).\n", __func__, resp);
		assert(false);
		return -EINVAL;
	}

	/* Update the runtime state of the partition. */
	ec->rt_state = RT_STATE_WAITING;

	/* Return the status code returned by the SP */
	return read_ctx_reg(gpregs_ctx, CTX_GPREG_X3);
}

/*******************************************************************************
 * spmc_cpu_suspend_finish_handler
 ******************************************************************************/
static void spmc_cpu_suspend_finish_handler(u_register_t unused)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	unsigned int linear_id = plat_my_core_pos();
	int32_t rc;

	/* Sanity check for a NULL pointer dereference. */
	assert(sp != NULL);

	/*
	 * Check if the SP has subscribed for this power management message.
	 * If not then we don't have anything else to do here.
	 */
	if ((sp->pwr_mgmt_msgs & FFA_PM_MSG_SUB_CPU_SUSPEND_RESUME) == 0U) {
		goto exit;
	}

	rc = spmc_send_pm_msg(FFA_PM_MSG_WB_REQ, FFA_WB_TYPE_NOTS2RAM);
	if (rc < 0) {
		ERROR("%s failed (%d) on CPU%u\n", __func__, rc, linear_id);
		return;
	}

exit:
	VERBOSE("CPU %u resumed!\n", linear_id);
}

/*******************************************************************************
 * spmc_cpu_suspend_handler
 ******************************************************************************/
static void spmc_cpu_suspend_handler(u_register_t unused)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	unsigned int linear_id = plat_my_core_pos();
	int32_t rc;

	/* Sanity check for a NULL pointer dereference. */
	assert(sp != NULL);

	/*
	 * Check if the SP has subscribed for this power management message.
	 * If not then we don't have anything else to do here.
	 */
	if ((sp->pwr_mgmt_msgs & FFA_PM_MSG_SUB_CPU_SUSPEND) == 0U) {
		goto exit;
	}

	rc = spmc_send_pm_msg(FFA_FWK_MSG_PSCI, PSCI_CPU_SUSPEND_AARCH64);
	if (rc < 0) {
		ERROR("%s failed (%d) on CPU%u\n", __func__, rc, linear_id);
		return;
	}
exit:
	VERBOSE("CPU %u suspend!\n", linear_id);
}

/*******************************************************************************
 * spmc_cpu_off_handler
 ******************************************************************************/
static int32_t spmc_cpu_off_handler(u_register_t unused)
{
	struct secure_partition_desc *sp = spmc_get_current_sp_ctx();
	unsigned int linear_id = plat_my_core_pos();
	int32_t ret = 0;

	/* Sanity check for a NULL pointer dereference. */
	assert(sp != NULL);

	/*
	 * Check if the SP has subscribed for this power management message.
	 * If not then we don't have anything else to do here.
	 */
	if ((sp->pwr_mgmt_msgs & FFA_PM_MSG_SUB_CPU_OFF) == 0U) {
		goto exit;
	}

	ret = spmc_send_pm_msg(FFA_FWK_MSG_PSCI, PSCI_CPU_OFF);
	if (ret < 0) {
		ERROR("%s failed (%d) on CPU%u\n", __func__, ret, linear_id);
		return ret;
	}

exit:
	VERBOSE("CPU %u off!\n", linear_id);
	return ret;
}

/*******************************************************************************
 * Structure populated by the SPM Core to perform any bookkeeping before
 * PSCI executes a power mgmt. operation.
 ******************************************************************************/
const spd_pm_ops_t spmc_pm = {
	.svc_on_finish = spmc_cpu_on_finish_handler,
	.svc_off = spmc_cpu_off_handler,
	.svc_suspend = spmc_cpu_suspend_handler,
	.svc_suspend_finish = spmc_cpu_suspend_finish_handler
};
