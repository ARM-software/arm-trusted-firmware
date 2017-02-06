/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <dram.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <dfs.h>
#include <dram.h>
#include <rk3399_def.h>
#include <secure.h>
#include <soc.h>
#include <m0_ctl.h>

/* Table of regions to map using the MMU.  */
const mmap_region_t plat_rk_mmap[] = {
	MAP_REGION_FLAT(DEV_RNG0_BASE, DEV_RNG0_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PMUSRAM_BASE, PMUSRAM_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE),

	{ 0 }
};

/* The RockChip power domain tree descriptor */
const unsigned char rockchip_power_domain_tree_desc[] = {
	/* No of root nodes */
	PLATFORM_SYSTEM_COUNT,
	/* No of children for the root node */
	PLATFORM_CLUSTER_COUNT,
	/* No of children for the first cluster node */
	PLATFORM_CLUSTER0_CORE_COUNT,
	/* No of children for the second cluster node */
	PLATFORM_CLUSTER1_CORE_COUNT
};

/* sleep data for pll suspend */
static struct deepsleep_data_s slp_data;

static void set_pll_slow_mode(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3), PLL_SLOW_MODE);
	else
		mmio_write_32((CRU_BASE +
			      CRU_PLL_CON(pll_id, 3)), PLL_SLOW_MODE);
}

static void set_pll_normal_mode(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3), PLL_NOMAL_MODE);
	else
		mmio_write_32(CRU_BASE +
			      CRU_PLL_CON(pll_id, 3), PLL_NOMAL_MODE);
}

static void set_pll_bypass(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE +
			      PMUCRU_PPLL_CON(3), PLL_BYPASS_MODE);
	else
		mmio_write_32(CRU_BASE +
			      CRU_PLL_CON(pll_id, 3), PLL_BYPASS_MODE);
}

static void _pll_suspend(uint32_t pll_id)
{
	set_pll_slow_mode(pll_id);
	set_pll_bypass(pll_id);
}

/**
 * disable_dvfs_plls - To suspend the specific PLLs
 *
 * When we close the center logic, the DPLL will be closed,
 * so we need to keep the ABPLL and switch to it to supply
 * clock for DDR during suspend, then we should not close
 * the ABPLL and exclude ABPLL_ID.
 */
void disable_dvfs_plls(void)
{
	_pll_suspend(CPLL_ID);
	_pll_suspend(NPLL_ID);
	_pll_suspend(VPLL_ID);
	_pll_suspend(GPLL_ID);
	_pll_suspend(ALPLL_ID);
}

/**
 * disable_nodvfs_plls - To suspend the PPLL
 */
void disable_nodvfs_plls(void)
{
	_pll_suspend(PPLL_ID);
}

/**
 * restore_pll - Copy PLL settings from memory to a PLL.
 *
 * This will copy PLL settings from an array in memory to the memory mapped
 * registers for a PLL.
 *
 * Note that: above the PLL exclude PPLL.
 *
 * pll_id: One of the values from enum plls_id
 * src: Pointer to the array of values to restore from
 */
static void restore_pll(int pll_id, uint32_t *src)
{
	/* Nice to have PLL off while configuring */
	mmio_write_32((CRU_BASE + CRU_PLL_CON(pll_id, 3)), PLL_SLOW_MODE);

	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 0), src[0] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 1), src[1] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 2), src[2]);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 4), src[4] | REG_SOC_WMSK);
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 5), src[5] | REG_SOC_WMSK);

	/* Do PLL_CON3 since that will enable things */
	mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3), src[3] | REG_SOC_WMSK);

	/* Wait for PLL lock done */
	while ((mmio_read_32(CRU_BASE + CRU_PLL_CON(pll_id, 2)) &
		0x80000000) == 0x0)
		;
}

/**
 * save_pll - Copy PLL settings a PLL to memory
 *
 * This will copy PLL settings from the memory mapped registers for a PLL to
 * an array in memory.
 *
 * Note that: above the PLL exclude PPLL.
 *
 * pll_id: One of the values from enum plls_id
 * src: Pointer to the array of values to save to.
 */
static void save_pll(uint32_t *dst, int pll_id)
{
	int i;

	for (i = 0; i < PLL_CON_COUNT; i++)
		dst[i] = mmio_read_32(CRU_BASE + CRU_PLL_CON(pll_id, i));
}

/**
 * prepare_abpll_for_ddrctrl - Copy DPLL settings to ABPLL
 *
 * This will copy DPLL settings from the memory mapped registers for a PLL to
 * an array in memory.
 */
