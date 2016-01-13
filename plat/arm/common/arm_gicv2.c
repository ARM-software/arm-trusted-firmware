/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <gicv2.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way the GICv2 driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_gic_driver_init
#pragma weak plat_arm_gic_init
#pragma weak plat_arm_gic_cpuif_enable
#pragma weak plat_arm_gic_cpuif_disable
#pragma weak plat_arm_gic_pcpu_init

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const unsigned int g0_interrupt_array[] = {
	PLAT_ARM_G1S_IRQS,
	PLAT_ARM_G0_IRQS
};

static const gicv2_driver_data_t arm_gic_data = {
	.gicd_base = PLAT_ARM_GICD_BASE,
	.gicc_base = PLAT_ARM_GICC_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
};

/******************************************************************************
 * ARM common helper to initialize the GICv2 only driver.
 *****************************************************************************/
void plat_arm_gic_driver_init(void)
{
	gicv2_driver_init(&arm_gic_data);
}

void plat_arm_gic_init(void)
{
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to enable the GICv2 CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_enable(void)
{
	gicv2_cpuif_enable();
}

/******************************************************************************
 * ARM common helper to disable the GICv2 CPU interface
 *****************************************************************************/
void plat_arm_gic_cpuif_disable(void)
{
	gicv2_cpuif_disable();
}

/******************************************************************************
 * ARM common helper to initialize the per cpu distributor interface in GICv2
 *****************************************************************************/
void plat_arm_gic_pcpu_init(void)
{
	gicv2_pcpu_distif_init();
}
