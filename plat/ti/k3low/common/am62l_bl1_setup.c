/*
 * Copyright (C) 2025-2026 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <ti_sci_transport.h>

#include <am62l_ddrss.h>
#include <board_config.h>
#include <cps_drv_lpddr4.h>
#include <k3_console.h>
#include <plat_private.h>
#include <platform_def.h>
#include <plat/common/platform.h>

#define WKUP_BOOT_MODE (0x43010030)
#define MAIN_PLL_MMR_BASE (0x04060000UL)
/*
 * PLL8 is distinct from PLL0 (which drives DDR at 2 GHz VCO).
 * PLL8 drives the A53 cores and has a VCO frequency of 2.5 GHz.
 * HSDIV0 of PLL8 feeds the A53 cluster.
 */
#define MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0 (0x00008080UL)
/* bit 15: enable HSDIV0 clock output */
#define MAIN_PLL8_HSDIV0_CLKOUT_EN GENMASK(15, 15)
/* bits [6:0]: divider value; output = VCO_FREQ / (div + 1) */
#define MAIN_PLL8_HSDIV0_DIV_MASK  GENMASK(6, 0)
/* div=1 → 2.5 GHz / 2 = 1.25 GHz */
#define MAIN_PLL8_HSDIV0_1250MHZ   (MAIN_PLL8_HSDIV0_CLKOUT_EN | U(1))
/* div=2 → 2.5 GHz / 3 = 833.33 MHz */
#define MAIN_PLL8_HSDIV0_833MHZ    (MAIN_PLL8_HSDIV0_CLKOUT_EN | U(2))
#define BL1_DONE_MSG_ID (0x810A)
#define BL1_HANDOFF_MAGIC_NUM (0x11112222)
#define BL1_MSG_SIZE_FLAG (0x0c000000)

#define DEVSTAT_PRIMARY_BOOTMODE_MASK GENMASK(6, 3)
#define DEVSTAT_PRIMARY_BOOTMODE_SHIFT (3)
#define BOOT_DEVICE_MMC (0x08)
#define WKUP_JTAG_DEVICE_ID (WKUP_CTRL_MMR0_BASE + 0x18)
#define JTAG_DEV_SPEED_MASK GENMASK(10, 6)
#define JTAG_DEV_SPEED_SHIFT (6)

#define AM62L3_EFUSE_E_MPU_OPP 5
#define AM62L3_EFUSE_O_MPU_OPP 15

const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(0,	PLAT_PHY_ADDR_SPACE_SIZE,	MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

/**
 * Handoff message for 2nd stage boot
 *
 * @cmdid:	The command ID
 * @hostid:	Identifies the queue number used for replies to a received message
 * @sizeandflags:	Flags in LSB 3 bytes and the MSB byte set to size.
 * @magicnum:	Magic number for payload validation.
 * @rsvd:	Reserved word.
 * @imageoffset:	Image offset for block/image mode.
 * @filename:	File name for filesystem mode.
 */
struct a53_rom_msg {
	uint16_t cmdid;
	uint8_t  hostid;
	uint8_t  seqnum;
	uint32_t sizeandflags;
	uint32_t magicnum;
	uint32_t rsvd;
	union {
		uint32_t imageoffset[4];
		char  filename[32];
	} imagelocator;
} __packed;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	/*
	 * AM62L BL1 never loads BL2: after DDR init it signals the ROM via
	 * the secure transport and enters WFI (see k3_bl1_handoff / bl1_platform_setup).
	 * Returning NULL is intentional — the BL2 load path is never reached.
	 */
	return NULL;
}

