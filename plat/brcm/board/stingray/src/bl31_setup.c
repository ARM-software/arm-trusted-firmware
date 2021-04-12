/*
 * Copyright (c) 2015 - 2021, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a72.h>
#include <drivers/arm/sp805.h>
#include <drivers/console.h>
#include <drivers/delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>

#include <bl33_info.h>
#include <chimp.h>
#include <cmn_plat_util.h>
#include <dmu.h>
#include <fsx.h>
#include <iommu.h>
#include <ncsi.h>
#include <paxb.h>
#include <paxc.h>
#include <platform_def.h>
#ifdef USE_USB
#include <platform_usb.h>
#endif
#include <sdio.h>
#include <sr_utils.h>
#include <timer_sync.h>

/*******************************************************************************
 * Perform any BL3-1 platform setup common to ARM standard platforms
 ******************************************************************************/

static void brcm_stingray_gain_qspi_control(void)
{
	if (boot_source_get() != BOOT_SOURCE_QSPI) {
		if (bcm_chimp_is_nic_mode() &&
		(!bcm_chimp_handshake_done())) {
			/*
			 * Last chance to wait for ChiMP firmware to report
			 * "I am done" before grabbing the QSPI
			 */
			WARN("ChiMP still not booted\n");
#ifndef CHIMP_ALWAYS_NEEDS_QSPI
			WARN("ChiMP is given the last chance to boot (%d s)\n",
				CHIMP_HANDSHAKE_TIMEOUT_MS / 1000);

			if (!bcm_chimp_wait_handshake()) {
				ERROR("ChiMP failed to boot\n");
			} else {
				INFO("ChiMP booted successfully\n");
			}
#endif
		}

#ifndef CHIMP_ALWAYS_NEEDS_QSPI
		INFO("AP grabs QSPI\n");
		/*
		 * For QSPI boot sbl/bl1 has already taken care.
		 * For other boot sources QSPI needs to be muxed to
		 * AP for exclusive use
		 */
		brcm_stingray_set_qspi_mux(1);
		INFO("AP (bl31) gained control over QSPI\n");
#endif
	}
}

static void brcm_stingray_dma_pl330_init(void)
{
	unsigned int val;

	VERBOSE("dma pl330 init start\n");

	/* Set DMAC boot_manager_ns = 0x1 */
	VERBOSE(" - configure boot security state\n");
	mmio_setbits_32(DMAC_M0_IDM_IO_CONTROL_DIRECT, BOOT_MANAGER_NS);
	/* Set boot_peripheral_ns[n:0] = 0xffffffff */
	mmio_write_32(ICFG_DMAC_CONFIG_2, BOOT_PERIPHERAL_NS);
	/* Set boot_irq_ns[n:0] = 0x0000ffff */
	mmio_write_32(ICFG_DMAC_CONFIG_3, BOOT_IRQ_NS);

	/* Set DMAC stream_id */
	VERBOSE(" - configure stream_id = 0x6000\n");
	val = (DMAC_STREAM_ID << DMAC_SID_SHIFT);
	mmio_write_32(ICFG_DMAC_SID_ARADDR_CONTROL, val);
	mmio_write_32(ICFG_DMAC_SID_AWADDR_CONTROL, val);

	/* Reset DMAC */
	VERBOSE(" - reset dma pl330\n");

	mmio_setbits_32(DMAC_M0_IDM_RESET_CONTROL, 0x1);
	udelay(500);

	mmio_clrbits_32(DMAC_M0_IDM_RESET_CONTROL, 0x1);
	udelay(500);

	INFO("dma pl330 init done\n");
}

static void brcm_stingray_spi_pl022_init(uintptr_t idm_reset_control)
{
	VERBOSE("spi pl022 init start\n");

	/* Reset APB SPI bridge */
	VERBOSE(" - reset apb spi bridge\n");
	mmio_setbits_32(idm_reset_control, 0x1);
	udelay(500);

	mmio_clrbits_32(idm_reset_control, 0x1);
	udelay(500);

	INFO("spi pl022 init done\n");
}

#define CDRU_SATA_RESET_N \
	BIT(CDRU_MISC_RESET_CONTROL__CDRU_SATA_RESET_N_R)
#define CDRU_MISC_CLK_SATA \
	BIT(CDRU_MISC_CLK_ENABLE_CONTROL__CDRU_SATA_CLK_EN_R)
#define CCN_CONFIG_CLK_ENABLE		(1 << 2)
#define MMU_CONFIG_CLK_ENABLE		(0x3F << 16)

#define SATA_SATA_TOP_CTRL_BUS_CTRL	(SATA_BASE + 0x2044)
#define DMA_BIT_CTRL_MASK		0x003
#define DMA_DESCR_ENDIAN_CTRL		(DMA_BIT_CTRL_MASK << 0x002)
#define DMA_DATA_ENDIAN_CTRL		(DMA_BIT_CTRL_MASK << 0x004)

#define SATA_PORT_SATA3_PCB_REG8	(SATA_BASE + 0x2320)
#define SATA_PORT_SATA3_PCB_REG11	(SATA_BASE + 0x232c)
#define SATA_PORT_SATA3_PCB_BLOCK_ADDR	(SATA_BASE + 0x233c)

#define SATA3_AFE_TXRX_ACTRL		0x1d0
/* TXDriver swing setting is 800mV */
#define DFS_SWINGNOPE_VALUE		(0x0 << 6)
#define DFS_SWINGNOPE_MASK		(0x3 << 6)

#define DFS_SWINGPE_VALUE		(0x1 << 4)
#define DFS_SWINGPE_MASK		(0x3 << 4)

#define DFS_INJSTRENGTH_VALUE		(0x0 << 4)
#define DFS_INJSTRENGTH_MASK		(0x3 << 4)

#define DFS_INJEN			(0x1 << 3)

#define SATA_CORE_MEM_CTRL		(SATA_BASE + 0x3a08)
#define SATA_CORE_MEM_CTRL_ISO		BIT(0)
#define SATA_CORE_MEM_CTRL_ARRPOWEROKIN	BIT(1)
#define SATA_CORE_MEM_CTRL_ARRPOWERONIN	BIT(2)
#define SATA_CORE_MEM_CTRL_POWEROKIN	BIT(3)
#define SATA_CORE_MEM_CTRL_POWERONIN	BIT(4)

