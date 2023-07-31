/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (c) 2017-2023 Nuvoton Technology Corp.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/interrupt_props.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/arm/gic_common.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <npcm845x_arm_def.h>
#include <plat/arm/common/smccc_def.h>
#include <plat/common/common_def.h>

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "=" VALUE(var)

#define PLATFORM_LINKER_FORMAT "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH aarch64

#define PLATFORM_STACK_SIZE 0x400

#define PLATFORM_CORE_COUNT NPCM845x_PLATFORM_CORE_COUNT
#define PLATFORM_CLUSTER_COUNT NPCM845x_CLUSTER_COUNT
#define PLATFORM_MAX_CPU_PER_CLUSTER NPCM845x_MAX_CPU_PER_CLUSTER
#define PLAT_PRIMARY_CPU NPCM845x_PLAT_PRIMARY_CPU
#define PLATFORM_SYSTEM_COUNT NPCM845x_SYSTEM_COUNT

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF U(2)

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE PLAT_LOCAL_STATE_OFF
#define PLAT_MAX_RET_STATE PLAT_LOCAL_STATE_RET

#define PLAT_NUM_PWR_DOMAINS (PLATFORM_CLUSTER_COUNT + PLATFORM_CORE_COUNT)
#define NPCM845x_MAX_PWR_LVL ARM_PWR_LVL1

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH 4
#define PLAT_LOCAL_PSTATE_MASK ((1 << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Required ARM standard platform porting definitions
 */
#define PLAT_ARM_CLUSTER_COUNT PLATFORM_CLUSTER_COUNT

#define PLAT_NUM_PWR_DOMAINS (PLATFORM_CLUSTER_COUNT + PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL NPCM845x_MAX_PWR_LVL

#define PLAT_LOCAL_PSTATE_WIDTH 4
#define PLAT_LOCAL_PSTATE_MASK ((1 << PLAT_LOCAL_PSTATE_WIDTH) - 1)

#ifdef BL32_BASE

#ifndef CONFIG_TARGET_ARBEL_PALLADIUM
#define PLAT_ARM_TRUSTED_DRAM_BASE BL32_BASE
#else
#define PLAT_ARM_TRUSTED_DRAM_BASE BL32_BASE
#endif /* CONFIG_TARGET_ARBEL_PALLADIUM */

#define PLAT_ARM_TRUSTED_DRAM_SIZE UL(0x02000000) /* 32 MB */
#endif /* BL32_BASE */

#define PWR_DOMAIN_AT_MAX_LVL U(1)

#define PLAT_VIRT_ADDR_SPACE_SIZE (1ull << 32)
#define PLAT_PHY_ADDR_SPACE_SIZE (1ull << 32)
#define MAX_XLAT_TABLES 16
#define PLAT_ARM_MMAP_ENTRIES 17

#ifdef NPCM845X_DEBUG
#define MAX_MMAP_REGIONS 8
#define NPCM845X_TZ1_BASE 0xFFFB0000
#endif /* NPCM845X_DEBUG */

#define FIQ_SMP_CALL_SGI 10

/* (0x00040000) 128  KB, the rest 128K if it is non secured */
#define PLAT_ARM_TRUSTED_SRAM_SIZE UL(0x00020000)

#define ARM_SHARED_RAM_SIZE UL(0x00001000) /* 4 KB */

/* UL(0xfffCE000) add calc ARM_TRUSTED_SRAM_BASE */
#define ARM_SHARED_RAM_BASE (BL31_BASE + 0x00020000 - ARM_SHARED_RAM_SIZE)

/* The remaining Trusted SRAM is used to load the BL images */
#define ARM_BL_RAM_BASE (ARM_SHARED_RAM_BASE + ARM_SHARED_RAM_SIZE)

/*
 * PLAT_ARM_TRUSTED_SRAM_SIZE is taken from platform_def.h 0x20000
 * because only half is secured in this specific implementation
 */
#define ARM_BL_RAM_SIZE (PLAT_ARM_TRUSTED_SRAM_SIZE - ARM_SHARED_RAM_SIZE)

#if RESET_TO_BL31
/* Size of Trusted SRAM - the first 4KB of shared memory */
#define PLAT_ARM_MAX_BL31_SIZE                                                 \
	(PLAT_ARM_TRUSTED_SRAM_SIZE - ARM_SHARED_RAM_SIZE)
