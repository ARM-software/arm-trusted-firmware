/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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


/* GICv3 Re-distributor interface registers & shifts */
#define GICR_PCPUBASE_SHIFT	0x11
#define GICR_TYPER		0x08
#define GICR_WAKER		0x14

/* GICR_WAKER bit definitions */
#define WAKER_CA		(1UL << 2)
#define WAKER_PS		(1UL << 1)

/* GICR_TYPER bit definitions */
#define GICR_TYPER_AFF_SHIFT	32
#define GICR_TYPER_AFF_MASK	0xffffffff
#define GICR_TYPER_LAST		(1UL << 4)

/* GICv3 ICC_SRE register bit definitions*/
#define ICC_SRE_EN		(1UL << 3)
#define ICC_SRE_SRE		(1UL << 0)

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
uintptr_t gicv3_get_rdist(uintptr_t gicr_base, uint64_t mpidr);

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
