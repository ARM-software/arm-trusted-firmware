/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/css/common/css_pm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/*
 * This file implements the SCP helper functions using SCMI protocol.
 */

/*
 * SCMI power state parameter bit field encoding for ARM CSS platforms.
 *
 * 31  20 19       16 15      12 11       8 7        4 3         0
 * +-------------------------------------------------------------+
 * | SBZ | Max level |  Level 3 |  Level 2 |  Level 1 |  Level 0 |
 * |     |           |   state  |   state  |   state  |   state  |
 * +-------------------------------------------------------------+
 *
 * `Max level` encodes the highest level that has a valid power state
 * encoded in the power state.
 */
#define SCMI_PWR_STATE_MAX_PWR_LVL_SHIFT	16
#define SCMI_PWR_STATE_MAX_PWR_LVL_WIDTH	4
#define SCMI_PWR_STATE_MAX_PWR_LVL_MASK		\
				((1 << SCMI_PWR_STATE_MAX_PWR_LVL_WIDTH) - 1)
#define SCMI_SET_PWR_STATE_MAX_PWR_LVL(_power_state, _max_level)		\
		(_power_state) |= ((_max_level) & SCMI_PWR_STATE_MAX_PWR_LVL_MASK)\
				<< SCMI_PWR_STATE_MAX_PWR_LVL_SHIFT
#define SCMI_GET_PWR_STATE_MAX_PWR_LVL(_power_state)		\
		(((_power_state) >> SCMI_PWR_STATE_MAX_PWR_LVL_SHIFT)	\
				& SCMI_PWR_STATE_MAX_PWR_LVL_MASK)

#define SCMI_PWR_STATE_LVL_WIDTH		4
#define SCMI_PWR_STATE_LVL_MASK			\
				((1 << SCMI_PWR_STATE_LVL_WIDTH) - 1)
#define SCMI_SET_PWR_STATE_LVL(_power_state, _level, _level_state)		\
		(_power_state) |= ((_level_state) & SCMI_PWR_STATE_LVL_MASK)	\
				<< (SCMI_PWR_STATE_LVL_WIDTH * (_level))
#define SCMI_GET_PWR_STATE_LVL(_power_state, _level)		\
		(((_power_state) >> (SCMI_PWR_STATE_LVL_WIDTH * (_level))) &	\
				SCMI_PWR_STATE_LVL_MASK)

/*
 * The SCMI power state enumeration for a power domain level
 */
typedef enum {
	scmi_power_state_off = 0,
	scmi_power_state_on = 1,
	scmi_power_state_sleep = 2,
} scmi_power_state_t;

/*
 * The global handle for invoking the SCMI driver APIs after the driver
 * has been initialized.
 */
static void *scmi_handle;

/* The SCMI channel global object */
static scmi_channel_t channel;

ARM_SCMI_INSTANTIATE_LOCK;

/*
 * Helper function to suspend a CPU power domain and its parent power domains
 * if applicable.
 */
void css_scp_suspend(const struct psci_power_state *target_state)
{
	int ret;

	/* At least power domain level 0 should be specified to be suspended */
	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
						ARM_LOCAL_STATE_OFF);

	/* Check if power down at system power domain level is requested */
	if (css_system_pwr_state(target_state) == ARM_LOCAL_STATE_OFF) {
		/* Issue SCMI command for SYSTEM_SUSPEND */
		ret = scmi_sys_pwr_state_set(scmi_handle,
				SCMI_SYS_PWR_FORCEFUL_REQ,
				SCMI_SYS_PWR_SUSPEND);
		if (ret != SCMI_E_SUCCESS) {
			ERROR("SCMI system power domain suspend return 0x%x unexpected\n",
					ret);
			panic();
		}
		return;
	}
#if !HW_ASSISTED_COHERENCY
	unsigned int lvl;
	uint32_t scmi_pwr_state = 0;
	/*
	 * If we reach here, then assert that power down at system power domain
	 * level is running.
	 */
	assert(css_system_pwr_state(target_state) == ARM_LOCAL_STATE_RUN);

	/* For level 0, specify `scmi_power_state_sleep` as the power state */
	SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, ARM_PWR_LVL0,
						scmi_power_state_sleep);

	for (lvl = ARM_PWR_LVL1; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		if (target_state->pwr_domain_state[lvl] == ARM_LOCAL_STATE_RUN)
			break;

		assert(target_state->pwr_domain_state[lvl] ==
							ARM_LOCAL_STATE_OFF);
		/*
		 * Specify `scmi_power_state_off` as power state for higher
		 * levels.
		 */
		SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, lvl,
						scmi_power_state_off);
	}

	SCMI_SET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state, lvl - 1);

	ret = scmi_pwr_state_set(scmi_handle,
		plat_css_core_pos_to_scmi_dmn_id_map[plat_my_core_pos()],
		scmi_pwr_state);

	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
