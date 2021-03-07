/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef A8K_PLAT_DEF_H
#define A8K_PLAT_DEF_H

#include <marvell_def.h>

#define MVEBU_PRIMARY_CPU		0x0
#define MVEBU_AP0			0x0

/* APN806 revision ID */
#define MVEBU_CSS_GWD_CTRL_IIDR2_REG	(MVEBU_REGS_BASE + 0x610FCC)
#define GWD_IIDR2_REV_ID_OFFSET		12
#define GWD_IIDR2_REV_ID_MASK		0xF
#define GWD_IIDR2_CHIP_ID_OFFSET	20
#define GWD_IIDR2_CHIP_ID_MASK		(0xFFFu << GWD_IIDR2_CHIP_ID_OFFSET)

#define CHIP_ID_AP806			0x806
#define CHIP_ID_AP807			0x807

#define COUNTER_FREQUENCY		25000000

#define MVEBU_REGS_BASE			0xF0000000
#define MVEBU_REGS_BASE_MASK		0xF0000000
#define MVEBU_REGS_BASE_AP(ap)		MVEBU_REGS_BASE
#define MVEBU_AP_IO_BASE(ap)		0xF2000000
#define MVEBU_CP_OFFSET			0x2000000
#define MVEBU_CP_REGS_BASE(cp_index)	(MVEBU_AP_IO_BASE(0) + \
						(cp_index) * MVEBU_CP_OFFSET)
#define MVEBU_RFU_BASE			(MVEBU_REGS_BASE + 0x6F0000)
#define MVEBU_IO_WIN_BASE(ap_index)	(MVEBU_RFU_BASE)
#define MVEBU_IO_WIN_GCR_OFFSET		(0x70)
#define MVEBU_IO_WIN_MAX_WINS		(7)

/* Misc SoC configurations Base */
#define MVEBU_MISC_SOC_BASE		(MVEBU_REGS_BASE + 0x6F4300)

#define MVEBU_CCU_BASE(ap_index)	(MVEBU_REGS_BASE + 0x4000)
#define MVEBU_CCU_MAX_WINS		(8)

#define MVEBU_LLC_BASE(ap_index)	(MVEBU_REGS_BASE + 0x8000)
#define MVEBU_DRAM_MAC_BASE		(MVEBU_REGS_BASE + 0x20000)
#define MVEBU_DRAM_PHY_BASE		(MVEBU_REGS_BASE + 0x20000)
#define MVEBU_SMMU_BASE			(MVEBU_REGS_BASE + 0x100000)
#define MVEBU_CP_MPP_REGS(cp_index, n)	(MVEBU_CP_REGS_BASE(cp_index) + \
						0x440000 + ((n) << 2))
#define MVEBU_PM_MPP_REGS(cp_index, n)	(MVEBU_CP_REGS_BASE(cp_index) + \
						0x440000 + ((n / 8) << 2))
#define MVEBU_CP_GPIO_DATA_OUT(cp_index, n) \
					(MVEBU_CP_REGS_BASE(cp_index) + \
					0x440100 + ((n > 31) ? 0x40 : 0x00))
#define MVEBU_CP_GPIO_DATA_OUT_EN(cp_index, n) \
					(MVEBU_CP_REGS_BASE(cp_index) + \
					0x440104 + ((n > 31) ? 0x40 : 0x00))
#define MVEBU_CP_GPIO_DATA_IN(cp_index, n) (MVEBU_CP_REGS_BASE(cp_index) + \
					0x440110 + ((n > 31) ? 0x40 : 0x00))
#define MVEBU_AP_MPP_REGS(n)		(MVEBU_RFU_BASE + 0x4000 + ((n) << 2))
#define MVEBU_AP_GPIO_REGS		(MVEBU_RFU_BASE + 0x5040)
#define MVEBU_AP_GPIO_DATA_IN		(MVEBU_AP_GPIO_REGS + 0x10)
#define MVEBU_AP_I2C_BASE		(MVEBU_REGS_BASE + 0x511000)
#define MVEBU_CP0_I2C_BASE		(MVEBU_CP_REGS_BASE(0) + 0x701000)
#define MVEBU_AP_GEN_MGMT_BASE		(MVEBU_RFU_BASE + 0x8000)
#define MVEBU_AP_EXT_TSEN_BASE		(MVEBU_AP_GEN_MGMT_BASE + 0x84)

#define MVEBU_AP_MC_TRUSTZONE_REG_LOW(ap, win)	(MVEBU_REGS_BASE_AP(ap) + \
							0x20080 + ((win) * 0x8))
#define MVEBU_AP_MC_TRUSTZONE_REG_HIGH(ap, win)	(MVEBU_REGS_BASE_AP(ap) + \
							0x20084 + ((win) * 0x8))

/* MCI indirect access definitions */
#define MCI_MAX_UNIT_ID				2
/* SoC RFU / IHBx4 Control */
#define MCIX4_REG_START_ADDRESS_REG(unit_id)	(MVEBU_RFU_BASE + \
						0x4218 + (unit_id * 0x20))
#define MCI_REMAP_OFF_SHIFT			8

