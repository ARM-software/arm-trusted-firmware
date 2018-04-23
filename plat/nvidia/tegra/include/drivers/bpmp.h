/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BPMP_H
#define BPMP_H

#include <stdint.h>

/* macro to enable clock to the Atomics block */
#define CAR_ENABLE_ATOMICS	(1U << 16)

/* command to get the channel base addresses from bpmp */
#define ATOMIC_CMD_GET		4U

/* Hardware IRQ # used to signal bpmp of an incoming command */
#define INT_SHR_SEM_OUTBOX_FULL	6U

/* macros to decode the bpmp's state */
#define CH_MASK(ch)		((uint32_t)0x3 << ((ch) * 2U))
#define MA_FREE(ch)		((uint32_t)0x2 << ((ch) * 2U))
#define MA_ACKD(ch)		((uint32_t)0x3 << ((ch) * 2U))

/* response from bpmp to indicate it has powered up */
#define SIGN_OF_LIFE		0xAAAAAAAAU

/* flags to indicate bpmp driver's state */
#define BPMP_NOT_PRESENT	0xF00DBEEFU
#define BPMP_INIT_COMPLETE	0xBEEFF00DU
#define BPMP_INIT_PENDING	0xDEADBEEFU
#define BPMP_SUSPEND_ENTRY	0xF00DCAFEU

/* requests serviced by the bpmp */
#define MRQ_PING		0
#define MRQ_QUERY_TAG		1
#define MRQ_DO_IDLE		2
#define MRQ_TOLERATE_IDLE	3
#define MRQ_MODULE_LOAD		4
#define MRQ_MODULE_UNLOAD	5
#define MRQ_SWITCH_CLUSTER	6
#define MRQ_TRACE_MODIFY	7
#define MRQ_WRITE_TRACE		8
#define MRQ_THREADED_PING	9
#define MRQ_CPUIDLE_USAGE	10
#define MRQ_MODULE_MAIL		11
#define MRQ_SCX_ENABLE		12
#define MRQ_BPMPIDLE_USAGE	14
#define MRQ_HEAP_USAGE		15
#define MRQ_SCLK_SKIP_SET_RATE	16
#define MRQ_ENABLE_SUSPEND	17
#define MRQ_PASR_MASK		18
#define MRQ_DEBUGFS		19
#define MRQ_THERMAL		27

/* Tegra PM states as known to BPMP */
#define TEGRA_PM_CC1		9
#define TEGRA_PM_CC4		12
#define TEGRA_PM_CC6		14
#define TEGRA_PM_CC7		15
#define TEGRA_PM_SC1		17
#define TEGRA_PM_SC2		18
#define TEGRA_PM_SC3		19
#define TEGRA_PM_SC4		20
#define TEGRA_PM_SC7		23

/* flag to indicate if entry into a CCx power state is allowed */
#define BPMP_CCx_ALLOWED	0U

/* number of communication channels to interact with the bpmp */
#define NR_CHANNELS		4U

/* flag to ask bpmp to acknowledge command packet */
#define NO_ACK			(0U << 0U)
#define DO_ACK			(1U << 0U)

/* size of the command/response data */
#define MSG_DATA_MAX_SZ		120U

/**
 * command/response packet to/from the bpmp
 *
 * command
 * -------
 * code: MRQ_* command
 * flags: DO_ACK or NO_ACK
 * data:
 * 	[0] = cpu #
 * 	[1] = cluster power state (TEGRA_PM_CCx)
 * 	[2] = system power state (TEGRA_PM_SCx)
 *
 * response
 * ---------
 * code: error code
 * flags: not used
 * data:
 * 	[0-3] = response value
 */
typedef struct mb_data {
	int32_t code;
	uint32_t flags;
	uint8_t data[MSG_DATA_MAX_SZ];
} mb_data_t;

/**
 * Function to initialise the interface with the bpmp
 */
int tegra_bpmp_init(void);

/**
 * Function to suspend the interface with the bpmp
 */
void tegra_bpmp_suspend(void);

/**
 * Function to resume the interface with the bpmp
 */
void tegra_bpmp_resume(void);

/**
 * Handler to send a MRQ_* command to the bpmp
 */
int32_t tegra_bpmp_send_receive_atomic(int mrq, const void *ob_data, int ob_sz,
		void *ib_data, int ib_sz);

#endif /* BPMP_H */