#define SATA0_IDM_RESET_CONTROL			(SATA_BASE + 0x500800)
#define SATA_APBT0_IDM_IO_CONTROL_DIRECT	(SATA_BASE + 0x51a408)
#define IO_CONTROL_DIRECT_CLK_ENABLE		BIT(0)
#define SATA_APBT0_IDM_RESET_CONTROL		(SATA_BASE + 0x51a800)
#define IDM_RESET_CONTROL_RESET			BIT(0)

#define NIC400_SATA_NOC_SECURITY1	0x6830000c
#define SATA_NOC_SECURITY1_FIELD	0xf
#define NIC400_SATA_NOC_SECURITY2	0x68300010
#define SATA_NOC_SECURITY2_FIELD	0xf
#define NIC400_SATA_NOC_SECURITY3	0x68300014
#define SATA_NOC_SECURITY3_FIELD	0x1
#define NIC400_SATA_NOC_SECURITY4	0x68300018
#define SATA_NOC_SECURITY4_FIELD	0x1
#define NIC400_SATA_NOC_SECURITY5	0x6830001c
#define SATA_NOC_SECURITY5_FIELD	0xf
#define NIC400_SATA_NOC_SECURITY6	0x68300020
#define SATA_NOC_SECURITY6_FIELD	0x1
#define NIC400_SATA_NOC_SECURITY7	0x68300024
#define SATA_NOC_SECURITY7_FIELD	0xf
#define NIC400_SATA_NOC_SECURITY8	0x68300028
#define SATA_NOC_SECURITY8_FIELD	0xf
#define NIC400_SATA_NOC_SECURITY9	0x6830002c
#define SATA_NOC_SECURITY9_FIELD	0x1

#define SATA_APBT_IDM_PORT_REG(port, reg) \
	(((port/4) << 12) + reg)

#define SATA_IDM_PORT_REG(port, reg)	((port << 12) + reg)

#define SATA_PORT_REG(port, reg) \
	(((port%4) << 16) + ((port/4) << 20) + reg)

#define MAX_SATA_PORTS	8
#define USE_SATA_PORTS	8

#ifdef USE_SATA
static const uint8_t sr_b0_sata_port[MAX_SATA_PORTS] = {
	0, 1, 2, 3, 4, 5, 6, 7
};

static uint32_t brcm_stingray_get_sata_port(unsigned int port)
{
	return sr_b0_sata_port[port];
}

static void brcm_stingray_sata_init(void)
{
	unsigned int port = 0;
	uint32_t sata_port;

	mmio_setbits_32(CDRU_MISC_CLK_ENABLE_CONTROL,
			CDRU_MISC_CLK_SATA);

	mmio_clrbits_32(CDRU_MISC_RESET_CONTROL, CDRU_SATA_RESET_N);
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL, CDRU_SATA_RESET_N);

	for (port = 0; port < USE_SATA_PORTS; port++) {

		sata_port = brcm_stingray_get_sata_port(port);
		mmio_write_32(SATA_APBT_IDM_PORT_REG(sata_port,
					SATA_APBT0_IDM_RESET_CONTROL),
			      0x0);
		mmio_setbits_32(SATA_APBT_IDM_PORT_REG(sata_port,
					SATA_APBT0_IDM_IO_CONTROL_DIRECT),
				IO_CONTROL_DIRECT_CLK_ENABLE);
		mmio_write_32(SATA_IDM_PORT_REG(sata_port,
						SATA0_IDM_RESET_CONTROL),
			      0x0);

		mmio_setbits_32(SATA_PORT_REG(sata_port, SATA_CORE_MEM_CTRL),
				SATA_CORE_MEM_CTRL_ARRPOWERONIN);
		mmio_setbits_32(SATA_PORT_REG(sata_port, SATA_CORE_MEM_CTRL),
				SATA_CORE_MEM_CTRL_ARRPOWEROKIN);
		mmio_setbits_32(SATA_PORT_REG(sata_port, SATA_CORE_MEM_CTRL),
				SATA_CORE_MEM_CTRL_POWERONIN);
		mmio_setbits_32(SATA_PORT_REG(sata_port, SATA_CORE_MEM_CTRL),
				SATA_CORE_MEM_CTRL_POWEROKIN);
		mmio_clrbits_32(SATA_PORT_REG(sata_port, SATA_CORE_MEM_CTRL),
				SATA_CORE_MEM_CTRL_ISO);

		mmio_clrbits_32(SATA_PORT_REG(sata_port,
					      SATA_SATA_TOP_CTRL_BUS_CTRL),
				(DMA_DESCR_ENDIAN_CTRL | DMA_DATA_ENDIAN_CTRL));
	}

	mmio_setbits_32(NIC400_SATA_NOC_SECURITY1, SATA_NOC_SECURITY1_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY2, SATA_NOC_SECURITY2_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY3, SATA_NOC_SECURITY3_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY4, SATA_NOC_SECURITY4_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY5, SATA_NOC_SECURITY5_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY6, SATA_NOC_SECURITY6_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY7, SATA_NOC_SECURITY7_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY8, SATA_NOC_SECURITY8_FIELD);
	mmio_setbits_32(NIC400_SATA_NOC_SECURITY9, SATA_NOC_SECURITY9_FIELD);

	INFO("sata init done\n");
}
#else
static void poweroff_sata_pll(void)
{
	/*
	 * SATA subsystem is clocked by LCPLL0 which is enabled by
	 * default by bootrom. Poweroff the PLL if SATA is not used
	 */

	/* enable isolation */
	mmio_setbits_32(CRMU_AON_CTRL1,
			BIT(CRMU_AON_CTRL1__LCPLL0_ISO_IN));

	/* Power off the SATA PLL/LDO */
	mmio_clrbits_32(CRMU_AON_CTRL1,
			(BIT(CRMU_AON_CTRL1__LCPLL0_PWRON_LDO) |
			 BIT(CRMU_AON_CTRL1__LCPLL0_PWR_ON)));
}
#endif

