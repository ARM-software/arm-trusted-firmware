/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <paxb.h>
#include <sr_def.h>
#include <sr_utils.h>

/* total number of PCIe Phys */
#define NUM_OF_PCIE_SERDES            8

#define CFG_RC_PMI_ADDR               0x1130
#define PMI_RX_TERM_SEQ               ((0x1 << 27) | (0x1ff << 16) | (0xd090))
#define PMI_RX_TERM_VAL               0x4c00
#define PMI_PLL_CTRL_4                0xd0b4
#define PMI_SERDES_CLK_ENABLE         (1 << 12)

#define WAR_PLX_PRESET_PARITY_FAIL

#define CFG_RC_REG_PHY_CTL_10         0x1838
#define PHY_CTL_10_GEN3_MATCH_PARITY  (1 << 15)

#define PMI_X8_CORE0_7_PATCH_SEQ      ((0x1 << 27) | (0x1ff << 16) | (0xd2a5))
#define PMI_X8_CORE0_7_PATCH_VAL      0xd864

#define PMI_ADDR_BCAST(addr)          ((0x1 << 27) | (0x1ff << 16) | (addr))
#define PMI_ADDR_LANE0(addr)          ((0x1 << 27) | (addr))
#define PMI_ADDR_LANE1(addr)          ((0x1 << 27) | (0x1 << 16) | (addr))

#define MERLIN16_PCIE_BLK2_PWRMGMT_7	((0x1 << 27) | (0x1ff << 16) | 0x1208)
#define MERLIN16_PCIE_BLK2_PWRMGMT_8	((0x1 << 27) | (0x1ff << 16) | 0x1209)
#define MERLIN16_AMS_TX_CTRL_5		((0x1 << 27) | (0x1ff << 16) | 0xd0a5)
#define MERLIN16_AMS_TX_CTRL_5_VAL	\
		((1 << 13) | (1 << 12) | (1 << 11) | (1 << 10))
#define MERLIN16_PCIE_BLK2_PWRMGMT_7_VAL   0x96
#define MERLIN16_PCIE_BLK2_PWRMGMT_8_VAL   0x12c

#define CFG_RC_PMI_WDATA              0x1134
#define CFG_RC_WCMD_SHIFT             31
#define CFG_RC_WCMD_MASK              ((uint32_t)1U << CFG_RC_WCMD_SHIFT)
#define CFG_RC_RCMD_SHIFT             30
#define CFG_RC_RCMD_MASK              ((uint32_t)1U << CFG_RC_RCMD_SHIFT)
#define CFG_RC_RWCMD_MASK             (CFG_RC_RCMD_MASK | CFG_RC_WCMD_MASK)
#define CFG_RC_PMI_RDATA              0x1138
#define CFG_RC_RACK_SHIFT             31
#define CFG_RC_RACK_MASK              ((uint32_t)1U << CFG_RC_RACK_SHIFT)

/* allow up to 5 ms for PMI write to finish */
#define PMI_TIMEOUT_MS                5

/* in 2x8 RC mode, one needs to patch up Serdes 3 and 7 for link to come up */
#define SERDES_PATCH_PIPEMUX_INDEX    0x3
#define SERDES_PATCH_INDEX            0x8

#define DSC_UC_CTRL                   0xd00d
#define DSC_UC_CTRL_RDY_CMD           (1 << 7)
#define LANE_DBG_RST_CTRL             0xd164
#define UC_A_CLK_CTRL0                0xd200
#define UC_A_RST_CTRL0                0xd201
#define UC_A_AHB_CTRL0                0xd202
#define UC_A_AHB_STAT0                0xd203
#define UC_A_AHB_WADDR_LSW            0xd204
#define UC_A_AHB_WADDR_MSW            0xd205
#define UC_A_AHB_WDATA_LSW            0xd206
#define UC_A_AHB_WDATA_MSW            0xd207
#define UC_A_AHB_RADDR_LSW            0xd208
#define UC_A_AHB_RADDR_MSW            0xd209
#define UC_A_AHB_RDATA_LSW            0xd20a
#define UC_A_AHB_RDATA_MSW            0xd20b
#define UC_VERSION_NUM                0xd230
#define DSC_SM_CTL22                  0xd267
#define UC_DBG1                       0xd251

