/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HISI_SRAM_MAP_H__
#define __HISI_SRAM_MAP_H__

/*
 * SRAM Memory Region Layout
 *
 *  +-----------------------+
 *  |  Low Power Mode       | 7KB
 *  +-----------------------+
 *  |  Secure OS            | 64KB
 *  +-----------------------+
 *  |  Software Flag        | 1KB
 *  +-----------------------+
 *
 */

#define SOC_SRAM_OFF_BASE_ADDR		(0xFFF80000)

/* PM Section: 7KB */
#define SRAM_PM_ADDR			(SOC_SRAM_OFF_BASE_ADDR)
#define SRAM_PM_SIZE			(0x00001C00)

/* TEE OS Section: 64KB */
#define SRAM_TEEOS_ADDR			(SRAM_PM_ADDR + SRAM_PM_SIZE)
#define SRAM_TEEOS_SIZE			(0x00010000)

/* General Use Section: 1KB */
#define SRAM_GENERAL_ADDR		(SRAM_TEEOS_ADDR + SRAM_TEEOS_SIZE)
#define SRAM_GENERAL_SIZE		(0x00000400)

/*
 * General Usage Section Layout:
 *
 *  +-----------------------+
 *  |  AP boot flag         | 64B
 *  +-----------------------+
 *  |  DICC flag            | 32B
 *  +-----------------------+
 *  |  Soft flag            | 256B
 *  +-----------------------+
 *  |  Thermal flag         | 128B
 *  +-----------------------+
 *  |  CSHELL               | 4B
 *  +-----------------------+
 *  |  Uart Switching       | 4B
 *  +-----------------------+
 *  |  ICC                  | 1024B
 *  +-----------------------+
 *  |  Memory Management    | 1024B
 *  +-----------------------+
 *  |  IFC                  | 32B
 *  +-----------------------+
 *  |  HIFI                 | 32B
 *  +-----------------------+
 *  |  DDR capacity         | 4B
 *  +-----------------------+
 *  |  Reserved             |
 *  +-----------------------+
 *
 */

/* App Core Boot Flags */
#define MEMORY_AXI_ACPU_START_ADDR		(SRAM_GENERAL_ADDR)
#define MEMORY_AXI_ACPU_START_SIZE		(64)

#define MEMORY_AXI_SRESET_FLAG_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0000)
#define MEMORY_AXI_SECOND_CPU_BOOT_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0004)
#define MEMORY_AXI_READY_FLAG_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0008)
#define MEMORY_AXI_FASTBOOT_ENTRY_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x000C)
#define MEMORY_AXI_PD_CHARGE_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0010)
#define MEMORY_AXI_DBG_ALARM_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0014)
#define MEMORY_AXI_CHIP_ADDR			(MEMORY_AXI_ACPU_START_ADDR + 0x0018)
#define MEMORY_AXI_BOARD_TYPE_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x001C)
#define MEMORY_AXI_BOARD_ID_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0020)
#define MEMORY_AXI_CHARGETYPE_FLAG_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0024)
#define MEMORY_AXI_COLD_START_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0028)
#define MEMORY_AXI_ANDROID_REBOOT_FLAG_ADDR	(MEMORY_AXI_ACPU_START_ADDR + 0x002C)
#define MEMORY_AXI_ACPU_WDTRST_REBOOT_FLAG_ADDR	(MEMORY_AXI_ACPU_START_ADDR + 0x0030)
#define MEMORY_AXI_ABNRST_BITMAP_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0034)
#define MEMORY_AXI_32K_CLK_TYPE_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x0038)
#define AXI_MODEM_PANIC_FLAG_ADDR		(MEMORY_AXI_ACPU_START_ADDR + 0x003C)
#define AXI_MODEM_PANIC_FLAG			(0x68697369)
#define MEMORY_AXI_ACPU_END_ADDR		(AXI_MODEM_PANIC_FLAG_ADDR + 4)

