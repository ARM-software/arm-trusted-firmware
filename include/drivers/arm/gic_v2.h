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

#ifndef __GIC_V2_H__
#define __GIC_V2_H__

#include <mmio.h>

/*******************************************************************************
 * GIC Distributor interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicd_read_ctlr(unsigned int base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(unsigned int base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_sgir(unsigned int base)
{
	return mmio_read_32(base + GICD_SGIR);
}


/*******************************************************************************
 * GIC Distributor interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicd_write_ctlr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

static inline void gicd_write_sgir(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICD_SGIR, val);
}


/*******************************************************************************
 * GIC CPU interface accessors for reading entire registers
 ******************************************************************************/

static inline unsigned int gicc_read_ctlr(unsigned int base)
{
	return mmio_read_32(base + GICC_CTLR);
}

static inline unsigned int gicc_read_pmr(unsigned int base)
{
	return mmio_read_32(base + GICC_PMR);
}

static inline unsigned int gicc_read_BPR(unsigned int base)
{
	return mmio_read_32(base + GICC_BPR);
}

static inline unsigned int gicc_read_IAR(unsigned int base)
{
	return mmio_read_32(base + GICC_IAR);
}

static inline unsigned int gicc_read_EOIR(unsigned int base)
{
	return mmio_read_32(base + GICC_EOIR);
}

static inline unsigned int gicc_read_hppir(unsigned int base)
{
	return mmio_read_32(base + GICC_HPPIR);
}

static inline unsigned int gicc_read_dir(unsigned int base)
{
	return mmio_read_32(base + GICC_DIR);
}

static inline unsigned int gicc_read_iidr(unsigned int base)
{
	return mmio_read_32(base + GICC_IIDR);
}


/*******************************************************************************
 * GIC CPU interface accessors for writing entire registers
 ******************************************************************************/

static inline void gicc_write_ctlr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_CTLR, val);
}

static inline void gicc_write_pmr(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_PMR, val);
}

static inline void gicc_write_BPR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_BPR, val);
}


static inline void gicc_write_IAR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_IAR, val);
}

static inline void gicc_write_EOIR(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_EOIR, val);
}

static inline void gicc_write_hppir(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_HPPIR, val);
}

static inline void gicc_write_dir(unsigned int base, unsigned int val)
{
	mmio_write_32(base + GICC_DIR, val);
}

#endif /* __GIC_V2_H__ */
