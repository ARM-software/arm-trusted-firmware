/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __ARM_GIC_H__
#define __ARM_GIC_H__

#include <common_def.h>
#include <stdint.h>

/*******************************************************************************
 * Function declarations
 ******************************************************************************/
void arm_gic_init(uintptr_t gicc_base,
		  uintptr_t gicd_base,
		  uintptr_t gicr_base,
		  const unsigned int *irq_sec_ptr,
		  unsigned int num_irqs) __warn_deprecated;
void arm_gic_setup(void) __warn_deprecated;
void arm_gic_cpuif_deactivate(void) __warn_deprecated;
void arm_gic_cpuif_setup(void) __warn_deprecated;
void arm_gic_pcpu_distif_setup(void) __warn_deprecated;

uint32_t arm_gic_interrupt_type_to_line(uint32_t type,
				uint32_t security_state) __warn_deprecated;
uint32_t arm_gic_get_pending_interrupt_type(void) __warn_deprecated;
uint32_t arm_gic_get_pending_interrupt_id(void) __warn_deprecated;
uint32_t arm_gic_acknowledge_interrupt(void) __warn_deprecated;
void arm_gic_end_of_interrupt(uint32_t id) __warn_deprecated;
uint32_t arm_gic_get_interrupt_type(uint32_t id) __warn_deprecated;

#endif /* __GIC_H__ */
