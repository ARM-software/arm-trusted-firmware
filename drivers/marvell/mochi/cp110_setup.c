/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* CP110 Marvell SoC driver */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/marvell/amb_adec.h>
#include <drivers/marvell/iob.h>
#include <drivers/marvell/mochi/cp110_setup.h>

#include <plat_marvell.h>

/*
 * AXI Configuration.
 */

 /* Used for Units of CP-110 (e.g. USB device, USB Host, and etc) */
#define MVEBU_AXI_ATTR_OFFSET			(0x441300)
#define MVEBU_AXI_ATTR_REG(index)		(MVEBU_AXI_ATTR_OFFSET + \
							0x4 * index)

/* AXI Protection bits */
#define MVEBU_AXI_PROT_OFFSET				(0x441200)

/* AXI Protection regs */
#define MVEBU_AXI_PROT_REG(index)		((index <= 4) ? \
						(MVEBU_AXI_PROT_OFFSET + \
							0x4 * index) : \
						(MVEBU_AXI_PROT_OFFSET + 0x18))
#define MVEBU_AXI_PROT_REGS_NUM			(6)

#define MVEBU_SOC_CFGS_OFFSET			(0x441900)
#define MVEBU_SOC_CFG_REG(index)		(MVEBU_SOC_CFGS_OFFSET + \
							0x4 * index)
#define MVEBU_SOC_CFG_REG_NUM			(0)
#define MVEBU_SOC_CFG_GLOG_SECURE_EN_MASK	(0xE)

/* SATA3 MBUS to AXI regs */
#define MVEBU_BRIDGE_WIN_DIS_REG		(MVEBU_SOC_CFGS_OFFSET + 0x10)
#define MVEBU_BRIDGE_WIN_DIS_OFF		(0x0)

/* SATA3 MBUS to AXI regs */
#define MVEBU_SATA_M2A_AXI_PORT_CTRL_REG	(0x54ff04)

/* AXI to MBUS bridge registers */
#define MVEBU_AMB_IP_OFFSET			(0x13ff00)
#define MVEBU_AMB_IP_BRIDGE_WIN_REG(win)	(MVEBU_AMB_IP_OFFSET + \
							(win * 0x8))
#define MVEBU_AMB_IP_BRIDGE_WIN_EN_OFFSET	0
#define MVEBU_AMB_IP_BRIDGE_WIN_EN_MASK		\
				(0x1 << MVEBU_AMB_IP_BRIDGE_WIN_EN_OFFSET)
#define MVEBU_AMB_IP_BRIDGE_WIN_SIZE_OFFSET	16
#define MVEBU_AMB_IP_BRIDGE_WIN_SIZE_MASK	\
				(0xffffu << MVEBU_AMB_IP_BRIDGE_WIN_SIZE_OFFSET)

#define MVEBU_SAMPLE_AT_RESET_REG	(0x440600)
#define SAR_PCIE1_CLK_CFG_OFFSET	31
#define SAR_PCIE1_CLK_CFG_MASK		(0x1u << SAR_PCIE1_CLK_CFG_OFFSET)
#define SAR_PCIE0_CLK_CFG_OFFSET	30
#define SAR_PCIE0_CLK_CFG_MASK		(0x1 << SAR_PCIE0_CLK_CFG_OFFSET)
#define SAR_I2C_INIT_EN_OFFSET		24
#define SAR_I2C_INIT_EN_MASK		(1 << SAR_I2C_INIT_EN_OFFSET)

/*******************************************************************************
 * PCIE clock buffer control
 ******************************************************************************/
#define MVEBU_PCIE_REF_CLK_BUF_CTRL			(0x4404F0)
#define PCIE1_REFCLK_BUFF_SOURCE			0x800
#define PCIE0_REFCLK_BUFF_SOURCE			0x400

/*******************************************************************************
 * MSS Device Push Set Register
 ******************************************************************************/
#define MVEBU_CP_MSS_DPSHSR_REG				(0x280040)
#define MSS_DPSHSR_REG_PCIE_CLK_SEL			0x8

/*******************************************************************************
 * RTC Configuration
 ******************************************************************************/
