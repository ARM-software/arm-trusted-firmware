/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the trusted firmware required platform port
 * definitions for the second generation platforms based on the N2/V2 CPUs. The
 * common platform support for Arm platforms expect platforms to define certain
 * definitions and those definitions are referred to as the platform port
 * definitions.
 */

#ifndef NRD_PLAT_ARM_DEF2_H
#define NRD_PLAT_ARM_DEF2_H

#ifndef __ASSEMBLER__
#include <lib/mmio.h>
#endif /* __ASSEMBLER__ */

#include <plat/arm/common/arm_def.h>
#include <plat/arm/common/arm_spm_def.h>
#include <plat/arm/css/common/css_def.h>
#include <nrd_css_fw_def2.h>
#include <nrd_ros_fw_def2.h>

/*******************************************************************************
 * Core count
 ******************************************************************************/

#define PLATFORM_CORE_COUNT		(NRD_CHIP_COUNT *		\
					PLAT_ARM_CLUSTER_COUNT *	\
					NRD_MAX_CPUS_PER_CLUSTER *	\
					NRD_MAX_PE_PER_CPU)

#if (NRD_PLATFORM_VARIANT == 1)
#define PLAT_ARM_CLUSTER_COUNT		U(8)
#elif (NRD_PLATFORM_VARIANT == 2)
#define PLAT_ARM_CLUSTER_COUNT		U(4)
#else
#define PLAT_ARM_CLUSTER_COUNT		U(16)
#endif

/*******************************************************************************
 * PA/VA config
 ******************************************************************************/

#ifdef __aarch64__
#define PLAT_PHY_ADDR_SPACE_SIZE	NRD_REMOTE_CHIP_MEM_OFFSET( \
						NRD_CHIP_COUNT)
#define PLAT_VIRT_ADDR_SPACE_SIZE	NRD_REMOTE_CHIP_MEM_OFFSET( \
						NRD_CHIP_COUNT)
#else
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#endif

/*******************************************************************************
 * XLAT definitions
 ******************************************************************************/

#if defined(IMAGE_BL31)
# if SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)
#  define PLAT_ARM_MMAP_ENTRIES		(10  + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(8  + ((NRD_CHIP_COUNT - 1) * 3))
#  define PLAT_SP_IMAGE_MMAP_REGIONS	U(12)
#  define PLAT_SP_IMAGE_MAX_XLAT_TABLES	U(14)
# else
#  define PLAT_ARM_MMAP_ENTRIES		(5 + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(6 + ((NRD_CHIP_COUNT - 1) * 3))
# endif
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		U(8)
# define MAX_XLAT_TABLES		U(5)
#elif defined(IMAGE_BL2)
# define PLAT_ARM_MMAP_ENTRIES		(11 + (NRD_CHIP_COUNT - 1))

/*
 * MAX_XLAT_TABLES entries need to be doubled because when the address width
 * exceeds 40 bits an additional level of translation is required. In case of
 * multichip platforms peripherals also fall into address space with width
 * > 40 bits
 *
 */
# define MAX_XLAT_TABLES		(7  + ((NRD_CHIP_COUNT - 1) * 2))
#elif !USE_ROMLIB
# define PLAT_ARM_MMAP_ENTRIES		U(11)
# define MAX_XLAT_TABLES		U(7)
#else
# define PLAT_ARM_MMAP_ENTRIES		U(12)
# define MAX_XLAT_TABLES		U(6)
#endif

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE	UL(64 * 1024)	/* 64 KB */

/*
 * PLAT_ARM_MAX_ROMLIB_RW_SIZE is define to use a full page
 */

#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0xe000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)
#endif

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth. Additional 8KiB space is added per chip in
 * order to accommodate the additional level of translation required for "TZC"
 * peripheral access which lies in >4TB address space.
 *
 */
#if TRUSTED_BOARD_BOOT
# define PLAT_ARM_MAX_BL2_SIZE		(0x20000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#else
# define PLAT_ARM_MAX_BL2_SIZE		(0x14000 + ((NRD_CHIP_COUNT - 1) * \
							0x2000))
#endif

#define PLAT_ARM_MAX_BL31_SIZE		(NRD_CSS_BL31_SIZE +		\
					PLAT_ARM_MAX_BL2_SIZE +		\
					PLAT_ARM_MAX_BL1_RW_SIZE)

/*******************************************************************************
 * Stack sizes
 ******************************************************************************/

#if defined(IMAGE_BL1)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x440)
# endif
#elif defined(IMAGE_BL2)
# if TRUSTED_BOARD_BOOT
#  define PLATFORM_STACK_SIZE		UL(0x1000)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL2U)
# define PLATFORM_STACK_SIZE		UL(0x400)
#elif defined(IMAGE_BL31)
# if SPM_MM
#  define PLATFORM_STACK_SIZE		UL(0x500)
# else
#  define PLATFORM_STACK_SIZE		UL(0x400)
# endif
#elif defined(IMAGE_BL32)
# define PLATFORM_STACK_SIZE		UL(0x440)
#endif