#define MVEBU_MCI_REG_BASE_REMAP(index)		(0xFD000000 + \
						((index) * 0x1000000))

#define MVEBU_PCIE_X4_MAC_BASE(x)	(MVEBU_CP_REGS_BASE(x) + 0x600000)
#define MVEBU_COMPHY_BASE(x)		(MVEBU_CP_REGS_BASE(x) + 0x441000)
#define MVEBU_HPIPE_BASE(x)		(MVEBU_CP_REGS_BASE(x) + 0x120000)
#define MVEBU_CP_DFX_OFFSET		(0x400200)

/*****************************************************************************
 * MVEBU memory map related constants
 *****************************************************************************
 */
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE			MVEBU_REGS_BASE
#define DEVICE0_SIZE			0x10000000

/*****************************************************************************
 * GIC-400 & interrupt handling related constants
 *****************************************************************************
 */
/* Base MVEBU compatible GIC memory map */
#define MVEBU_GICD_BASE			0x210000
#define MVEBU_GICC_BASE			0x220000


/*****************************************************************************
 * AXI Configuration
 *****************************************************************************
 */
#define MVEBU_AXI_ATTR_ARCACHE_OFFSET		4
#define MVEBU_AXI_ATTR_ARCACHE_MASK		(0xF << \
						 MVEBU_AXI_ATTR_ARCACHE_OFFSET)
#define MVEBU_AXI_ATTR_ARDOMAIN_OFFSET		12
#define MVEBU_AXI_ATTR_ARDOMAIN_MASK		(0x3 << \
						 MVEBU_AXI_ATTR_ARDOMAIN_OFFSET)
#define MVEBU_AXI_ATTR_AWCACHE_OFFSET		20
#define MVEBU_AXI_ATTR_AWCACHE_MASK		(0xF << \
						 MVEBU_AXI_ATTR_AWCACHE_OFFSET)
#define MVEBU_AXI_ATTR_AWDOMAIN_OFFSET		28
#define MVEBU_AXI_ATTR_AWDOMAIN_MASK		(0x3 << \
						 MVEBU_AXI_ATTR_AWDOMAIN_OFFSET)

/* SATA MBUS to AXI configuration */
#define MVEBU_SATA_M2A_AXI_ARCACHE_OFFSET	1
#define MVEBU_SATA_M2A_AXI_ARCACHE_MASK		(0xF << \
					MVEBU_SATA_M2A_AXI_ARCACHE_OFFSET)
#define MVEBU_SATA_M2A_AXI_AWCACHE_OFFSET	5
#define MVEBU_SATA_M2A_AXI_AWCACHE_MASK		(0xF << \
					MVEBU_SATA_M2A_AXI_AWCACHE_OFFSET)

/* ARM cache attributes */
#define CACHE_ATTR_BUFFERABLE			0x1
#define CACHE_ATTR_CACHEABLE			0x2
#define CACHE_ATTR_READ_ALLOC			0x4
#define CACHE_ATTR_WRITE_ALLOC			0x8
/* Domain */
#define DOMAIN_NON_SHAREABLE			0x0
#define DOMAIN_INNER_SHAREABLE			0x1
#define DOMAIN_OUTER_SHAREABLE			0x2
#define DOMAIN_SYSTEM_SHAREABLE			0x3

/************************************************************************
 * Required platform porting definitions common to all
 * Management Compute SubSystems (MSS)
 ************************************************************************
 */
/*
 * Load address of SCP_BL2
 * SCP_BL2 is loaded to the same place as BL31.
 * Once SCP_BL2 is transferred to the SCP,
 * it is discarded and BL31 is loaded over the top.
 */
#ifdef SCP_IMAGE
#define SCP_BL2_BASE			BL31_BASE
#define SCP_BL2_SIZE			BL31_LIMIT
#endif

#ifndef __ASSEMBLER__
enum ap806_sar_target_dev {
	SAR_PIDI_MCIX2		= 0x0,
	SAR_MCIX4		= 0x1,
	SAR_SPI			= 0x2,
	SAR_SD			= 0x3,
	SAR_PIDI_MCIX2_BD	= 0x4, /* BootRom disabled */
	SAR_MCIX4_DB		= 0x5, /* BootRom disabled */
	SAR_SPI_DB		= 0x6, /* BootRom disabled */
	SAR_EMMC		= 0x7
};

enum io_win_target_ids {
	MCI_0_TID	 = 0x0,
	MCI_1_TID	 = 0x1,
	MCI_2_TID	 = 0x2,
	PIDI_TID	 = 0x3,
	SPI_TID		 = 0x4,
	STM_TID		 = 0x5,
	BOOTROM_TID	 = 0x6,
	IO_WIN_MAX_TID
};

enum ccu_target_ids {
	IO_0_TID        = 0x00,
	DRAM_0_TID      = 0x03,
	IO_1_TID        = 0x0F,
	CFG_REG_TID     = 0x10,
	RAR_TID         = 0x20,
	SRAM_TID        = 0x40,
	DRAM_1_TID      = 0xC0,
	CCU_MAX_TID,
	INVALID_TID     = 0xFF
};
#endif /* __ASSEMBLER__ */

#endif /* A8K_PLAT_DEF_H */