#ifdef USE_AMAC
#ifdef EMULATION_SETUP
#define ICFG_AMAC_STRAP_CONFIG		(HSLS_ICFG_REGS_BASE + 0xa5c)
#define ICFG_AMAC_STRAP_DLL_BYPASS	(1 << 2)
#endif
#define ICFG_AMAC_MAC_CTRL_REG		(HSLS_ICFG_REGS_BASE + 0xa6c)
#define ICFG_AMAC_MAC_FULL_DUPLEX	(1 << 1)
#define ICFG_AMAC_RGMII_PHY_CONFIG	(HSLS_ICFG_REGS_BASE + 0xa60)
#define ICFG_AMAC_SID_CONTROL		(HSLS_ICFG_REGS_BASE + 0xb10)
#define ICFG_AMAC_SID_SHIFT		5
#define ICFG_AMAC_SID_AWADDR_OFFSET	0x0
#define ICFG_AMAC_SID_ARADDR_OFFSET	0x4
#define AMAC_RPHY_1000_DATARATE		(1 << 20)
#define AMAC_RPHY_FULL_DUPLEX		(1 << 5)
#define AMAC_RPHY_SPEED_OFFSET		2
#define AMAC_RPHY_SPEED_MASK		(7 << AMAC_RPHY_SPEED_OFFSET)
#define AMAC_RPHY_1G_SPEED		(2 << AMAC_RPHY_SPEED_OFFSET)
#define ICFG_AMAC_MEM_PWR_CTRL		(HSLS_ICFG_REGS_BASE + 0xa68)
#define AMAC_ISO			BIT(9)
#define AMAC_STDBY			BIT(8)
#define AMAC_ARRPOWEROKIN		BIT(7)
#define AMAC_ARRPOWERONIN		BIT(6)
#define AMAC_POWEROKIN			BIT(5)
#define AMAC_POWERONIN			BIT(4)

#define AMAC_IDM0_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x4408)
#define AMAC_IDM0_ARCACHE_OFFSET	16
#define AMAC_IDM0_AWCACHE_OFFSET	7
#define AMAC_IDM0_ARCACHE_MASK		(0xF << AMAC_IDM0_ARCACHE_OFFSET)
#define AMAC_IDM0_AWCACHE_MASK		(0xF << AMAC_IDM0_AWCACHE_OFFSET)
/* ARCACHE - AWCACHE is 0xB7 for write-back no allocate */
#define AMAC_IDM0_ARCACHE_VAL		(0xb << AMAC_IDM0_ARCACHE_OFFSET)
#define AMAC_IDM0_AWCACHE_VAL		(0x7 << AMAC_IDM0_AWCACHE_OFFSET)

static void brcm_stingray_amac_init(void)
{
	unsigned int val;
	uintptr_t icfg_amac_sid = ICFG_AMAC_SID_CONTROL;

	VERBOSE("amac init start\n");

	val = SR_SID_VAL(0x3, 0x0, 0x4) << ICFG_AMAC_SID_SHIFT;
	mmio_write_32(icfg_amac_sid + ICFG_AMAC_SID_AWADDR_OFFSET, val);
	mmio_write_32(icfg_amac_sid + ICFG_AMAC_SID_ARADDR_OFFSET, val);

	mmio_setbits_32(ICFG_AMAC_MEM_PWR_CTRL, AMAC_ARRPOWEROKIN);
	mmio_setbits_32(ICFG_AMAC_MEM_PWR_CTRL, AMAC_ARRPOWERONIN);
	mmio_setbits_32(ICFG_AMAC_MEM_PWR_CTRL, AMAC_POWEROKIN);
	mmio_setbits_32(ICFG_AMAC_MEM_PWR_CTRL, AMAC_POWERONIN);
	mmio_clrbits_32(ICFG_AMAC_MEM_PWR_CTRL, AMAC_ISO);
	mmio_write_32(APBR_IDM_RESET_CONTROL, 0x0);
	mmio_clrsetbits_32(ICFG_AMAC_RGMII_PHY_CONFIG, AMAC_RPHY_SPEED_MASK,
				AMAC_RPHY_1G_SPEED); /*1 Gbps line rate*/
	/* 1000 datarate set */
	mmio_setbits_32(ICFG_AMAC_RGMII_PHY_CONFIG, AMAC_RPHY_1000_DATARATE);
	/* full duplex */
	mmio_setbits_32(ICFG_AMAC_RGMII_PHY_CONFIG, AMAC_RPHY_FULL_DUPLEX);
#ifdef EMULATION_SETUP
	/* DLL bypass */
	mmio_setbits_32(ICFG_AMAC_STRAP_CONFIG, ICFG_AMAC_STRAP_DLL_BYPASS);
#endif
	/* serdes full duplex */
	mmio_setbits_32(ICFG_AMAC_MAC_CTRL_REG, ICFG_AMAC_MAC_FULL_DUPLEX);
	mmio_clrsetbits_32(AMAC_IDM0_IO_CONTROL_DIRECT, AMAC_IDM0_ARCACHE_MASK,
				AMAC_IDM0_ARCACHE_VAL);
	mmio_clrsetbits_32(AMAC_IDM0_IO_CONTROL_DIRECT, AMAC_IDM0_AWCACHE_MASK,
				AMAC_IDM0_AWCACHE_VAL);
	INFO("amac init done\n");
}
#endif    /* USE_AMAC */

