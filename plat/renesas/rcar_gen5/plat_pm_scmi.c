/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/scmi.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include "scmi_private.h"

#include <platform_def.h>
#include "rcar_def.h"
#include "rcar_private.h"

/*
 * This file implements the SCP helper functions using SCMI protocol.
 */

#define POWER_ON_STATE		0x00000000U
#define POWER_OFF_STATE		0x40000000U

/*
 * The handles for invoking the SCMI driver APIs after the driver
 * has been initialized.
 */
static void *scmi_handle;

/* The SCMI channel info */
static scmi_channel_t sscmi_channel;

/*
 * Allow use of channel specific lock instead of using a single lock for
 * all the channels.
 */
static RCAR_SCMI_INSTANTIATE_LOCK;

/* The SCMI platform info holder in system ram */
static scmi_channel_plat_info_t scmi_plat_info;

static uint32_t rcar_pwrc_core_pos(u_register_t mpidr)
{
	int cpu;

	cpu = plat_core_pos_by_mpidr(mpidr);
	if (cpu < 0) {
		ERROR("BL3-1 : The value of passed MPIDR is invalid.");
	}

	return (uint32_t)cpu;
}

static void rcar_ring_doorbell(struct scmi_channel_plat_info *plat_info)
{
	u_register_t reg = plat_info->db_reg_addr;
	uint32_t register_value;

	/* Write Access Control Register */
	/* MFIS_MFISASEICR0 Register address setting */
	register_value = MFISWACNTR_SCP_CODEVALUE_SET;
	register_value |= (reg & MFISWACNTR_SCP_REGISTERADDRESS_MASK);
	mmio_write_32(MFIS_MFISWACNTR_SCP, register_value);
	/* Send Interrupt */
	mmio_setbits_32(reg, plat_info->db_modify_mask);
}

/*
 * Function to obtain the SCMI Domain ID and SCMI Channel number from the linear
 * core position. The SCMI Channel number is encoded in the upper 16 bits and
 * the Domain ID is encoded in the lower 16 bits in each entry of the mapping
 * array exported by the platform.
 */
static void css_scp_core_pos_to_scmi_channel(unsigned int core_pos,
		unsigned int *scmi_domain_id, unsigned int *scmi_channel_id)
{
	unsigned int composite_id;

	composite_id = plat_css_core_pos_to_scmi_dmn_id_map[core_pos];

	*scmi_channel_id = GET_SCMI_CHANNEL_ID(composite_id);
	*scmi_domain_id = GET_SCMI_DOMAIN_ID(composite_id);
}

/*
 * Helper function to turn ON a CPU power domain and its parent power domains
 * if applicable.
 */
void rcar_scmi_cpuon(u_register_t mpidr)
{
	unsigned int channel_id, core_pos, domain_id;
	int ret;
	uint32_t scmi_pwr_state = 0;

	core_pos = rcar_pwrc_core_pos(mpidr);
	assert(core_pos < PLATFORM_CORE_COUNT);

	css_scp_core_pos_to_scmi_channel(core_pos, &domain_id,
			&channel_id);
	scmi_pwr_state = POWER_ON_STATE;
	ret = scmi_pwr_state_set(scmi_handle,
		domain_id, scmi_pwr_state);
	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
		      ret);
		panic();
	}
}

/*
 * Helper function to turn off a CPU power domain and its parent power domains
 * if applicable.
 */
void rcar_scmi_cpuoff(const struct psci_power_state *target_state)
{
	unsigned int lvl = 0, channel_id, domain_id;
	uint32_t scmi_pwr_state = 0;
	int ret;

	/* At-least the CPU level should be specified to be OFF */
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
							ARM_LOCAL_STATE_OFF);

	/* PSCI CPU OFF cannot be used to turn OFF system power domain */
	assert(css_system_pwr_state(target_state) == ARM_LOCAL_STATE_RUN);

	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		if (target_state->pwr_domain_state[lvl] == ARM_LOCAL_STATE_RUN)
			break;

		assert(target_state->pwr_domain_state[lvl] ==
							ARM_LOCAL_STATE_OFF);
	}


	scmi_pwr_state = POWER_OFF_STATE;
	css_scp_core_pos_to_scmi_channel(plat_my_core_pos(),
			&domain_id, &channel_id);
	ret = scmi_pwr_state_set(scmi_handle,
		domain_id, scmi_pwr_state);
	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
}

