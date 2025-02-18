// SPDX-License-Identifier: BSD-3-Clause
/*
 * Wrapper for Cadence DDR Driver
 *
 * Copyright (C) 2018-2026 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <libfdt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>

#include <k3_console.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4_ctl_regs.h"
#include "lpddr4_if.h"
#include "lpddr4_obj_if.h"
#include "lpddr4_structs_if.h"

/*
 * Each NS DRAM bank entry is 'reg' node property which is
 * a sequence of (address, length) pairs of 32-bit values.
 */
#define DRAM_ENTRY_SIZE (4 * sizeof(uint32_t))

#define HERTZ_PER_MEGAHERTZ 1000000UL
#define HERTZ_PER_GIGAHERTZ 1000000000UL
#define MAIN_PLL0_FREQ (2UL * HERTZ_PER_GIGAHERTZ)
#define MAIN_PLL0_HSDIV2_MAX_FREQ (400UL * HERTZ_PER_MEGAHERTZ)

struct k3_ddrss_desc {
	void *ddrss_ss_cfg;
	void *ddrss_ctrl_mmr;
	void *ddrss_ctl_cfg;
	uint32_t ddr_freq0;
	uint32_t ddr_freq1;
	uint32_t ddr_freq2;
	uint32_t ddr_fhs_cnt;
	uint32_t dram_class;
	uint32_t instance;
	lpddr4_obj *driverdt;
	lpddr4_config config;
	lpddr4_privatedata pd;
	uint64_t ecc_reserved_space;
	bool ti_ecc_enabled;
};

static struct k3_ddrss_desc ddrss;
const uint32_t *lpddr4_ctl_data;
const uint32_t *lpddr4_pi_data;
const uint32_t *lpddr4_phy_data;

/* set default DDR size to 2GB */
uint64_t ddr_ram_size = UL(0x80000000);

#define DDRSS_CTL_CFG 0x0f308000
#define DDRSS_CTRL_MMR_L 0x43014000
#define DDRSS_CTRL_MMR 0x43040000
#define DDRSS_SS_CFG 0x0f300000
#define DDR4_FSP_CLKCHNG_REQ 0x80
#define DDR4_FSP_CLKCHNG_ACK 0x84

#define PLL_CFG 0x04060000
#define PLL0_HSDIV2_CTRL 0x88

#define DDRSS_PI_REGISTER_BLOCK__OFFS 0x2000
#define DDRSS_PI_87__SFR_OFFS 0x15C
#define DDRSS_PI_83__SFR_OFFS 0x14C
#define DDRSS_CTL_350__SFR_OFFS 0x578
#define DDRSS_CTL_342__SFR_OFFS 0x558

#define DENALI_CTL_0_DRAM_CLASS_DDR4 0xAU
#define DENALI_CTL_0_DRAM_CLASS_LPDDR4 0xBU

#define DDRSS_V2A_CTL_REG 0x0020
#define DDRSS_ECC_CTRL_REG 0x0120

#define DDRSS_V2A_CTL_REG_SDRAM_IDX_CALC(x) ((log2(x) - 16) << 5)
#define DDRSS_V2A_CTL_REG_SDRAM_IDX_MASK (~(0x1F << 0x5))
#define DDRSS_V2A_CTL_REG_REGION_IDX_MASK (~(0X1F))

#define DDRSS_PLL_FREQUENCY_0 25000000

#define DDR32SS_PMCTRL (0x1000U)
#define CANUART_WAKE_OFF_MODE_STAT (0x1318U)
#define RTC_ONLY_PLUS_DDR_MAGIC_WORD (0x6D555555U)

#define PD_DDR 2

#define LPSC_MAIN_DDR_LOCAL 21
#define LPSC_MAIN_DDR_CFG_ISO_N 22
#define LPSC_MAIN_DDR_DATA_ISO_N 23

#define PSC_MD_SWRESETDISABLE (0x0)
#define PSC_MD_SYNCRESET (0x1)
#define PSC_MD_DISABLE (0x2)
#define PSC_MD_ENABLE (0x3)
#define PSC_PD_OFF (0x0)
#define PSC_PD_ON (0x1)

