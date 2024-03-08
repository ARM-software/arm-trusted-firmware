/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include "rpi_hw.h"

/* Special value used to verify platform parameters from BL2 to BL31 */
#define RPI3_BL31_PLAT_PARAM_VAL	ULL(0x0F1E2D3C4B5A6978)

#define PLATFORM_STACK_SIZE		ULL(0x1000)

#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CORE_COUNT		PLATFORM_CLUSTER0_CORE_COUNT

#define RPI_PRIMARY_CPU			U(0)

#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		U(0)
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		U(1)
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF		U(2)

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		U(4)
#define PLAT_LOCAL_PSTATE_MASK		((U(1) << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		U(6)
#define CACHE_WRITEBACK_GRANULE		(U(1) << CACHE_WRITEBACK_SHIFT)

/*
 * I/O registers.
 */
#define DEVICE0_BASE			RPI_IO_BASE
#define DEVICE0_SIZE			RPI_IO_SIZE

/*
 * Mailbox to control the secondary cores. All secondary cores are held in a
 * wait loop in cold boot. To release them perform the following steps (plus
 * any additional barriers that may be needed):
 *
 *     uint64_t *entrypoint = (uint64_t *)PLAT_RPI3_TM_ENTRYPOINT;
 *     *entrypoint = ADDRESS_TO_JUMP_TO;
 *
 *     uint64_t *mbox_entry = (uint64_t *)PLAT_RPI3_TM_HOLD_BASE;
 *     mbox_entry[cpu_id] = PLAT_RPI3_TM_HOLD_STATE_GO;
 *
 *     sev();
 */
/* The secure entry point to be used on warm reset by all CPUs. */
#define PLAT_RPI3_TM_ENTRYPOINT		0x100
#define PLAT_RPI3_TM_ENTRYPOINT_SIZE	ULL(8)

/* Hold entries for each CPU. */
#define PLAT_RPI3_TM_HOLD_BASE		(PLAT_RPI3_TM_ENTRYPOINT + \
					 PLAT_RPI3_TM_ENTRYPOINT_SIZE)
#define PLAT_RPI3_TM_HOLD_ENTRY_SIZE	ULL(8)
#define PLAT_RPI3_TM_HOLD_SIZE		(PLAT_RPI3_TM_HOLD_ENTRY_SIZE * \
					 PLATFORM_CORE_COUNT)

#define PLAT_RPI3_TRUSTED_MAILBOX_SIZE	(PLAT_RPI3_TM_ENTRYPOINT_SIZE + \
					 PLAT_RPI3_TM_HOLD_SIZE)

#define PLAT_RPI3_TM_HOLD_STATE_WAIT	ULL(0)
#define PLAT_RPI3_TM_HOLD_STATE_GO	ULL(1)
#define PLAT_RPI3_TM_HOLD_STATE_BSP_OFF	ULL(2)

/*
 * BL31 specific defines.
 *
 * Put BL31 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define PLAT_MAX_BL31_SIZE		ULL(0x80000)

#define BL31_BASE			ULL(0x1000)
#define BL31_LIMIT			ULL(0x80000)
#define BL31_PROGBITS_LIMIT		ULL(0x80000)

#define SEC_SRAM_ID			0
#define SEC_DRAM_ID			1

/*
 * Other memory-related defines.
 */
#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)

#define MAX_MMAP_REGIONS		8
#define MAX_XLAT_TABLES			4

#define MAX_IO_DEVICES			U(3)
#define MAX_IO_HANDLES			U(4)

#define MAX_IO_BLOCK_DEVICES		U(1)

/*
 * Serial-related constants.
 */
#define PLAT_RPI_MINI_UART_BASE		RPI4_MINI_UART_BASE
#define PLAT_RPI_PL011_UART_BASE	RPI4_PL011_UART_BASE
#define PLAT_RPI_PL011_UART_CLOCK       RPI4_PL011_UART_CLOCK
#define PLAT_RPI_UART_BAUDRATE          ULL(115200)
#define PLAT_RPI_CRASH_UART_BASE	PLAT_RPI_MINI_UART_BASE

/*
 * System counter
 */
#define SYS_COUNTER_FREQ_IN_TICKS	ULL(54000000)

#endif /* PLATFORM_DEF_H */
