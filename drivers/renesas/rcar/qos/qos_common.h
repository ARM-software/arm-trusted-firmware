/*
 * Copyright (c) 2017-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QOS_COMMON_H
#define QOS_COMMON_H

#define RCAR_REF_DEFAULT		0U

/* define used for get_refperiod. */
/* REFPERIOD_CYCLE need smaller than QOSWT_WTSET0_CYCLEs */
/* refere to plat/renesas/rcar/ddr/ddr_a/ddr_init_e3.h for E3. */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF default */
#define REFPERIOD_CYCLE		/* unit:ns */	\
	((126 * BASE_SUB_SLOT_NUM * 1000U) / 400)
#else					/* REF option */
#define REFPERIOD_CYCLE		/* unit:ns */	\
	((252 * BASE_SUB_SLOT_NUM * 1000U) / 400)
#endif

#if (RCAR_LSI == RCAR_E3)
/* define used for E3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 3.9usec */
#define SUB_SLOT_CYCLE_E3		0xAFU	/* 175 */
#else /* REF 7.8usec */
#define SUB_SLOT_CYCLE_E3		0x15EU	/* 350 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define OPERATING_FREQ_E3		266U	/* MHz */
#define SL_INIT_SSLOTCLK_E3		(SUB_SLOT_CYCLE_E3 - 1U)
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3N)
/* define used for M3N */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_M3N		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_M3N		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_M3N		(SUB_SLOT_CYCLE_M3N - 1U)
#define QOSWT_WTSET0_CYCLE_M3N		/* unit:ns */	\
	((SUB_SLOT_CYCLE_M3N * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_H3)
/* define used for H3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_H3_20		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_H3_20		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_H3_20		(SUB_SLOT_CYCLE_H3_20 - 1U)
#define QOSWT_WTSET0_CYCLE_H3_20	/* unit:ns */	\
	((SUB_SLOT_CYCLE_H3_20 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)

/* define used for H3 Cut 30 */
#define SUB_SLOT_CYCLE_H3_30		(SUB_SLOT_CYCLE_H3_20)	/* same as H3 Cut 20 */
#define SL_INIT_SSLOTCLK_H3_30		(SUB_SLOT_CYCLE_H3_30 - 1U)
#define QOSWT_WTSET0_CYCLE_H3_30	/* unit:ns */	\
	((SUB_SLOT_CYCLE_H3_30 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)

#endif

#if (RCAR_LSI == RCAR_H3N)
/* define used for H3N */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_H3N		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_H3N		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_H3N		(SUB_SLOT_CYCLE_H3N - 1U)
#define QOSWT_WTSET0_CYCLE_H3N		/* unit:ns */	\
	((SUB_SLOT_CYCLE_H3N * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)

#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RCAR_M3)
/* define used for M3 */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_M3_11		0x7EU	/* 126 */
#define SUB_SLOT_CYCLE_M3_30		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_M3_11		0xFCU	/* 252 */
#define SUB_SLOT_CYCLE_M3_30		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_M3_11		(SUB_SLOT_CYCLE_M3_11 - 1U)
#define SL_INIT_SSLOTCLK_M3_30		(SUB_SLOT_CYCLE_M3_30 - 1U)
#define QOSWT_WTSET0_CYCLE_M3_11	/* unit:ns */	\
	((SUB_SLOT_CYCLE_M3_11 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#define QOSWT_WTSET0_CYCLE_M3_30	/* unit:ns */	\
	((SUB_SLOT_CYCLE_M3_30 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#endif

#define OPERATING_FREQ			400U	/* MHz */
#define BASE_SUB_SLOT_NUM		0x6U
#define SUB_SLOT_CYCLE			0x7EU	/* 126 */

#define QOSWT_WTSET0_CYCLE		/* unit:ns */	\
	((SUB_SLOT_CYCLE * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)

#define SL_INIT_REFFSSLOT		(0x3U << 24U)
#define SL_INIT_SLOTSSLOT		((BASE_SUB_SLOT_NUM - 1U) << 16U)
#define SL_INIT_SSLOTCLK		(SUB_SLOT_CYCLE - 1U)

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

struct rcar_gen3_dbsc_qos_settings {
	uint32_t	reg;
	uint32_t	val;
};

extern uint32_t qos_init_ddr_ch;
extern uint8_t qos_init_ddr_phyvalid;

void rcar_qos_dbsc_setting(struct rcar_gen3_dbsc_qos_settings *qos,
			   unsigned int qos_size, bool dbsc_wren);

#endif /* QOS_COMMON_H */
