/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is limited to include the platform port definitions for the
 * third generation of platforms.
 */

#ifndef NRD_PLAT_ARM_DEF3_H
#define NRD_PLAT_ARM_DEF3_H

#include <common/tbbr/tbbr_img_def.h>

#ifndef __ASSEMBLER__
#include <lib/mmio.h>
#endif /* __ASSEMBLER__ */

#include <plat/arm/common/arm_spm_def.h>
#include <plat/common/common_def.h>
#include <nrd_css_fw_def3.h>
#include <nrd_ros_fw_def3.h>

/*******************************************************************************
 * Core count
 ******************************************************************************/

#define PLATFORM_CORE_COUNT		(NRD_CHIP_COUNT *		\
					PLAT_ARM_CLUSTER_COUNT *	\
					NRD_MAX_CPUS_PER_CLUSTER *	\
					NRD_MAX_PE_PER_CPU)

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

/*
 * PLAT_ARM_MMAP_ENTRIES depends on the number of entries in the
 * plat_arm_mmap array defined for each BL stage. In addition to that, on
 * multi-chip platforms, address regions on each of the remote chips are
 * also mapped. In BL31, for instance, three address regions on the remote
 * chips are accessed - secure ram, css device and soc device regions.
 */
#if defined(IMAGE_BL31)
#  define PLAT_ARM_MMAP_ENTRIES		(6 + ((NRD_CHIP_COUNT - 1) * 3))
#  define MAX_XLAT_TABLES		(6 + ((NRD_CHIP_COUNT - 1) * 3))
#elif defined(IMAGE_BL32)
# define PLAT_ARM_MMAP_ENTRIES		U(8)
# define MAX_XLAT_TABLES		U(5)
#elif defined(IMAGE_BL2)
# define PLAT_ARM_MMAP_ENTRIES		(11 + (NRD_CHIP_COUNT - 1))
# define MAX_XLAT_TABLES		(11  + ((NRD_CHIP_COUNT - 1) * 2))
#else
# define PLAT_ARM_MMAP_ENTRIES		U(6)
# define MAX_XLAT_TABLES		U(6)
#endif

/*******************************************************************************
 * BL sizes
 ******************************************************************************/

#define PLAT_ARM_MAX_ROMLIB_RW_SIZE	UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE	UL(0)

#define PLAT_ARM_MAX_BL1_RW_SIZE	NRD_CSS_BL1_RW_SIZE

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth. Additional 8KiB space is added per chip in
 * order to accommodate the additional level of translation required for "TZC"
 * peripheral access which lies in >4TB address space.
 *
 */
#define PLAT_ARM_MAX_BL2_SIZE		(NRD_CSS_BL2_SIZE +		\
						((NRD_CHIP_COUNT - 1) * 0x2000))

#define PLAT_ARM_MAX_BL31_SIZE		(NRD_CSS_BL31_SIZE +		\
						PLAT_ARM_MAX_BL2_SIZE +	\
						PLAT_ARM_MAX_BL1_RW_SIZE)

/*******************************************************************************
 * BL31 plat param
 ******************************************************************************/

/* Special value used to verify platform parameters from BL2 to BL31 */
#define ARM_BL31_PLAT_PARAM_VAL		ULL(0x0f1e2d3c4b5a6978)

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

/*******************************************************************************
 * Console config
 ******************************************************************************/

#define ARM_CONSOLE_BAUDRATE		(115200)

/* UART related constants */
#define PLAT_ARM_BOOT_UART_BASE		NRD_CSS_SECURE_UART_BASE
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_RUN_UART_BASE		NRD_CSS_SECURE_UART_BASE
#define PLAT_ARM_RUN_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

#define PLAT_ARM_CRASH_UART_BASE	NRD_CSS_SECURE_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	NRD_CSS_UART_CLK_IN_HZ

/*******************************************************************************
 * System counter and timer config
 ******************************************************************************/

#define ARM_SYS_CNTCTL_BASE		NRD_CSS_GENERIC_REFCLK_BASE
#define ARM_SYS_CNTREAD_BASE		NRD_CSS_CNTCTL_REFCLK_READFRAME_BASE
#define ARM_SYS_TIMCTL_BASE		NRD_CSS_SYS_TIMCTL_BASE
#define ARM_SYS_CNT_BASE_S		NRD_CSS_SECURE_TIMER_CTL_BASE
#define ARM_SYS_CNT_BASE_NS		NRD_CSS_NS_TIMER_CTL_BASE

/*******************************************************************************
 * SRAM and DRAM config for FW
 ******************************************************************************/