/*
 * Helper function to shutdown the system via SCMI.
 */
void rcar_scmi_sys_shutdown(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(scmi_handle,
			SCMI_SYS_PWR_GRACEFUL_REQ, SCMI_SYS_PWR_SHUTDOWN);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI system power state set (SHUTDOWN) returns 0x%x\n",
				ret);
		panic();
	}
}

/*
 * Helper function to reset the system via SCMI.
 */
void rcar_scmi_sys_reboot(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(scmi_handle,
			SCMI_SYS_PWR_GRACEFUL_REQ, SCMI_SYS_PWR_COLD_RESET);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI system power state set (COLD RESET) returns 0x%x\n",
				ret);
		panic();
	}
}

/*
 * Helper function to suspend the system via SCMI.
 */
void rcar_scmi_sys_suspend(void)
{
	int ret;

	ret = scmi_sys_pwr_state_set(scmi_handle,
			SCMI_SYS_PWR_GRACEFUL_REQ, SCMI_SYS_PWR_SUSPEND);
	if (ret != SCMI_E_SUCCESS) {
		/* Do not out logs and go panic when DRAM backup mode */
		while (true) {
		}
	}
}

void __init plat_rcar_scmi_setup(void)
{
	mailbox_mem_t *mbx_mem;

	INFO("Initializing SCMI driver on Channel\n");

	/* Preset info data to sram variable */
	scmi_plat_info.scmi_mbx_mem = RCAR_SCMI_CHANNEL_BASE;
	scmi_plat_info.db_reg_addr = RCAR_SCMI_MFIS_ADDR;
	scmi_plat_info.db_preserve_mask = RCAR_SCMI_MFIS_PRV_MASK;
	scmi_plat_info.db_modify_mask = RCAR_SCMI_MFIS_MOD_MASK;
	scmi_plat_info.ring_doorbell = rcar_ring_doorbell;

	/* Clean-up and initialize channel memory */
	mbx_mem = (mailbox_mem_t *)(scmi_plat_info.scmi_mbx_mem);
	(void)memset(mbx_mem, 0x0, RCAR_SCMI_CHANNEL_SIZE - 0xF00);

	/* Initialize channel status to free at first */
	mbx_mem->status |= (uint32_t)SCMI_CH_STATUS_FREE_MASK;

	/* Setup and initialize channel */
	sscmi_channel.info = &scmi_plat_info;
	sscmi_channel.lock = RCAR_SCMI_LOCK_GET_INSTANCE;

#if (SET_SCMI_PARAM == 1)
	scmi_handle = scmi_init(&sscmi_channel);

	if (scmi_handle == NULL) {
		ERROR("SCMI Initialization failed on channel\n");
		panic();
	}
#else
	scmi_handle = &sscmi_channel;
#endif
}

const plat_psci_ops_t *plat_rcar_psci_override_pm_ops(plat_psci_ops_t *ops)
{
#if (SET_SCMI_PARAM == 1)
	uint32_t msg_attr;
	int ret;

	/* Check that power domain POWER_STATE_SET message is supported */
	ret = scmi_proto_msg_attr(scmi_handle, SCMI_PWR_DMN_PROTO_ID,
				SCMI_PWR_STATE_SET_MSG, &msg_attr);
	if (ret != SCMI_E_SUCCESS) {
		ERROR("Set power state command is not supported by SCMI\n");
		panic();
	}

	/* Check if the SCMI SYSTEM_POWER_STATE_SET message is supported */
	ret = scmi_proto_msg_attr(scmi_handle, SCMI_SYS_PWR_PROTO_ID,
				SCMI_SYS_PWR_STATE_SET_MSG, &msg_attr);
	if (ret != SCMI_E_SUCCESS) {
		/* System power management operations are not supported */
		ops->system_off = NULL;
		ops->system_reset = NULL;
		ops->get_sys_suspend_power_state = NULL;
	} else {
		if ((msg_attr & (uint32_t)SCMI_SYS_PWR_SUSPEND_SUPPORTED)
				!= (uint32_t)SCMI_SYS_PWR_SUSPEND_SUPPORTED) {
			/*
			 * System power management protocol is available, but
			 * it does not support SYSTEM SUSPEND.
			 */
			ops->get_sys_suspend_power_state = NULL;
		}
	}
#endif

	return ops;
}
