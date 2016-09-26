/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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

#include <arm_config.h>
#include <arm_def.h>
#include <ccn.h>
#include <debug.h>
#include <gicv2.h>
#include <mmio.h>
#include <plat_arm.h>
#include <v2m_def.h>
#include "../fvp_def.h"

/* Defines for GIC Driver build time selection */
#define FVP_GICV2		1
#define FVP_GICV3		2
#define FVP_GICV3_LEGACY	3

/*******************************************************************************
 * arm_config holds the characteristics of the differences between the three FVP
 * platforms (Base, A53_A57 & Foundation). It will be populated during cold boot
 * at each boot stage by the primary before enabling the MMU (to allow
 * interconnect configuration) & used thereafter. Each BL will have its own copy
 * to allow independent operation.
 ******************************************************************************/
arm_config_t arm_config;

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)


/*
 * Table of memory regions for various BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 *
 * The flash needs to be mapped as writable in order to erase the FIP's Table of
 * Contents in case of unrecoverable error (see plat_error_handler()).
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
#if TRUSTED_BOARD_BOOT
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	MAP_DEVICE2,
	ARM_MAP_NS_DRAM1,
	ARM_MAP_TSP_SEC_MEM,
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
	{0}
};
#endif
#if IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	MAP_DEVICE0,
	V2M_MAP_IOFPGA,
	{0}
};
#endif
#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};
#endif
#if IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
#ifdef AARCH32
	ARM_MAP_SHARED_RAM,
#endif
	V2M_MAP_IOFPGA,
	MAP_DEVICE0,
	MAP_DEVICE1,
	{0}
};
#endif

ARM_CASSERT_MMAP


/*******************************************************************************
 * A single boot loader stack is expected to work on both the Foundation FVP
 * models and the two flavours of the Base FVP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 ******************************************************************************/
void fvp_config_setup(void)
{
	unsigned int rev, hbi, bld, arch, sys_id;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	rev = (sys_id >> V2M_SYS_ID_REV_SHIFT) & V2M_SYS_ID_REV_MASK;
	hbi = (sys_id >> V2M_SYS_ID_HBI_SHIFT) & V2M_SYS_ID_HBI_MASK;
	bld = (sys_id >> V2M_SYS_ID_BLD_SHIFT) & V2M_SYS_ID_BLD_MASK;
	arch = (sys_id >> V2M_SYS_ID_ARCH_SHIFT) & V2M_SYS_ID_ARCH_MASK;

	if (arch != ARCH_MODEL) {
		ERROR("This firmware is for FVP models\n");
		panic();
	}

	/*
	 * The build field in the SYS_ID tells which variant of the GIC
	 * memory is implemented by the model.
	 */
	switch (bld) {
	case BLD_GIC_VE_MMAP:
		ERROR("Legacy Versatile Express memory map for GIC peripheral"
				" is not supported\n");
		panic();
		break;
	case BLD_GIC_A53A57_MMAP:
		break;
	default:
		ERROR("Unsupported board build %x\n", bld);
		panic();
	}

	/*
	 * The hbi field in the SYS_ID is 0x020 for the Base FVP & 0x010
	 * for the Foundation FVP.
	 */
	switch (hbi) {
	case HBI_FOUNDATION_FVP:
		arm_config.flags = 0;

		/*
		 * Check for supported revisions of Foundation FVP
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_FOUNDATION_FVP_V2_0:
		case REV_FOUNDATION_FVP_V2_1:
		case REV_FOUNDATION_FVP_v9_1:
		case REV_FOUNDATION_FVP_v9_6:
			break;
		default:
			WARN("Unrecognized Foundation FVP revision %x\n", rev);
			break;
		}
		break;
	case HBI_BASE_FVP:
		arm_config.flags |= ARM_CONFIG_BASE_MMAP |
			ARM_CONFIG_HAS_INTERCONNECT | ARM_CONFIG_HAS_TZC;

		/*
		 * Check for supported revisions
		 * Allow future revisions to run but emit warning diagnostic
		 */
		switch (rev) {
		case REV_BASE_FVP_V0:
			break;
		default:
			WARN("Unrecognized Base FVP revision %x\n", rev);
			break;
		}
		break;
	default:
		ERROR("Unsupported board HBI number 0x%x\n", hbi);
		panic();
	}
}


void fvp_interconnect_init(void)
{
	if (arm_config.flags & ARM_CONFIG_HAS_INTERCONNECT) {
#if FVP_INTERCONNECT_DRIVER == FVP_CCN
		if (ccn_get_part0_id(PLAT_ARM_CCN_BASE) != CCN_502_PART0_ID) {
			ERROR("Unrecognized CCN variant detected. Only CCN-502"
					" is supported");
			panic();
		}
#endif
		plat_arm_interconnect_init();
	}
}

void fvp_interconnect_enable(void)
{
	if (arm_config.flags & ARM_CONFIG_HAS_INTERCONNECT)
		plat_arm_interconnect_enter_coherency();
}

void fvp_interconnect_disable(void)
{
	if (arm_config.flags & ARM_CONFIG_HAS_INTERCONNECT)
		plat_arm_interconnect_exit_coherency();
}
