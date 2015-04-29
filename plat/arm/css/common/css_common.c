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

#include <arm_gic.h>
#include <bl_common.h>
#include <platform_def.h>


#if IMAGE_BL31 || IMAGE_BL32

const unsigned int irq_sec_array[] = {
	PLAT_CSS_IRQ_SEC_LIST,
	ARM_IRQ_SEC_PHY_TIMER,
	ARM_IRQ_SEC_SGI_0,
	ARM_IRQ_SEC_SGI_1,
	ARM_IRQ_SEC_SGI_2,
	ARM_IRQ_SEC_SGI_3,
	ARM_IRQ_SEC_SGI_4,
	ARM_IRQ_SEC_SGI_5,
	ARM_IRQ_SEC_SGI_6,
	ARM_IRQ_SEC_SGI_7
};


/* Weak definitions may be overridden in specific CSS based platform */
#pragma weak plat_arm_gic_init

void plat_arm_gic_init(void)
{
	arm_gic_init(PLAT_CSS_GICC_BASE,
		PLAT_CSS_GICD_BASE,
		PLAT_CSS_GICR_BASE,
		irq_sec_array,
		ARRAY_SIZE(irq_sec_array));
}

#endif /* IMAGE_BL31 || IMAGE_BL32 */
