/*
 * Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <am62l-ddrss.h>
#include <k3_console.h>
#include <plat_private.h>
#include <ti_sci_transport.h>
#include <ti_sci_protocol.h>
#include <ti_sci.h>
#include <cps_drv_lpddr4.h>
#include <board_config.h>

#define WKUP_BOOT_MODE (0x43010030)
#define WKUP_BOOT_MODE_XSPI_MODE (0x273)
#define MAIN_PLL_MMR_BASE (0x04060000UL)
#define MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0 (0x00008080UL)
#define BL1_DONE_MSG_ID (0x810A)
#define BL1_HANDOFF_MAGIC_NUM (0x11112222)
#define BL1_MSG_SIZE_FLAG (0x0c000000)
#define WKUP_CANUART_WAKE_OFF_MODE_STAT (0x43051318U)
#define RTC_ONLY_PLUS_DDR_MAGIC_WORD (0x6D555555U)

#define DEVSTAT_PRIMARY_BOOTMODE_MASK GENMASK(6, 3)
#define DEVSTAT_PRIMARY_BOOTMODE_SHIFT (3)
#define BOOT_DEVICE_MMC (0x08)
#define WKUP_JTAG_DEVICE_ID (WKUP_CTRL_MMR0_BASE + 0x18)
#define JTAG_DEV_SPEED_MASK GENMASK(10, 6)
#define JTAG_DEV_SPEED_SHIFT (6)
/* OSPI related registers */
#define FSS_OSPI_CTRL_BASE (0xFC40000)
#define FSS_OSPI_FLASH_CMD_CTRL_REG (0x90)
#define FSS_OSPI_OPCODE_EXT_LOWER_REG (0xE0)

#define AM62L3_EFUSE_E_MPU_OPP 5
#define AM62L3_EFUSE_O_MPU_OPP 15

const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(0,	PLAT_PHY_ADDR_SPACE_SIZE,	MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

/**
 * Handof message for 2nd stage boot
 *
 * @cmdid:	The command ID
 * @hostid:	Identifies the queue number used for replies to a received message
 * @sizeandflags:	Flags in LSB 3 bytes and the MSB byte set to size.
 * @magicnum:	Magic number for payload validation.
 * @rsvd:	Reserved word.
 * @imageoffset:	Image offset for block/image mode.
 * @filename:	File name for filesystem mode.
 */
struct {
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
} __packed a53_rom_msg_obj;

/**
 * @brief Message structure for TIFS minimal context restore
 *
 * Used during RTC + DDR resume to notify TIFS that DDR is active and
 * ready for minimal context restoration. Sent from BL1 to TIFS after
 * DDR has been restored from self-refresh mode.
 *
 * @secure_hdr: TI-SCI secure message header for authentication
 * @req: Context restore request containing DDR address for context data
 */
struct {
	struct tisci_msg_min_context_restore_req req;
} __packed a53_tifs_msg_obj;

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return NULL;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	uint32_t device_id;
	uint32_t speed_grade;

	board_init();
	/* Read OPN and set the A53 clock rate */
	device_id = mmio_read_32(WKUP_JTAG_DEVICE_ID);
	speed_grade = (device_id & JTAG_DEV_SPEED_MASK) >>
			JTAG_DEV_SPEED_SHIFT;
	if (speed_grade == AM62L3_EFUSE_O_MPU_OPP) {
		/* PLL8_HSDIV0 feeds A53, bump it up to 1.25GHz, VCO @2.5GHz */
		mmio_write_32(MAIN_PLL_MMR_BASE + MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0, 0x8001);
	} else {
		/* PLL8_HSDIV0 feeds A53, bump it up to 833.33 MHz, VCO @2.5GHz */
		mmio_write_32(MAIN_PLL_MMR_BASE + MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0, 0x8002);
	}
}

/******************************************************************************
 * Perform the very early platform specific architecture setup.  This only
 * does basic initialization. Later architectural setup (bl1_arch_setup())
 * does not do anything platform specific.
 *****************************************************************************/
void bl1_plat_arch_setup(void)
{
	INFO("K3 BL1 arch setup\n");

	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL1_RO_BASE,	BL_RO_DATA_BASE - BL1_RO_BASE,	MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL_RO_DATA_BASE,	BL1_RW_BASE - BL_RO_DATA_BASE,	MT_MEMORY | MT_RO | MT_SECURE),
		MAP_REGION_FLAT(BL1_RW_BASE,	BL1_RW_LIMIT - BL1_RW_BASE,	MT_MEMORY | MT_RW | MT_SECURE),
		{ /* sentinel */ }
	};

	setup_page_tables(bl_regions, plat_k3_mmap);
	enable_mmu_el3(0);
}

