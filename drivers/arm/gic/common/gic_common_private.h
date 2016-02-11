/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef GIC_COMMON_PRIVATE_H_
#define GIC_COMMON_PRIVATE_H_

#include <gic_common.h>
#include <mmio.h>
#include <stdint.h>

/*******************************************************************************
 * GIC Distributor interface register accessors that are common to GICv3 & GICv2
 ******************************************************************************/
static inline unsigned int gicd_read_ctlr(uintptr_t base)
{
	return mmio_read_32(base + GICD_CTLR);
}

static inline unsigned int gicd_read_typer(uintptr_t base)
{
	return mmio_read_32(base + GICD_TYPER);
}

static inline unsigned int gicd_read_iidr(uintptr_t base)
{
	return mmio_read_32(base + GICD_IIDR);
}

static inline void gicd_write_ctlr(uintptr_t base, unsigned int val)
{
	mmio_write_32(base + GICD_CTLR, val);
}

/*******************************************************************************
 * GIC Distributor function prototypes for accessing entire registers.
 * Note: The raw register values correspond to multiple interrupt IDs and
 * the number of interrupt IDs involved depends on the register accessed.
 ******************************************************************************/
unsigned int gicd_read_igroupr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_icenabler(uintptr_t base, unsigned int id);
unsigned int gicd_read_ispendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icpendr(uintptr_t base, unsigned int id);
unsigned int gicd_read_isactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_icactiver(uintptr_t base, unsigned int id);
unsigned int gicd_read_ipriorityr(uintptr_t base, unsigned int id);
unsigned int gicd_read_icfgr(uintptr_t base, unsigned int id);
unsigned int gicd_read_nsacr(uintptr_t base, unsigned int id);
void gicd_write_igroupr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icenabler(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ispendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icpendr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_isactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icactiver(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_icfgr(uintptr_t base, unsigned int id, unsigned int val);
void gicd_write_nsacr(uintptr_t base, unsigned int id, unsigned int val);

/*******************************************************************************
 * GIC Distributor function prototypes for accessing the GIC registers
 * corresponding to a single interrupt ID. These functions use bitwise
 * operations or appropriate register accesses to modify or return
 * the bit-field corresponding the single interrupt ID.
 ******************************************************************************/
unsigned int gicd_get_igroupr(uintptr_t base, unsigned int id);
void gicd_set_igroupr(uintptr_t base, unsigned int id);
void gicd_clr_igroupr(uintptr_t base, unsigned int id);
void gicd_set_isenabler(uintptr_t base, unsigned int id);
void gicd_set_icenabler(uintptr_t base, unsigned int id);
void gicd_set_ispendr(uintptr_t base, unsigned int id);
void gicd_set_icpendr(uintptr_t base, unsigned int id);
void gicd_set_isactiver(uintptr_t base, unsigned int id);
void gicd_set_icactiver(uintptr_t base, unsigned int id);
void gicd_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri);

#endif /* GIC_COMMON_PRIVATE_H_ */