void bl1_early_platform_setup(void)
{
	uint32_t device_id;
	uint32_t speed_grade;

#if DEBUG
	ERROR("AM62L k3low BL1: DEBUG builds are not supported on this platform!\n");
	panic();
#endif

	board_init();
	/* Read OPN and set the A53 clock rate */
	device_id = mmio_read_32(WKUP_JTAG_DEVICE_ID);
	speed_grade = (device_id & JTAG_DEV_SPEED_MASK) >>
			JTAG_DEV_SPEED_SHIFT;
	if (speed_grade == AM62L3_EFUSE_O_MPU_OPP) {
		/* PLL8_HSDIV0 feeds A53, set to 1.25 GHz (PLL8 VCO = 2.5 GHz) */
		mmio_write_32(MAIN_PLL_MMR_BASE + MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0,
			      MAIN_PLL8_HSDIV0_1250MHZ);
	} else {
		/* PLL8_HSDIV0 feeds A53, set to 833.33 MHz (PLL8 VCO = 2.5 GHz) */
		mmio_write_32(MAIN_PLL_MMR_BASE + MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0,
			      MAIN_PLL8_HSDIV0_833MHZ);
	}
}

void bl1_plat_arch_setup(void)
{
	INFO("K3 BL1 arch setup\n");

	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL1_RO_BASE,
				BL_RO_DATA_BASE - BL1_RO_BASE,
				MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,
				BL1_RW_BASE - BL_RO_DATA_BASE,
				MT_MEMORY | MT_RO | MT_SECURE),
		MAP_REGION_FLAT(BL1_RW_BASE,
				BL1_RW_LIMIT - BL1_RW_BASE,
				MT_MEMORY | MT_RW | MT_SECURE),
		{ /* sentinel */ }
	};

	setup_page_tables(bl_regions, plat_k3_mmap);
	enable_mmu_el3(0);
}

static void __dead2 k3_bl1_handoff(void)
{
	struct a53_rom_msg a53_rom_msg_obj = { 0 };
	struct ti_sci_msg msg;
	volatile uint32_t devstat;
	uint32_t boot_mode;

	a53_rom_msg_obj.cmdid = BL1_DONE_MSG_ID;
	a53_rom_msg_obj.sizeandflags = BL1_MSG_SIZE_FLAG;
	a53_rom_msg_obj.magicnum = BL1_HANDOFF_MAGIC_NUM;

	devstat = mmio_read_32(WKUP_BOOT_MODE);
	boot_mode = (devstat & DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
			DEVSTAT_PRIMARY_BOOTMODE_SHIFT;

	switch (boot_mode) {
	case BOOT_DEVICE_MMC:
		/*
		 * snprintf writes into a char[] member of a __packed struct.
		 * The pointer is naturally byte-aligned so the access is safe,
		 * but GCC still warns about address-of-packed-member.
		 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
		snprintf(a53_rom_msg_obj.imagelocator.filename,
			 sizeof(a53_rom_msg_obj.imagelocator.filename),
			 "%s%s", "\\", "tispl.bin");
#pragma GCC diagnostic pop
		break;
	default:
		a53_rom_msg_obj.imagelocator.imageoffset[0] = K3_SPL_IMG_OFFSET;
		break;
	}

	msg.buf = (uint8_t *)&a53_rom_msg_obj;
	msg.len = sizeof(a53_rom_msg_obj);
	ti_sci_transport_send(TX_SECURE_TRANSPORT_CHANNEL_ID, &msg);
	NOTICE("ENTERING WFI - end of bl1\n");
	console_flush();
	while (true)
		wfi();
}

void bl1_platform_setup(void)
{
	/*
	 * AM62L uses a non-standard BL1 flow: instead of loading BL2, BL1
	 * initialises DDR, then calls k3_bl1_handoff() which sends a message
	 * to the ROM over the secure transport and enters WFI. This function
	 * therefore never returns; the BL2 load path in the generic BL1 code
	 * is dead for this platform.
	 */
	if (am62l_lpddr4_init() != 0) {
		ERROR("DDR init failed\n");
		panic();
	} else {
		NOTICE("DDR init done\n");
		k3_bl1_handoff();
	}
}

void platform_mem_init(void)
{
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	return -ENOTSUP;
}