#define PLAT_ARM_TRUSTED_ROM_BASE	NRD_CSS_SECURE_ROM_BASE
#define PLAT_ARM_TRUSTED_ROM_SIZE	NRD_CSS_SECURE_ROM_SIZE

#define PLAT_ARM_DRAM2_BASE		NRD_CSS_DRAM2_BASE
#define PLAT_ARM_DRAM2_SIZE		NRD_CSS_DRAM2_SIZE

#define PLAT_ARM_TRUSTED_SRAM_SIZE	NRD_CSS_SECURE_SRAM_SIZE

#define PLAT_ARM_NSTIMER_FRAME_ID	(0)

#define PLAT_ARM_NSRAM_BASE		NRD_CSS_NS_SRAM_BASE
#define PLAT_ARM_NSRAM_SIZE		NRD_CSS_NS_SRAM_SIZE

/*******************************************************************************
 * Power config
 ******************************************************************************/

/*
 * Macros mapping the MPIDR Affinity levels to ARM Platform Power levels. The
 * power levels have a 1:1 mapping with the MPIDR affinity levels.
 */
#define ARM_PWR_LVL0			MPIDR_AFFLVL0
#define ARM_PWR_LVL1			MPIDR_AFFLVL1
#define ARM_PWR_LVL2			MPIDR_AFFLVL2
#define ARM_PWR_LVL3			MPIDR_AFFLVL3

/* Local power state for power domains in Run state. */
#define ARM_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define ARM_LOCAL_STATE_RET		U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains
 */
#define ARM_LOCAL_STATE_OFF		U(2)
/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE		ARM_LOCAL_STATE_RET
/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE		ARM_LOCAL_STATE_OFF

#define CSS_SYSTEM_PWR_DMN_LVL		ARM_PWR_LVL2
#define PLAT_MAX_PWR_LVL		ARM_PWR_LVL1

/*******************************************************************************
 * MHU config
 ******************************************************************************/

#define PLAT_CSS_MHU_BASE		NRD_CSS_AP_SCP_SECURE_MHU_BASE
#define PLAT_MHUV2_BASE			PLAT_CSS_MHU_BASE

/*******************************************************************************
 * Cache config
 ******************************************************************************/

#define ARM_CACHE_WRITEBACK_SHIFT	U(6)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_GRANULE		(U(1) << ARM_CACHE_WRITEBACK_SHIFT)

/*******************************************************************************
 * SCMI config
 ******************************************************************************/

/* Number of SCMI channels on the platform */
#define PLAT_ARM_SCMI_CHANNEL_COUNT	NRD_CHIP_COUNT

/*******************************************************************************
 * GIC/EHF config
 ******************************************************************************/

/* CPU Fault Handling Interrupt(FHI) PPI interrupt ID */
#define PLAT_CORE_FAULT_IRQ		U(17)

/* ARM platforms use 3 upper bits of secure interrupt priority */
#define PLAT_PRI_BITS			U(3)

#if ENABLE_FEAT_RAS && FFH_SUPPORT
#define PLAT_RAS_PRI			U(0x10)
#endif

#if ENABLE_FEAT_RAS && FFH_SUPPORT
#define PLAT_SP_PRI			PLAT_RAS_PRI
#else
#define PLAT_SP_PRI			U(0x10)
#endif

#define ARM_IRQ_SEC_PHY_TIMER		U(29)

#define ARM_IRQ_SEC_SGI_0		U(8)
#define ARM_IRQ_SEC_SGI_1		U(9)
#define ARM_IRQ_SEC_SGI_2		U(10)
#define ARM_IRQ_SEC_SGI_3		U(11)
#define ARM_IRQ_SEC_SGI_4		U(12)
#define ARM_IRQ_SEC_SGI_5		U(13)
#define ARM_IRQ_SEC_SGI_6		U(14)
#define ARM_IRQ_SEC_SGI_7		U(15)

#define ARM_G0_IRQ_PROPS(grp)						\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI,		\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE)

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define ARM_G1S_IRQ_PROPS(grp)						\
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_LEVEL),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE)

#define ARM_G0_IRQ_PROPS(grp)						\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, PLAT_SDEI_NORMAL_PRI,		\
			(grp), GIC_INTR_CFG_EDGE),			\
	INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,	\
			(grp), GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G1S_IRQ_PROPS(grp)	CSS_G1S_IRQ_PROPS(grp)
#define PLAT_ARM_G0_IRQ_PROPS(grp)	ARM_G0_IRQ_PROPS(grp)