#endif
}

/*
 * Helper function to turn off a CPU power domain and its parent power domains
 * if applicable.
 */
void css_scp_off(const struct psci_power_state *target_state)
{
	unsigned int lvl = 0;
	int ret;
	uint32_t scmi_pwr_state = 0;

	/* At-least the CPU level should be specified to be OFF */
	assert(target_state->pwr_domain_state[ARM_PWR_LVL0] ==
							ARM_LOCAL_STATE_OFF);

	/* PSCI CPU OFF cannot be used to turn OFF system power domain */
	assert(css_system_pwr_state(target_state) == ARM_LOCAL_STATE_RUN);

	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		if (target_state->pwr_domain_state[lvl] == ARM_LOCAL_STATE_RUN)
			break;

		assert(target_state->pwr_domain_state[lvl] ==
							ARM_LOCAL_STATE_OFF);
		SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, lvl,
				scmi_power_state_off);
	}

	SCMI_SET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state, lvl - 1);

	ret = scmi_pwr_state_set(scmi_handle,
		plat_css_core_pos_to_scmi_dmn_id_map[plat_my_core_pos()],
		scmi_pwr_state);

	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
}

/*
 * Helper function to turn ON a CPU power domain and its parent power domains
 * if applicable.
 */
void css_scp_on(u_register_t mpidr)
{
	unsigned int lvl = 0;
	int ret, core_pos;
	uint32_t scmi_pwr_state = 0;

	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++)
		SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, lvl,
				scmi_power_state_on);

	SCMI_SET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state, lvl - 1);

	core_pos = plat_core_pos_by_mpidr(mpidr);
	assert(core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT);

	ret = scmi_pwr_state_set(scmi_handle,
		plat_css_core_pos_to_scmi_dmn_id_map[core_pos],
		scmi_pwr_state);

	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
}

/*
 * Helper function to get the power state of a power domain node as reported
 * by the SCP.
 */
int css_scp_get_power_state(u_register_t mpidr, unsigned int power_level)
{
	int ret, cpu_idx;
	uint32_t scmi_pwr_state = 0, lvl_state;

	/* We don't support get power state at the system power domain level */
	if ((power_level > PLAT_MAX_PWR_LVL) ||
			(power_level == CSS_SYSTEM_PWR_DMN_LVL)) {
		WARN("Invalid power level %u specified for SCMI get power state\n",
				power_level);
		return PSCI_E_INVALID_PARAMS;
	}

	cpu_idx = plat_core_pos_by_mpidr(mpidr);
	assert(cpu_idx > -1);

	ret = scmi_pwr_state_get(scmi_handle,
		plat_css_core_pos_to_scmi_dmn_id_map[cpu_idx],
		&scmi_pwr_state);

	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI get power state command return 0x%x unexpected\n",
				ret);
		return PSCI_E_INVALID_PARAMS;
	}

	/*
	 * Find the maximum power level described in the get power state
	 * command. If it is less than the requested power level, then assume
	 * the requested power level is ON.
	 */
	if (SCMI_GET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state) < power_level)
		return HW_ON;

	lvl_state = SCMI_GET_PWR_STATE_LVL(scmi_pwr_state, power_level);
	if (lvl_state == scmi_power_state_on)
		return HW_ON;

	assert((lvl_state == scmi_power_state_off) ||
				(lvl_state == scmi_power_state_sleep));
	return HW_OFF;
}

void __dead2 css_scp_system_off(int state)
{
	int ret;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt from waking
	 * up the AP from WFI.
	 */
	plat_arm_gic_cpuif_disable();

	/*
	 * Issue SCMI command. First issue a graceful
	 * request and if that fails force the request.
	 */
	ret = scmi_sys_pwr_state_set(scmi_handle,
			SCMI_SYS_PWR_FORCEFUL_REQ,
			state);

	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI system power state set 0x%x returns unexpected 0x%x\n",
			state, ret);
		panic();
	}
	wfi();
	ERROR("CSS set power state: operation not handled.\n");
	panic();
}

/*
 * Helper function to shutdown the system via SCMI.
 */
void __dead2 css_scp_sys_shutdown(void)
{
	css_scp_system_off(SCMI_SYS_PWR_SHUTDOWN);
}

