/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2018-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <def.h>
#include <plat_common.h>
#include <plat_ipi.h>
#include <plat_private.h>

uint32_t platform_id, platform_version;

/*
 * Table of regions to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * configure_mmu_elx() will give the available subset of that,
 */
const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(DEVICE0_BASE, DEVICE0_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE1_BASE, DEVICE1_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(DEVICE2_BASE, DEVICE2_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(CRF_BASE, CRF_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(IPI_BASE, IPI_SIZE, MT_DEVICE | MT_RW | MT_SECURE),
#if TRANSFER_LIST
	MAP_REGION_FLAT(FW_HANDOFF_BASE, FW_HANDOFF_BASE + FW_HANDOFF_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
#endif
	{ 0 }
};

const mmap_region_t *plat_get_mmap(void)
{
	return plat_mmap;
}

/* For saving cpu clock for certain platform */
uint32_t cpu_clock;

const char *board_name_decode(void)
{
	const char *platform;

	switch (platform_id) {
	case SPP:
		platform = "IPP";
		break;
	case EMU:
		platform = "EMU";
		break;
	case SILICON:
		platform = "Silicon";
		break;
	case QEMU:
		platform = "QEMU";
		break;
	default:
		platform = "Unknown";
	}

	return platform;
}

void board_detection(void)
{
	uint32_t version_type;

	version_type = mmio_read_32(PMC_TAP_VERSION);
	platform_id = FIELD_GET(PLATFORM_MASK, version_type);
	platform_version = FIELD_GET(PLATFORM_VERSION_MASK, version_type);

	if (platform_id == QEMU_COSIM) {
		platform_id = QEMU;
	}

	/* Make sure that console is setup to see this message */
	VERBOSE("Platform id: %d version: %d.%d\n", platform_id,
		platform_version / 10U, platform_version % 10U);
}

uint32_t get_uart_clk(void)
{
	uint32_t uart_clock = 0;

	switch (platform_id) {
	case SPP:
	case SPP_MMD:
		uart_clock = cpu_clock;
		break;
	case EMU:
	case EMU_MMD:
		uart_clock = 25000000;
		break;
	case QEMU:
		/* Random values now */
		uart_clock = 25000000;
		break;
	case SILICON:
		uart_clock = 100000000;
		break;
	default:
		panic();
	}

	return uart_clock;
}

void config_setup(void)
{
	uint32_t val;
	uintptr_t crl_base, iou_scntrs_base, psx_base;

	crl_base = CRL;
	iou_scntrs_base = IOU_SCNTRS;
	psx_base = PSX_CRF;

	/* Reset for system timestamp generator in FPX */
	mmio_write_32(psx_base + PSX_CRF_RST_TIMESTAMP_OFFSET, 0);

	/* Global timer init - Program time stamp reference clk */
	val = mmio_read_32(crl_base + CRL_TIMESTAMP_REF_CTRL_OFFSET);
	val |= CRL_APB_TIMESTAMP_REF_CTRL_CLKACT_BIT;
	mmio_write_32(crl_base + CRL_TIMESTAMP_REF_CTRL_OFFSET, val);

	/* Clear reset of timestamp reg */
	mmio_write_32(crl_base + CRL_RST_TIMESTAMP_OFFSET, 0);

	/* Program freq register in System counter and enable system counter. */
	mmio_write_32(iou_scntrs_base + IOU_SCNTRS_BASE_FREQ_OFFSET,
		      cpu_clock);
	mmio_write_32(iou_scntrs_base + IOU_SCNTRS_COUNTER_CONTROL_REG_OFFSET,
		      IOU_SCNTRS_CONTROL_EN);

	generic_delay_timer_init();

	/* Configure IPI data */
	soc_ipi_config_table_init();
}

uint32_t plat_get_syscnt_freq2(void)
{
	return cpu_clock;
}
