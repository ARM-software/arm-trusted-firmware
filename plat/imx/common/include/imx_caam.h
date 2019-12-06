/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_CAAM_H
#define IMX_CAAM_H

#include <cdefs.h>
#include <stdint.h>
#include <arch.h>
#include <imx_regs.h>

struct caam_job_ring {
	uint32_t jrmidr_ms;
	uint32_t jrmidr_ls;
};

struct caam_rtic_mid {
	uint32_t rticmidr_ms;
	uint32_t rticmidr_ls;
};

struct caam_deco {
	uint32_t		deco_mid_ms;
	uint32_t		deco_mid_ls;
};

#define JOB_RING_OFFSET 0x10
#define DEBUGCTL_OFFSET	0x58
#define RES2_SIZE (DEBUGCTL_OFFSET - JOB_RING_OFFSET - \
	(sizeof(struct caam_job_ring) * CAAM_NUM_JOB_RINGS))

#define RTIC_MID_OFFSET 0x60
#define DECORR_OFFSET	0x9C
#define RES3_SIZE (DECORR_OFFSET - RTIC_MID_OFFSET - \
	(sizeof(struct caam_rtic_mid) * CAAM_NUM_RTIC))

#define DECO_MID_OFFSET	0xA0
#define DAR_OFFSET	0x120
#define RES4_SIZE (DAR_OFFSET - DECO_MID_OFFSET - \
	(sizeof(struct caam_deco) * CAAM_NUM_DECO))

struct caam_ctrl {
	uint32_t		res0;
	uint32_t		mcfgr;
	uint32_t		res1;
	uint32_t		scfgr;
	struct caam_job_ring	jr[CAAM_NUM_JOB_RINGS];
	uint8_t			res2[RES2_SIZE];
	uint32_t		debuctl;
	uint32_t		jrstartr;
	struct caam_rtic_mid	mid[CAAM_NUM_RTIC];
	uint8_t			res3[RES3_SIZE];
	uint32_t		decorr;
	struct caam_deco	deco[CAAM_NUM_DECO];
	uint8_t			res4[RES4_SIZE];
	uint32_t		dar;
	uint32_t		drr;
} __packed;

/* Job ring control bits */
#define JROWN_NS		BIT(3)
#define JROWN_MID		0x01

/* Declare CAAM API */
void imx_caam_init(void);

#endif /* IMX_CAAM_H */
