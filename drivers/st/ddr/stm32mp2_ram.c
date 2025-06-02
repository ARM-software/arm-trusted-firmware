/*
 * Copyright (C) 2021-2025, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <drivers/clk.h>
#include <drivers/st/stm32mp2_ddr.h>
#include <drivers/st/stm32mp2_ddr_helpers.h>
#include <drivers/st/stm32mp2_ram.h>
#include <drivers/st/stm32mp_ddr.h>
#include <drivers/st/stm32mp_ddr_test.h>
#include <drivers/st/stm32mp_ram.h>

#include <lib/mmio.h>
#include <libfdt.h>

#include <platform_def.h>

static struct stm32mp_ddr_priv ddr_priv_data;
static bool ddr_self_refresh;

static int ddr_dt_get_ui_param(void *fdt, int node, struct stm32mp_ddr_config *config)
{
	int ret;
	uint32_t size;

	size = sizeof(struct user_input_basic) / sizeof(int);
	ret = fdt_read_uint32_array(fdt, node, "st,phy-basic", size, (uint32_t *)&config->uib);

	VERBOSE("%s: %s[0x%x] = %d\n", __func__, "st,phy-basic", size, ret);
	if (ret != 0) {
		ERROR("%s: can't read %s, error=%d\n", __func__, "st,phy-basic", ret);
		return -EINVAL;
	}

	size = sizeof(struct user_input_advanced) / sizeof(int);
	ret = fdt_read_uint32_array(fdt, node, "st,phy-advanced", size, (uint32_t *)&config->uia);

	VERBOSE("%s: %s[0x%x] = %d\n", __func__, "st,phy-advanced", size, ret);
	if (ret != 0) {
		ERROR("%s: can't read %s, error=%d\n", __func__, "st,phy-advanced", ret);
		return -EINVAL;
	}

	size = sizeof(struct user_input_mode_register) / sizeof(int);
	ret = fdt_read_uint32_array(fdt, node, "st,phy-mr", size, (uint32_t *)&config->uim);

	VERBOSE("%s: %s[0x%x] = %d\n", __func__, "st,phy-mr", size, ret);
	if (ret != 0) {
		ERROR("%s: can't read %s, error=%d\n", __func__, "st,phy-mr", ret);
		return -EINVAL;
	}

	size = sizeof(struct user_input_swizzle) / sizeof(int);
	ret = fdt_read_uint32_array(fdt, node, "st,phy-swizzle", size, (uint32_t *)&config->uis);

	VERBOSE("%s: %s[0x%x] = %d\n", __func__, "st,phy-swizzle", size, ret);
	if (ret != 0) {
		ERROR("%s: can't read %s, error=%d\n", __func__, "st,phy-swizzle", ret);
		return -EINVAL;
	}

	return 0;
}

static int stm32mp2_ddr_setup(void)
{
	struct stm32mp_ddr_priv *priv = &ddr_priv_data;
	int ret;
	struct stm32mp_ddr_config config;
	int node;
	uintptr_t uret;
	void *fdt;

	const struct stm32mp_ddr_param param[] = {
		CTL_PARAM(reg),
		CTL_PARAM(timing),
		CTL_PARAM(map),
		CTL_PARAM(perf)
	};

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		ERROR("%s: can't read DDR node in DT\n", __func__);
		return -EINVAL;
	}

	ret = stm32mp_ddr_dt_get_info(fdt, node, &config.info);
	if (ret < 0) {
		return ret;
	}

	ret = stm32mp_ddr_dt_get_param(fdt, node, param, ARRAY_SIZE(param), (uintptr_t)&config);
	if (ret < 0) {
		return ret;
	}

	ret = ddr_dt_get_ui_param(fdt, node, &config);
	if (ret < 0) {
		return ret;
	}

	config.self_refresh = false;

	if (stm32mp_is_wakeup_from_standby()) {
		config.self_refresh = true;
	}

	/*  Map dynamically RETRAM area to save or restore PHY retention registers */
	if (stm32mp_map_retram() != 0) {
		panic();
	}

	stm32mp2_ddr_init(priv, &config);

	/*  Unmap RETRAM, no more used until next DDR initialization call */
	if (stm32mp_unmap_retram() != 0) {
		panic();
	}

	priv->info.size = config.info.size;

	VERBOSE("%s : ram size(%lx, %lx)\n", __func__, priv->info.base, priv->info.size);

	if (stm32mp_map_ddr_non_cacheable() != 0) {
		panic();
	}

	if (config.self_refresh) {
		uret = stm32mp_ddr_test_rw_access();
		if (uret != 0UL) {
			ERROR("DDR rw test: can't access memory @ 0x%lx\n", uret);
			panic();
		}
	} else {
		size_t retsize;

		uret = stm32mp_ddr_test_data_bus();
		if (uret != 0UL) {
			ERROR("DDR data bus test: can't access memory @ 0x%lx\n", uret);
			panic();
		}

		uret = stm32mp_ddr_test_addr_bus(config.info.size);
		if (uret != 0UL) {
			ERROR("DDR addr bus test: can't access memory @ 0x%lx\n", uret);
			panic();
		}

		retsize = stm32mp_ddr_check_size();
		if (retsize < config.info.size) {
			ERROR("DDR size: 0x%zx does not match DT config: 0x%zx\n",
			      retsize, config.info.size);
			panic();
		}

		INFO("Memory size = 0x%zx (%zu MB)\n", retsize, retsize / (1024U * 1024U));
	}

	/*
	 * Initialization sequence has configured DDR registers with settings.
	 * The Self Refresh (SR) mode corresponding to these settings has now
	 * to be set.
	 */
	ddr_set_sr_mode(ddr_read_sr_mode());

	if (stm32mp_unmap_ddr() != 0) {
		panic();
	}

	/* Save DDR self_refresh state */
	ddr_self_refresh = config.self_refresh;

	return 0;
}

bool stm32mp2_ddr_is_restored(void)
{
	return ddr_self_refresh;
}

int stm32mp2_ddr_probe(void)
{
	struct stm32mp_ddr_priv *priv = &ddr_priv_data;

	VERBOSE("STM32MP DDR probe\n");

	priv->ctl = (struct stm32mp_ddrctl *)stm32mp_ddrctrl_base();
	priv->phy = (struct stm32mp_ddrphy *)stm32mp_ddrphyc_base();
	priv->pwr = stm32mp_pwr_base();
	priv->rcc = stm32mp_rcc_base();

	priv->info.base = STM32MP_DDR_BASE;
	priv->info.size = 0;

	return stm32mp2_ddr_setup();
}