#define LOAD_UC_CHECK                 0
#define UC_RAM_INIT_TIMEOUT           100
#define UC_RAM_CONTROL                0xd225
#define UC_INIT_TIMEOUT               100
#define SIZE_ALIGN(x, a)              (((x) + (a) - 1) & ~((a) - 1))
#define SZ_4                          4
#define GET_2_BYTES(p, i)             ((uint16_t)p[i] | (uint16_t)p[i+1] << 8)

/*
 * List of PCIe LCPLL related registers
 *
 * LCPLL channel 0 provides the Serdes pad clock when running in RC mode
 */
#define PCIE_LCPLL_BASE             0x40000000

#define PCIE_LCPLL_CTRL0_OFFSET     0x00
#define PCIE_LCPLL_RESETB_SHIFT     31
#define PCIE_LCPLL_RESETB_MASK      BIT(PCIE_LCPLL_RESETB_SHIFT)
#define PCIE_LCPLL_P_RESETB_SHIFT   30
#define PCIE_LCPLL_P_RESETB_MASK    BIT(PCIE_LCPLL_P_RESETB_SHIFT)

#define PCIE_LCPLL_CTRL3_OFFSET     0x0c
#define PCIE_LCPLL_EN_CTRL_SHIFT    16
#define PCIE_LCPLL_CM_ENA           0x1a
#define PCIE_LCPLL_CM_BUF_ENA       0x18
#define PCIE_LCPLL_D2C2_ENA         0x2
#define PCIE_LCPLL_REF_CLK_SHIFT    1
#define PCIE_LCPLL_REF_CLK_MASK     BIT(PCIE_LCPLL_REF_CLK_SHIFT)
#define PCIE_LCPLL_CTRL13_OFFSET    0x34
#define PCIE_LCPLL_D2C2_CTRL_SHIFT  16
#define PCIE_LCPLL_D2C2_TERM_DISC   0xe0

#define PCIE_LCPLL_STATUS_OFFSET    0x40
#define PCIE_LCPLL_LOCK_SHIFT       12
#define PCIE_LCPLL_LOCK_MASK        BIT(PCIE_LCPLL_LOCK_SHIFT)

#define PCIE_PIPE_MUX_RC_MODE_OVERRIDE_CFG  0x114
#define PCIE_TX_CLKMASTER_CTRL_OVERRIDE_CFG 0x11c

/* wait 500 microseconds for PCIe LCPLL to power up */
#define PCIE_LCPLL_DELAY_US         500

/* allow up to 5 ms for PCIe LCPLL VCO to lock */
#define PCIE_LCPLL_TIMEOUT_MS       5

#define PCIE_PIPE_MUX_CONFIGURATION_CFG  0x4000010c

#define PCIE_PIPEMUX_SHIFT        19
#define PCIE_PIPEMUX_MASK         0xf

/* keep track of PIPEMUX index to use */
static unsigned int pipemux_idx;

/*
 * PCIe PIPEMUX lookup table
 *
 * Each array index represents a PIPEMUX strap setting
 * The array element represents a bitmap where a set bit means the PCIe core
 * needs to be enabled as RC
 */
static uint8_t pipemux_table[] = {
	/* PIPEMUX = 0, EP 1x16 */
	0x00,
	/* PIPEMUX = 1, EP 1x8 + RC 1x8, core 7 */
	0x80,
	/* PIPEMUX = 2, EP 4x4 */
	0x00,
	/* PIPEMUX = 3, RC 2x8, cores 0, 7 */
	0x81,
	/* PIPEMUX = 4, RC 4x4, cores 0, 1, 6, 7 */
	0xc3,
	/* PIPEMUX = 5, RC 8x2, all 8 cores */
	0xff,
	/* PIPEMUX = 6, RC 3x4 + 2x2, cores 0, 2, 3, 6, 7 */
	0xcd,
	/* PIPEMUX = 7, RC 1x4 + 6x2, cores 0, 2, 3, 4, 5, 6, 7 */
	0xfd,
	/* PIPEMUX = 8, EP 1x8 + RC 4x2, cores 4, 5, 6, 7 */
	0xf0,
	/* PIPEMUX = 9, EP 1x8 + RC 2x4, cores 6, 7 */
	0xc0,
	/* PIPEMUX = 10, EP 2x4 + RC 2x4, cores 1, 6 */
	0x42,
	/* PIPEMUX = 11, EP 2x4 + RC 4x2, cores 2, 3, 4, 5 */
	0x3c,
	/* PIPEMUX = 12, EP 1x4 + RC 6x2, cores 2, 3, 4, 5, 6, 7 */
	0xfc,
	/* PIPEMUX = 13, RC 2x4 + RC 1x4 + 2x2, cores 2, 3, 6 */
	0x4c,
};

