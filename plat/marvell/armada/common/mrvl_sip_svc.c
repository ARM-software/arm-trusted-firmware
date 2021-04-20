/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/marvell/cache_llc.h>
#include <drivers/marvell/mochi/ap_setup.h>
#include <drivers/rambus/trng_ip_76.h>
#include <lib/smccc.h>

#include <marvell_plat_priv.h>
#include <plat_marvell.h>

#include "comphy/phy-comphy-cp110.h"
#include "secure_dfx_access/dfx.h"
#include "ddr_phy_access.h"
#include <stdbool.h>

/* #define DEBUG_COMPHY */
#ifdef DEBUG_COMPHY
#define debug(format...) NOTICE(format)
#else
#define debug(format, arg...)
#endif

/* Comphy related FID's */
#define MV_SIP_COMPHY_POWER_ON	0x82000001
#define MV_SIP_COMPHY_POWER_OFF	0x82000002
#define MV_SIP_COMPHY_PLL_LOCK	0x82000003
#define MV_SIP_COMPHY_XFI_TRAIN	0x82000004
#define MV_SIP_COMPHY_DIG_RESET	0x82000005

/* Miscellaneous FID's' */
#define MV_SIP_DRAM_SIZE	0x82000010
#define MV_SIP_LLC_ENABLE	0x82000011
#define MV_SIP_PMU_IRQ_ENABLE	0x82000012
#define MV_SIP_PMU_IRQ_DISABLE	0x82000013
#define MV_SIP_DFX		0x82000014
#define MV_SIP_DDR_PHY_WRITE	0x82000015
#define MV_SIP_DDR_PHY_READ	0x82000016

/* TRNG */
#define MV_SIP_RNG_64		0xC200FF11

#define MAX_LANE_NR		6
#define MVEBU_COMPHY_OFFSET	0x441000
#define MVEBU_CP_BASE_MASK	(~0xffffff)

/* Common PHY register */
#define COMPHY_TRX_TRAIN_CTRL_REG_0_OFFS	0x120a2c

/* This macro is used to identify COMPHY related calls from SMC function ID */
#define is_comphy_fid(fid)	\
	((fid) >= MV_SIP_COMPHY_POWER_ON && (fid) <= MV_SIP_COMPHY_DIG_RESET)

_Bool is_cp_range_valid(u_register_t *addr)
{
	int cp_nr;

	*addr &= MVEBU_CP_BASE_MASK;
	for (cp_nr = 0; cp_nr < CP_NUM; cp_nr++) {
		if (*addr == MVEBU_CP_REGS_BASE(cp_nr))
			return true;
	}

	return false;
}

uintptr_t mrvl_sip_smc_handler(uint32_t smc_fid,
			       u_register_t x1,
			       u_register_t x2,
			       u_register_t x3,
			       u_register_t x4,
			       void *cookie,
			       void *handle,
			       u_register_t flags)
{
	u_register_t ret, read, x5 = x1;
	int i;

	debug("%s: got SMC (0x%x) x1 0x%lx, x2 0x%lx, x3 0x%lx\n",
						 __func__, smc_fid, x1, x2, x3);

	if (is_comphy_fid(smc_fid)) {
		/* validate address passed via x1 */
		if (!is_cp_range_valid(&x1)) {
			ERROR("%s: Wrong smc (0x%x) address: %lx\n",
			      __func__, smc_fid, x1);
			SMC_RET1(handle, SMC_UNK);
		}

		x5 = x1 + COMPHY_TRX_TRAIN_CTRL_REG_0_OFFS;
		x1 += MVEBU_COMPHY_OFFSET;

		if (x2 >= MAX_LANE_NR) {
			ERROR("%s: Wrong smc (0x%x) lane nr: %lx\n",
			      __func__, smc_fid, x2);
			SMC_RET1(handle, SMC_UNK);
		}
	}

	switch (smc_fid) {

	/* Comphy related FID's */
	case MV_SIP_COMPHY_POWER_ON:
		/* x1:  comphy_base, x2: comphy_index, x3: comphy_mode */
		ret = mvebu_cp110_comphy_power_on(x1, x2, x3, x5);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_POWER_OFF:
		/* x1:  comphy_base, x2: comphy_index */
		ret = mvebu_cp110_comphy_power_off(x1, x2, x3);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_PLL_LOCK:
		/* x1:  comphy_base, x2: comphy_index */
		ret = mvebu_cp110_comphy_is_pll_locked(x1, x2);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_XFI_TRAIN:
		/* x1:  comphy_base, x2: comphy_index */
		ret = mvebu_cp110_comphy_xfi_rx_training(x1, x2);
		SMC_RET1(handle, ret);
	case MV_SIP_COMPHY_DIG_RESET:
		/* x1:  comphy_base, x2: comphy_index, x3: mode, x4: command */
		ret = mvebu_cp110_comphy_digital_reset(x1, x2, x3, x4);
		SMC_RET1(handle, ret);

	/* Miscellaneous FID's' */
	case MV_SIP_DRAM_SIZE:
		ret = mvebu_get_dram_size(MVEBU_REGS_BASE);
		SMC_RET1(handle, ret);
	case MV_SIP_LLC_ENABLE:
		for (i = 0; i < ap_get_count(); i++)
			llc_runtime_enable(i);

		SMC_RET1(handle, 0);
#ifdef MVEBU_PMU_IRQ_WA
	case MV_SIP_PMU_IRQ_ENABLE:
		mvebu_pmu_interrupt_enable();
		SMC_RET1(handle, 0);
	case MV_SIP_PMU_IRQ_DISABLE:
		mvebu_pmu_interrupt_disable();
		SMC_RET1(handle, 0);
#endif
	case MV_SIP_DFX:
		if (x1 >= MV_SIP_DFX_THERMAL_INIT &&
		    x1 <= MV_SIP_DFX_THERMAL_SEL_CHANNEL) {
			ret = mvebu_dfx_thermal_handle(x1, &read, x2, x3);
			SMC_RET2(handle, ret, read);
		}
		if (x1 >= MV_SIP_DFX_SREAD && x1 <= MV_SIP_DFX_SWRITE) {
			ret = mvebu_dfx_misc_handle(x1, &read, x2, x3);
			SMC_RET2(handle, ret, read);
		}

		SMC_RET1(handle, SMC_UNK);
	case MV_SIP_DDR_PHY_WRITE:
		ret = mvebu_ddr_phy_write(x1, x2);
		SMC_RET1(handle, ret);
	case MV_SIP_DDR_PHY_READ:
		read = 0;
		ret = mvebu_ddr_phy_read(x1, (uint16_t *)&read);
		SMC_RET2(handle, ret, read);
	case MV_SIP_RNG_64:
		if ((x1 % 2 + 1) > sizeof(read)/4) {
			ERROR("%s: Maximum %ld random bytes per SMC call\n",
			      __func__, sizeof(read));
			SMC_RET1(handle, SMC_UNK);
		}
		ret = eip76_rng_get_random((uint8_t *)&read, 4 * (x1 % 2 + 1));
		SMC_RET2(handle, ret, read);
	default:
		ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Define a runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	marvell_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	mrvl_sip_smc_handler
);