/*
 * Helper function to reset the system via SCMI.
 */
void __dead2 css_scp_sys_reboot(void)
{
	css_scp_system_off(SCMI_SYS_PWR_COLD_RESET);
}

static int scmi_ap_core_init(scmi_channel_t *ch)
{
#if PROGRAMMABLE_RESET_ADDRESS
	uint32_t version;
	int ret;

	ret = scmi_proto_version(ch, SCMI_AP_CORE_PROTO_ID, &version);
	if (ret != SCMI_E_SUCCESS) {
		WARN("SCMI AP core protocol version message failed\n");
		return -1;
	}

	if (!is_scmi_version_compatible(SCMI_AP_CORE_PROTO_VER, version)) {
		WARN("SCMI AP core protocol version 0x%x incompatible with driver version 0x%x\n",
			version, SCMI_AP_CORE_PROTO_VER);
		return -1;
	}
	INFO("SCMI AP core protocol version 0x%x detected\n", version);
#endif
	return 0;
}

void __init plat_arm_pwrc_setup(void)
{
	channel.info = plat_css_get_scmi_info();
	channel.lock = ARM_SCMI_LOCK_GET_INSTANCE;
	scmi_handle = scmi_init(&channel);
	if (scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
	if (scmi_ap_core_init(&channel) < 0) {
		ERROR("SCMI AP core protocol initialization failed\n");
		panic();
	}
}

/******************************************************************************
 * This function overrides the default definition for ARM platforms. Initialize
 * the SCMI driver, query capability via SCMI and modify the PSCI capability
 * based on that.
 *****************************************************************************/
const plat_psci_ops_t *css_scmi_override_pm_ops(plat_psci_ops_t *ops)
{
	uint32_t msg_attr;
	int ret;

	assert(scmi_handle);

	/* Check that power domain POWER_STATE_SET message is supported */
	ret = scmi_proto_msg_attr(scmi_handle, SCMI_PWR_DMN_PROTO_ID,
				SCMI_PWR_STATE_SET_MSG, &msg_attr);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("Set power state command is not supported by SCMI\n");
		panic();
	}

	/*
	 * Don't support PSCI NODE_HW_STATE call if SCMI doesn't support
	 * POWER_STATE_GET message.
	 */
	ret = scmi_proto_msg_attr(scmi_handle, SCMI_PWR_DMN_PROTO_ID,
				SCMI_PWR_STATE_GET_MSG, &msg_attr);
	if (ret != SCMI_E_SUCCESS)
		ops->get_node_hw_state = NULL;

	/* Check if the SCMI SYSTEM_POWER_STATE_SET message is supported */
	ret = scmi_proto_msg_attr(scmi_handle, SCMI_SYS_PWR_PROTO_ID,
				SCMI_SYS_PWR_STATE_SET_MSG, &msg_attr);
	if (ret != SCMI_E_SUCCESS) {
		/* System power management operations are not supported */
		ops->system_off = NULL;
		ops->system_reset = NULL;
		ops->get_sys_suspend_power_state = NULL;
	} else {
		if (!(msg_attr & SCMI_SYS_PWR_SUSPEND_SUPPORTED)) {
			/*
			 * System power management protocol is available, but
			 * it does not support SYSTEM SUSPEND.
			 */
			ops->get_sys_suspend_power_state = NULL;
		}
		if (!(msg_attr & SCMI_SYS_PWR_WARM_RESET_SUPPORTED)) {
			/*
			 * WARM reset is not available.
			 */
			ops->system_reset2 = NULL;
		}
	}

	return ops;
}

int css_system_reset2(int is_vendor, int reset_type, u_register_t cookie)
{
	if (is_vendor || (reset_type != PSCI_RESET2_SYSTEM_WARM_RESET))
		return PSCI_E_INVALID_PARAMS;

	css_scp_system_off(SCMI_SYS_PWR_WARM_RESET);
	/*
	 * css_scp_system_off cannot return (it is a __dead function),
	 * but css_system_reset2 has to return some value, even in
	 * this case.
	 */
	return 0;
}

#if PROGRAMMABLE_RESET_ADDRESS
void plat_arm_program_trusted_mailbox(uintptr_t address)
{
	int ret;

	assert(scmi_handle);
	ret = scmi_ap_core_set_reset_addr(scmi_handle, address,
		SCMI_AP_CORE_LOCK_ATTR);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("CSS: Failed to program reset address: %d\n", ret);
		panic();
	}
}
#endif
