/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GIC_V3_H__
#define __GIC_V3_H__

/******************************************************************************
 * THIS DRIVER IS DEPRECATED. For GICv2 systems, use the driver in gicv2.h
 * and for GICv3 systems, use the driver in gicv3.h.
 *****************************************************************************/
#if ERROR_DEPRECATED
#error " The legacy ARM GIC driver is deprecated."
#endif

#include <mmio.h>
#include <stdint.h>
#include <types.h>


/* GICv3 Re-distributor interface registers & shifts */
#define GICR_PCPUBASE_SHIFT	0x11
#define GICR_TYPER		0x08
#define GICR_WAKER		0x14

/* GICR_WAKER bit definitions */
#define WAKER_CA		(U(1) << 2)
#define WAKER_PS		(U(1) << 1)

/* GICR_TYPER bit definitions */
#define GICR_TYPER_AFF_SHIFT	32
#define GICR_TYPER_AFF_MASK	0xffffffff
#define GICR_TYPER_LAST		(U(1) << 4)

/* GICv3 ICC_SRE register bit definitions*/
#define ICC_SRE_EN		(U(1) << 3)
#define ICC_SRE_SRE		(U(1) << 0)

/*******************************************************************************
 * GICv3 defintions
 ******************************************************************************/
#define GICV3_AFFLVL_MASK	0xff
#define GICV3_AFF0_SHIFT	0
#define GICV3_AFF1_SHIFT	8
#define GICV3_AFF2_SHIFT	16
#define GICV3_AFF3_SHIFT	24
#define GICV3_AFFINITY_MASK	0xffffffff

/*******************************************************************************
 * Function prototypes
 ******************************************************************************/
uintptr_t gicv3_get_rdist(uintptr_t gicr_base, u_register_t mpidr);

/*******************************************************************************
 * GIC Redistributor interface accessors
 ******************************************************************************/
static inline uint32_t gicr_read_waker(uintptr_t base)
{
	return mmio_read_32(base + GICR_WAKER);
}

static inline void gicr_write_waker(uintptr_t base, uint32_t val)
{
	mmio_write_32(base + GICR_WAKER, val);
}

static inline uint64_t gicr_read_typer(uintptr_t base)
{
	return mmio_read_64(base + GICR_TYPER);
}


#endif /* __GIC_V3_H__ */