/*
 * Return 1 if pipemux strap is supported
 */
static int pipemux_strap_is_valid(uint32_t pipemux)
{
	if (pipemux < ARRAY_SIZE(pipemux_table))
		return 1;
	else
		return 0;
}

/*
 * Read the PCIe PIPEMUX from strap
 */
static uint32_t pipemux_strap_read(void)
{
	uint32_t pipemux;

	pipemux = mmio_read_32(PCIE_PIPE_MUX_CONFIGURATION_CFG);
	pipemux &= PCIE_PIPEMUX_MASK;
	if (pipemux == PCIE_PIPEMUX_MASK) {
		/* read the PCIe PIPEMUX strap setting */
		pipemux = mmio_read_32(CDRU_CHIP_STRAP_DATA_LSW);
		pipemux >>= PCIE_PIPEMUX_SHIFT;
		pipemux &= PCIE_PIPEMUX_MASK;
	}

	return pipemux;
}

/*
 * Store the PIPEMUX index (set for each boot)
 */
static void pipemux_save_index(unsigned int idx)
{
	pipemux_idx = idx;
}

static int paxb_sr_core_needs_enable(unsigned int core_idx)
{
	return !!((pipemux_table[pipemux_idx] >> core_idx) & 0x1);
}

static int pipemux_sr_init(void)
{
	uint32_t pipemux;

	/* read the PCIe PIPEMUX strap setting */
	pipemux = pipemux_strap_read();
	if (!pipemux_strap_is_valid(pipemux)) {
		ERROR("Invalid PCIe PIPEMUX strap %u\n", pipemux);
		return -EIO;
	}

	/* no PCIe RC is needed */
	if (!pipemux_table[pipemux]) {
		WARN("PIPEMUX indicates no PCIe RC required\n");
		return -ENODEV;
	}

	/* save the PIPEMUX strap */
	pipemux_save_index(pipemux);

	return 0;
}

/*
 * PCIe RC serdes link width
 *
 * The array is first organized in rows as indexed by the PIPEMUX setting.
 * Within each row, eight lane width entries are specified -- one entry
 * per PCIe core, from 0 to 7.
 *
 * Note: The EP lanes/cores are not mapped in this table!  EP cores are
 *       controlled and thus configured by Nitro.
 */
static uint8_t link_width_table[][NUM_OF_SR_PCIE_CORES] = {
	/* PIPEMUX = 0, EP 1x16 */
	{0, 0, 0, 0, 0, 0, 0, 0},
	/* PIPEMUX = 1, EP 1x8 + RC 1x8, core 7 */
	{0, 0, 0, 0, 0, 0, 0, 8},
	/* PIPEMUX = 2, EP 4x4 */
	{0, 0, 0, 0, 0, 0, 0, 0},
	/* PIPEMUX = 3, RC 2x8, cores 0, 7 */
	{8, 0, 0, 0, 0, 0, 0, 8},
	/* PIPEMUX = 4, RC 4x4, cores 0, 1, 6, 7 */
	{4, 4, 0, 0, 0, 0, 4, 4},
	/* PIPEMUX = 5, RC 8x2, all 8 cores */
	{2, 2, 2, 2, 2, 2, 2, 2},
	/* PIPEMUX = 6, RC 3x4 (cores 0, 6, 7), RC 2x2 (cores 2, 3) */
	{4, 0, 2, 2, 0, 0, 4, 4},
	/* PIPEMUX = 7, RC 1x4 (core 0), RC 6x2 (cores 2, 3, 4, 5, 6, 7 */
	{4, 0, 2, 2, 2, 2, 2, 2},
	/* PIPEMUX = 8, EP 1x8 + RC 4x2 (cores 4, 5, 6, 7) */
	{0, 0, 0, 0, 2, 2, 2, 2},
	/* PIPEMUX = 9, EP 1x8 + RC 2x4 (cores 6, 7) */
	{0, 0, 0, 0, 0, 0, 4, 4},
	/* PIPEMUX = 10, EP 2x4 + RC 2x4 (cores 1, 6) */
	{0, 4, 0, 0, 0, 0, 4, 0},
	/* PIPEMUX = 11, EP 2x4 + RC 4x2 (cores 2, 3, 4, 5) */
	{0, 0, 2, 2, 2, 2, 0, 0},
	/* PIPEMUX = 12, EP 1x4 + RC 6x2 (cores 2, 3, 4, 5, 6, 7) */
	{0, 0, 2, 2, 2, 2, 2, 2},
	/* PIPEMUX = 13, EP 2x4 + RC 1x4 (core 6) + RC 2x2 (cores 2, 3) */
	{0, 0, 2, 2, 0, 0, 4, 0}
};