#define PLAT_ARM_GICD_BASE		NRD_CSS_GIC_BASE
#define PLAT_ARM_GICR_BASE		NRD_CSS_GIC_BASE + UL(0x001C0000)

/*******************************************************************************
 * SDEI config
 ******************************************************************************/

#define PLAT_SDEI_CRITICAL_PRI		U(0x60)
#define PLAT_SDEI_NORMAL_PRI		U(0x70)

/* SGI used for SDEI signalling */
#define ARM_SDEI_SGI			ARM_IRQ_SEC_SGI_0

#if SDEI_IN_FCONF
/* ARM SDEI dynamic private event max count */
#define ARM_SDEI_DP_EVENT_MAX_CNT	U(3)

/* ARM SDEI dynamic shared event max count */
#define ARM_SDEI_DS_EVENT_MAX_CNT	U(3)
#else
/* ARM SDEI dynamic private event numbers */
#define ARM_SDEI_DP_EVENT_0		UL(1000)
#define ARM_SDEI_DP_EVENT_1		UL(1001)
#define ARM_SDEI_DP_EVENT_2		UL(1002)

/* ARM SDEI dynamic shared event numbers */
#define ARM_SDEI_DS_EVENT_0		UL(2000)
#define ARM_SDEI_DS_EVENT_1		UL(2001)
#define ARM_SDEI_DS_EVENT_2		UL(2002)

#define ARM_SDEI_PRIVATE_EVENTS						\
	SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI),				\
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_0,				\
	SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),				\
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_1,				\
	SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),				\
	SDEI_PRIVATE_EVENT(ARM_SDEI_DP_EVENT_2, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC)

#define ARM_SDEI_SHARED_EVENTS						\
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_0,				\
	SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),				\
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_1,				\
	SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),				\
	SDEI_SHARED_EVENT(ARM_SDEI_DS_EVENT_2,				\
	SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC)
#endif /* SDEI_IN_FCONF */

/*******************************************************************************
 * SDS config
 ******************************************************************************/

/* SDS ID for unusable CPU MPID list structure */
#define SDS_ISOLATED_CPU_LIST_ID	U(128)

/* Index of SDS region used in the communication with SCP */
#define SDS_SCP_AP_REGION_ID		U(0)

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
#define BOARD_CSS_PLAT_ID_REG_ADDR	NRD_ROS_PLATFORM_PERIPH_BASE +	\
						UL(0x00FE00E0)

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

#define V2M_SYSREGS_BASE		NRD_ROS_SYSTEM_PERIPH_BASE +	\
						UL(0x00010000)
#define V2M_FLASH0_BASE			NRD_ROS_SMC0_BASE
#define V2M_FLASH0_SIZE			NRD_ROS_SMC0_SIZE
#define V2M_FLASH_BLOCK_SIZE		UL(0x00040000)	/* 256 KB */

#define PLAT_ARM_FLASH_IMAGE_BASE	V2M_FLASH0_BASE
#define PLAT_ARM_FLASH_IMAGE_MAX_SIZE	(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_MEM_PROT_ADDR		(V2M_FLASH0_BASE +	\
					V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

#define PLAT_ARM_NVM_BASE		V2M_FLASH0_BASE
#define PLAT_ARM_NVM_SIZE		(V2M_FLASH0_SIZE - V2M_FLASH_BLOCK_SIZE)

/*******************************************************************************
 * ROS peripheral config
 ******************************************************************************/

/* Non-volatile counters */
#define SOC_TRUSTED_NVCTR_BASE		NRD_ROS_PLATFORM_PERIPH_BASE +	\
						UL(0x00E70000)
#define TFW_NVCTR_BASE			(SOC_TRUSTED_NVCTR_BASE	+ 0x0000)
#define TFW_NVCTR_SIZE			U(4)
#define NTFW_CTR_BASE			(SOC_TRUSTED_NVCTR_BASE + 0x0004)
#define NTFW_CTR_SIZE			U(4)

/*******************************************************************************
 * MMU mapping
 ******************************************************************************/

#define V2M_MAP_FLASH0_RW						\
		MAP_REGION_FLAT(V2M_FLASH0_BASE,			\
			V2M_FLASH0_SIZE,				\
			MT_DEVICE | MT_RW | EL3_PAS)

#define V2M_MAP_FLASH0_RO						\
		MAP_REGION_FLAT(V2M_FLASH0_BASE,			\
			V2M_FLASH0_SIZE,				\
			MT_RO_DATA | EL3_PAS)

#endif /* NRD_PLAT_ARM_DEF3_H */