#else
/*
 * Since BL31 NOBITS overlays BL2 and BL1-RW, PLAT_ARM_MAX_BL31_SIZE
 * is calculated using the current BL31 PROGBITS debug size plus the sizes
 * of BL2 and BL1-RW
 */
#define PLAT_ARM_MAX_BL31_SIZE                                                 \
	(PLAT_ARM_TRUSTED_SRAM_SIZE - ARM_SHARED_RAM_SIZE)
#endif /* RESET_TO_BL31 */
/*
 * Load address of BL33 for this platform port
 */
#define PLAT_ARM_NS_IMAGE_BASE (ARM_DRAM1_BASE + UL(0x6208000))

#ifdef NPCM845X_DEBUG
#define COUNTER_FREQUENCY 0x07735940 /* f/4 = 125MHz */
#endif /* NPCM845X_DEBUG */

#define COUNTER_FREQUENCY 0x0EE6B280 /* f/2 = 250MHz */
#define PLAT_ARM_NSTIMER_FRAME_ID U(1)

/* GIC parameters */

/* Base  compatible GIC memory map */
#define NT_GIC_BASE (0xDFFF8000)
#define BASE_GICD_BASE (NT_GIC_BASE + 0x1000)
#define BASE_GICC_BASE (NT_GIC_BASE + 0x2000)
#define BASE_GICR_BASE (NT_GIC_BASE + 0x200000)
#define BASE_GICH_BASE (NT_GIC_BASE + 0x4000)
#define BASE_GICV_BASE (NT_GIC_BASE + 0x6000)

#define DEVICE1_BASE BASE_GICD_BASE
#define DEVICE1_SIZE 0x7000

#ifdef NPCM845X_DEBUG
/* ((BASE_GICR_BASE - BASE_GICD_BASE) +	 (PLATFORM_CORE_COUNT * 0x20000)) */
#define ARM_CPU_START_ADDRESS(m) UL(0xf0800e00 + 0x10 + m * 4)
#endif /* NPCM845X_DEBUG */

#define PLAT_REG_BASE NPCM845x_REG_BASE
#define PLAT_REG_SIZE NPCM845x_REG_SIZE

/* MMU entry for internal (register) space access */
#define MAP_DEVICE0                                                            \
	MAP_REGION_FLAT(PLAT_REG_BASE, PLAT_REG_SIZE, MT_DEVICE | MT_RW | MT_NS)

#define MAP_DEVICE1                                                            \
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE,                            \
			MT_DEVICE | MT_RW | MT_SECURE)

/*
 * Define a list of Group 1 Secure and Group 0 interrupt properties
 * as per GICv3 terminology. On a GICv2 system or mode,
 * the lists will be merged and treated as Group 0 interrupts.
 */
#define PLAT_ARM_GICD_BASE BASE_GICD_BASE
#define PLAT_ARM_GICC_BASE BASE_GICC_BASE

#define PLAT_ARM_G1S_IRQ_PROPS(grp)                                            \
	INTR_PROP_DESC(ARM_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp,   \
		       GIC_INTR_CFG_LEVEL),                                    \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE),                        \
		INTR_PROP_DESC(ARM_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY,    \
			       grp, GIC_INTR_CFG_EDGE)

#define PLAT_ARM_G0_IRQ_PROPS(grp)

/* Required for compilation: */

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_ARM_MAX_BL1_RW_SIZE 0 /* UL(0xB000) */
#if USE_ROMLIB
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE UL(0x1000)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE UL(0xe000)
#define FVP_BL2_ROMLIB_OPTIMIZATION UL(0x6000)
#else
#define PLAT_ARM_MAX_ROMLIB_RW_SIZE UL(0)
#define PLAT_ARM_MAX_ROMLIB_RO_SIZE UL(0)
#define FVP_BL2_ROMLIB_OPTIMIZATION UL(0)
#endif /* USE_ROMLIB */

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size
 * plus a little space for growth.
 */
#if TRUSTED_BOARD_BOOT
#define PLAT_ARM_MAX_BL2_SIZE (UL(0x1D000) * FVP_BL2_ROMLIB_OPTIMIZATION)
#else
/* (UL(0x13000) - FVP_BL2_ROMLIB_OPTIMIZATION) */
#define PLAT_ARM_MAX_BL2_SIZE 0
#endif /* TRUSTED_BOARD_BOOT */