/* DICC Flags */
#define MEMORY_AXI_DICC_ADDR			(MEMORY_AXI_ACPU_START_ADDR + MEMORY_AXI_ACPU_START_SIZE)
#define MEMORY_AXI_DICC_SIZE			(32)

#define MEMORY_AXI_SOFT_FLAG_ADDR		(MEMORY_AXI_DICC_ADDR + MEMORY_AXI_DICC_SIZE)
#define MEMORY_AXI_SOFT_FLAG_SIZE		(256)

/* Thermal Flags */
#define MEMORY_AXI_TEMP_PROTECT_ADDR		(MEMORY_AXI_SOFT_FLAG_ADDR + MEMORY_AXI_SOFT_FLAG_SIZE)
#define MEMORY_AXI_TEMP_PROTECT_SIZE		(128)

/* CSHELL */
#define MEMORY_AXI_USB_CSHELL_ADDR		(MEMORY_AXI_TEMP_PROTECT_ADDR + MEMORY_AXI_TEMP_PROTECT_SIZE)
#define MEMORY_AXI_USB_CSHELL_SIZE		(4)

/* Uart and A/C Shell Switch Flags */
#define MEMORY_AXI_UART_INOUT_ADDR		(MEMORY_AXI_USB_CSHELL_ADDR + MEMORY_AXI_USB_CSHELL_SIZE)
#define MEMORY_AXI_UART_INOUT_SIZE		(4)

/* IFC Flags */
#define MEMORY_AXI_IFC_ADDR			(MEMORY_AXI_UART_INOUT_ADDR + MEMORY_AXI_UART_INOUT_SIZE)
#define MEMORY_AXI_IFC_SIZE			(32)

/* HIFI Data */
#define MEMORY_AXI_HIFI_ADDR			(MEMORY_AXI_IFC_ADDR + MEMORY_AXI_IFC_SIZE)
#define MEMORY_AXI_HIFI_SIZE			(32)

/* CONFIG Flags */
#define MEMORY_AXI_CONFIG_ADDR			(MEMORY_AXI_HIFI_ADDR + MEMORY_AXI_HIFI_SIZE)
#define MEMORY_AXI_CONFIG_SIZE			(32)

/* DDR Capacity Flags */
#define MEMORY_AXI_DDR_CAPACITY_ADDR		(MEMORY_AXI_CONFIG_ADDR + MEMORY_AXI_CONFIG_SIZE)
#define MEMORY_AXI_DDR_CAPACITY_SIZE		(4)

/* USB Shell Flags */
#define MEMORY_AXI_USB_SHELL_FLAG_ADDR		(MEMORY_AXI_DDR_CAPACITY_ADDR + MEMORY_AXI_DDR_CAPACITY_SIZE )
#define MEMORY_AXI_USB_SHELL_FLAG_SIZE		(4)

/* MCU WDT Switch Flag */
#define MEMORY_AXI_MCU_WDT_FLAG_ADDR		(MEMORY_AXI_USB_SHELL_FLAG_ADDR + MEMORY_AXI_USB_SHELL_FLAG_SIZE)
#define MEMORY_AXI_MCU_WDT_FLAG_SIZE		(4)

/* TLDSP Mailbox MNTN */
#define SRAM_DSP_MNTN_INFO_ADDR			(MEMORY_AXI_MCU_WDT_FLAG_ADDR + MEMORY_AXI_MCU_WDT_FLAG_SIZE)
#define SRAM_DSP_MNTN_SIZE			(32)

/* TLDSP ARM Mailbox Protect Flag */
#define SRAM_DSP_ARM_MAILBOX_PROTECT_FLAG_ADDR	(SRAM_DSP_MNTN_INFO_ADDR + SRAM_DSP_MNTN_SIZE)
#define SRAM_DSP_ARM_MAILBOX_PROTECT_FLAG_SIZE	(4)