static void brcm_stingray_pka_meminit(void)
{
	uintptr_t icfg_mem_ctrl = ICFG_PKA_MEM_PWR_CTRL;

	VERBOSE("pka meminit start\n");

	VERBOSE(" - arrpoweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_PKA_MEM_PWR_CTRL__ARRPOWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_PKA_MEM_PWR_CTRL__ARRPOWERONOUT))
		;

	VERBOSE(" - arrpowerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_PKA_MEM_PWR_CTRL__ARRPOWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_PKA_MEM_PWR_CTRL__ARRPOWEROKOUT))
		;

	VERBOSE(" - poweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_PKA_MEM_PWR_CTRL__POWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_PKA_MEM_PWR_CTRL__POWERONOUT))
		;

	VERBOSE(" - powerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_PKA_MEM_PWR_CTRL__POWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_PKA_MEM_PWR_CTRL__POWEROKOUT))
		;

	/* Wait sometime */
	mdelay(1);

	VERBOSE(" - remove isolation\n");
	mmio_clrbits_32(icfg_mem_ctrl, ICFG_PKA_MEM_PWR_CTRL__ISO);

	INFO("pka meminit done\n");
}

static void brcm_stingray_smmu_init(void)
{
	unsigned int val;
	uintptr_t smmu_base = SMMU_BASE;

	VERBOSE("smmu init start\n");

	/* Configure SCR0 */
	VERBOSE(" - configure scr0\n");
	val = mmio_read_32(smmu_base + 0x0);
	val |= (0x1 << 12);
	mmio_write_32(smmu_base + 0x0, val);

	/* Reserve context banks for secure masters */
	arm_smmu_reserve_secure_cntxt();

	/* Print configuration */
	VERBOSE(" - scr0=0x%x scr1=0x%x scr2=0x%x\n",
		mmio_read_32(smmu_base + 0x0),
		mmio_read_32(smmu_base + 0x4),
		mmio_read_32(smmu_base + 0x8));

	VERBOSE(" - idr0=0x%x idr1=0x%x idr2=0x%x\n",
		mmio_read_32(smmu_base + 0x20),
		mmio_read_32(smmu_base + 0x24),
		mmio_read_32(smmu_base + 0x28));

	VERBOSE(" - idr3=0x%x idr4=0x%x idr5=0x%x\n",
		mmio_read_32(smmu_base + 0x2c),
		mmio_read_32(smmu_base + 0x30),
		mmio_read_32(smmu_base + 0x34));

	VERBOSE(" - idr6=0x%x idr7=0x%x\n",
		mmio_read_32(smmu_base + 0x38),
		mmio_read_32(smmu_base + 0x3c));

	INFO("smmu init done\n");
}

static void brcm_stingray_dma_pl330_meminit(void)
{
	uintptr_t icfg_mem_ctrl = ICFG_DMAC_MEM_PWR_CTRL;

	VERBOSE("dmac meminit start\n");

	VERBOSE(" - arrpoweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_DMAC_MEM_PWR_CTRL__ARRPOWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_DMAC_MEM_PWR_CTRL__ARRPOWERONOUT))
		;

	VERBOSE(" - arrpowerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_DMAC_MEM_PWR_CTRL__ARRPOWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_DMAC_MEM_PWR_CTRL__ARRPOWEROKOUT))
		;

	VERBOSE(" - poweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_DMAC_MEM_PWR_CTRL__POWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_DMAC_MEM_PWR_CTRL__POWERONOUT))
		;

	VERBOSE(" - powerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_DMAC_MEM_PWR_CTRL__POWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_DMAC_MEM_PWR_CTRL__POWEROKOUT))
		;

	/* Wait sometime */
	mdelay(1);

	VERBOSE(" - remove isolation\n");
	mmio_clrbits_32(icfg_mem_ctrl, ICFG_DMAC_MEM_PWR_CTRL__ISO);

	INFO("dmac meminit done\n");
}

/* program the crmu access ranges for allowing non sec access*/
static void brcm_stingray_crmu_access_init(void)
{
	/* Enable 0x6641c001 - 0x6641c701 for non secure access */
	mmio_write_32(CRMU_CORE_ADDR_RANGE0_LOW, 0x6641c001);
	mmio_write_32(CRMU_CORE_ADDR_RANGE0_LOW + 0x4, 0x6641c701);

	/* Enable 0x6641d001 - 0x66424b01 for non secure access */
	mmio_write_32(CRMU_CORE_ADDR_RANGE1_LOW, 0x6641d001);
	mmio_write_32(CRMU_CORE_ADDR_RANGE1_LOW + 0x4, 0x66424b01);

	/* Enable 0x66425001 - 0x66425f01 for non secure access */
	mmio_write_32(CRMU_CORE_ADDR_RANGE2_LOW, 0x66425001);
	mmio_write_32(CRMU_CORE_ADDR_RANGE2_LOW + 0x4, 0x66425f01);

	INFO("crmu access init done\n");
}

static void brcm_stingray_scr_init(void)
{
	unsigned int val;
	uintptr_t scr_base = SCR_BASE;
	unsigned int clr_mask = SCR_AXCACHE_CONFIG_MASK;
	unsigned int set_mask = SCR_TBUX_AXCACHE_CONFIG;

	VERBOSE("scr init start\n");

	/* awdomain=0x1 and ardomain=0x1 */
	mmio_clrsetbits_32(scr_base + 0x0, clr_mask, set_mask);
	val = mmio_read_32(scr_base + 0x0);
	VERBOSE(" - set tbu0_config=0x%x\n", val);

	/* awdomain=0x1 and ardomain=0x1 */
	mmio_clrsetbits_32(scr_base + 0x4, clr_mask, set_mask);
	val = mmio_read_32(scr_base + 0x4);
	VERBOSE(" - set tbu1_config=0x%x\n", val);

	/* awdomain=0x1 and ardomain=0x1 */
	mmio_clrsetbits_32(scr_base + 0x8, clr_mask, set_mask);
	val = mmio_read_32(scr_base + 0x8);
	VERBOSE(" - set tbu2_config=0x%x\n", val);

	/* awdomain=0x1 and ardomain=0x1 */
	mmio_clrsetbits_32(scr_base + 0xc, clr_mask, set_mask);
	val = mmio_read_32(scr_base + 0xc);
	VERBOSE(" - set tbu3_config=0x%x\n", val);

	/* awdomain=0x1 and ardomain=0x1 */
	mmio_clrsetbits_32(scr_base + 0x10, clr_mask, set_mask);
	val = mmio_read_32(scr_base + 0x10);
	VERBOSE(" - set tbu4_config=0x%x\n", val);

	/* awdomain=0x0 and ardomain=0x0 */
	mmio_clrbits_32(scr_base + 0x14, clr_mask);
	val = mmio_read_32(scr_base + 0x14);
	VERBOSE(" - set gic_config=0x%x\n", val);

	INFO("scr init done\n");
}

static void brcm_stingray_hsls_tzpcprot_init(void)
{
	unsigned int val;
	uintptr_t tzpcdecprot_base = HSLS_TZPC_BASE;

	VERBOSE("hsls tzpcprot init start\n");

	/* Treat third-party masters as non-secured */
	val = 0;
	val |= BIT(6); /* SDIO1 */
	val |= BIT(5); /* SDIO0 */
	val |= BIT(0); /* AMAC */
	mmio_write_32(tzpcdecprot_base + 0x810, val);

	/* Print TZPC decode status registers */
	VERBOSE(" - tzpcdecprot0=0x%x\n",
		mmio_read_32(tzpcdecprot_base + 0x800));

	VERBOSE(" - tzpcdecprot1=0x%x\n",
		mmio_read_32(tzpcdecprot_base + 0x80c));

	INFO("hsls tzpcprot init done\n");
}

#ifdef USE_I2S
#define ICFG_AUDIO_POWER_CTRL			(HSLS_ICFG_REGS_BASE + 0xaa8)
#define ICFG_AUDIO_POWER_CTRL__POWERONIN	BIT(0)
#define ICFG_AUDIO_POWER_CTRL__POWEROKIN	BIT(1)
#define ICFG_AUDIO_POWER_CTRL__ARRPOWERONIN	BIT(2)
#define ICFG_AUDIO_POWER_CTRL__ARRPOWEROKIN	BIT(3)
#define ICFG_AUDIO_POWER_CTRL__POWERONOUT	BIT(4)
#define ICFG_AUDIO_POWER_CTRL__POWEROKOUT	BIT(5)
#define ICFG_AUDIO_POWER_CTRL__ARRPOWERONOUT	BIT(6)
#define ICFG_AUDIO_POWER_CTRL__ARRPOWEROKOUT	BIT(7)
#define ICFG_AUDIO_POWER_CTRL__ISO		BIT(8)
#define ICFG_AUDIO_SID_CONTROL			(HSLS_ICFG_REGS_BASE + 0xaf8)
#define ICFG_AUDIO_SID_SHIFT			5
#define ICFG_AUDIO_SID_AWADDR_OFFSET		0x0
#define ICFG_AUDIO_SID_ARADDR_OFFSET		0x4

#define I2S_RESET_CONTROL        (HSLS_IDM_REGS_BASE + 0x1800)
#define I2S_IDM_IO_CONTROL       (HSLS_IDM_REGS_BASE + 0x1408)
#define IO_CONTROL_CLK_ENABLE    BIT(0)
#define I2S_IDM0_ARCACHE_OFFSET  16
#define I2S_IDM0_AWCACHE_OFFSET  20
#define I2S_IDM0_ARCACHE_MASK    (0xF << I2S_IDM0_ARCACHE_OFFSET)
#define I2S_IDM0_AWCACHE_MASK    (0xF << I2S_IDM0_AWCACHE_OFFSET)
/* ARCACHE - AWCACHE is 0x22 Normal Non-cacheable Non-bufferable. */
#define I2S_IDM0_ARCACHE_VAL     (0x2 << I2S_IDM0_ARCACHE_OFFSET)
#define I2S_IDM0_AWCACHE_VAL     (0x2 << I2S_IDM0_AWCACHE_OFFSET)

static void brcm_stingray_audio_init(void)
{
	unsigned int val;
	uintptr_t icfg_mem_ctrl = ICFG_AUDIO_POWER_CTRL;
	uintptr_t icfg_audio_sid = ICFG_AUDIO_SID_CONTROL;

	mmio_write_32(I2S_RESET_CONTROL, 0x0);

	mmio_clrsetbits_32(I2S_IDM_IO_CONTROL, I2S_IDM0_ARCACHE_MASK,
			   I2S_IDM0_ARCACHE_VAL);

	mmio_clrsetbits_32(I2S_IDM_IO_CONTROL, I2S_IDM0_AWCACHE_MASK,
			   I2S_IDM0_AWCACHE_VAL);

	mmio_setbits_32(I2S_IDM_IO_CONTROL, IO_CONTROL_CLK_ENABLE);

	VERBOSE("audio meminit start\n");

	VERBOSE(" - configure stream_id = 0x6001\n");
	val = SR_SID_VAL(0x3, 0x0, 0x1) << ICFG_AUDIO_SID_SHIFT;
	mmio_write_32(icfg_audio_sid + ICFG_AUDIO_SID_AWADDR_OFFSET, val);
	mmio_write_32(icfg_audio_sid + ICFG_AUDIO_SID_ARADDR_OFFSET, val);

	VERBOSE(" - arrpoweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_AUDIO_POWER_CTRL__ARRPOWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_AUDIO_POWER_CTRL__ARRPOWERONOUT))
		;

	VERBOSE(" - arrpowerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_AUDIO_POWER_CTRL__ARRPOWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_AUDIO_POWER_CTRL__ARRPOWEROKOUT))
		;

	VERBOSE(" - poweron\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_AUDIO_POWER_CTRL__POWERONIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_AUDIO_POWER_CTRL__POWERONOUT))
		;

	VERBOSE(" - powerok\n");
	mmio_setbits_32(icfg_mem_ctrl,
			ICFG_AUDIO_POWER_CTRL__POWEROKIN);
	while (!(mmio_read_32(icfg_mem_ctrl) &
		 ICFG_AUDIO_POWER_CTRL__POWEROKOUT))
		;

	/* Wait sometime */
	mdelay(1);

	VERBOSE(" - remove isolation\n");
	mmio_clrbits_32(icfg_mem_ctrl, ICFG_AUDIO_POWER_CTRL__ISO);

	INFO("audio meminit done\n");
}
#endif    /* USE_I2S */

/*
 * These defines do not match the regfile but they are renamed in a way such
 * that they are much more readible
 */

#define SCR_GPV_SMMU_NS			(SCR_GPV_BASE + 0x28)
#define SCR_GPV_GIC500_NS		(SCR_GPV_BASE + 0x34)
#define HSLS_GPV_NOR_S0_NS		(HSLS_GPV_BASE + 0x14)
#define HSLS_GPV_IDM1_NS		(HSLS_GPV_BASE + 0x18)
#define HSLS_GPV_IDM2_NS		(HSLS_GPV_BASE + 0x1c)
#define HSLS_SDIO0_SLAVE_NS		(HSLS_GPV_BASE + 0x20)
#define HSLS_SDIO1_SLAVE_NS		(HSLS_GPV_BASE + 0x24)
#define HSLS_GPV_APBY_NS		(HSLS_GPV_BASE + 0x2c)
#define HSLS_GPV_APBZ_NS		(HSLS_GPV_BASE + 0x30)
#define HSLS_GPV_APBX_NS		(HSLS_GPV_BASE + 0x34)
#define HSLS_GPV_APBS_NS		(HSLS_GPV_BASE + 0x38)
#define HSLS_GPV_QSPI_S0_NS		(HSLS_GPV_BASE + 0x68)
#define HSLS_GPV_APBR_NS		(HSLS_GPV_BASE + 0x6c)
#define FS4_CRYPTO_GPV_RM_SLAVE_NS	(FS4_CRYPTO_GPV_BASE + 0x8)
#define FS4_CRYPTO_GPV_APB_SWITCH_NS	(FS4_CRYPTO_GPV_BASE + 0xc)
#define FS4_RAID_GPV_RM_SLAVE_NS	(FS4_RAID_GPV_BASE + 0x8)
#define FS4_RAID_GPV_APB_SWITCH_NS	(FS4_RAID_GPV_BASE + 0xc)
#define FS4_CRYPTO_IDM_NS		(NIC400_FS_NOC_ROOT + 0x1c)
#define FS4_RAID_IDM_NS			(NIC400_FS_NOC_ROOT + 0x28)

#define FS4_CRYPTO_RING_COUNT          32
#define FS4_CRYPTO_DME_COUNT           10
#define FS4_CRYPTO_AE_COUNT            10
#define FS4_CRYPTO_START_STREAM_ID     0x4000
#define FS4_CRYPTO_MSI_DEVICE_ID       0x4100

#define FS4_RAID_RING_COUNT            32
#define FS4_RAID_DME_COUNT             8
#define FS4_RAID_AE_COUNT              8
#define FS4_RAID_START_STREAM_ID       0x4200
#define FS4_RAID_MSI_DEVICE_ID         0x4300

#define FS6_PKI_AXI_SLAVE_NS \
		(NIC400_FS_NOC_ROOT + NIC400_FS_NOC_SECURITY2_OFFSET)

#define FS6_PKI_AE_DME_APB_NS \
		(NIC400_FS_NOC_ROOT + NIC400_FS_NOC_SECURITY7_OFFSET)
#define FS6_PKI_IDM_IO_CONTROL_DIRECT	0x0
#define FS6_PKI_IDM_RESET_CONTROL	0x0
#define FS6_PKI_RING_COUNT		32
#define FS6_PKI_DME_COUNT		1
#define FS6_PKI_AE_COUNT		4
#define FS6_PKI_START_STREAM_ID		0x4000
#define FS6_PKI_MSI_DEVICE_ID		0x4100

static void brcm_stingray_security_init(void)
{
	unsigned int val;

	val = mmio_read_32(SCR_GPV_SMMU_NS);
	val |= BIT(0);				/* SMMU NS = 1 */
	mmio_write_32(SCR_GPV_SMMU_NS, val);

	val = mmio_read_32(SCR_GPV_GIC500_NS);
	val |= BIT(0);				/* GIC-500 NS = 1 */
	mmio_write_32(SCR_GPV_GIC500_NS, val);

	val = mmio_read_32(HSLS_GPV_NOR_S0_NS);
	val |= BIT(0);				/* NOR SLAVE NS = 1 */
	mmio_write_32(HSLS_GPV_NOR_S0_NS, val);

	val = mmio_read_32(HSLS_GPV_IDM1_NS);
	val |= BIT(0);				/* DMA IDM NS = 1 */
	val |= BIT(1);				/* I2S IDM NS = 1 */
	val |= BIT(2);				/* AMAC IDM NS = 1 */
	val |= BIT(3);				/* SDIO0 IDM NS = 1 */
	val |= BIT(4);				/* SDIO1 IDM NS = 1 */
	val |= BIT(5);				/* DS_3 IDM NS = 1 */
	mmio_write_32(HSLS_GPV_IDM1_NS, val);

	val = mmio_read_32(HSLS_GPV_IDM2_NS);
	val |= BIT(2);				/* QSPI IDM NS = 1 */
	val |= BIT(1);				/* NOR IDM NS = 1 */
	val |= BIT(0);				/* NAND IDM NS = 1 */
	mmio_write_32(HSLS_GPV_IDM2_NS, val);

	val = mmio_read_32(HSLS_GPV_APBY_NS);
	val |= BIT(10);				/* I2S NS = 1 */
	val |= BIT(4);				/* IOPAD NS = 1 */
	val |= 0xf;				/* UARTx NS = 1 */
	mmio_write_32(HSLS_GPV_APBY_NS, val);

	val = mmio_read_32(HSLS_GPV_APBZ_NS);
	val |= BIT(2);			/* RNG NS = 1 */
	mmio_write_32(HSLS_GPV_APBZ_NS, val);

	val = mmio_read_32(HSLS_GPV_APBS_NS);
	val |= 0x3;				/* SPIx NS = 1 */
	mmio_write_32(HSLS_GPV_APBS_NS, val);

	val = mmio_read_32(HSLS_GPV_APBR_NS);
	val |= BIT(7);				/* QSPI APB NS = 1 */
	val |= BIT(6);				/* NAND APB NS = 1 */
	val |= BIT(5);				/* NOR APB NS = 1 */
	val |= BIT(4);				/* AMAC APB NS = 1 */
	val |= BIT(1);				/* DMA S1 APB NS = 1 */
	mmio_write_32(HSLS_GPV_APBR_NS, val);

	val = mmio_read_32(HSLS_SDIO0_SLAVE_NS);
	val |= BIT(0);				/* SDIO0 NS = 1 */
	mmio_write_32(HSLS_SDIO0_SLAVE_NS, val);

	val = mmio_read_32(HSLS_SDIO1_SLAVE_NS);
	val |= BIT(0);				/* SDIO1 NS = 1 */
	mmio_write_32(HSLS_SDIO1_SLAVE_NS, val);

	val = mmio_read_32(HSLS_GPV_APBX_NS);
	val |= BIT(14);				/* SMBUS1 NS = 1 */
	val |= BIT(13);				/* GPIO NS = 1 */
	val |= BIT(12);				/* WDT NS = 1 */
	val |= BIT(11);				/* SMBUS0 NS = 1 */
	val |= BIT(10);				/* Timer7 NS = 1 */
	val |= BIT(9);				/* Timer6 NS = 1 */
	val |= BIT(8);				/* Timer5 NS = 1 */
	val |= BIT(7);				/* Timer4 NS = 1 */
	val |= BIT(6);				/* Timer3 NS = 1 */
	val |= BIT(5);				/* Timer2 NS = 1 */
	val |= BIT(4);				/* Timer1 NS = 1 */
	val |= BIT(3);				/* Timer0 NS = 1 */
	val |= BIT(2);				/* MDIO NS = 1 */
	val |= BIT(1);				/* PWM NS = 1 */
	mmio_write_32(HSLS_GPV_APBX_NS, val);

	val = mmio_read_32(HSLS_GPV_QSPI_S0_NS);
	val |= BIT(0);				/* QSPI NS = 1 */
	mmio_write_32(HSLS_GPV_QSPI_S0_NS, val);

#ifdef USE_FS4
	val = 0x1;				/* FS4 Crypto rm_slave */
	mmio_write_32(FS4_CRYPTO_GPV_RM_SLAVE_NS, val);
	val = 0x1;				/* FS4 Crypto apb_switch */
	mmio_write_32(FS4_CRYPTO_GPV_APB_SWITCH_NS, val);

	val = 0x1;				/* FS4 Raid rm_slave */
	mmio_write_32(FS4_RAID_GPV_RM_SLAVE_NS, val);
	val = 0x1;				/* FS4 Raid apb_switch */
	mmio_write_32(FS4_RAID_GPV_APB_SWITCH_NS, val);

	val = 0x1;				/* FS4 Crypto IDM */
	mmio_write_32(FS4_CRYPTO_IDM_NS, val);
	val = 0x1;				/* FS4 RAID IDM */
	mmio_write_32(FS4_RAID_IDM_NS, val);
#endif

#ifdef BL31_CCN_NONSECURE
	/* Enable non-secure access to CCN registers */
	mmio_write_32(OLY_MN_REGISTERS_NODE0_SECURE_ACCESS, 0x1);
#endif

#ifdef DDR_CTRL_PHY_NONSECURE
	mmio_write_32(SCR_NOC_DDR_REGISTER_ACCESS, 0x1);
#endif

	paxc_mhb_ns_init();

	/* unlock scr idm for non secure access */
	mmio_write_32(SCR_NOC_SECURITY0, 0xffffffff);

	INFO("security init done\r\n");
}

void brcm_gpio_pad_ns_init(void)
{
	/* configure all GPIO pads for non secure world access*/
	mmio_write_32(GPIO_S_CNTRL_REG, 0xffffffff); /* 128-140 gpio pads */
	mmio_write_32(GPIO_S_CNTRL_REG + 0x4, 0xffffffff); /* 96-127 gpio pad */
	mmio_write_32(GPIO_S_CNTRL_REG + 0x8, 0xffffffff); /* 64-95 gpio pad */
	mmio_write_32(GPIO_S_CNTRL_REG + 0xc, 0xffffffff); /* 32-63 gpio pad */
	mmio_write_32(GPIO_S_CNTRL_REG + 0x10, 0xffffffff); /* 0-31 gpio pad */
}

#ifndef USE_DDR
static void brcm_stingray_sram_ns_init(void)
{
	uintptr_t sram_root = TZC400_FS_SRAM_ROOT;
	uintptr_t noc_root = NIC400_FS_NOC_ROOT;

	mmio_write_32(sram_root + GATE_KEEPER_OFFSET, 1);
	mmio_write_32(sram_root + REGION_ATTRIBUTES_0_OFFSET, 0xc0000000);
	mmio_write_32(sram_root + REGION_ID_ACCESS_0_OFFSET, 0x00010001);
	mmio_write_32(noc_root + NIC400_FS_NOC_SECURITY4_OFFSET, 0x1);
	INFO(" stingray sram ns init done.\n");
}
#endif

static void ccn_pre_init(void)
{
	/*
	 * Set WFC bit of RN-I nodes where FS4 is connected.
	 * This is required inorder to wait for read/write requests
	 * completion acknowledgment. Otherwise FS4 Ring Manager is
	 * getting stale data because of re-ordering of read/write
	 * requests at CCN level
	 */
	mmio_setbits_32(OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL,
			OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL_WFC);
}

static void ccn_post_init(void)
{
	mmio_setbits_32(OLY_HNI_REGISTERS_NODE0_PCIERC_RNI_NODEID_LIST,
			SRP_RNI_PCIE_CONNECTED);
	mmio_setbits_32(OLY_HNI_REGISTERS_NODE0_SA_AUX_CTL,
			SA_AUX_CTL_SER_DEVNE_WR);

	mmio_clrbits_32(OLY_HNI_REGISTERS_NODE0_POS_CONTROL,
			POS_CONTROL_HNI_POS_EN);
	mmio_clrbits_32(OLY_HNI_REGISTERS_NODE0_SA_AUX_CTL,
			SA_AUX_CTL_POS_EARLY_WR_COMP_EN);
}

#ifndef BL31_BOOT_PRELOADED_SCP
static void crmu_init(void)
{
	/*
	 * Configure CRMU for using SMMU
	 */

	/*Program CRMU Stream ID */
	mmio_write_32(CRMU_MASTER_AXI_ARUSER_CONFIG,
			(CRMU_STREAM_ID << CRMU_SID_SHIFT));
	mmio_write_32(CRMU_MASTER_AXI_AWUSER_CONFIG,
			(CRMU_STREAM_ID << CRMU_SID_SHIFT));

	/* Create Identity mapping */
	arm_smmu_create_identity_map(DOMAIN_CRMU);

	/* Enable Client Port for Secure Masters*/
	arm_smmu_enable_secure_client_port();
}
#endif

static void brcm_fsx_init(void)
{
#if defined(USE_FS4) && defined(USE_FS6)
	#error "USE_FS4 and USE_FS6 should not be used together"
#endif

#ifdef USE_FS4
	fsx_init(eFS4_CRYPTO, FS4_CRYPTO_RING_COUNT, FS4_CRYPTO_DME_COUNT,
		FS4_CRYPTO_AE_COUNT, FS4_CRYPTO_START_STREAM_ID,
		FS4_CRYPTO_MSI_DEVICE_ID, FS4_CRYPTO_IDM_IO_CONTROL_DIRECT,
		FS4_CRYPTO_IDM_RESET_CONTROL, FS4_CRYPTO_BASE,
		FS4_CRYPTO_DME_BASE);

	fsx_init(eFS4_RAID, FS4_RAID_RING_COUNT, FS4_RAID_DME_COUNT,
		FS4_RAID_AE_COUNT, FS4_RAID_START_STREAM_ID,
		FS4_RAID_MSI_DEVICE_ID, FS4_RAID_IDM_IO_CONTROL_DIRECT,
		FS4_RAID_IDM_RESET_CONTROL, FS4_RAID_BASE,
		FS4_RAID_DME_BASE);

	fsx_meminit("raid",
		FS4_RAID_IDM_IO_CONTROL_DIRECT,
		FS4_RAID_IDM_IO_STATUS);
#endif
}

static void bcm_bl33_pass_info(void)
{
	struct bl33_info *info = (struct bl33_info *)BL33_SHARED_DDR_BASE;

	if (sizeof(*info) > BL33_SHARED_DDR_SIZE)
		WARN("bl33 shared area not reserved\n");

	info->version = BL33_INFO_VERSION;
	info->chip.chip_id = PLAT_CHIP_ID_GET;
	info->chip.rev_id = PLAT_CHIP_REV_GET;
}

DEFINE_RENAME_SYSREG_RW_FUNCS(l2ctlr_el1, CORTEX_A72_L2CTLR_EL1)

void plat_bcm_bl31_early_platform_setup(void *from_bl2,
					bl_params_t *plat_params_from_bl2)
{
#ifdef BL31_BOOT_PRELOADED_SCP
	image_info_t scp_image_info;

	scp_image_info.image_base = PRELOADED_SCP_BASE;
	scp_image_info.image_size = PRELOADED_SCP_SIZE;
	plat_bcm_bl2_plat_handle_scp_bl2(&scp_image_info);
#endif
	/*
	 * In BL31, logs are saved to DDR and we have much larger space to
	 * store logs. We can now afford to save all logs >= the 'INFO' level
	 */
	bcm_elog_init((void *)BCM_ELOG_BL31_BASE, BCM_ELOG_BL31_SIZE,
		      LOG_LEVEL_INFO);

	INFO("L2CTLR = 0x%lx\n", read_l2ctlr_el1());

	brcm_timer_sync_init();

	brcm_stingray_dma_pl330_init();

	brcm_stingray_dma_pl330_meminit();

	brcm_stingray_spi_pl022_init(APBS_IDM_IDM_RESET_CONTROL);

#ifdef USE_AMAC
	brcm_stingray_amac_init();
#endif

	brcm_stingray_sdio_init();

#ifdef NCSI_IO_DRIVE_STRENGTH_MA
	brcm_stingray_ncsi_init();
#endif

#ifdef USE_USB
	xhci_phy_init();
#endif

#ifdef USE_SATA
	brcm_stingray_sata_init();
#else
	poweroff_sata_pll();
#endif

	ccn_pre_init();

	brcm_fsx_init();

	brcm_stingray_smmu_init();

	brcm_stingray_pka_meminit();

	brcm_stingray_crmu_access_init();

	brcm_stingray_scr_init();

	brcm_stingray_hsls_tzpcprot_init();

#ifdef USE_I2S
	brcm_stingray_audio_init();
#endif

	ccn_post_init();

	paxb_init();

	paxc_init();

#ifndef BL31_BOOT_PRELOADED_SCP
	crmu_init();
#endif

	/* Note: this should be last thing because
	 * FS4 GPV registers only work after FS4 block
	 * (i.e. crypto,raid,cop) is out of reset.
	 */
	brcm_stingray_security_init();

	brcm_gpio_pad_ns_init();

#ifndef USE_DDR
	brcm_stingray_sram_ns_init();
#endif

#ifdef BL31_FORCE_CPU_FULL_FREQ
	bcm_set_ihost_pll_freq(0x0, PLL_FREQ_FULL);
#endif

	brcm_stingray_gain_qspi_control();

#ifdef USE_PAXC
	/*
	 * Check that the handshake has occurred and report ChiMP status.
	 * This is required. Otherwise (especially on Palladium)
	 * Linux might have booted to the pcie stage whereas
	 * ChiMP has not yet booted. Note that nic_mode case has already
	 * been considered above.
	 */
	if ((boot_source_get() != BOOT_SOURCE_QSPI) &&
	    (!bcm_chimp_is_nic_mode()) &&
	    (!bcm_chimp_wait_handshake())
	   ) {
		/* Does ChiMP report an error ? */
		uint32_t err;

		err = bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_STAT_REG);
		if ((err & CHIMP_ERROR_MASK) == 0)
		/* ChiMP has not booted yet, but no error reported */
			WARN("ChiMP not booted yet, but no error reported.\n");
	}

#if DEBUG
	if (boot_source_get() != BOOT_SOURCE_QSPI)
		INFO("Current ChiMP Status: 0x%x; bpe_mod reg: 0x%x\n"
		     "fastboot register: 0x%x; handshake register 0x%x\n",
		     bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_STAT_REG),
		     bcm_chimp_read_ctrl(CHIMP_REG_CTRL_BPE_MODE_REG),
		     bcm_chimp_read_ctrl(CHIMP_REG_CTRL_FSTBOOT_PTR_REG),
		     bcm_chimp_read(CHIMP_REG_ECO_RESERVED));
#endif /* DEBUG */
#endif

#ifdef FS4_DISABLE_CLOCK
	flush_dcache_range(
		PLAT_BRCM_TRUSTED_SRAM_BASE,
		PLAT_BRCM_TRUSTED_SRAM_SIZE);
	fs4_disable_clocks(true, true, true);
#endif

	/* pass information to BL33 through shared DDR region */
	bcm_bl33_pass_info();

	/*
	 * We are not yet at the end of BL31, but we can stop log here so we do
	 * not need to add 'bcm_elog_exit' to the standard BL31 code. The
	 * benefit of capturing BL31 logs after this is very minimal in a
	 * production system
	 */
	bcm_elog_exit();

#if !BRCM_DISABLE_TRUSTED_WDOG
	/*
	 * Secure watchdog was started earlier in BL2, now it's time to stop
	 * it
	 */
	sp805_stop(ARM_SP805_TWDG_BASE);
#endif
}