#define MVEBU_RTC_BASE					(0x284000)
#define MVEBU_RTC_STATUS_REG				(MVEBU_RTC_BASE + 0x0)
#define MVEBU_RTC_STATUS_ALARM1_MASK			0x1
#define MVEBU_RTC_STATUS_ALARM2_MASK			0x2
#define MVEBU_RTC_IRQ_1_CONFIG_REG			(MVEBU_RTC_BASE + 0x4)
#define MVEBU_RTC_IRQ_2_CONFIG_REG			(MVEBU_RTC_BASE + 0x8)
#define MVEBU_RTC_TIME_REG				(MVEBU_RTC_BASE + 0xC)
#define MVEBU_RTC_ALARM_1_REG				(MVEBU_RTC_BASE + 0x10)
#define MVEBU_RTC_ALARM_2_REG				(MVEBU_RTC_BASE + 0x14)
#define MVEBU_RTC_CCR_REG				(MVEBU_RTC_BASE + 0x18)
#define MVEBU_RTC_NOMINAL_TIMING			0x2000
#define MVEBU_RTC_NOMINAL_TIMING_MASK			0x7FFF
#define MVEBU_RTC_TEST_CONFIG_REG			(MVEBU_RTC_BASE + 0x1C)
#define MVEBU_RTC_BRIDGE_TIMING_CTRL0_REG		(MVEBU_RTC_BASE + 0x80)
#define MVEBU_RTC_WRCLK_PERIOD_MASK			0xFFFF
#define MVEBU_RTC_WRCLK_PERIOD_DEFAULT			0x3FF
#define MVEBU_RTC_WRCLK_SETUP_OFFS			16
#define MVEBU_RTC_WRCLK_SETUP_MASK			0xFFFF0000
#define MVEBU_RTC_WRCLK_SETUP_DEFAULT			0x29
#define MVEBU_RTC_BRIDGE_TIMING_CTRL1_REG		(MVEBU_RTC_BASE + 0x84)
#define MVEBU_RTC_READ_OUTPUT_DELAY_MASK		0xFFFF
#define MVEBU_RTC_READ_OUTPUT_DELAY_DEFAULT		0x1F

enum axi_attr {
	AXI_ADUNIT_ATTR = 0,
	AXI_COMUNIT_ATTR,
	AXI_EIP197_ATTR,
	AXI_USB3D_ATTR,
	AXI_USB3H0_ATTR,
	AXI_USB3H1_ATTR,
	AXI_SATA0_ATTR,
	AXI_SATA1_ATTR,
	AXI_DAP_ATTR,
	AXI_DFX_ATTR,
	AXI_DBG_TRC_ATTR = 12,
	AXI_SDIO_ATTR,
	AXI_MSS_ATTR,
	AXI_MAX_ATTR,
};

/* Most stream IDS are configured centrally in the CP-110 RFU
 * but some are configured inside the unit registers
 */
#define RFU_STREAM_ID_BASE	(0x450000)
#define USB3H_0_STREAM_ID_REG	(RFU_STREAM_ID_BASE + 0xC)
#define USB3H_1_STREAM_ID_REG	(RFU_STREAM_ID_BASE + 0x10)
#define SATA_0_STREAM_ID_REG	(RFU_STREAM_ID_BASE + 0x14)
#define SATA_1_STREAM_ID_REG	(RFU_STREAM_ID_BASE + 0x18)

#define CP_DMA_0_STREAM_ID_REG  (0x6B0010)
#define CP_DMA_1_STREAM_ID_REG  (0x6D0010)

/* We allocate IDs 128-255 for PCIe */
#define MAX_STREAM_ID		(0x80)

uintptr_t stream_id_reg[] = {
	USB3H_0_STREAM_ID_REG,
	USB3H_1_STREAM_ID_REG,
	CP_DMA_0_STREAM_ID_REG,
	CP_DMA_1_STREAM_ID_REG,
	SATA_0_STREAM_ID_REG,
	SATA_1_STREAM_ID_REG,
	0
};