unsigned int plat_get_syscnt_freq2(void)
{
	return 200 * MHZ_TICKS_PER_SEC;
}
static void __dead2 k3_bl1_handoff(void)
{
	struct ti_sci_msg msg;
	volatile uint32_t devstat;
	uint32_t boot_mode;
	bool is_rtc_only_ddr_exit;

	/* Workaround for errata i2462, soft reset the flash */
	if (mmio_read_32(WKUP_BOOT_MODE) == WKUP_BOOT_MODE_XSPI_MODE) {
		/* Send flash reset sequence, 0x66, 0x99 */
		mmio_write_32(FSS_OSPI_CTRL_BASE + FSS_OSPI_OPCODE_EXT_LOWER_REG, 0x66);
		mmio_write_32(FSS_OSPI_CTRL_BASE + FSS_OSPI_FLASH_CMD_CTRL_REG, 0x66000001);
		udelay(200);
		mmio_write_32(FSS_OSPI_CTRL_BASE + FSS_OSPI_OPCODE_EXT_LOWER_REG, 0x99);
		mmio_write_32(FSS_OSPI_CTRL_BASE + FSS_OSPI_FLASH_CMD_CTRL_REG, 0x99000001);
		udelay(200);
	}

	is_rtc_only_ddr_exit =
		((mmio_read_32(WKUP_CANUART_WAKE_OFF_MODE_STAT)) ==
		 RTC_ONLY_PLUS_DDR_MAGIC_WORD);
	if (is_rtc_only_ddr_exit) {

		a53_tifs_msg_obj.req.hdr.host = 0xA;
		a53_tifs_msg_obj.req.hdr.seq = 0x12;
		a53_tifs_msg_obj.req.hdr.type = 0x0308;
		a53_tifs_msg_obj.req.ctx_lo = TIFS_LPM_SAVE_CTX;
		a53_tifs_msg_obj.req.ctx_hi = 0x00000000U;

		ti_sci_boot_notification();

		msg.buf = (uint8_t *)&a53_tifs_msg_obj;
		msg.len = sizeof(a53_tifs_msg_obj);
		ti_sci_transport_send(TX_SECURE_TRANSPORT_CHANNEL_ID, &msg);
		NOTICE("%s sent message to tifs\n", __func__);
		while (true)
			asm ("wfi");
	} else {
		a53_rom_msg_obj.cmdid = BL1_DONE_MSG_ID;
		a53_rom_msg_obj.hostid = 0;
		a53_rom_msg_obj.seqnum = 0;
		a53_rom_msg_obj.sizeandflags = BL1_MSG_SIZE_FLAG;
		a53_rom_msg_obj.magicnum = BL1_HANDOFF_MAGIC_NUM;
		a53_rom_msg_obj.rsvd = 0x0;
		a53_rom_msg_obj.imagelocator.imageoffset[0] = 0x0;
		a53_rom_msg_obj.imagelocator.imageoffset[1] = 0x0;
		a53_rom_msg_obj.imagelocator.imageoffset[2] = 0x0;
		a53_rom_msg_obj.imagelocator.imageoffset[3] = 0x0;

		devstat = mmio_read_32(WKUP_BOOT_MODE);
		boot_mode = (devstat & DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				DEVSTAT_PRIMARY_BOOTMODE_SHIFT;

		switch (boot_mode) {
		case BOOT_DEVICE_MMC:
			memset(a53_rom_msg_obj.imagelocator.filename, 0, sizeof(a53_rom_msg_obj.imagelocator.filename));
			snprintf(a53_rom_msg_obj.imagelocator.filename, sizeof(a53_rom_msg_obj.imagelocator.filename), "%s%s", "\\", "tispl.bin");
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
			asm ("wfi");
	}
}

void bl1_platform_setup(void)
{
	/* Initialize tick timer required for udelays */
	generic_delay_timer_init();

	if (am62l_lpddr4_init() != 0U) {
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

/*
 * This platform does not use the standard image loading mechanism.
 * BL1 initializes DDR and hands off to ROM for loading the next stage.
 * Provide a stub to satisfy the linker.
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	return -1;
}