#if (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP)) &&			\
ENABLE_FEAT_RAS && FFH_SUPPORT
/*
 * Secure partition stack follows right after the memory space reserved for
 * CPER buffer memory.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +   \
					 PLAT_SP_IMAGE_NS_BUF_SIZE +   \
					 NRD_CSS_SP_CPER_BUF_SIZE)
#elif (SPM_MM || (SPMC_AT_EL3 && SPMC_AT_EL3_SEL0_SP))
/*
 * Secure partition stack follows right after the memory region that is shared
 * between EL3 and S-EL0.
 */
#define PLAT_ARM_SP_IMAGE_STACK_BASE	(PLAT_SP_IMAGE_NS_BUF_BASE +	\
					 PLAT_SP_IMAGE_NS_BUF_SIZE)
#endif /* SPM_MM && ENABLE_FEAT_RAS && FFH_SUPPORT */

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define PLAT_ARM_BOOT_UART_BASE		NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	NRD_CSS_SEC_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

/*******************************************************************************
 * SCMI config
 ******************************************************************************/

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	NRD_CHIP_COUNT

/*******************************************************************************
 * ROM, SRAM and DRAM config
 ******************************************************************************/

#define PLAT_ARM_TRUSTED_ROM_BASE	NRD_CSS_SECURE_ROM_BASE
#define PLAT_ARM_TRUSTED_ROM_SIZE	NRD_CSS_SECURE_ROM_SIZE

#define PLAT_ARM_DRAM2_BASE		NRD_CSS_DRAM2_BASE
#define PLAT_ARM_DRAM2_SIZE		NRD_CSS_DRAM2_SIZE

#define PLAT_ARM_TRUSTED_SRAM_SIZE	NRD_CSS_SECURE_SRAM_SIZE

#define PLAT_ARM_NSTIMER_FRAME_ID	0

#define PLAT_ARM_NSRAM_BASE		NRD_CSS_NS_SRAM_BASE
#define PLAT_ARM_NSRAM_SIZE		NRD_CSS_NS_SRAM_SIZE
/*
 * Required platform porting definitions common to all ARM CSS SoCs
 */
/* 2MB used for SCP DDR retraining */
#define PLAT_ARM_SCP_TZC_DRAM1_SIZE	UL(0x00200000)

/*******************************************************************************
 * GIC/EHF config
 ******************************************************************************/

/* GIC related constants */
#define PLAT_ARM_GICD_BASE		NRD_CSS_GIC_BASE

#if (NRD_PLATFORM_VARIANT == 1)
#define PLAT_ARM_GICR_BASE		NRD_CSS_GIC_BASE + UL(0x00100000)
#elif (NRD_PLATFORM_VARIANT == 3)
#define PLAT_ARM_GICR_BASE		NRD_CSS_GIC_BASE + UL(0x00300000)
#else
#define PLAT_ARM_GICR_BASE		NRD_CSS_GIC_BASE + UL(0x001C0000)
#endif

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#if ENABLE_FEAT_RAS && FFH_SUPPORT
#define PLAT_SP_PRI			PLAT_RAS_PRI
#else
#define PLAT_SP_PRI			(0x10)
#endif

/* Interrupt priority level for shutdown/reboot */
#define PLAT_REBOOT_PRI		GIC_HIGHEST_SEC_PRIORITY
#define PLAT_EHF_DESC		EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_REBOOT_PRI)

/*******************************************************************************
 * Secure world config
 ******************************************************************************/

#define SECURE_PARTITION_COUNT		1
#define NS_PARTITION_COUNT		1
#define MAX_EL3_LP_DESCS_COUNT		1

/*******************************************************************************
 * MHU config
 ******************************************************************************/

#define PLAT_CSS_MHU_BASE		NRD_CSS_AP_SCP_S_MHU_BASE
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/*******************************************************************************
 * Power config
 ******************************************************************************/

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/*******************************************************************************
 * TZ config
 ******************************************************************************/

#define PLAT_ARM_TZC_BASE		NRD_ROS_MEMCNTRL_BASE + UL(0x00720000)
#define PLAT_ARM_TZC_FILTERS		TZC_400_REGION_ATTR_FILTER_BIT(0)

/*******************************************************************************
 * SDS config
 ******************************************************************************/

/* SDS ID for unusable CPU MPID list structure */
#define SDS_ISOLATED_CPU_LIST_ID	U(128)

/* Index of SDS region used in the communication with SCP */
#define SDS_SCP_AP_REGION_ID		U(0)

