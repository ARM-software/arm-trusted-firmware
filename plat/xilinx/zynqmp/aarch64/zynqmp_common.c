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

#include <arch_helpers.h>
#include <cci.h>
#include <debug.h>
#include <gicv2.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <xlat_tables.h>
#include "../zynqmp_private.h"

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_arm_mmap[] = {
	{ DEVICE0_BASE, DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{ DEVICE1_BASE, DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{ CRF_APB_BASE, CRF_APB_BASE, CRF_APB_SIZE, MT_DEVICE | MT_RW | MT_SECURE },
	{0}
};

static unsigned int zynqmp_get_silicon_ver(void)
{
	unsigned int ver;

	ver = mmio_read_32(ZYNQMP_CSU_BASEADDR + ZYNQMP_CSU_VERSION_OFFSET);
	ver &= ZYNQMP_SILICON_VER_MASK;
	ver >>= ZYNQMP_SILICON_VER_SHIFT;

	return ver;
}

unsigned int zynqmp_get_uart_clk(void)
{
	unsigned int ver = zynqmp_get_silicon_ver();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 48000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 25000000;
	case ZYNQMP_CSU_VERSION_QEMU:
		return 133000000;
	}

	return 100000000;
}

static unsigned int zynqmp_get_system_timer_freq(void)
{
	unsigned int ver = zynqmp_get_silicon_ver();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 10000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 4000000;
	case ZYNQMP_CSU_VERSION_QEMU:
		return 50000000;
	}

	return 100000000;
}

#if LOG_LEVEL >= LOG_LEVEL_NOTICE
static const struct {
	unsigned int id;
	char *name;
} zynqmp_devices[] = {
	{
		.id = 0x10,
		.name = "3EG",
	},
	{
		.id = 0x11,
		.name = "2EG",
	},
	{
		.id = 0x20,
		.name = "5EV",
	},
	{
		.id = 0x21,
		.name = "4EV",
	},
	{
		.id = 0x30,
		.name = "7EV",
	},
	{
		.id = 0x38,
		.name = "9EG",
	},
	{
		.id = 0x39,
		.name = "6EG",
	},
	{
		.id = 0x40,
		.name = "11EG",
	},
	{
		.id = 0x50,
		.name = "15EG",
	},
	{
		.id = 0x58,
		.name = "19EG",
	},
	{
		.id = 0x59,
		.name = "17EG",
	},
};

static unsigned int zynqmp_get_silicon_id(void)
{
	uint32_t id;

	id = mmio_read_32(ZYNQMP_CSU_BASEADDR + ZYNQMP_CSU_IDCODE_OFFSET);

	id &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK | ZYNQMP_CSU_IDCODE_SVD_MASK;
	id >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;

	return id;
}

static char *zynqmp_get_silicon_idcode_name(void)
{
	unsigned int id;

	id = zynqmp_get_silicon_id();
	for (size_t i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if (zynqmp_devices[i].id == id)
			return zynqmp_devices[i].name;
	}
	return "UNKN";
}

static unsigned int zynqmp_get_rtl_ver(void)
{
	uint32_t ver;

	ver = mmio_read_32(ZYNQMP_CSU_BASEADDR + ZYNQMP_CSU_VERSION_OFFSET);
	ver &= ZYNQMP_RTL_VER_MASK;
	ver >>= ZYNQMP_RTL_VER_SHIFT;

	return ver;
}

static char *zynqmp_print_silicon_idcode(void)
{
	uint32_t id, maskid, tmp;

	id = mmio_read_32(ZYNQMP_CSU_BASEADDR + ZYNQMP_CSU_IDCODE_OFFSET);

	tmp = id;
	tmp &= ZYNQMP_CSU_IDCODE_XILINX_ID_MASK |
	       ZYNQMP_CSU_IDCODE_FAMILY_MASK |
	       ZYNQMP_CSU_IDCODE_REVISION_MASK;
	maskid = ZYNQMP_CSU_IDCODE_XILINX_ID << ZYNQMP_CSU_IDCODE_XILINX_ID_SHIFT |
		 ZYNQMP_CSU_IDCODE_FAMILY << ZYNQMP_CSU_IDCODE_FAMILY_SHIFT |
		 ZYNQMP_CSU_IDCODE_REVISION << ZYNQMP_CSU_IDCODE_REVISION_SHIFT;
	if (tmp != maskid) {
		ERROR("Incorrect XILINX IDCODE 0x%x, maskid 0x%x\n", id, maskid);
		return "UNKN";
	}
	VERBOSE("Xilinx IDCODE 0x%x\n", id);
	return zynqmp_get_silicon_idcode_name();
}

