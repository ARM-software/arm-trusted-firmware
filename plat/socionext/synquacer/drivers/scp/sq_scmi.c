/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/css_mhu_doorbell.h>
#include <drivers/arm/css/css_scp.h>
#include <drivers/arm/css/scmi.h>
#include <plat/arm/css/common/css_pm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

#include <scmi_sq.h>
#include <sq_common.h>

/*
 * This file implements the SCP helper functions using SCMI protocol.
 */

DEFINE_BAKERY_LOCK(sq_scmi_lock);
#define SQ_SCMI_LOCK_GET_INSTANCE	(&sq_scmi_lock)

#define SQ_SCMI_PAYLOAD_BASE		PLAT_SQ_SCP_COM_SHARED_MEM_BASE
#define MHU_CPU_INTR_S_SET_OFFSET	0x308

const uint32_t sq_core_pos_to_scmi_dmn_id_map[PLATFORM_CORE_COUNT] = {
	0,   1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23
};

static scmi_channel_plat_info_t sq_scmi_plat_info = {
		.scmi_mbx_mem = SQ_SCMI_PAYLOAD_BASE,
		.db_reg_addr = PLAT_SQ_MHU_BASE + MHU_CPU_INTR_S_SET_OFFSET,
		.db_preserve_mask = 0xfffffffe,
		.db_modify_mask = 0x1,
		.ring_doorbell = &mhu_ring_doorbell,
};

/*
 * SCMI power state parameter bit field encoding for SynQuacer platform.
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
static void *sq_scmi_handle;

/* The SCMI channel global object */
static scmi_channel_t channel;

/*
 * Helper function to turn off a CPU power domain and
 * its parent power domains if applicable.
 */
void sq_scmi_off(const struct psci_power_state *target_state)
{
	int lvl = 0, ret;
	uint32_t scmi_pwr_state = 0;

	/* At-least the CPU level should be specified to be OFF */
	assert(target_state->pwr_domain_state[SQ_PWR_LVL0] ==
							SQ_LOCAL_STATE_OFF);

	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++) {
		if (target_state->pwr_domain_state[lvl] == SQ_LOCAL_STATE_RUN)
			break;

		assert(target_state->pwr_domain_state[lvl] ==
							SQ_LOCAL_STATE_OFF);
		SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, lvl,
				scmi_power_state_off);
	}

	SCMI_SET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state, lvl - 1);

	ret = scmi_pwr_state_set(sq_scmi_handle,
		sq_core_pos_to_scmi_dmn_id_map[plat_my_core_pos()],
		scmi_pwr_state);

	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
}

/*
 * Helper function to turn ON a CPU power domain and
 *its parent power domains if applicable.
 */
void sq_scmi_on(u_register_t mpidr)
{
	int lvl = 0, ret, core_pos;
	uint32_t scmi_pwr_state = 0;

	for (; lvl <= PLAT_MAX_PWR_LVL; lvl++)
		SCMI_SET_PWR_STATE_LVL(scmi_pwr_state, lvl,
				scmi_power_state_on);

	SCMI_SET_PWR_STATE_MAX_PWR_LVL(scmi_pwr_state, lvl - 1);

	core_pos = plat_core_pos_by_mpidr(mpidr);
	assert(core_pos >= 0 && core_pos < PLATFORM_CORE_COUNT);

	ret = scmi_pwr_state_set(sq_scmi_handle,
		sq_core_pos_to_scmi_dmn_id_map[core_pos],
		scmi_pwr_state);

	if (ret != SCMI_E_QUEUED && ret != SCMI_E_SUCCESS) {
		ERROR("SCMI set power state command return 0x%x unexpected\n",
				ret);
		panic();
	}
}

void __dead2 sq_scmi_system_off(int state)
{
	int ret;

	/*
	 * Disable GIC CPU interface to prevent pending interrupt from waking
	 * up the AP from WFI.
	 */
	sq_gic_cpuif_disable();

	/*
	 * Issue SCMI command. First issue a graceful
	 * request and if that fails force the request.
	 */
	ret = scmi_sys_pwr_state_set(sq_scmi_handle,
			SCMI_SYS_PWR_FORCEFUL_REQ,
			state);

	if (ret != SCMI_E_SUCCESS) {
		ERROR("SCMI system power state set 0x%x returns unexpected 0x%x\n",
			state, ret);
		panic();
	}
	wfi();
	ERROR("SCMI set power state: operation not handled.\n");
	panic();
}

/*
 * Helper function to reset the system via SCMI.
 */
void __dead2 sq_scmi_sys_shutdown(void)
{
	sq_scmi_system_off(SCMI_SYS_PWR_SHUTDOWN);
}

void __dead2 sq_scmi_sys_reboot(void)
{
	sq_scmi_system_off(SCMI_SYS_PWR_COLD_RESET);
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

void __init plat_sq_pwrc_setup(void)
{
	channel.info = &sq_scmi_plat_info;
	channel.lock = SQ_SCMI_LOCK_GET_INSTANCE;
	sq_scmi_handle = scmi_init(&channel);
	if (sq_scmi_handle == NULL) {
		ERROR("SCMI Initialization failed\n");
		panic();
	}
	if (scmi_ap_core_init(&channel) < 0) {
		ERROR("SCMI AP core protocol initialization failed\n");
		panic();
	}
}

uint32_t sq_scmi_get_draminfo(struct draminfo *info)
{
	scmi_get_draminfo(sq_scmi_handle, info);

	return 0;
}