static void cp110_errata_wa_init(uintptr_t base)
{
	uint32_t data;

	/* ERRATA GL-4076863:
	 * Reset value for global_secure_enable inputs must be changed
	 * from '1' to '0'.
	 * When asserted, only "secured" transactions can enter IHB
	 * configuration space.
	 * However, blocking AXI transactions is performed by IOB.
	 * Performing it also at IHB/HB complicates programming model.
	 *
	 * Enable non-secure access in SOC configuration register
	 */
	data = mmio_read_32(base + MVEBU_SOC_CFG_REG(MVEBU_SOC_CFG_REG_NUM));
	data &= ~MVEBU_SOC_CFG_GLOG_SECURE_EN_MASK;
	mmio_write_32(base + MVEBU_SOC_CFG_REG(MVEBU_SOC_CFG_REG_NUM), data);
}

static void cp110_pcie_clk_cfg(uintptr_t base)
{
	uint32_t pcie0_clk, pcie1_clk, reg;

	/*
	 * Determine the pcie0/1 clock direction (input/output) from the
	 * sample at reset.
	 */
	reg = mmio_read_32(base + MVEBU_SAMPLE_AT_RESET_REG);
	pcie0_clk = (reg & SAR_PCIE0_CLK_CFG_MASK) >> SAR_PCIE0_CLK_CFG_OFFSET;
	pcie1_clk = (reg & SAR_PCIE1_CLK_CFG_MASK) >> SAR_PCIE1_CLK_CFG_OFFSET;

	/* CP110 revision A2 */
	if (cp110_rev_id_get(base) == MVEBU_CP110_REF_ID_A2) {
		/*
		 * PCIe Reference Clock Buffer Control register must be
		 * set according to the clock direction (input/output)
		 */
		reg = mmio_read_32(base + MVEBU_PCIE_REF_CLK_BUF_CTRL);
		reg &= ~(PCIE0_REFCLK_BUFF_SOURCE | PCIE1_REFCLK_BUFF_SOURCE);
		if (!pcie0_clk)
			reg |= PCIE0_REFCLK_BUFF_SOURCE;
		if (!pcie1_clk)
			reg |= PCIE1_REFCLK_BUFF_SOURCE;

		mmio_write_32(base + MVEBU_PCIE_REF_CLK_BUF_CTRL, reg);
	}

	/* CP110 revision A1 */
	if (cp110_rev_id_get(base) == MVEBU_CP110_REF_ID_A1) {
		if (!pcie0_clk || !pcie1_clk) {
			/*
			 * if one of the pcie clocks is set to input,
			 * we need to set mss_push[131] field, otherwise,
			 * the pcie clock might not work.
			 */
			reg = mmio_read_32(base + MVEBU_CP_MSS_DPSHSR_REG);
			reg |= MSS_DPSHSR_REG_PCIE_CLK_SEL;
			mmio_write_32(base + MVEBU_CP_MSS_DPSHSR_REG, reg);
		}
	}
}

/* Set a unique stream id for all DMA capable devices */
static void cp110_stream_id_init(uintptr_t base, uint32_t stream_id)
{
	int i = 0;

	while (stream_id_reg[i]) {
		if (i > MAX_STREAM_ID_PER_CP) {
			NOTICE("Only first %d (maximum) Stream IDs allocated\n",
			       MAX_STREAM_ID_PER_CP);
			return;
		}

		if ((stream_id_reg[i] == CP_DMA_0_STREAM_ID_REG) ||
		    (stream_id_reg[i] == CP_DMA_1_STREAM_ID_REG))
			mmio_write_32(base + stream_id_reg[i],
				      stream_id << 16 |  stream_id);
		else
			mmio_write_32(base + stream_id_reg[i], stream_id);

		/* SATA port 0/1 are in the same SATA unit, and they should use
		 * the same STREAM ID number
		 */
		if (stream_id_reg[i] != SATA_0_STREAM_ID_REG)
			stream_id++;

		i++;
	}
}