static unsigned int zynqmp_get_ps_ver(void)
{
	uint32_t ver = mmio_read_32(ZYNQMP_CSU_BASEADDR + ZYNQMP_CSU_VERSION_OFFSET);

	ver &= ZYNQMP_PS_VER_MASK;
	ver >>= ZYNQMP_PS_VER_SHIFT;

	return ver + 1;
}

static void zynqmp_print_platform_name(void)
{
	unsigned int ver = zynqmp_get_silicon_ver();
	unsigned int rtl = zynqmp_get_rtl_ver();
	char *label = "Unknown";

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		label = "VELOCE";
		break;
	case ZYNQMP_CSU_VERSION_EP108:
		label = "EP108";
		break;
	case ZYNQMP_CSU_VERSION_QEMU:
		label = "QEMU";
		break;
	case ZYNQMP_CSU_VERSION_SILICON:
		label = "silicon";
		break;
	}

	NOTICE("ATF running on XCZU%s/%s v%d/RTL%d.%d at 0x%x%s\n",
	       zynqmp_print_silicon_idcode(), label, zynqmp_get_ps_ver(),
	       (rtl & 0xf0) >> 4, rtl & 0xf, BL31_BASE,
	       zynqmp_is_pmu_up() ? ", with PMU firmware" : "");
}
#else
static inline void zynqmp_print_platform_name(void) { }
#endif

/*
 * Indicator for PMUFW discovery:
 *   0 = No FW found
 *   non-zero = FW is present
 */
static int zynqmp_pmufw_present;

/*
 * zynqmp_discover_pmufw - Discover presence of PMUFW
 *
 * Discover the presence of PMUFW and store it for later run-time queries
 * through zynqmp_is_pmu_up.
 * NOTE: This discovery method is fragile and will break if:
 *  - setting FW_PRESENT is done by PMUFW itself and could be left out in PMUFW
 *    (be it by error or intentionally)
 *  - XPPU/XMPU may restrict ATF's access to the PMU address space
 */
static int zynqmp_discover_pmufw(void)
{
	zynqmp_pmufw_present = mmio_read_32(PMU_GLOBAL_CNTRL);
	zynqmp_pmufw_present &= PMU_GLOBAL_CNTRL_FW_IS_PRESENT;

	return !!zynqmp_pmufw_present;
}

/*
 * zynqmp_is_pmu_up - Find if PMU firmware is up and running
 *
 * Return 0 if firmware is not available, non 0 otherwise
 */
int zynqmp_is_pmu_up(void)
{
	return zynqmp_pmufw_present;
}

/*
 * A single boot loader stack is expected to work on both the Foundation ZYNQMP
 * models and the two flavours of the Base ZYNQMP models (AEMv8 & Cortex). The
 * SYS_ID register provides a mechanism for detecting the differences between
 * these platforms. This information is stored in a per-BL array to allow the
 * code to take the correct path.Per BL platform configuration.
 */
void zynqmp_config_setup(void)
{
	zynqmp_discover_pmufw();
	zynqmp_print_platform_name();

	/* Global timer init - Program time stamp reference clk */
	uint32_t val = mmio_read_32(CRL_APB_TIMESTAMP_REF_CTRL);
	val |= CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	mmio_write_32(CRL_APB_TIMESTAMP_REF_CTRL, val);

	/* Program freq register in System counter and enable system counter. */
	mmio_write_32(IOU_SCNTRS_BASEFREQ, zynqmp_get_system_timer_freq());
	mmio_write_32(IOU_SCNTRS_CONTROL, IOU_SCNTRS_CONTROL_EN);
}

unsigned int plat_get_syscnt_freq2(void)
{
	unsigned int counter_base_frequency;

	/* FIXME: Read the frequency from Frequency modes table */
	counter_base_frequency = zynqmp_get_system_timer_freq();

	return counter_base_frequency;
}