/*
 * function for writes to the Serdes registers through the PMI interface
 */
static int paxb_pmi_write(unsigned int core_idx, uint32_t pmi, uint32_t val)
{
	uint32_t status;
	unsigned int timeout = PMI_TIMEOUT_MS;

	paxb_rc_cfg_write(core_idx, CFG_RC_PMI_ADDR, pmi);

	val &= ~CFG_RC_RWCMD_MASK;
	val |= CFG_RC_WCMD_MASK;
	paxb_rc_cfg_write(core_idx, CFG_RC_PMI_WDATA, val);

	do {
		status = paxb_rc_cfg_read(core_idx, CFG_RC_PMI_WDATA);

		/* wait for write command bit to clear */
		if ((status & CFG_RC_WCMD_MASK) == 0)
			return 0;
	} while (--timeout);

	return -EIO;
}

/*
 * function for reads from the Serdes registers through the PMI interface
 */
static int paxb_pmi_read(unsigned int core_idx, uint32_t pmi, uint32_t *val)
{
	uint32_t status;
	unsigned int timeout = PMI_TIMEOUT_MS;

	paxb_rc_cfg_write(core_idx, CFG_RC_PMI_ADDR, pmi);

	paxb_rc_cfg_write(core_idx, CFG_RC_PMI_WDATA, CFG_RC_RCMD_MASK);

	do {
		status = paxb_rc_cfg_read(core_idx, CFG_RC_PMI_RDATA);

		/* wait for read ack bit set */
		if ((status & CFG_RC_RACK_MASK)) {
			*val = paxb_rc_cfg_read(core_idx, CFG_RC_PMI_RDATA);
			return 0;
		}
	} while (--timeout);

	return -EIO;
}


#ifndef BOARD_PCIE_EXT_CLK
/*
 * PCIe Override clock lookup table
 *
 * Each array index represents pcie override clock has been done
 * by CFW or not.
 */
static uint8_t pcie_override_clk_table[] = {
	/* PIPEMUX = 0, EP 1x16 */
	0x0,
	/* PIPEMUX = 1, EP 1x8 + RC 1x8, core 7 */
	0x1,
	/* PIPEMUX = 2, EP 4x4 */
	0x0,
	/* PIPEMUX = 3, RC 2x8, cores 0, 7 */
	0x0,
	/* PIPEMUX = 4, RC 4x4, cores 0, 1, 6, 7 */
	0x0,
	/* PIPEMUX = 5, RC 8x2, all 8 cores */
	0x0,
	/* PIPEMUX = 6, RC 3x4 + 2x2, cores 0, 2, 3, 6, 7 */
	0x0,
	/* PIPEMUX = 7, RC 1x4 + 6x2, cores 0, 2, 3, 4, 5, 6, 7 */
	0x0,
	/* PIPEMUX = 8, EP 1x8 + RC 4x2, cores 4, 5, 6, 7 */
	0x0,
	/* PIPEMUX = 9, EP 1x8 + RC 2x4, cores 6, 7 */
	0x0,
	/* PIPEMUX = 10, EP 2x4 + RC 2x4, cores 1, 6 */
	0x0,
	/* PIPEMUX = 11, EP 2x4 + RC 4x2, cores 2, 3, 4, 5 */
	0x0,
	/* PIPEMUX = 12, EP 1x4 + RC 6x2, cores 2, 3, 4, 5, 6, 7 */
	0x0,
	/* PIPEMUX = 13, RC 2x4 + RC 1x4 + 2x2, cores 2, 3, 6 */
	0x0,
};

