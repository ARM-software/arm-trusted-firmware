/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTISECLIB_DEFS_H
#define QTISECLIB_DEFS_H

#include <stdint.h>

#ifndef u_register_t
typedef uintptr_t u_register_t;
#endif

/*
 * Different Log Level supported in qtiseclib.
 * TODO: Currently no filtering done on QTISECLIB logs.
 */
#define QTISECLIB_LOG_LEVEL_NONE	0
#define QTISECLIB_LOG_LEVEL_ERROR	10
#define QTISECLIB_LOG_LEVEL_NOTICE	20
#define QTISECLIB_LOG_LEVEL_WARNING	30
#define QTISECLIB_LOG_LEVEL_INFO	40
#define QTISECLIB_LOG_LEVEL_VERBOSE	50

#define QTI_GICV3_IRM_PE		0
#define QTI_GICV3_IRM_ANY		1

/* Common interrupt number/ID defs. */
#define QTISECLIB_INT_ID_RESET_SGI			(0xf)
#define QTISECLIB_INT_ID_CPU_WAKEUP_SGI			(0x8)

#define	QTISECLIB_INT_INVALID_INT_NUM			(0xFFFFFFFFU)

typedef struct qtiseclib_cb_spinlock {
	volatile uint32_t lock;
} qtiseclib_cb_spinlock_t;

#if QTI_SDI_BUILD
/* External CPU Dump Structure - 64 bit EL */
typedef struct {
	uint64_t x0;
	uint64_t x1;
	uint64_t x2;
	uint64_t x3;
	uint64_t x4;
	uint64_t x5;
	uint64_t x6;
	uint64_t x7;
	uint64_t x8;
	uint64_t x9;
	uint64_t x10;
	uint64_t x11;
	uint64_t x12;
	uint64_t x13;
	uint64_t x14;
	uint64_t x15;
	uint64_t x16;
	uint64_t x17;
	uint64_t x18;
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t x29;
	uint64_t x30;
	uint64_t pc;
	uint64_t currentEL;
	uint64_t sp_el3;
	uint64_t elr_el3;
	uint64_t spsr_el3;
	uint64_t sp_el2;
	uint64_t elr_el2;
	uint64_t spsr_el2;
	uint64_t sp_el1;
	uint64_t elr_el1;
	uint64_t spsr_el1;
	uint64_t sp_el0;
	uint64_t __reserved1;
	uint64_t __reserved2;
	uint64_t __reserved3;
	uint64_t __reserved4;
	uint64_t __reserved5;
	uint64_t __reserved6;
	uint64_t __reserved7;
	uint64_t __reserved8;
} qtiseclib_dbg_a64_ctxt_regs_type;

typedef enum qtiseclib_mmap_attr_s {
	QTISECLIB_MAP_NS_RO_XN_DATA = 1,
	QTISECLIB_MAP_RW_XN_NC_DATA = 2,
	QTISECLIB_MAP_RW_XN_DATA = 3,
} qtiseclib_mmap_attr_t;

#endif /* QTI_SDI_BUILD */

#endif /* QTISECLIB_DEFS_H */