#define LPSC_DDR16SS0 21
#define LPSC_EMIF_CFG 22
#define LPSC_EMIF_DATA 23

#define MAIN_PSC_BASE 0x00400000UL
#define MAIN_PSC_MDCTL_BASE 0x00400A00UL
#define MAIN_PSC_MDSTAT_BASE 0x00400800UL
#define MAIN_PSC_PDCTL_BASE 0x00400300UL
#define MAIN_PSC_PDSTAT_BASE 0x00400200UL
#define MAIN_PSC_PTSTAT (MAIN_PSC_BASE + PSC_PTSTAT)
#define MAIN_PSC_PTCMD (MAIN_PSC_BASE + PSC_PTCMD)

#define PSC_PTCMD 0x120
#define PSC_PTCMD_H 0x124
#define PSC_PTSTAT 0x128
#define PSC_PTSTAT_H 0x12C
#define PSC_PDSTAT 0x200
#define PSC_PDCTL 0x300
#define PSC_MDSTAT 0x800
#define PSC_MDCTL 0xa00

#define PDCTL_STATE_MASK 0x1
#define PDCTL_STATE_OFF 0x0
#define PDCTL_STATE_ON 0x1

#define MDSTAT_STATE_MASK 0x3f
#define MDSTAT_BUSY_MASK 0x30
#define MDSTAT_STATE_SWRSTDISABLE 0x0
#define MDSTAT_STATE_ENABLE 0x3

#define str(s) #s
#define xstr(s) str(s)

#define  CTL_SHIFT 11
#define  PHY_SHIFT 11
#define  PI_SHIFT 10

#define TH_OFFSET_FROM_REG(REG, SHIFT, offset) do { \
		char *i, *pstr = xstr(REG); offset = 0; \
		for (i = &pstr[SHIFT]; *i != '\0'; ++i) { \
			offset = offset * 10 + (*i - '0'); } \
} while (0)

extern void *dtb_array;
void *dtb = &dtb_array;

static inline uint64_t log2(uint64_t n)
{
	uint64_t val;

	for (val = 0; n > 1U; val++, n >>= 1)
	;

	return val;
}

/********************************************
 * Internal function to set psc/lpsc state.
 ********************************************/
static int set_main_psc_state(uint32_t pd_id, uint32_t md_id, uint32_t pd_state, uint32_t md_state)
{
	uintptr_t mdctrl_ptr;
	volatile uint32_t mdctrl;
	uintptr_t mdstat_ptr;
	volatile uint32_t mdstat;
	uintptr_t pdctrl_ptr;
	volatile uint32_t pdctrl;
	uintptr_t pdstat_ptr;
	volatile uint32_t pdstat;
	volatile uint32_t psc_ptstat;
	volatile uint32_t psc_ptcmd;

	mdctrl_ptr = (uintptr_t)(MAIN_PSC_MDCTL_BASE + (4U * md_id));
	mdctrl = mmio_read_32(mdctrl_ptr);
	mdstat_ptr = (uintptr_t)(MAIN_PSC_MDSTAT_BASE + (4U * md_id));
	mdstat = mmio_read_32(mdstat_ptr);
	pdctrl_ptr = (uintptr_t)(MAIN_PSC_PDCTL_BASE + (4U * pd_id));
	pdctrl = mmio_read_32(pdctrl_ptr);
	pdstat_ptr = (uintptr_t)(MAIN_PSC_PDSTAT_BASE + (4U * pd_id));
	pdstat = mmio_read_32((uintptr_t)pdstat_ptr);

	if (((pdstat & 0x1U) == pd_state) && ((mdstat & 0x1fU) == md_state))
		return 0;

	psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);

	/* wait for GOSTAT to get cleared */
	while ((psc_ptstat & (0x1U << pd_id)) != 0U) {
		psc_ptstat = mmio_read_32(MAIN_PSC_PTSTAT);
	}

	/* Set PDCTL NEXT to new state */
	mmio_write_32((uintptr_t) pdctrl_ptr, (pdctrl & ~(0x1U)) | pd_state);

	/* Set MDCTL NEXT to new state */
	mmio_write_32((uintptr_t)mdctrl_ptr, (mdctrl & ~(0x1fU)) | md_state);

	/* start power transisition by setting PTCMD Go to 1 */
	psc_ptcmd = mmio_read_32((uintptr_t)MAIN_PSC_PTCMD);
	psc_ptcmd |= (0x1U << pd_id);
	mmio_write_32((uintptr_t)MAIN_PSC_PTCMD, psc_ptcmd);

	/* wait for GOSTAT to get cleared */
	psc_ptstat = mmio_read_32((uintptr_t)MAIN_PSC_PTSTAT);
	while ((psc_ptstat & (0x1U << pd_id)) != 0U) {
		psc_ptstat = mmio_read_32((uintptr_t)MAIN_PSC_PTSTAT);
	}

	return 0;
}