/*
 * Bring up LCPLL channel 0 reference clock for PCIe serdes used in RC mode
 */
static int pcie_lcpll_init(void)
{
	uintptr_t reg;
	unsigned int timeout = PCIE_LCPLL_TIMEOUT_MS;
	uint32_t val;

	if (pcie_override_clk_table[pipemux_idx]) {
		/*
		 * Check rc_mode_override again to avoid halt
		 * because of cfw uninitialized lcpll.
		 */
		reg = (uintptr_t)(PCIE_LCPLL_BASE +
				  PCIE_PIPE_MUX_RC_MODE_OVERRIDE_CFG);
		val = mmio_read_32(reg);
		if (val & 0x1)
			return 0;
		else
			return -ENODEV;
	}

	/* power on PCIe LCPLL and its LDO */
	reg = (uintptr_t)CRMU_AON_CTRL1;
	mmio_setbits_32(reg, CRMU_PCIE_LCPLL_PWR_ON_MASK |
			     CRMU_PCIE_LCPLL_PWRON_LDO_MASK);
	udelay(PCIE_LCPLL_DELAY_US);

	/* remove isolation */
	mmio_clrbits_32(reg, CRMU_PCIE_LCPLL_ISO_IN_MASK);
	udelay(PCIE_LCPLL_DELAY_US);

	/* disconnect termination */
	reg = (uintptr_t)(PCIE_LCPLL_BASE + PCIE_LCPLL_CTRL13_OFFSET);
	mmio_setbits_32(reg, PCIE_LCPLL_D2C2_TERM_DISC <<
			PCIE_LCPLL_D2C2_CTRL_SHIFT);

	/* enable CML buf1/2 and D2C2 */
	reg = (uintptr_t)(PCIE_LCPLL_BASE + PCIE_LCPLL_CTRL3_OFFSET);
	mmio_setbits_32(reg, PCIE_LCPLL_CM_ENA << PCIE_LCPLL_EN_CTRL_SHIFT);

	/* select diff clock mux out as ref clock */
	mmio_clrbits_32(reg, PCIE_LCPLL_REF_CLK_MASK);

	/* delay for 500 microseconds per ASIC spec for PCIe LCPLL */
	udelay(PCIE_LCPLL_DELAY_US);

	/* now bring PCIe LCPLL out of reset */
	reg = (uintptr_t)(PCIE_LCPLL_BASE + PCIE_LCPLL_CTRL0_OFFSET);
	mmio_setbits_32(reg, PCIE_LCPLL_RESETB_MASK);

	/* wait for PLL to lock */
	reg = (uintptr_t)(PCIE_LCPLL_BASE + PCIE_LCPLL_STATUS_OFFSET);
	do {
		val = mmio_read_32(reg);
		if ((val & PCIE_LCPLL_LOCK_MASK) == PCIE_LCPLL_LOCK_MASK) {
			/* now bring the post divider out of reset */
			reg = (uintptr_t)(PCIE_LCPLL_BASE +
					  PCIE_LCPLL_CTRL0_OFFSET);
			mmio_setbits_32(reg, PCIE_LCPLL_P_RESETB_MASK);
			VERBOSE("PCIe LCPLL locked\n");
			return 0;
		}
		mdelay(1);
	} while (--timeout);

	ERROR("PCIe LCPLL failed to lock\n");
	return -EIO;
}
#else
/*
 * Bring up EXT CLK reference clock for PCIe serdes used in RC mode
 * XTAL_BYPASS		(3 << 0)
 * INTR_LC_REF		(5 << 0)
 * PD_CML_LC_REF_OUT	(1 << 4)
 * PD_CML_REF_CH_OUT	(1 << 8)
 * CLK_MASTER_SEL	(1 << 11)
 * CLK_MASTER_CTRL_A	(1 << 12)
 * CLK_MASTER_CTRL_B	(2 << 14)
 */
