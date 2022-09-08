/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (C) 2022, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <plat_ipi.h>

#include <plat_private.h>
#include <versal_net_def.h>

uint32_t platform_id, platform_version;

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_versal_net_mmap[] = {
	MAP_REGION_FLAT(DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE2_BASE, DEVICE2_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRF_BASE, CRF_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(IPI_BASE, IPI_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }
};

const mmap_region_t *plat_versal_net_get_mmap(void)
{
	return plat_versal_net_mmap;
}

/* For saving cpu clock for certain platform */
uint32_t cpu_clock;

char *board_name_decode(void)
{
	switch (platform_id) {
	case VERSAL_NET_SPP:
		return "IPP";
	case VERSAL_NET_EMU:
		return "EMU";
	case VERSAL_NET_SILICON:
		return "Silicon";
	case VERSAL_NET_QEMU:
		return "QEMU";
	default:
		return "Unknown";
	}
}

void board_detection(void)
{
	uint32_t version;

	version = mmio_read_32(PMC_TAP_VERSION);
	platform_id = FIELD_GET(PLATFORM_MASK, version);
	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, version);

	if (platform_id == VERSAL_NET_QEMU_COSIM) {
		platform_id = VERSAL_NET_QEMU;
	}

	if ((platform_id == VERSAL_NET_SPP) ||
	    (platform_id == VERSAL_NET_EMU) ||
	    (platform_id == VERSAL_NET_QEMU)) {
		/*
		 * 9 is diff for
		 * 0 means 0.9 version
		 * 1 means 1.0 version
		 * 2 means 1.1 version
		 * etc,
		 */
		platform_version += 9U;
	}

	/* Make sure that console is setup to see this message */
	VERBOSE("Platform id: %d version: %d.%d\n", platform_id,
	      platform_version / 10U, platform_version % 10U);
}

void versal_net_config_setup(void)
{
	uint32_t val;
	uintptr_t crl_base, iou_scntrs_base, psx_base;

	crl_base = VERSAL_NET_CRL;
	iou_scntrs_base = VERSAL_NET_IOU_SCNTRS;
	psx_base = PSX_CRF;

	/* Reset for system timestamp generator in FPX */
	mmio_write_32(psx_base + PSX_CRF_RST_TIMESTAMP_OFFSET, 0);

	/* Global timer init - Program time stamp reference clk */
	val = mmio_read_32(crl_base + VERSAL_NET_CRL_TIMESTAMP_REF_CTRL_OFFSET);
	val |= VERSAL_NET_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	mmio_write_32(crl_base + VERSAL_NET_CRL_TIMESTAMP_REF_CTRL_OFFSET, val);

	/* Clear reset of timestamp reg */
	mmio_write_32(crl_base + VERSAL_NET_CRL_RST_TIMESTAMP_OFFSET, 0);

	/* Program freq register in System counter and enable system counter. */
	mmio_write_32(iou_scntrs_base + VERSAL_NET_IOU_SCNTRS_BASE_FREQ_OFFSET,
		      cpu_clock);
	mmio_write_32(iou_scntrs_base + VERSAL_NET_IOU_SCNTRS_COUNTER_CONTROL_REG_OFFSET,
		      VERSAL_NET_IOU_SCNTRS_CONTROL_EN);

	generic_delay_timer_init();

#if (TFA_NO_PM == 0)
	/* Configure IPI data for versal_net */
	versal_net_ipi_config_table_init();
#endif
}

uint32_t plat_get_syscnt_freq2(void)
{
	return cpu_clock;
}
