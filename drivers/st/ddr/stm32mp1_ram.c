/*
 * Copyright (C) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#include <errno.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/stm32mp1_ddr.h>
#include <drivers/st/stm32mp1_ddr_helpers.h>
#include <drivers/st/stm32mp1_ram.h>
#include <lib/mmio.h>

#define DDR_PATTERN	0xAAAAAAAAU
#define DDR_ANTIPATTERN	0x55555555U

static struct ddr_info ddr_priv_data;

int stm32mp1_ddr_clk_enable(struct ddr_info *priv, uint32_t mem_speed)
{
	unsigned long ddrphy_clk, ddr_clk, mem_speed_hz;

	ddr_enable_clock();

	ddrphy_clk = stm32mp_clk_get_rate(DDRPHYC);

	VERBOSE("DDR: mem_speed (%d kHz), RCC %ld kHz\n",
		mem_speed, ddrphy_clk / 1000U);

	mem_speed_hz = mem_speed * 1000U;

	/* Max 10% frequency delta */
	if (ddrphy_clk > mem_speed_hz) {
		ddr_clk = ddrphy_clk - mem_speed_hz;
	} else {
		ddr_clk = mem_speed_hz - ddrphy_clk;
	}
	if (ddr_clk > (mem_speed_hz / 10)) {
		ERROR("DDR expected freq %d kHz, current is %ld kHz\n",
		      mem_speed, ddrphy_clk / 1000U);
		return -1;
	}
	return 0;
}

/*******************************************************************************
 * This function tests the DDR data bus wiring.
 * This is inspired from the Data Bus Test algorithm written by Michael Barr
 * in "Programming Embedded Systems in C and C++" book.
 * resources.oreilly.com/examples/9781565923546/blob/master/Chapter6/
 * File: memtest.c - This source code belongs to Public Domain.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
static uint32_t ddr_test_data_bus(void)
{
	uint32_t pattern;

	for (pattern = 1U; pattern != 0U; pattern <<= 1) {
		mmio_write_32(STM32MP_DDR_BASE, pattern);

		if (mmio_read_32(STM32MP_DDR_BASE) != pattern) {
			return (uint32_t)STM32MP_DDR_BASE;
		}
	}

	return 0;
}

/*******************************************************************************
 * This function tests the DDR address bus wiring.
 * This is inspired from the Data Bus Test algorithm written by Michael Barr
 * in "Programming Embedded Systems in C and C++" book.
 * resources.oreilly.com/examples/9781565923546/blob/master/Chapter6/
 * File: memtest.c - This source code belongs to Public Domain.
 * Returns 0 if success, and address value else.
 ******************************************************************************/
static uint32_t ddr_test_addr_bus(void)
{
	uint64_t addressmask = (ddr_priv_data.info.size - 1U);
	uint64_t offset;
	uint64_t testoffset = 0;

	/* Write the default pattern at each of the power-of-two offsets. */
	for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
	     offset <<= 1) {
		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)offset,
			      DDR_PATTERN);
	}

	/* Check for address bits stuck high. */
	mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
		      DDR_ANTIPATTERN);

	for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
	     offset <<= 1) {
		if (mmio_read_32(STM32MP_DDR_BASE + (uint32_t)offset) !=
		    DDR_PATTERN) {
			return (uint32_t)(STM32MP_DDR_BASE + offset);
		}
	}

	mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset, DDR_PATTERN);

	/* Check for address bits stuck low or shorted. */
	for (testoffset = sizeof(uint32_t); (testoffset & addressmask) != 0U;
	     testoffset <<= 1) {
		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
			      DDR_ANTIPATTERN);

		if (mmio_read_32(STM32MP_DDR_BASE) != DDR_PATTERN) {
			return STM32MP_DDR_BASE;
		}

		for (offset = sizeof(uint32_t); (offset & addressmask) != 0U;
		     offset <<= 1) {
			if ((mmio_read_32(STM32MP_DDR_BASE +
					  (uint32_t)offset) != DDR_PATTERN) &&
			    (offset != testoffset)) {
				return (uint32_t)(STM32MP_DDR_BASE + offset);
			}
		}

		mmio_write_32(STM32MP_DDR_BASE + (uint32_t)testoffset,
			      DDR_PATTERN);
	}

	return 0;
}

/*******************************************************************************
 * This function checks the DDR size. It has to be run with Data Cache off.
 * This test is run before data have been put in DDR, and is only done for
 * cold boot. The DDR data can then be overwritten, and it is not useful to
 * restore its content.
 * Returns DDR computed size.
 ******************************************************************************/