/* RTT Sleep Flag */
#define SRAM_RTT_SLEEP_FLAG_ADDR                (SRAM_DSP_ARM_MAILBOX_PROTECT_FLAG_ADDR + SRAM_DSP_ARM_MAILBOX_PROTECT_FLAG_SIZE)
#define SRAM_RTT_SLEEP_FLAG_SIZE                (32)

/* LDSP Awake Flag */
#define MEMORY_AXI_LDSP_AWAKE_ADDR              (SRAM_RTT_SLEEP_FLAG_ADDR + SRAM_RTT_SLEEP_FLAG_SIZE)
#define MEMORY_AXI_LDSP_AWAKE_SIZE              (4)

#define NVUPDATE_SUCCESS			0x5555AAAA
#define NVUPDATE_FAILURE			0xAAAA5555

/*
 * Low Power Mode Region
 */
#define PWRCTRL_ACPU_ASM_SPACE_ADDR		(SRAM_PM_ADDR)
#define PWRCTRL_ACPU_ASM_SPACE_SIZE		(SRAM_PM_SIZE)

#define PWRCTRL_ACPU_ASM_MEM_BASE		(PWRCTRL_ACPU_ASM_SPACE_ADDR)
#define PWRCTRL_ACPU_ASM_MEM_SIZE		(PWRCTRL_ACPU_ASM_SPACE_SIZE)
#define PWRCTRL_ACPU_ASM_CODE_BASE		(PWRCTRL_ACPU_ASM_MEM_BASE + 0x200)
#define PWRCTRL_ACPU_ASM_DATA_BASE		(PWRCTRL_ACPU_ASM_MEM_BASE + 0xE00)
#define PWRCTRL_ACPU_ASM_DATA_SIZE		(0xE00)

#define PWRCTRL_ACPU_ASM_D_C0_ADDR		(PWRCTRL_ACPU_ASM_DATA_BASE)
#define PWRCTRL_ACPU_ASM_D_C0_MMU_PARA_AD	(PWRCTRL_ACPU_ASM_DATA_BASE + 0)
#define PWRCTRL_ACPU_ASM_D_ARM_PARA_AD		(PWRCTRL_ACPU_ASM_DATA_BASE + 0x20)

#define PWRCTRL_ACPU_ASM_D_COMM_ADDR		(PWRCTRL_ACPU_ASM_DATA_BASE + 0x700)

#define PWRCTRL_ACPU_REBOOT			(PWRCTRL_ACPU_ASM_D_COMM_ADDR)
#define PWRCTRL_ACPU_REBOOT_SIZE		(0x200)
#define PWRCTRL_ACPU_ASM_SLICE_BAK_ADDR		(PWRCTRL_ACPU_REBOOT + PWRCTRL_ACPU_REBOOT_SIZE)
#define PWRCTRL_ACPU_ASM_SLICE_BAK_SIZE		(4)
#define PWRCTRL_ACPU_ASM_DEBUG_FLAG_ADDR	(PWRCTRL_ACPU_ASM_SLICE_BAK_ADDR + PWRCTRL_ACPU_ASM_SLICE_BAK_SIZE)
#define PWRCTRL_ACPU_ASM_DEBUG_FLAG_SIZE	(4)
#define EXCH_A_CORE_POWRCTRL_CONV_ADDR		(PWRCTRL_ACPU_ASM_DEBUG_FLAG_ADDR + PWRCTRL_ACPU_ASM_DEBUG_FLAG_SIZE)
#define EXCH_A_CORE_POWRCTRL_CONV_SIZE		(4)

/*
 * Below region memory mapping is:
 * 4 + 12 + 16 + 28 + 28 + 16 + 28 + 12 + 24 + 20 + 64 +
 * 4 + 4 + 4 + 4 + 12 + 4 + 4 + 4 + 4 + 16 + 4 + 0x2BC +
 * 24 + 20 + 12 + 16
 */

#define MEMORY_AXI_CPU_IDLE_ADDR		(EXCH_A_CORE_POWRCTRL_CONV_ADDR + EXCH_A_CORE_POWRCTRL_CONV_SIZE)
#define MEMORY_AXI_CPU_IDLE_SIZE		(4)