static const uint16_t pcie_ext_clk[][NUM_OF_PCIE_SERDES] = {
	/* PIPEMUX = 0, EP 1x16 */
	{0},
	/* PIPEMUX = 1, EP 1x8 + RC 1x8, core 7 */
	{0},
	/* PIPEMUX = 2, EP 4x4 */
	{0},
	/* PIPEMUX = 3, RC 2x8, cores 0, 7 */
	{0x8803, 0x9115, 0x9115, 0x1115, 0x8803, 0x9115, 0x9115, 0x1115},
	/* PIPEMUX = 4, RC 4x4, cores 0, 1, 6, 7 */
	{0x8803, 0x1115, 0x8915, 0x1115, 0x8803, 0x1115, 0x8915, 0x1115,},
	/* PIPEMUX = 5, RC 8x2, all 8 cores */
	{0x0803, 0x0915, 0x0915, 0x0915, 0x0803, 0x0915, 0x0915, 0x0915,},
	/* PIPEMUX = 6, RC 3x4 + 2x2, cores 0, 2, 3, 6, 7 */
	{0},
	/* PIPEMUX = 7, RC 1x4 + 6x2, cores 0, 2, 3, 4, 5, 6, 7 */
	{0},
	/* PIPEMUX = 8, EP 1x8 + RC 4x2, cores 4, 5, 6, 7 */
	{0},
	/* PIPEMUX = 9, EP 1x8 + RC 2x4, cores 6, 7 */
	{0},
	/* PIPEMUX = 10, EP 2x4 + RC 2x4, cores 1, 6 */
	{0},
	/* PIPEMUX = 11, EP 2x4 + RC 4x2, cores 2, 3, 4, 5 */
	{0},
	/* PIPEMUX = 12, EP 1x4 + RC 6x2, cores 2, 3, 4, 5, 6, 7 */
	{0},
	/* PIPEMUX = 13, RC 2x4 + RC 1x4 + 2x2, cores 2, 3, 6 */
	{0},
};

static void pcie_ext_clk_init(void)
{
	unsigned int serdes;
	uint32_t val;

	for (serdes = 0; serdes < NUM_OF_PCIE_SERDES; serdes++) {
		val = pcie_ext_clk[pipemux_idx][serdes];
		if (!val)
			return;
		mmio_write_32(PCIE_CORE_RESERVED_CFG +
			      serdes * PCIE_CORE_PWR_OFFSET, val);
	}
	/* disable CML buf1/2 and enable D2C2 */
	mmio_clrsetbits_32((PCIE_LCPLL_BASE + PCIE_LCPLL_CTRL3_OFFSET),
			PCIE_LCPLL_CM_BUF_ENA << PCIE_LCPLL_EN_CTRL_SHIFT,
			PCIE_LCPLL_D2C2_ENA << PCIE_LCPLL_EN_CTRL_SHIFT);
	mmio_write_32(PCIE_LCPLL_BASE + PCIE_TX_CLKMASTER_CTRL_OVERRIDE_CFG, 1);
	INFO("Overriding Clocking - using REF clock from PAD...\n");
}
#endif

static int load_uc(unsigned int core_idx)
{
	return 0;
}

static int paxb_serdes_gate_clock(unsigned int core_idx, int gate_clk)
{
	unsigned int link_width, serdes, nr_serdes;
	uintptr_t pmi_base;
	unsigned int rdata;
	uint32_t core_offset = core_idx * PCIE_CORE_PWR_OFFSET;

	link_width = paxb->get_link_width(core_idx);
	if (!link_width) {
		ERROR("Unsupported PIPEMUX\n");
		return -EOPNOTSUPP;
	}

	nr_serdes = link_width / 2;
	pmi_base = (uintptr_t)(PCIE_CORE_PMI_CFG_BASE + core_offset);

	for (serdes = 0; serdes < nr_serdes; serdes++) {
		mmio_write_32(pmi_base, serdes);
		paxb_pmi_read(core_idx, PMI_ADDR_LANE0(PMI_PLL_CTRL_4), &rdata);
		if (!gate_clk)
			rdata |= PMI_SERDES_CLK_ENABLE;
		else
			rdata &= ~PMI_SERDES_CLK_ENABLE;
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(PMI_PLL_CTRL_4), rdata);
	}
	return 0;
}

