/*
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QOS_COMMON_H
#define QOS_COMMON_H

#define RCAR_REF_DEFAULT		0U

/* define used for get_refperiod. */
/* REFPERIOD_CYCLE need smaller than QOSWT_WTSET0_CYCLEs */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF default */
#define REFPERIOD_CYCLE		/* unit:ns */	\
	((126U * BASE_SUB_SLOT_NUM * 1000U) / 400U)
#else					/* REF option */
#define REFPERIOD_CYCLE		/* unit:ns */	\
	((252U * BASE_SUB_SLOT_NUM * 1000U) / 400U)
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2M)
/* define used for G2M */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_G2M_11		0x7EU	/* 126 */
#define SUB_SLOT_CYCLE_G2M_30		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_G2M_11		0xFCU	/* 252 */
#define SUB_SLOT_CYCLE_G2M_30		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_G2M_11		(SUB_SLOT_CYCLE_G2M_11 - 1U)
#define SL_INIT_SSLOTCLK_G2M_30		(SUB_SLOT_CYCLE_G2M_30 - 1U)
#define QOSWT_WTSET0_CYCLE_G2M_11	/* unit:ns */	\
	((SUB_SLOT_CYCLE_G2M_11 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#define QOSWT_WTSET0_CYCLE_G2M_30	/* unit:ns */	\
	((SUB_SLOT_CYCLE_G2M_30 * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2N)
/* define used for G2N */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_G2N		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_G2N		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_G2N		(SUB_SLOT_CYCLE_G2N - 1U)
#define QOSWT_WTSET0_CYCLE_G2N		/* unit:ns */	\
	((SUB_SLOT_CYCLE_G2N * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#endif /* (RCAR_LSI == RZ_G2N) */

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2H)
/* define used for G2H */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 1.95usec */
#define SUB_SLOT_CYCLE_G2H		0x7EU	/* 126 */
#else /* REF 3.9usec */
#define SUB_SLOT_CYCLE_G2H		0xFCU	/* 252 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define SL_INIT_SSLOTCLK_G2H		(SUB_SLOT_CYCLE_G2H - 1U)
#define QOSWT_WTSET0_CYCLE_G2H		/* unit:ns */	\
	((SUB_SLOT_CYCLE_G2H * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)
#endif

#if (RCAR_LSI == RCAR_AUTO) || (RCAR_LSI == RZ_G2E)
/* define used for G2E */
#if (RCAR_REF_INT == RCAR_REF_DEFAULT)	/* REF 3.9usec */
#define SUB_SLOT_CYCLE_G2E		0xAFU	/* 175 */
#else /* REF 7.8usec */
#define SUB_SLOT_CYCLE_G2E		0x15EU	/* 350 */
#endif /* (RCAR_REF_INT == RCAR_REF_DEFAULT) */

#define OPERATING_FREQ_G2E		266U	/* MHz */
#define SL_INIT_SSLOTCLK_G2E		(SUB_SLOT_CYCLE_G2E - 1U)
#endif

#define OPERATING_FREQ			400U	/* MHz */
#define BASE_SUB_SLOT_NUM		0x6U
#define SUB_SLOT_CYCLE			0x7EU	/* 126 */

#define QOSWT_WTSET0_CYCLE		/* unit:ns */	\
	((SUB_SLOT_CYCLE * BASE_SUB_SLOT_NUM * 1000U) / OPERATING_FREQ)

#define SL_INIT_REFFSSLOT		(0x3U << 24U)
#define SL_INIT_SLOTSSLOT		((BASE_SUB_SLOT_NUM - 1U) << 16U)
#define SL_INIT_SSLOTCLK		(SUB_SLOT_CYCLE - 1U)

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

void rzg_qos_dbsc_setting(const struct rcar_gen3_dbsc_qos_settings *qos,
			  unsigned int qos_size, bool dbsc_wren);

#endif /* QOS_COMMON_H */