/************************************************
 * Function to set PLL divider for DDR SS clocks
 ************************************************/
static int set_ddr_pll_div(unsigned int div)
{
	volatile uint32_t val;

	/* Set reset to high */
	val = mmio_read_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL));
	val |= 0x80000000U;
	mmio_write_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL), val);

	/* Set divider value */
	val = mmio_read_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL));
	val &= 0xFFFFFF80U;
	val |= div;
	mmio_write_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL), val);

	/* Set enable bit */
	val = mmio_read_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL));
	val |= 0x8000U;
	mmio_write_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL), val);

	/* clear reset */
	val = mmio_read_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL));
	val &= 0x7FFFFFFFU;
	mmio_write_32((uintptr_t)(PLL_CFG + PLL0_HSDIV2_CTRL),  val);

	return 0;
}

/*******************************
 * Function to set DDRSS PLL
 ******************************/
static int ddrss_set_pll(unsigned long freq)
{
	int ret = 0;
	unsigned int div = 4U;

	if (freq <= 0 || freq > MAIN_PLL0_HSDIV2_MAX_FREQ) {
		return -1;
	}

	div = MAIN_PLL0_FREQ/freq;
	div -= 1;
	ret = set_ddr_pll_div(div);

	return ret;
}

/*************************************************************************
 * Function to change DDRSS PLL clock. It is called by the lpddr4 driver
 * during training
 ************************************************************************/
static void k3_lpddr4_freq_update(struct k3_ddrss_desc *ddr)
{
	volatile uint32_t val;
	uint32_t counter = 0;
	uint32_t req_type;

	for (counter = 0; counter < ddr->ddr_fhs_cnt; counter++) {
		/* update the PLL divisor for DDR PLL */
		val = mmio_read_32(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_REQ);
		val &= 0x80U;
		while (val == 0U) {
			val = mmio_read_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_REQ));
			val &= 0x80U;
		}
		val = mmio_read_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_REQ));

		req_type = val & 0x03;
		if (req_type == 1U)
			ddrss_set_pll(ddr->ddr_freq1);
		else if (req_type == 2U)
			ddrss_set_pll(ddr->ddr_freq2);
		else if (req_type == 0U)
			ddrss_set_pll(ddr->ddr_freq0);
		else
			WARN("invalid DDR freq request type\n");

		mmio_write_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_ACK),  0x01);
		val = mmio_read_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_REQ));
		val &= 0x80U;
		while (val == 0x80U) {
			val = mmio_read_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_REQ));
			val &= 0x80U;
		}
		mmio_write_32((uintptr_t)(DDRSS_CTRL_MMR + DDR4_FSP_CLKCHNG_ACK), 0x00);
	}
	INFO("DDR Freq change complete\n");
}

/*************************************************************************
 * Function to change DDRSS PLL clock. It is called by the lpddr4 driver
 * during training
 ************************************************************************/
