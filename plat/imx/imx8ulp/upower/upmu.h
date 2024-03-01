/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MU_H
#define MU_H

#include <stdint.h>

typedef volatile unsigned int vuint32_t;

/****************************************************************************/
/*				MODULE: Message Unit			    */
/****************************************************************************/
/* VER Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t FEATURE : 16;
		vuint32_t MINOR : 8;
		vuint32_t MAJOR : 8;
	} B;
} MU_VER_t;

/* PAR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t TR_NUM : 8;
		vuint32_t RR_NUM : 8;
		vuint32_t GIR_NUM : 8;
		vuint32_t FLAG_WIDTH : 8;
	} B;
} MU_PAR_t;

/* CR Register */
typedef union  {
	vuint32_t R;
	struct {
		vuint32_t MUR : 1;
		vuint32_t MURIE : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_CR_t;

/* SR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t MURS : 1;
		vuint32_t MURIP : 1;
		vuint32_t EP : 1;
		vuint32_t FUP : 1;
		vuint32_t GIRP : 1;
		vuint32_t TEP : 1;
		vuint32_t RFP : 1;
		vuint32_t CEP : 1;
		vuint32_t rsrv_1 : 24;

	} B;
} MU_SR_t;

/* CCR0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t NMI : 1;
		vuint32_t HR  : 1;
		vuint32_t HRM : 1;
		vuint32_t CLKE : 1;
		vuint32_t RSTH : 1;
		vuint32_t BOOT : 2;
		vuint32_t rsrv_1 : 25;

	} B;
} MU_CCR0_t;

/* CIER0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t rsrv_1 : 1;
		vuint32_t HRIE : 1;
		vuint32_t RUNIE : 1;
		vuint32_t RAIE : 1;
		vuint32_t HALTIE : 1;
		vuint32_t WAITIE : 1;
		vuint32_t STOPIE : 1;
		vuint32_t PDIE : 1;
		vuint32_t rsrv_2 : 24;
	} B;
} MU_CIER0_t;

/* CSSR0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t NMIC : 1;
		vuint32_t HRIP : 1;
		vuint32_t RUN  : 1;
		vuint32_t RAIP : 1;
		vuint32_t HALT : 1;
		vuint32_t WAIT : 1;
		vuint32_t STOP : 1;
		vuint32_t PD : 1;
		vuint32_t rsrv_1 : 24;
	} B;
} MU_CSSR0_t;

/* CSR0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t rsrv_1 : 1;
		vuint32_t HRIP : 1;
		vuint32_t RUN : 1;
		vuint32_t RAIP : 1;
		vuint32_t HALT : 1;
		vuint32_t WAIT : 1;
		vuint32_t STOP : 1;
		vuint32_t PD : 1;
		vuint32_t rsrv_2 : 24;
	} B;
} MU_CSR0_t;

/* FCR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t F0 : 1;
		vuint32_t F1 : 1;
		vuint32_t F2 : 1;
		vuint32_t rsrv_1 : 29;
	} B;
} MU_FCR_t;

/* FSR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t F0 : 1;
		vuint32_t F1 : 1;
		vuint32_t F2 : 1;
		vuint32_t rsrv_1 : 29;
	} B;
} MU_FSR_t;

/* GIER Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t GIE0 : 1;
		vuint32_t GIE1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_GIER_t;

/* GCR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t GIR0 : 1;
		vuint32_t GIR1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_GCR_t;

/* GSR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t GIP0 : 1;
		vuint32_t GIP1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_GSR_t;

/* TCR Register */
typedef union{
	vuint32_t R;
	struct {
		vuint32_t TIE0 : 1;
		vuint32_t TIE1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_TCR_t;

/* TSR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t TE0 : 1;
		vuint32_t TE1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_TSR_t;

/* RCR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t RIE0 : 1;
		vuint32_t RIE1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_RCR_t;

/* RSR Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t RF0 : 1;
		vuint32_t RF1 : 1;
		vuint32_t rsrv_1 : 30;
	} B;
} MU_RSR_t;

/* TR0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t TR_DATA : 32;
	} B;
} MU_TR0_t;

/* TR1 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t TR_DATA : 32;
	} B;
} MU_TR1_t;

/* RR0 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t RR_DATA : 32;
	} B;
} MU_RR0_t;

/* RR1 Register */
typedef union {
	vuint32_t R;
	struct {
		vuint32_t RR_DATA : 32;
	} B;
} MU_RR1_t;

struct MU_t {
	MU_VER_t VER;
	MU_PAR_t PAR;
	MU_CR_t CR;
	MU_SR_t SR;
	MU_CCR0_t CCR0;
	MU_CIER0_t CIER0;
	MU_CSSR0_t CSSR0;
	MU_CSR0_t CSR0;
	uint8_t MU_reserved0[224];
	MU_FCR_t FCR;
	MU_FSR_t FSR;
	uint8_t MU_reserved1[8];
	MU_GIER_t GIER;
	MU_GCR_t GCR;
	MU_GSR_t GSR;
	uint8_t MU_reserved2[4];
	MU_TCR_t TCR;
	MU_TSR_t TSR;
	MU_RCR_t RCR;
	MU_RSR_t RSR;
	uint8_t MU_reserved3[208];
	MU_TR0_t TR[2];
	uint8_t MU_reserved4[120];
	MU_RR0_t RR[2];
};

#endif /* MU_H */
