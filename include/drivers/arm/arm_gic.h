/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __ARM_GIC_H__
#define __ARM_GIC_H__

#include <stdint.h>

/*******************************************************************************
 * Function declarations
 ******************************************************************************/
void arm_gic_init(uintptr_t gicc_base,
		  uintptr_t gicd_base,
		  uintptr_t gicr_base,
		  const unsigned int *irq_sec_ptr,
		  unsigned int num_irqs) __deprecated;
void arm_gic_setup(void) __deprecated;
void arm_gic_cpuif_deactivate(void) __deprecated;
void arm_gic_cpuif_setup(void) __deprecated;
void arm_gic_pcpu_distif_setup(void) __deprecated;

uint32_t arm_gic_interrupt_type_to_line(uint32_t type,
				uint32_t security_state) __deprecated;
uint32_t arm_gic_get_pending_interrupt_type(void) __deprecated;
uint32_t arm_gic_get_pending_interrupt_id(void) __deprecated;
uint32_t arm_gic_acknowledge_interrupt(void) __deprecated;
void arm_gic_end_of_interrupt(uint32_t id) __deprecated;
uint32_t arm_gic_get_interrupt_type(uint32_t id) __deprecated;

#endif /* __GIC_H__ */