static void k3_lpddr4_ack_freq_upd_req(const lpddr4_privatedata *pd)
{
	if (ddrss.dram_class == DENALI_CTL_0_DRAM_CLASS_LPDDR4)
		k3_lpddr4_freq_update(&ddrss);
}

static void k3_lpddr4_info_handler(const lpddr4_privatedata *pd,
				   lpddr4_infotype infotype)
{
	if (infotype == LPDDR4_DRV_SOC_PLL_UPDATE)
		k3_lpddr4_ack_freq_upd_req(pd);
}

/**
 * @brief Restore DDR controller during RTC + DDR resume
 *
 * Performs special initialization sequence required when resuming from
 * RTC-only + DDR retention mode. Configures DDR controller registers to
 * exit self-refresh without losing memory contents, including:
 * - Disabling auto-entry/exit modes
 * - Setting PHY DFI input impedance
 * - Configuring power-up self-refresh exit
 * - Setting training mode parameters
 * - De-asserting data retention signal
 *
 * This differs from normal DDR initialization as it preserves existing
 * memory contents rather than performing full training.
 *
 * @param pd LPDDR4 private data structure
 * @param driverdt LPDDR4 driver object with register access functions
 */
static void lpm_restore_ddr(lpddr4_privatedata *pd, lpddr4_obj *driverdt)
{
	uint32_t regval;

	/* disable auto-entry / -exit */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x0000029C / 4), &regval);
	regval = (regval & (0xF0F0FFFF));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x0000029C / 4), regval);

	/*
	 * Set the phy_set_dfi_input_Z parameter bit corresponding to the
	 * reset signal to 1'b1. Program the controller to a state
	 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00005468 / 4), &regval);
	regval = (regval | (0x1));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00005468 / 4), regval);

	/* Configure the DDR controller (and not the PI) to issue a PWRUP SREFRESH EXIT */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x000001A8 / 4), &regval);
	regval = (regval | (0x1));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x000001A8 / 4), regval);

	/* PI_PWRUP_SREFRESH_EXIT = 0 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00002218 / 4), &regval);
	regval = (regval & 0x0);
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00002218 / 4), regval);

	/*  PI_DRAM_INIT_EN = 0 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00002228 / 4), &regval);
	regval = (regval & (0xFFFFFEFF));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00002228 / 4), regval);

	/* PI_DFI_PHYMSTR_STATE_SEL_R = 1 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00002018 / 4), &regval);
	regval = (regval | (0x1 << 8));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00002018 / 4), regval);

	/* PHY_INDEP_INIT_MODE = 0 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00000054 / 4), &regval);
	regval = (regval & (0xFFFFFEFF));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00000054 / 4), regval);

	/* PHY_INDEP_TRAIN_MODE = 1 */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, (0x00000050 / 4), &regval);
	regval = (regval | (0x1 << 24));
	driverdt->writereg(pd, LPDDR4_CTL_REGS, (0x00000050 / 4), regval);

	/* De-assert the data_retention signal */
	mmio_write_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0);
	mmio_write_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), (0x1U << 31));
	while ((mmio_read_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL)) & 0x80000000) ==  0x0) {
	}
	mmio_write_32((WKUP_CTRL_MMR_SEC_4_BASE + DDR32SS_PMCTRL), 0x0);
}

/*************************************************************************
 * Function to change DDRSS PLL clock. It is called by the lpddr4 driver
 * during training
 ************************************************************************/