static uint32_t ddr_check_size(void)
{
	uint32_t offset = sizeof(uint32_t);

	mmio_write_32(STM32MP_DDR_BASE, DDR_PATTERN);

	while (offset < STM32MP_DDR_MAX_SIZE) {
		mmio_write_32(STM32MP_DDR_BASE + offset, DDR_ANTIPATTERN);
		dsb();

		if (mmio_read_32(STM32MP_DDR_BASE) != DDR_PATTERN) {
			break;
		}

		offset <<= 1;
	}

	INFO("Memory size = 0x%x (%d MB)\n", offset, offset / (1024U * 1024U));

	return offset;
}

static int stm32mp1_ddr_setup(void)
{
	struct ddr_info *priv = &ddr_priv_data;
	int ret;
	struct stm32mp1_ddr_config config;
	int node, len;
	uint32_t uret, idx;
	void *fdt;

#define PARAM(x, y)							\
	{								\
		.name = x,						\
		.offset = offsetof(struct stm32mp1_ddr_config, y),	\
		.size = sizeof(config.y) / sizeof(uint32_t)		\
	}

#define CTL_PARAM(x) PARAM("st,ctl-"#x, c_##x)
#define PHY_PARAM(x) PARAM("st,phy-"#x, p_##x)

	const struct {
		const char *name; /* Name in DT */
		const uint32_t offset; /* Offset in config struct */
		const uint32_t size;   /* Size of parameters */
	} param[] = {
		CTL_PARAM(reg),
		CTL_PARAM(timing),
		CTL_PARAM(map),
		CTL_PARAM(perf),
		PHY_PARAM(reg),
		PHY_PARAM(timing),
		PHY_PARAM(cal)
	};

	if (fdt_get_address(&fdt) == 0) {
		return -ENOENT;
	}

	node = fdt_node_offset_by_compatible(fdt, -1, DT_DDR_COMPAT);
	if (node < 0) {
		ERROR("%s: Cannot read DDR node in DT\n", __func__);
		return -EINVAL;
	}

	config.info.speed = fdt_read_uint32_default(node, "st,mem-speed", 0);
	if (!config.info.speed) {
		VERBOSE("%s: no st,mem-speed\n", __func__);
		return -EINVAL;
	}
	config.info.size = fdt_read_uint32_default(node, "st,mem-size", 0);
	if (!config.info.size) {
		VERBOSE("%s: no st,mem-size\n", __func__);
		return -EINVAL;
	}
	config.info.name = fdt_getprop(fdt, node, "st,mem-name", &len);
	if (config.info.name == NULL) {
		VERBOSE("%s: no st,mem-name\n", __func__);
		return -EINVAL;
	}
	INFO("RAM: %s\n", config.info.name);

	for (idx = 0; idx < ARRAY_SIZE(param); idx++) {
		ret = fdt_read_uint32_array(node, param[idx].name,
					    (void *)((uintptr_t)&config +
						     param[idx].offset),
					    param[idx].size);

		VERBOSE("%s: %s[0x%x] = %d\n", __func__,
			param[idx].name, param[idx].size, ret);
		if (ret != 0) {
			ERROR("%s: Cannot read %s\n",
			      __func__, param[idx].name);
			return -EINVAL;
		}
	}

	/* Disable axidcg clock gating during init */
	mmio_clrbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_AXIDCGEN);

	stm32mp1_ddr_init(priv, &config);

	/* Enable axidcg clock gating */
	mmio_setbits_32(priv->rcc + RCC_DDRITFCR, RCC_DDRITFCR_AXIDCGEN);

	priv->info.size = config.info.size;

	VERBOSE("%s : ram size(%x, %x)\n", __func__,
		(uint32_t)priv->info.base, (uint32_t)priv->info.size);

	write_sctlr(read_sctlr() & ~SCTLR_C_BIT);
	dcsw_op_all(DC_OP_CISW);

	uret = ddr_test_data_bus();
	if (uret != 0U) {
		ERROR("DDR data bus test: can't access memory @ 0x%x\n",
		      uret);
		panic();
	}

	uret = ddr_test_addr_bus();
	if (uret != 0U) {
		ERROR("DDR addr bus test: can't access memory @ 0x%x\n",
		      uret);
		panic();
	}

	uret = ddr_check_size();
	if (uret < config.info.size) {
		ERROR("DDR size: 0x%x does not match DT config: 0x%x\n",
		      uret, config.info.size);
		panic();
	}

	write_sctlr(read_sctlr() | SCTLR_C_BIT);

	return 0;
}

int stm32mp1_ddr_probe(void)
{
	struct ddr_info *priv = &ddr_priv_data;

	VERBOSE("STM32MP DDR probe\n");

	priv->ctl = (struct stm32mp1_ddrctl *)stm32mp_ddrctrl_base();
	priv->phy = (struct stm32mp1_ddrphy *)stm32mp_ddrphyc_base();
	priv->pwr = stm32mp_pwr_base();
	priv->rcc = stm32mp_rcc_base();

	priv->info.base = STM32MP_DDR_BASE;
	priv->info.size = 0;

	return stm32mp1_ddr_setup();
}