#define MEMORY_AXI_CUR_FREQ_ADDR		(MEMORY_AXI_CPU_IDLE_ADDR + MEMORY_AXI_CPU_IDLE_SIZE)
#define MEMORY_AXI_CUR_FREQ_SIZE		(12)

#define MEMORY_AXI_ACPU_FREQ_VOL_ADDR		(MEMORY_AXI_CUR_FREQ_ADDR + MEMORY_AXI_CUR_FREQ_SIZE)
#define MEMORY_AXI_ACPU_FREQ_VOL_SIZE		(16 + 28 + 28)

#define MEMORY_AXI_DDR_FREQ_VOL_ADDR		(MEMORY_AXI_ACPU_FREQ_VOL_ADDR + MEMORY_AXI_ACPU_FREQ_VOL_SIZE)
#define MEMORY_AXI_DDR_FREQ_VOL_SIZE		(16 + 28)

#define MEMORY_AXI_ACPU_FIQ_TEST_ADDR		(MEMORY_AXI_DDR_FREQ_VOL_ADDR + MEMORY_AXI_DDR_FREQ_VOL_SIZE)
#define MEMORY_AXI_ACPU_FIQ_TEST_SIZE		(12)

#define MEMORY_AXI_ACPU_FIQ_CPU_INFO_ADDR	(MEMORY_AXI_ACPU_FIQ_TEST_ADDR + MEMORY_AXI_ACPU_FIQ_TEST_SIZE)
#define MEMORY_AXI_ACPU_FIQ_CPU_INFO_SIZE	(24)

#define MEMORY_AXI_ACPU_FIQ_DEBUG_INFO_ADDR	(MEMORY_AXI_ACPU_FIQ_CPU_INFO_ADDR + MEMORY_AXI_ACPU_FIQ_CPU_INFO_SIZE)
#define MEMORY_AXI_ACPU_FIQ_DEBUG_INFO_SIZE	(20)

#define MEMORY_FREQDUMP_ADDR			(MEMORY_AXI_ACPU_FIQ_DEBUG_INFO_ADDR + MEMORY_AXI_ACPU_FIQ_DEBUG_INFO_SIZE)
#define MEMORY_FREQDUMP_SIZE			(64)

#define MEMORY_AXI_CCPU_LOG_ADDR		(MEMORY_FREQDUMP_ADDR + MEMORY_FREQDUMP_SIZE)
#define MEMORY_AXI_CCPU_LOG_SIZE		(4)

#define MEMORY_AXI_MCU_LOG_ADDR			(MEMORY_AXI_CCPU_LOG_ADDR + MEMORY_AXI_CCPU_LOG_SIZE)
#define MEMORY_AXI_MCU_LOG_SIZE			(4)

#define MEMORY_AXI_SEC_CORE_BOOT_ADDR		(MEMORY_AXI_MCU_LOG_ADDR + MEMORY_AXI_MCU_LOG_SIZE)
#define MEMORY_AXI_SEC_CORE_BOOT_SIZE		(4)

#define MEMORY_AXI_BBP_PS_VOTE_FLAG_ADDR	(MEMORY_AXI_SEC_CORE_BOOT_ADDR + MEMORY_AXI_SEC_CORE_BOOT_SIZE)
#define MEMORY_AXI_BBP_PS_VOTE_FLAG_SIZE	(0x4)

#define POLICY_AREA_RESERVED			(MEMORY_AXI_BBP_PS_VOTE_FLAG_ADDR + MEMORY_AXI_BBP_PS_VOTE_FLAG_SIZE)
#define POLICY_AREA_RESERVED_SIZE		(12)

#define DDR_POLICY_VALID_MAGIC			(POLICY_AREA_RESERVED + POLICY_AREA_RESERVED_SIZE)
#define DDR_POLICY_VALID_MAGIC_SIZE		(4)