static int paxb_gen3_serdes_init(unsigned int core_idx, uint32_t nSerdes)
{
	uint32_t rdata;
	int serdes;
	uintptr_t pmi_base;
	unsigned int timeout;
	unsigned int reg_d230, reg_d267;


	pmi_base = (uintptr_t)(PCIE_CORE_PMI_CFG_BASE +
			(core_idx * PCIE_CORE_PWR_OFFSET));

	for (serdes = 0; serdes < nSerdes; serdes++) {
		/* select the PMI interface */
		mmio_write_32(pmi_base, serdes);

		/* Clock enable */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_CLK_CTRL0),
				0x3);

		/* Release reset of master */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_RST_CTRL0),
				0x1);

		/* clearing PRAM memory */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_AHB_CTRL0),
				0x100);

		timeout = UC_RAM_INIT_TIMEOUT;
		do {
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE0(UC_A_AHB_STAT0),
					&rdata);
		} while ((rdata & 0x01) == 0 && timeout--);

		if (!timeout)
			return -EIO;

		timeout = UC_RAM_INIT_TIMEOUT;
		do {
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE1(UC_A_AHB_STAT0),
					&rdata);
		} while ((rdata & 0x01) == 0 && timeout--);

		if (!timeout)
			return -EIO;

		/* clearing PRAM memory */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_AHB_CTRL0),
				0);

		/* to identify 2 lane serdes */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_DBG1), 0x1);

		/* De-Assert Pram & master resets */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_RST_CTRL0),
				0x9);

		if (load_uc(core_idx))
			return -EIO;

		/* UC UC ready for command */
		paxb_pmi_read(core_idx, PMI_ADDR_LANE0(DSC_UC_CTRL),
				&rdata);
		rdata |= DSC_UC_CTRL_RDY_CMD;
		paxb_pmi_write(core_idx, PMI_ADDR_LANE0(DSC_UC_CTRL),
				rdata);

		paxb_pmi_read(core_idx, PMI_ADDR_LANE1(DSC_UC_CTRL),
				&rdata);
		rdata |= DSC_UC_CTRL_RDY_CMD;
		paxb_pmi_write(core_idx, PMI_ADDR_LANE1(DSC_UC_CTRL),
				rdata);

		/* Lane reset */
		paxb_pmi_write(core_idx,
				PMI_ADDR_BCAST(LANE_DBG_RST_CTRL), 0x3);

		/* De-Assert Core and Master resets */
		paxb_pmi_write(core_idx, PMI_ADDR_BCAST(UC_A_RST_CTRL0),
				0x3);

		timeout = UC_INIT_TIMEOUT;
		while (timeout--) {
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE0(UC_VERSION_NUM),
					&reg_d230);
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE0(DSC_SM_CTL22),
					&reg_d267);

			if (((reg_d230 & 0xffff) != 0) &
					((reg_d267 & 0xc000) == 0xc000)) {
				break;
			}
			mdelay(1);
		}

		if (!timeout)
			return -EIO;

		timeout = UC_INIT_TIMEOUT;
		while (timeout--) {
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE1(UC_VERSION_NUM),
					&reg_d230);
			paxb_pmi_read(core_idx,
					PMI_ADDR_LANE1(DSC_SM_CTL22),
					&reg_d267);

			if (((reg_d230 & 0xffff) != 0) &
					((reg_d267 & 0xc000) == 0xc000)) {
				break;
			}
			mdelay(1);
		}

		if (!timeout)
			return -EIO;
	}
	return 0;
}

static int pcie_serdes_requires_patch(unsigned int serdes_idx)
{
	if (pipemux_idx != SERDES_PATCH_PIPEMUX_INDEX)
		return 0;

	return !!((SERDES_PATCH_INDEX >> serdes_idx) & 0x1);
}

static void pcie_tx_coeff_p7(unsigned int core_idx)
{
	paxb_pmi_write(core_idx, PMI_ADDR_BCAST(0xd11b), 0x00aa);
	paxb_pmi_write(core_idx, PMI_ADDR_BCAST(0xd11c), 0x1155);
	paxb_pmi_write(core_idx, PMI_ADDR_BCAST(0xd11d), 0x2449);
	paxb_pmi_write(core_idx, PMI_ADDR_BCAST(0xd11e), 0x000f);
	paxb_pmi_write(core_idx, PMI_ADDR_BCAST(0xd307), 0x0001);
}


