/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef PLAT_PM_TRACE_H
#define PLAT_PM_TRACE_H

/*
 * PM Trace is for Debug purpose only!!!
 * It should not be enabled during System Run time
 */
#undef PM_TRACE_ENABLE


/* trace entry time */
struct pm_trace_entry {
	/* trace entry time stamp */
	unsigned int timestamp;

	/* trace info
	 * [16-31] - API Trace Id
	 * [00-15] - API Step Id
	 */
	unsigned int trace_info;
};

struct pm_trace_ctrl {
	/* trace pointer - points to next free entry in trace cyclic queue */
	unsigned int trace_pointer;

	/* trace count - number of entries in the queue, clear upon read */
	unsigned int trace_count;
};

/* trace size definition */
#define AP_MSS_ATF_CORE_INFO_SIZE	(256)
#define AP_MSS_ATF_CORE_ENTRY_SIZE	(8)
#define AP_MSS_ATF_TRACE_SIZE_MASK	(0xFF)

/* trace address definition */
#define AP_MSS_TIMER_BASE		(MVEBU_REGS_BASE_MASK + 0x580110)

#define AP_MSS_ATF_CORE_0_CTRL_BASE	(MVEBU_REGS_BASE_MASK + 0x520140)
#define AP_MSS_ATF_CORE_1_CTRL_BASE	(MVEBU_REGS_BASE_MASK + 0x520150)
#define AP_MSS_ATF_CORE_2_CTRL_BASE	(MVEBU_REGS_BASE_MASK + 0x520160)
#define AP_MSS_ATF_CORE_3_CTRL_BASE	(MVEBU_REGS_BASE_MASK + 0x520170)
#define AP_MSS_ATF_CORE_CTRL_BASE	(AP_MSS_ATF_CORE_0_CTRL_BASE)

#define AP_MSS_ATF_CORE_0_INFO_BASE	(MVEBU_REGS_BASE_MASK + 0x5201C0)
#define AP_MSS_ATF_CORE_0_INFO_TRACE	(MVEBU_REGS_BASE_MASK + 0x5201C4)
#define AP_MSS_ATF_CORE_1_INFO_BASE	(MVEBU_REGS_BASE_MASK + 0x5209C0)
#define AP_MSS_ATF_CORE_1_INFO_TRACE	(MVEBU_REGS_BASE_MASK + 0x5209C4)
#define AP_MSS_ATF_CORE_2_INFO_BASE	(MVEBU_REGS_BASE_MASK + 0x5211C0)
#define AP_MSS_ATF_CORE_2_INFO_TRACE	(MVEBU_REGS_BASE_MASK + 0x5211C4)
#define AP_MSS_ATF_CORE_3_INFO_BASE	(MVEBU_REGS_BASE_MASK + 0x5219C0)
#define AP_MSS_ATF_CORE_3_INFO_TRACE	(MVEBU_REGS_BASE_MASK + 0x5219C4)
#define AP_MSS_ATF_CORE_INFO_BASE	(AP_MSS_ATF_CORE_0_INFO_BASE)

/* trace info definition */
#define TRACE_PWR_DOMAIN_OFF		(0x10000)
#define TRACE_PWR_DOMAIN_SUSPEND	(0x20000)
#define TRACE_PWR_DOMAIN_SUSPEND_FINISH	(0x30000)
#define TRACE_PWR_DOMAIN_ON		(0x40000)
#define TRACE_PWR_DOMAIN_ON_FINISH	(0x50000)

#define TRACE_PWR_DOMAIN_ON_MASK	(0xFF)

#ifdef PM_TRACE_ENABLE

/* trace API definition */
void pm_core_0_trace(unsigned int trace);
void pm_core_1_trace(unsigned int trace);
void pm_core_2_trace(unsigned int trace);
void pm_core_3_trace(unsigned int trace);

typedef void (*core_trace_func)(unsigned int);

extern core_trace_func funcTbl[PLATFORM_CORE_COUNT];

#define PM_TRACE(trace) funcTbl[plat_my_core_pos()](trace)

#else

#define PM_TRACE(trace)

#endif

/*******************************************************************************
 * pm_trace_add
 *
 * DESCRIPTION: Add PM trace
 ******************************************************************************
 */
void pm_trace_add(unsigned int trace, unsigned int core);

#endif /* PLAT_PM_TRACE_H */