#define DDR_POLICY_MAX_NUM			(DDR_POLICY_VALID_MAGIC + DDR_POLICY_VALID_MAGIC_SIZE)
#define DDR_POLICY_MAX_NUM_SIZE			(4)

#define DDR_POLICY_SUPPORT_NUM			(DDR_POLICY_MAX_NUM + DDR_POLICY_MAX_NUM_SIZE)
#define DDR_POLICY_SUPPORT_NUM_SIZE		(4)

#define DDR_POLICY_CUR_POLICY			(DDR_POLICY_SUPPORT_NUM + DDR_POLICY_SUPPORT_NUM_SIZE)
#define DDR_POLICY_CUR_POLICY_SIZE		(4)

#define ACPU_POLICY_VALID_MAGIC			(DDR_POLICY_CUR_POLICY + DDR_POLICY_CUR_POLICY_SIZE)
#define ACPU_POLICY_VALID_MAGIC_SIZE		(4)

#define ACPU_POLICY_MAX_NUM			(ACPU_POLICY_VALID_MAGIC + ACPU_POLICY_VALID_MAGIC_SIZE)
#define ACPU_POLICY_MAX_NUM_SIZE		(4)

#define ACPU_POLICY_SUPPORT_NUM			(ACPU_POLICY_MAX_NUM + ACPU_POLICY_MAX_NUM_SIZE)
#define ACPU_POLICY_SUPPORT_NUM_SIZE		(4)

#define ACPU_POLICY_CUR_POLICY			(ACPU_POLICY_SUPPORT_NUM + ACPU_POLICY_SUPPORT_NUM_SIZE)
#define ACPU_POLICY_CUR_POLICY_SIZE		(4)

#define LPDDR_OPTION_ADDR			(ACPU_POLICY_CUR_POLICY + ACPU_POLICY_CUR_POLICY_SIZE)
#define LPDDR_OPTION_SIZE			(4)

#define MEMORY_AXI_DDR_DDL_ADDR			(LPDDR_OPTION_ADDR + LPDDR_OPTION_SIZE)
#define MEMORY_AXI_DDR_DDL_SIZE			(0x2BC)

#define DDR_TEST_DFS_ADDR			(MEMORY_AXI_DDR_DDL_ADDR + MEMORY_AXI_DDR_DDL_SIZE)
#define DDR_TEST_DFS_ADDR_SIZE			(4)

#define DDR_TEST_DFS_TIMES_ADDR			(DDR_TEST_DFS_ADDR + DDR_TEST_DFS_ADDR_SIZE)
#define DDR_TEST_DFS_TIMES_ADDR_SIZE		(4)

#define DDR_TEST_QOS_ADDR			(DDR_TEST_DFS_TIMES_ADDR + DDR_TEST_DFS_TIMES_ADDR_SIZE)
#define DDR_TEST_QOS_ADDR_SIZE			(4)

#define DDR_TEST_FUN_ADDR			(DDR_TEST_QOS_ADDR + DDR_TEST_QOS_ADDR_SIZE)
#define DDR_TEST_FUN_ADDR_SIZE			(4)

#define BOARD_TYPE_ADDR				(DDR_TEST_FUN_ADDR + DDR_TEST_FUN_ADDR_SIZE)
#define BOARD_ADDR_SIZE				(4)
#define DDR_DFS_FREQ_ADDR			(BOARD_TYPE_ADDR + BOARD_ADDR_SIZE)
#define DDR_DFS_FREQ_SIZE			(4)

#define DDR_PASR_ADDR				(DDR_DFS_FREQ_ADDR + DDR_DFS_FREQ_SIZE)
#define DDR_PASR_SIZE				(20)

#define ACPU_DFS_FREQ_ADDR			(DDR_PASR_ADDR + DDR_PASR_SIZE)
#define ACPU_DFS_FREQ_ADDR_SIZE			(12)

#define ACPU_CHIP_MAX_FREQ			(ACPU_DFS_FREQ_ADDR + ACPU_DFS_FREQ_ADDR_SIZE)
#define ACPU_CHIP_MAX_FREQ_SIZE			(4)