int am62l_lpddr4_init(void)
{
	uint16_t configsize = 0U;
	uint32_t status = 0U;
	lpddr4_config *config = &ddrss.config;
	lpddr4_privatedata *pd = &ddrss.pd;
	lpddr4_obj *driverdt;
	volatile uint32_t val;
	uint32_t offset = 0;
	uint32_t regval;
	int node;
	uint32_t *prop;
	int prop_length;
	fdt32_t prop_val;
	uint64_t reg_val;
	int num_banks = 1;
	uint32_t sdram_idx;
	uint32_t v2a_ctl_reg;
	int ret;
	bool restore;

	node = fdt_node_offset_by_compatible(dtb, -1, "ti,am62l-ddrss");

	if (node < 0) {
		ERROR("no ddr node found\n");
		return -ENODEV;
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,ddr-fhs-cnt", &prop_length);
	if (prop) {
		prop_val = (fdt32_t) *prop;
		ddrss.ddr_fhs_cnt = fdt32_to_cpu(prop_val);
	} else {
		ERROR("no ddr-fhs-cnt found\n");
		return -EINVAL;
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,ddr-freq0", &prop_length);
	if (prop) {
		prop_val = (fdt32_t) *prop;
		ddrss.ddr_freq0 = fdt32_to_cpu(prop_val);
	} else {
		ddrss.ddr_freq0 = DDRSS_PLL_FREQUENCY_0;
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,ddr-freq1", &prop_length);
	if (prop) {
		prop_val = (fdt32_t) *prop;
		ddrss.ddr_freq1 = fdt32_to_cpu(prop_val);
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,ddr-freq2", &prop_length);
	if (prop) {
		prop_val = (fdt32_t) *prop;
		ddrss.ddr_freq2 = fdt32_to_cpu(prop_val);
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,ctl-data", &prop_length);
	if (prop) {
		lpddr4_ctl_data = prop;
	} else {
		ERROR("no ti,ctl-data not found\n");
		return -EINVAL;
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,pi-data", &prop_length);
	if (prop) {
		lpddr4_pi_data = prop;
	} else {
		ERROR("no ti,pi-data not found\n");
		return -EINVAL;
	}

	prop = (void *) fdt_getprop(dtb, node, "ti,phy-data", &prop_length);
	if (prop) {
		lpddr4_phy_data = prop;
	} else {
		ERROR("no ti,phy-data not found\n");
		return -EINVAL;
	}

	regval = *((uint32_t *)lpddr4_ctl_data);
	regval = (uint32_t) fdt32_to_cpu(regval);
	ddrss.dram_class = CPS_FLD_READ(LPDDR4__DRAM_CLASS__FLD, regval);
	NOTICE("BL1: dram_class: %d\n", ddrss.dram_class);

	/* Find 'memory' node */
	node = fdt_node_offset_by_prop_value(dtb, -1, "device_type", "memory", sizeof("memory"));
	INFO("memory node =0x%x\n", node);
	if (node < 0) {
		WARN("FCONF: Unable to locate 'memory' node\n");
		goto set_psc_def_pll;
	}

	prop = (void *) fdt_getprop(dtb, node, "reg", &prop_length);
	if (prop == NULL) {
		WARN("failed to read 'reg' property\n");
		goto set_psc_def_pll;
	}

	num_banks = prop_length / DRAM_ENTRY_SIZE;
	ddr_ram_size = 0;
	for (int i = 0; i < num_banks; i++) {
		/* first property is bank base addess followed by bank size */
		reg_val = fdt32_to_cpu(prop[i*4+2]);
		reg_val = (reg_val << 32) | fdt32_to_cpu(prop[i*4+3]);
		ddr_ram_size += reg_val;
	}

set_psc_def_pll:
	if (ddrss.dram_class == DENALI_CTL_0_DRAM_CLASS_LPDDR4)
		ddrss_set_pll(ddrss.ddr_freq0);
	else
		ddrss_set_pll(ddrss.ddr_freq1);

	/* Disable the DDR LPSCs to start in known state */
	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_DATA_ISO_N, PSC_PD_ON, PSC_MD_SWRESETDISABLE);
	if (ret != 0)
		return ret;

	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_CFG_ISO_N, PSC_PD_ON, PSC_MD_SWRESETDISABLE);
	if (ret != 0)
		return ret;

	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_LOCAL, PSC_PD_OFF, PSC_MD_SWRESETDISABLE);
	if (ret != 0)
		return ret;

	/* Enable DDR LPSCs to configure the controllers */
	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_LOCAL, PSC_PD_ON, PSC_MD_ENABLE);
	if (ret != 0)
		return ret;

	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_CFG_ISO_N, PSC_PD_ON, PSC_MD_ENABLE);
	if (ret != 0)
		return ret;

	ret = set_main_psc_state(PD_DDR, LPSC_MAIN_DDR_DATA_ISO_N, PSC_PD_ON, PSC_MD_ENABLE);
	if (ret != 0)
		return ret;

	ddrss.ddrss_ctl_cfg = (void *)DDRSS_CTL_CFG;
	ddrss.ddrss_ctrl_mmr = (void *)DDRSS_CTRL_MMR;
	ddrss.ddrss_ss_cfg = (void *)DDRSS_SS_CFG;

	ddrss.driverdt = lpddr4_getinstance();
	driverdt = ddrss.driverdt;

	driverdt->probe(config, &configsize);
	INFO("lpddr4: probe done\n");

	/* set LPDDR4 size */
	sdram_idx = DDRSS_V2A_CTL_REG_SDRAM_IDX_CALC(ddr_ram_size);
	v2a_ctl_reg = (uint32_t) *((uint32_t *)(DDRSS_SS_CFG + DDRSS_V2A_CTL_REG));
	v2a_ctl_reg = (v2a_ctl_reg & DDRSS_V2A_CTL_REG_SDRAM_IDX_MASK) | sdram_idx;
	mmio_write_32((uintptr_t)(DDRSS_SS_CFG + DDRSS_V2A_CTL_REG), v2a_ctl_reg);

	/* Initialize LPDDR4 */
	config->ctlbase = ddrss.ddrss_ctl_cfg;
	config->infohandler = k3_lpddr4_info_handler;
	driverdt->init(pd, config);
	INFO("lpddr4/ddr4: init done\n");

	/* Configure DDR with config and control structures */
	driverdt->writectlconfigex(pd, lpddr4_ctl_data, LPDDR4_INTR_CTL_REG_COUNT);
	driverdt->writephyindepconfigex(pd, lpddr4_pi_data, LPDDR4_INTR_PHY_INDEP_REG_COUNT);
	driverdt->writephyconfigex(pd, lpddr4_phy_data, LPDDR4_INTR_PHY_REG_COUNT);

	TH_OFFSET_FROM_REG(LPDDR4__START__REG, CTL_SHIFT, offset);
	driverdt->readreg(pd, LPDDR4_CTL_REGS, offset, &regval);
	if (CPS_FLD_READ(LPDDR4__START__FLD, regval) != 0) {
		ERROR("LPDDR4 prestart failed\n");
		return -ENXIO;
	}

	restore = (mmio_read_32((WKUP_CTRL_MMR_SEC_5_BASE +
				 CANUART_WAKE_OFF_MODE_STAT)) ==
		   RTC_ONLY_PLUS_DDR_MAGIC_WORD);
	if (restore) {
		INFO("Exiting RTC only + DDR");
		lpm_restore_ddr(pd, driverdt);
	} else {
		INFO("Doing normal DDR init");
	}

	INFO("lpddr4: Start DDR controller\n");
	status = driverdt->start(pd);
	if (status != 0) {
		ERROR("lpddr4: start failed status = 0x%x\n", status);
		return status;
	}

	/* check poststart status */
	driverdt->readreg(pd, LPDDR4_CTL_REGS, offset, &regval);
	INFO("start-status reg: after =0x%x\n", regval);
	if (CPS_FLD_READ(LPDDR4__START__FLD, regval) != 1) {
		ERROR("LPDDR4 poststart failed\n");
		return -ENXIO;
	}

	val = mmio_read_32((uintptr_t)(DDRSS_CTL_CFG + DDRSS_PI_REGISTER_BLOCK__OFFS + DDRSS_PI_83__SFR_OFFS));
	INFO("lpddr4: post start - PI training status=0x%x\n", val);

	val = mmio_read_32((uintptr_t)(DDRSS_CTL_CFG + DDRSS_CTL_342__SFR_OFFS));
	INFO("lpddr4: post start - CTL Interrupt status=0x%x\n", val);

	return 0;
}