static void cp110_axi_attr_init(uintptr_t base)
{
	uint32_t index, data;

	/* Initialize AXI attributes for Armada-7K/8K SoC */

	/* Go over the AXI attributes and set Ax-Cache and Ax-Domain */
	for (index = 0; index < AXI_MAX_ATTR; index++) {
		switch (index) {
		/* DFX and MSS unit works with no coherent only -
		 * there's no option to configure the Ax-Cache and Ax-Domain
		 */
		case AXI_DFX_ATTR:
		case AXI_MSS_ATTR:
			continue;
		default:
			/* Set Ax-Cache as cacheable, no allocate, modifiable,
			 * bufferable
			 * The values are different because Read & Write
			 * definition is different in Ax-Cache
			 */
			data = mmio_read_32(base + MVEBU_AXI_ATTR_REG(index));
			data &= ~MVEBU_AXI_ATTR_ARCACHE_MASK;
			data |= (CACHE_ATTR_WRITE_ALLOC |
				 CACHE_ATTR_CACHEABLE   |
				 CACHE_ATTR_BUFFERABLE) <<
				 MVEBU_AXI_ATTR_ARCACHE_OFFSET;
			data &= ~MVEBU_AXI_ATTR_AWCACHE_MASK;
			data |= (CACHE_ATTR_READ_ALLOC |
				 CACHE_ATTR_CACHEABLE  |
				 CACHE_ATTR_BUFFERABLE) <<
				 MVEBU_AXI_ATTR_AWCACHE_OFFSET;
			/* Set Ax-Domain as Outer domain */
			data &= ~MVEBU_AXI_ATTR_ARDOMAIN_MASK;
			data |= DOMAIN_OUTER_SHAREABLE <<
				MVEBU_AXI_ATTR_ARDOMAIN_OFFSET;
			data &= ~MVEBU_AXI_ATTR_AWDOMAIN_MASK;
			data |= DOMAIN_OUTER_SHAREABLE <<
				MVEBU_AXI_ATTR_AWDOMAIN_OFFSET;
			mmio_write_32(base + MVEBU_AXI_ATTR_REG(index), data);
		}
	}

	/* SATA IOCC supported, cache attributes
	 * for SATA MBUS to AXI configuration.
	 */
	data = mmio_read_32(base + MVEBU_SATA_M2A_AXI_PORT_CTRL_REG);
	data &= ~MVEBU_SATA_M2A_AXI_AWCACHE_MASK;
	data |= (CACHE_ATTR_WRITE_ALLOC |
		 CACHE_ATTR_CACHEABLE   |
		 CACHE_ATTR_BUFFERABLE) <<
		 MVEBU_SATA_M2A_AXI_AWCACHE_OFFSET;
	data &= ~MVEBU_SATA_M2A_AXI_ARCACHE_MASK;
	data |= (CACHE_ATTR_READ_ALLOC |
		 CACHE_ATTR_CACHEABLE  |
		 CACHE_ATTR_BUFFERABLE) <<
		 MVEBU_SATA_M2A_AXI_ARCACHE_OFFSET;
	mmio_write_32(base + MVEBU_SATA_M2A_AXI_PORT_CTRL_REG, data);

	/* Set all IO's AXI attribute to non-secure access. */
	for (index = 0; index < MVEBU_AXI_PROT_REGS_NUM; index++)
		mmio_write_32(base + MVEBU_AXI_PROT_REG(index),
			      DOMAIN_SYSTEM_SHAREABLE);
}

void cp110_amb_init(uintptr_t base)
{
	uint32_t reg;

	/* Open AMB bridge Window to Access COMPHY/MDIO registers */
	reg = mmio_read_32(base + MVEBU_AMB_IP_BRIDGE_WIN_REG(0));
	reg &= ~(MVEBU_AMB_IP_BRIDGE_WIN_SIZE_MASK |
		 MVEBU_AMB_IP_BRIDGE_WIN_EN_MASK);
	reg |= (0x7ff << MVEBU_AMB_IP_BRIDGE_WIN_SIZE_OFFSET) |
	       (0x1 << MVEBU_AMB_IP_BRIDGE_WIN_EN_OFFSET);
	mmio_write_32(base + MVEBU_AMB_IP_BRIDGE_WIN_REG(0), reg);
}

