/*
 * Copyright (c) 2017-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QOS_COMMON_H
#define QOS_COMMON_H

#define RCAR_REF_DEFAULT		(0U)

#if (RCAR_LSI == RCAR_E3)
/* define used for E3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 3.9usec */
#define SUB_SLOT_CYCLE_E3		(0xAFU)	/* 175 */
#else /* REF 7.8usec */
#define SUB_SLOT_CYCLE_E3		(0x15EU)	/* 350 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define OPERATING_FREQ_E3		(266U)	/* MHz */
#define SL_INIT_SSLOTCLK_E3		(SUB_SLOT_CYCLE_E3 -1U)
#define QOSWT_WTSET0_CYCLE_E3		((SUB_SLOT_CYCLE_E3 * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ_E3)	/* unit:ns */
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3N)
/* define used for M3N */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_M3N		(0x84U)	/* 132 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_M3N		(0x108U)	/* 264 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_M3N		(SUB_SLOT_CYCLE_M3N -1U)
#define QOSWT_WTSET0_CYCLE_M3N		((SUB_SLOT_CYCLE_M3N * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_H3)
/* define used for H3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_H3_20		(0x84U)	/* 132 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_H3_20		(0x108U)	/* 264 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_H3_20		(SUB_SLOT_CYCLE_H3_20 -1U)
#define QOSWT_WTSET0_CYCLE_H3_20	((SUB_SLOT_CYCLE_H3_20 * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */

/* define used for H3 Cut 30 */
#define SUB_SLOT_CYCLE_H3_30		(SUB_SLOT_CYCLE_H3_20)	/* same as H3 Cut 20 */
#define SL_INIT_SSLOTCLK_H3_30		(SUB_SLOT_CYCLE_H3_30 -1U)
#define QOSWT_WTSET0_CYCLE_H3_30	((SUB_SLOT_CYCLE_H3_30 * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */

#endif

#if (RCAR_LSI == RCAR_H3N)
/* define used for H3N */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_H3N		(0x84U)	/* 132 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_H3N		(0x108U)	/* 264 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_H3N		(SUB_SLOT_CYCLE_H3N -1U)
#define QOSWT_WTSET0_CYCLE_H3N	((SUB_SLOT_CYCLE_H3N * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */

#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3)
/* define used for M3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_M3_11		(0x84U)	/* 132 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_M3_11		(0x108U)	/* 264 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_M3_11		(SUB_SLOT_CYCLE_M3_11 -1U)
#define QOSWT_WTSET0_CYCLE_M3_11	((SUB_SLOT_CYCLE_M3_11 * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */
#endif

#define OPERATING_FREQ			(400U)	/* MHz */
#define BASE_SUB_SLOT_NUM		(0x6U)
#define SUB_SLOT_CYCLE			(0x7EU)	/* 126 */

#define QOSWT_WTSET0_CYCLE		((SUB_SLOT_CYCLE * BASE_SUB_SLOT_NUM * 1000U)/OPERATING_FREQ)	/* unit:ns */

#define SL_INIT_REFFSSLOT		(0x3U << 24U)
#define SL_INIT_SLOTSSLOT		((BASE_SUB_SLOT_NUM - 1U) << 16U)
#define SL_INIT_SSLOTCLK		(SUB_SLOT_CYCLE -1U)

static inline void io_write_32(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
}

static inline uint32_t io_read_32(uintptr_t addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void io_write_64(uintptr_t addr, uint64_t value)
{
	*(volatile uint64_t *)addr = value;
}

typedef struct {
	uintptr_t addr;
	uint64_t value;
} mstat_slot_t;

extern uint32_t qos_init_ddr_ch;
extern uint8_t qos_init_ddr_phyvalid;

#endif /* QOS_COMMON_H */
