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

#include <arch.h>
#include <cci.h>
#include <plat_arm.h>
#include <platform_def.h>
#include <utils.h>

static const int cci_map[] = {
	PLAT_ARM_CCI_CLUSTER0_SL_IFACE_IX,
	PLAT_ARM_CCI_CLUSTER1_SL_IFACE_IX
};

/******************************************************************************
 * The following functions are defined as weak to allow a platform to override
 * the way ARM CCI driver is initialised and used.
 *****************************************************************************/
#pragma weak plat_arm_interconnect_init
#pragma weak plat_arm_interconnect_enter_coherency
#pragma weak plat_arm_interconnect_exit_coherency


/******************************************************************************
 * Helper function to initialize ARM CCI driver.
 *****************************************************************************/
void plat_arm_interconnect_init(void)
{
	cci_init(PLAT_ARM_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_arm_interconnect_enter_coherency(void)
{
	cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_arm_interconnect_exit_coherency(void)
{
	cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
}