/*******************************************************************************
 * Flash config
 ******************************************************************************/

#define MAX_IO_DEVICES			U(3)
#define MAX_IO_HANDLES			U(4)

#define V2M_SYS_LED			U(0x8)

#define V2M_SYS_LED_SS_SHIFT		U(0)
#define V2M_SYS_LED_EL_SHIFT		U(1)
#define V2M_SYS_LED_EC_SHIFT		U(3)

#define V2M_SYS_LED_SS_MASK		U(0x01)
#define V2M_SYS_LED_EL_MASK		U(0x03)
#define V2M_SYS_LED_EC_MASK		U(0x1f)

#define PLAT_ARM_MEM_PROTEC_VA_FRAME	UL(0xC0000000)

#define V2M_SYSREGS_BASE		UL(0x0C010000)
#define V2M_FLASH0_BASE			UL(0x08000000)
#define V2M_FLASH0_SIZE			UL(0x04000000)
#define V2M_FLASH_BLOCK_SIZE		UL(0x00040000)	/* 256 KB */

#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE +	\
					V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/*******************************************************************************
 * Platform type identification macro
 ******************************************************************************/

/* Platform ID related accessors */
#define BOARD_CSS_PLAT_ID_REG_ID_MASK		U(0x0F)
#define BOARD_CSS_PLAT_ID_REG_ID_SHIFT		U(0x00)
#define BOARD_CSS_PLAT_ID_REG_VERSION_MASK	U(0xF00)
#define BOARD_CSS_PLAT_ID_REG_VERSION_SHIFT	U(0x08)
#define BOARD_CSS_PLAT_TYPE_RTL			U(0x00)
#define BOARD_CSS_PLAT_TYPE_FPGA		U(0x01)
#define BOARD_CSS_PLAT_TYPE_EMULATOR		U(0x02)
#define BOARD_CSS_PLAT_TYPE_FVP			U(0x03)

#ifndef __ASSEMBLER__
#define BOARD_CSS_GET_PLAT_TYPE(addr)					\
	((mmio_read_32(addr) & BOARD_CSS_PLAT_ID_REG_ID_MASK)		\
	>> BOARD_CSS_PLAT_ID_REG_ID_SHIFT)
#endif /* __ASSEMBLER__ */

/* Platform ID address */
#define BOARD_CSS_PLAT_ID_REG_ADDR		UL(0x0EFE00E0)

/*******************************************************************************
 * ROS peripheral config
 ******************************************************************************/

#define SOC_CSS_NIC400_USB_EHCI			U(0)
#define SOC_CSS_NIC400_TLX_MASTER		U(1)
#define SOC_CSS_NIC400_USB_OHCI			U(2)
#define SOC_CSS_NIC400_PL354_SMC		U(3)
#define SOC_CSS_NIC400_APB4_BRIDGE		U(4)
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE		U(5)
#define SOC_CSS_NIC400_BOOTSEC_BRIDGE_UART1	UL(1 << 12)

#define SOC_CSS_PCIE_CONTROL_BASE		UL(0x0ef20000)

/* SoC NIC-400 Global Programmers View (GPV) */
#define SOC_CSS_NIC400_BASE			UL(0x0ED00000)

/* Non-volatile counters */
#define SOC_TRUSTED_NVCTR_BASE			UL(0x0EE70000)
#define TFW_NVCTR_BASE				(SOC_TRUSTED_NVCTR_BASE	+\
						0x0000)
#define TFW_NVCTR_SIZE				U(4)
#define NTFW_CTR_BASE				(SOC_TRUSTED_NVCTR_BASE +\
						0x0004)
#define NTFW_CTR_SIZE				U(4)

/* Keys */
#define SOC_KEYS_BASE				UL(0x0EE80000)
#define TZ_PUB_KEY_HASH_BASE			(SOC_KEYS_BASE + 0x0000)
#define TZ_PUB_KEY_HASH_SIZE			U(32)
#define HU_KEY_BASE				(SOC_KEYS_BASE + 0x0020)
#define HU_KEY_SIZE				U(16)
#define END_KEY_BASE				(SOC_KEYS_BASE + 0x0044)
#define END_KEY_SIZE				U(32)

/*******************************************************************************
 * MMU config
 ******************************************************************************/

#define V2M_MAP_FLASH0_RW						\
		MAP_REGION_FLAT(					\
			V2M_FLASH0_BASE,				\
			V2M_FLASH0_SIZE,				\
			MT_DEVICE | MT_RW | MT_SECURE)

#define V2M_MAP_FLASH0_RO						\
		MAP_REGION_FLAT(					\
			V2M_FLASH0_BASE,				\
			V2M_FLASH0_SIZE,				\
			MT_RO_DATA | MT_SECURE)

#endif /* NRD_PLAT_ARM_DEF2_H */