static unsigned int paxb_sr_get_rc_link_width(unsigned int core_idx)
{
	return link_width_table[pipemux_idx][core_idx];
}

static uint32_t paxb_sr_get_rc_link_speed(void)
{
	return GEN3_LINK_SPEED;
}


static int paxb_serdes_init(unsigned int core_idx, unsigned int nr_serdes)
{
	uint32_t core_offset = core_idx * PCIE_CORE_PWR_OFFSET;
	unsigned int serdes;
	uintptr_t pmi_base;
	int ret;

	/*
	 * Each serdes has a x2 link width
	 *
	 * Use PAXB to patch the serdes for proper RX termination through the
	 * PMI interface
	 */
	pmi_base = (uintptr_t)(PCIE_CORE_PMI_CFG_BASE + core_offset);
	for (serdes = 0; serdes < nr_serdes; serdes++) {
		/* select the PMI interface */
		mmio_write_32(pmi_base, serdes);

		/* patch Serdes for RX termination */
		ret = paxb_pmi_write(core_idx, PMI_RX_TERM_SEQ,
				     PMI_RX_TERM_VAL);
		if (ret)
			goto err_pmi;

		ret = paxb_pmi_write(core_idx, MERLIN16_PCIE_BLK2_PWRMGMT_7,
				     MERLIN16_PCIE_BLK2_PWRMGMT_7_VAL);
		if (ret)
			goto err_pmi;

		ret = paxb_pmi_write(core_idx, MERLIN16_PCIE_BLK2_PWRMGMT_8,
				     MERLIN16_PCIE_BLK2_PWRMGMT_8_VAL);
		if (ret)
			goto err_pmi;

		ret = paxb_pmi_write(core_idx, MERLIN16_AMS_TX_CTRL_5,
				     MERLIN16_AMS_TX_CTRL_5_VAL);
		if (ret)
			goto err_pmi;

		pcie_tx_coeff_p7(core_idx);

		if (pcie_serdes_requires_patch(serdes)) {
			if (((core_idx == 0) || (core_idx == 7))) {
				ret = paxb_pmi_write(core_idx,
						PMI_X8_CORE0_7_PATCH_SEQ,
						PMI_X8_CORE0_7_PATCH_VAL);
				if (ret)
					goto err_pmi;
			}
		}
	}

	return 0;

err_pmi:
	ERROR("PCIe PMI write failed\n");
	return ret;
}

static int paxb_sr_phy_init(void)
{
	int ret;
	unsigned int core_idx;

#ifndef BOARD_PCIE_EXT_CLK
	ret = pcie_lcpll_init();
	if (ret)
		return ret;
#else
	pcie_ext_clk_init();
#endif

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;
		unsigned int link_width;

		paxb_serdes_gate_clock(core_idx, 0);

		link_width = paxb->get_link_width(core_idx);
		if (!link_width) {
			ERROR("Unsupported PIPEMUX\n");
			return -EOPNOTSUPP;
		}

		ret = paxb_serdes_init(core_idx, link_width / 2);
		if (ret) {
			ERROR("PCIe serdes initialization failed for core %u\n",
			      core_idx);
			return ret;
		}


		ret = paxb_gen3_serdes_init(core_idx, link_width / 2);
		if (ret) {
			ERROR("PCIe GEN3 serdes initialization failed\n");
			return ret;
		}

	}
	return 0;
}

const paxb_cfg sr_paxb_cfg = {
	.type = PAXB_SR,
	.device_id = SR_B0_DEVICE_ID,
	.pipemux_init = pipemux_sr_init,
	.phy_init = paxb_sr_phy_init,
	.core_needs_enable = paxb_sr_core_needs_enable,
	.num_cores = NUM_OF_SR_PCIE_CORES,
	.get_link_width = paxb_sr_get_rc_link_width,
	.get_link_speed = paxb_sr_get_rc_link_speed,
};

const paxb_cfg *paxb_get_sr_config(void)
{
	return &sr_paxb_cfg;
}