#undef NPCM_PRINT_ONCE
#ifdef NPCM_PRINT_ONCE
#define PRINT_ONLY_ONCE
#pragma message(VAR_NAME_VALUE(ARM_AP_TZC_DRAM1_BASE))
#pragma message(VAR_NAME_VALUE(BL31_BASE))
#pragma message(VAR_NAME_VALUE(BL31_LIMIT))
#pragma message(VAR_NAME_VALUE(PLAT_ARM_MAX_BL31_SIZE))
#pragma message(VAR_NAME_VALUE(BL32_BASE))
#pragma message(VAR_NAME_VALUE(BL32_LIMIT))
#pragma message(VAR_NAME_VALUE(PLAT_ARM_MAX_BL32_SIZE)
#pragma message(VAR_NAME_VALUE(SPMD_SPM_AT_SEL2_KKO))
#endif /* NPCM_PRINT_ONCE */

#define MAX_IO_DEVICES 4
#define MAX_IO_HANDLES 4

#define PLAT_ARM_FIP_BASE 0x0
#define PLAT_ARM_FIP_MAX_SIZE PLAT_ARM_MAX_BL31_SIZE

#define PLAT_ARM_BOOT_UART_BASE 0xF0000000
#define PLAT_ARM_BOOT_UART_CLK_IN_HZ 115200
#define PLAT_ARM_RUN_UART_BASE 0xF0000000
#define PLAT_ARM_RUN_UART_CLK_IN_HZ 115200
#define PLAT_ARM_CRASH_UART_BASE 0xF0000000
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ 115200

/*
 * Mailbox to control the secondary cores.All secondary cores are held in a wait
 * loop in cold boot. To release them perform the following steps (plus any
 * additional barriers that may be needed):
 *
 *     uint64_t *entrypoint = (uint64_t *)PLAT_NPCM_TM_ENTRYPOINT;
 *     *entrypoint = ADDRESS_TO_JUMP_TO;
 *
 *     uint64_t *mbox_entry = (uint64_t *)PLAT_NPCM_TM_HOLD_BASE;
 *     mbox_entry[cpu_id] = PLAT_NPCM_TM_HOLD_BASE;
 *
 *     sev();
 */
#define PLAT_NPCM_TRUSTED_MAILBOX_BASE ARM_SHARED_RAM_BASE

/* The secure entry point to be used on warm reset by all CPUs. */
#define PLAT_NPCM_TM_ENTRYPOINT PLAT_NPCM_TRUSTED_MAILBOX_BASE
#define PLAT_NPCM_TM_ENTRYPOINT_SIZE ULL(8)

/* Hold entries for each CPU. */
#define PLAT_NPCM_TM_HOLD_BASE                                                 \
	(PLAT_NPCM_TM_ENTRYPOINT + PLAT_NPCM_TM_ENTRYPOINT_SIZE)
#define PLAT_NPCM_TM_HOLD_ENTRY_SIZE ULL(8)
#define PLAT_NPCM_TM_HOLD_SIZE                                                 \
	(PLAT_NPCM_TM_HOLD_ENTRY_SIZE * PLATFORM_CORE_COUNT)
#define PLAT_NPCM_TRUSTED_NOTIFICATION_BASE                                    \
	(PLAT_NPCM_TM_ENTRYPOINT_SIZE + PLAT_NPCM_TM_HOLD_SIZE)

#define PLAT_NPCM_TRUSTED_NOTIFICATION_ENTRY_SIZE ULL(8)

#define PLAT_NPCM_TRUSTED_NOTIFICATION_SIZE                                    \
	(PLAT_NPCM_TRUSTED_NOTIFICATION_ENTRY_SIZE * PLATFORM_CORE_COUNT)

#define PLAT_NPCM_TRUSTED_MAILBOX_SIZE                                         \
	(PLAT_NPCM_TM_ENTRYPOINT_SIZE + PLAT_NPCM_TM_HOLD_SIZE +               \
	 PLAT_NPCM_TRUSTED_NOTIFICATION_ENTRY_SIZE)

#define PLAT_NPCM_TM_HOLD_STATE_WAIT ULL(0)
#define PLAT_NPCM_TM_HOLD_STATE_GO ULL(1)
#define PLAT_NPCM_TM_HOLD_STATE_BSP_OFF ULL(2)

#define PLAT_NPCM_TM_NOTIFICATION_START ULL(0xAA)
#define PLAT_NPCM_TM_NOTIFICATION_BR ULL(0xCC)

#ifdef NPCM845X_DEBUG
#define PLAT_ARM_TRUSTED_MAILBOX_BASE 0xfffB0000
#endif /* NPCM845X_DEBUG */

#endif /* PLATFORM_DEF_H */