void prepare_abpll_for_ddrctrl(void)
{
	save_pll(slp_data.plls_con[ABPLL_ID], ABPLL_ID);
	save_pll(slp_data.plls_con[DPLL_ID], DPLL_ID);

	restore_pll(ABPLL_ID, slp_data.plls_con[DPLL_ID]);
}

void restore_abpll(void)
{
	restore_pll(ABPLL_ID, slp_data.plls_con[ABPLL_ID]);
}

void restore_dpll(void)
{
	restore_pll(DPLL_ID, slp_data.plls_con[DPLL_ID]);
}

void clk_gate_con_save(void)
{
	uint32_t i = 0;

	for (i = 0; i < PMUCRU_GATE_COUNT; i++)
		slp_data.pmucru_gate_con[i] =
			mmio_read_32(PMUCRU_BASE + PMUCRU_GATE_CON(i));

	for (i = 0; i < CRU_GATE_COUNT; i++)
		slp_data.cru_gate_con[i] =
			mmio_read_32(CRU_BASE + CRU_GATE_CON(i));
}

void clk_gate_con_disable(void)
{
	uint32_t i;

	for (i = 0; i < PMUCRU_GATE_COUNT; i++)
		mmio_write_32(PMUCRU_BASE + PMUCRU_GATE_CON(i), REG_SOC_WMSK);

	for (i = 0; i < CRU_GATE_COUNT; i++)
		mmio_write_32(CRU_BASE + CRU_GATE_CON(i), REG_SOC_WMSK);
}

void clk_gate_con_restore(void)
{
	uint32_t i;

	for (i = 0; i < PMUCRU_GATE_COUNT; i++)
		mmio_write_32(PMUCRU_BASE + PMUCRU_GATE_CON(i),
			      REG_SOC_WMSK | slp_data.pmucru_gate_con[i]);

	for (i = 0; i < CRU_GATE_COUNT; i++)
		mmio_write_32(CRU_BASE + CRU_GATE_CON(i),
			      REG_SOC_WMSK | slp_data.cru_gate_con[i]);
}

static void set_plls_nobypass(uint32_t pll_id)
{
	if (pll_id == PPLL_ID)
		mmio_write_32(PMUCRU_BASE + PMUCRU_PPLL_CON(3),
			      PLL_NO_BYPASS_MODE);
	else
		mmio_write_32(CRU_BASE + CRU_PLL_CON(pll_id, 3),
			      PLL_NO_BYPASS_MODE);
}

static void _pll_resume(uint32_t pll_id)
{
	set_plls_nobypass(pll_id);
	set_pll_normal_mode(pll_id);
}

/**
 * enable_dvfs_plls - To resume the specific PLLs
 *
 * Please see the comment at the disable_dvfs_plls()
 * we don't suspend the ABPLL, so don't need resume
 * it too.
 */
void enable_dvfs_plls(void)
{
	_pll_resume(ALPLL_ID);
	_pll_resume(GPLL_ID);
	_pll_resume(VPLL_ID);
	_pll_resume(NPLL_ID);
	_pll_resume(CPLL_ID);
}

/**
 * enable_nodvfs_plls - To resume the PPLL
 */
void enable_nodvfs_plls(void)
{
	_pll_resume(PPLL_ID);
}

void soc_global_soft_reset_init(void)
{
	mmio_write_32(PMUCRU_BASE + CRU_PMU_RSTHOLD_CON(1),
		      CRU_PMU_SGRF_RST_RLS);

	mmio_clrbits_32(CRU_BASE + CRU_GLB_RST_CON,
			CRU_PMU_WDTRST_MSK | CRU_PMU_FIRST_SFTRST_MSK);
}

void __dead2 soc_global_soft_reset(void)
{
	set_pll_slow_mode(VPLL_ID);
	set_pll_slow_mode(NPLL_ID);
	set_pll_slow_mode(GPLL_ID);
	set_pll_slow_mode(CPLL_ID);
	set_pll_slow_mode(PPLL_ID);
	set_pll_slow_mode(ABPLL_ID);
	set_pll_slow_mode(ALPLL_ID);

	dsb();

	mmio_write_32(CRU_BASE + CRU_GLB_SRST_FST, GLB_SRST_FST_CFG_VAL);

	/*
	 * Maybe the HW needs some times to reset the system,
	 * so we do not hope the core to excute valid codes.
	 */
	while (1)
		;
}

void plat_rockchip_soc_init(void)
{
	secure_timer_init();
	secure_sgrf_init();
	secure_sgrf_ddr_rgn_init();
	soc_global_soft_reset_init();
	plat_rockchip_gpio_init();
	m0_init();
	dram_init();
	dram_dfs_init();
}
