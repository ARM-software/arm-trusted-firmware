/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <drivers/delay_timer.h>
#include <drivers/scmi.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include <plat_private.h>
#include "rk3588_clk.h"
#include <scmi_rstd.h>
#include <soc.h>

#define DEFAULT_RESET_DOM_ATTRIBUTE	0

#define RK3588_SCMI_RESET(_id, _name, _attribute, _ops)		\
{								\
	.id = _id,						\
	.name = _name,						\
	.attribute = _attribute,				\
	.rstd_ops = _ops,					\
}

static int rk3588_reset_explicit(rk_scmi_rstd_t *reset_domain,
				 bool assert_not_deassert)
{
	int bank = reset_domain->id / 16;
	int offset = reset_domain->id % 16;

	mmio_write_32(SCRU_BASE + CRU_SOFTRST_CON(bank),
		      BITS_WITH_WMASK(assert_not_deassert, 0x1U, offset));
	return SCMI_SUCCESS;
}

static struct rk_scmi_rstd_ops rk3588_reset_domain_ops = {
	.reset_explicit = rk3588_reset_explicit,
};

static rk_scmi_rstd_t rk3588_reset_domain_table[] = {
	RK3588_SCMI_RESET(SRST_CRYPTO_CORE, "scmi_sr_cy_core", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_CRYPTO_PKA, "scmi_sr_cy_pka", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_CRYPTO_RNG, "scmi_sr_cy_rng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_A_CRYPTO, "scmi_sr_a_cy", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_CRYPTO, "scmi_sr_h_cy", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_KEYLADDER_CORE, "scmi_sr_k_core", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_KEYLADDER_RNG, "scmi_sr_k_rng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_P_OTPC_S, "scmi_sr_p_otp", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_OTPC_S, "scmi_sr_otp", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_WDT_S, "scmi_sr_wdt", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_T_WDT_S, "scmi_sr_t_wdt", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_BOOTROM, "scmi_sr_h_boot", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_P_KEYLADDER, "scmi_sr_p_ky", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_TRNG_S, "scmi_sr_h_trng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_TRNG_NS, "scmi_sr_t_trng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_D_SDMMC_BUFFER, "scmi_sr_d_sd", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_SDMMC, "scmi_sr_h_sd", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_H_SDMMC_BUFFER, "scmi_sr_h_sd_b", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_SDMMC, "scmi_sr_sd", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_P_TRNG_CHK, "scmi_sr_p_trng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_TRNG_S, "scmi_sr_trng", DEFAULT_RESET_DOM_ATTRIBUTE, &rk3588_reset_domain_ops),
	RK3588_SCMI_RESET(SRST_INVALID, "scmi_sr_invalid", DEFAULT_RESET_DOM_ATTRIBUTE, NULL),
};

static rk_scmi_rstd_t *
rockchip_get_reset_domain_table(int id)
{
	rk_scmi_rstd_t *reset = rk3588_reset_domain_table;
	int i = 0, cnt = ARRAY_SIZE(rk3588_reset_domain_table);

	for (i = 0; i < cnt; i++) {
		if (reset->id == id)
			return &rk3588_reset_domain_table[i];
		reset++;
	}

	return &rk3588_reset_domain_table[cnt - 1];
}

rk_scmi_rstd_t *rockchip_scmi_get_rstd(unsigned int agent_id,
				       unsigned int scmi_id)

{
	return rockchip_get_reset_domain_table(scmi_id);
}

size_t rockchip_scmi_rstd_count(unsigned int agent_id)
{
	return SRST_TRNG_S;
}