#define MEMORY_MEDPLL_STATE_ADDR		(ACPU_CHIP_MAX_FREQ + ACPU_CHIP_MAX_FREQ_SIZE)
#define MEMORY_MEDPLL_STATE_SIZE		(8)

#define MEMORY_CCPU_LOAD_FLAG_ADDR		(MEMORY_MEDPLL_STATE_ADDR + MEMORY_MEDPLL_STATE_SIZE)
#define MEMORY_CCPU_LOAD_FLAG_SIZE		(4)


#define ACPU_CORE_BITS_ADDR			(MEMORY_CCPU_LOAD_FLAG_ADDR + MEMORY_CCPU_LOAD_FLAG_SIZE)
#define ACPU_CORE_BITS_SIZE			(4)

#define ACPU_CLUSTER_IDLE_ADDR			(ACPU_CORE_BITS_ADDR + ACPU_CORE_BITS_SIZE)
#define ACPU_CLUSTER_IDLE_SIZE			(4)

#define ACPU_A53_FLAGS_ADDR			(ACPU_CLUSTER_IDLE_ADDR + ACPU_CLUSTER_IDLE_SIZE)
#define ACPU_A53_FLAGS_SIZE			(4)

#define ACPU_POWER_STATE_QOS_ADDR		(ACPU_A53_FLAGS_ADDR+ACPU_A53_FLAGS_SIZE)
#define ACPU_POWER_STATE_QOS_SIZE		(4)

#define ACPU_UNLOCK_CORE_FLAGS_ADDR		(ACPU_POWER_STATE_QOS_ADDR+ACPU_POWER_STATE_QOS_SIZE)
#define ACPU_UNLOCK_CORE_FLAGS_SIZE		(8)

#define ACPU_SUBSYS_POWERDOWN_FLAGS_ADDR	(ACPU_UNLOCK_CORE_FLAGS_ADDR + ACPU_UNLOCK_CORE_FLAGS_SIZE)
#define ACPU_SUBSYS_POWERDOWN_FLAGS_SIZE	(4)

#define ACPU_CORE_POWERDOWN_FLAGS_ADDR		(ACPU_SUBSYS_POWERDOWN_FLAGS_ADDR + ACPU_SUBSYS_POWERDOWN_FLAGS_SIZE)
#define ACPU_CORE_POWERDOWN_FLAGS_SIZE		(4)

#define ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR	(ACPU_CORE_POWERDOWN_FLAGS_ADDR + ACPU_CORE_POWERDOWN_FLAGS_SIZE)
#define ACPU_CLUSTER_POWERDOWN_FLAGS_SIZE	(4)

#define ACPU_ARM64_FLAGA			(ACPU_CLUSTER_POWERDOWN_FLAGS_ADDR + ACPU_CLUSTER_POWERDOWN_FLAGS_SIZE)
#define ACPU_ARM64_FLAGA_SIZE			(4)

#define ACPU_ARM64_FLAGB			(ACPU_ARM64_FLAGA + ACPU_ARM64_FLAGA_SIZE)
#define ACPU_ARM64_FLAGB_SIZE			(4)

#define MCU_EXCEPTION_FLAGS_ADDR		(ACPU_ARM64_FLAGB + ACPU_ARM64_FLAGB_SIZE)
#define MCU_EXCEPTION_FLAGS_SIZE		(4)

#define ACPU_MASTER_CORE_STATE_ADDR		(MCU_EXCEPTION_FLAGS_ADDR + MCU_EXCEPTION_FLAGS_SIZE)
#define ACPU_MASTER_CORE_STATE_SIZE		(4)

#define PWRCTRL_AXI_RESERVED_ADDR		(ACPU_MASTER_CORE_STATE_ADDR + ACPU_MASTER_CORE_STATE_SIZE)

#endif /* __HISI_SRAM_MAP_H__ */