static void cp110_rtc_init(uintptr_t base)
{
	/* Update MBus timing parameters before accessing RTC registers */
	mmio_clrsetbits_32(base + MVEBU_RTC_BRIDGE_TIMING_CTRL0_REG,
			   MVEBU_RTC_WRCLK_PERIOD_MASK,
			   MVEBU_RTC_WRCLK_PERIOD_DEFAULT);

	mmio_clrsetbits_32(base + MVEBU_RTC_BRIDGE_TIMING_CTRL0_REG,
			   MVEBU_RTC_WRCLK_SETUP_MASK,
			   MVEBU_RTC_WRCLK_SETUP_DEFAULT <<
			   MVEBU_RTC_WRCLK_SETUP_OFFS);

	mmio_clrsetbits_32(base + MVEBU_RTC_BRIDGE_TIMING_CTRL1_REG,
			   MVEBU_RTC_READ_OUTPUT_DELAY_MASK,
			   MVEBU_RTC_READ_OUTPUT_DELAY_DEFAULT);

	/*
	 * Issue reset to the RTC if Clock Correction register
	 * contents did not sustain the reboot/power-on.
	 */
	if ((mmio_read_32(base + MVEBU_RTC_CCR_REG) &
	    MVEBU_RTC_NOMINAL_TIMING_MASK) != MVEBU_RTC_NOMINAL_TIMING) {
		/* Reset Test register */
		mmio_write_32(base + MVEBU_RTC_TEST_CONFIG_REG, 0);
		mdelay(500);

		/* Reset Status register */
		mmio_write_32(base + MVEBU_RTC_STATUS_REG,
			      (MVEBU_RTC_STATUS_ALARM1_MASK |
			      MVEBU_RTC_STATUS_ALARM2_MASK));
		udelay(62);

		/* Turn off Int1 and Int2 sources & clear the Alarm count */
		mmio_write_32(base + MVEBU_RTC_IRQ_1_CONFIG_REG, 0);
		mmio_write_32(base + MVEBU_RTC_IRQ_2_CONFIG_REG, 0);
		mmio_write_32(base + MVEBU_RTC_ALARM_1_REG, 0);
		mmio_write_32(base + MVEBU_RTC_ALARM_2_REG, 0);

		/* Setup nominal register access timing */
		mmio_write_32(base + MVEBU_RTC_CCR_REG,
			      MVEBU_RTC_NOMINAL_TIMING);

		/* Reset Status register */
		mmio_write_32(base + MVEBU_RTC_STATUS_REG,
			      (MVEBU_RTC_STATUS_ALARM1_MASK |
			      MVEBU_RTC_STATUS_ALARM2_MASK));
		udelay(50);
	}
}

static void cp110_amb_adec_init(uintptr_t base)
{
	/* enable AXI-MBUS by clearing "Bridge Windows Disable" */
	mmio_clrbits_32(base + MVEBU_BRIDGE_WIN_DIS_REG,
			(1 << MVEBU_BRIDGE_WIN_DIS_OFF));

	/* configure AXI-MBUS windows for CP */
	init_amb_adec(base);
}

void cp110_init(uintptr_t cp110_base, uint32_t stream_id)
{
	INFO("%s: Initialize CPx - base = %lx\n", __func__, cp110_base);

	/* configure IOB windows for CP0*/
	init_iob(cp110_base);

	/* configure AXI-MBUS windows for CP0*/
	cp110_amb_adec_init(cp110_base);

	/* configure axi for CP0*/
	cp110_axi_attr_init(cp110_base);

	/* Execute SW WA for erratas */
	cp110_errata_wa_init(cp110_base);

	/* Confiure pcie clock according to clock direction */
	cp110_pcie_clk_cfg(cp110_base);

	/* configure stream id for CP0 */
	cp110_stream_id_init(cp110_base, stream_id);

	/* Open AMB bridge for comphy for CP0 & CP1*/
	cp110_amb_init(cp110_base);

	/* Reset RTC if needed */
	cp110_rtc_init(cp110_base);
}

/* Do the minimal setup required to configure the CP in BLE */
void cp110_ble_init(uintptr_t cp110_base)
{
#if PCI_EP_SUPPORT
	INFO("%s: Initialize CPx - base = %lx\n", __func__, cp110_base);

	cp110_amb_init(cp110_base);

	/* Configure PCIe clock */
	cp110_pcie_clk_cfg(cp110_base);

	/* Configure PCIe endpoint */
	ble_plat_pcie_ep_setup();
#endif
}
