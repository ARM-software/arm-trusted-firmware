/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#include <arm_def.h>
#include <plat_arm.h>

/*
 * Table of memory regions for different BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as arm_setup_page_tables() already
 * takes care of mapping it.
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
#if TRUSTED_BOARD_BOOT
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RO,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
	ARM_MAP_TSP_SEC_MEM,
	{0}
};
#endif
#if IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#if IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif

ARM_CASSERT_MMAP

