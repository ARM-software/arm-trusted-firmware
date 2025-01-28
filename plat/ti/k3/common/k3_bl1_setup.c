/*
 * Copyright (c) 2015-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <k3_console.h>
#include <k3-ddrss.h>
#include <mailbox.h>
#include <ti_sci_protocol.h>

#define ADDR_DOWN(_adr) (_adr & XLAT_ADDR_MASK(2U))
#define SIZE_UP(_adr, _sz) (round_up((_adr + _sz), XLAT_BLOCK_SIZE(2U)) - ADDR_DOWN(_adr))

#define K3_MAP_REGION_FLAT(_adr, _sz, _attr) \
	MAP_REGION_FLAT(ADDR_DOWN(_adr), SIZE_UP(_adr, _sz), _attr)

const mmap_region_t plat_k3_mmap[] = {
	K3_MAP_REGION_FLAT(0,       0x100000000,       MT_DEVICE | MT_RW | MT_SECURE),
	{ /* sentinel */ }
};

#define PADCONF_ADDR 0x4084000
#define WKUP_BOOT_MODE 0x43010030
#define MAIN_PLL_MMR_BASE			(0x04060000UL)
#define MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0	(0x00008080UL)
#define CTL_MMR_BASE_CFG5			(0x43050000U)
#define CANUART_WAKE_OFF_MODE_STA		(0x1318U)
#define RTC_ONLY_PLUS_DDR_MAGIC_WORD 		(0x6D555555U)
#define BL1_DONE_MSG_ID	0x810A

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return NULL;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	volatile uint32_t* uart_ptr;

	/* Set UART pins */
	uart_ptr = (uint32_t*) (PADCONF_ADDR+0x1b4);
	*uart_ptr = 0x50000;
	uart_ptr = (uint32_t*) (PADCONF_ADDR+0x1b8);
	*uart_ptr = 0x10000;
	/* Set WKUP UART pins */
	uart_ptr = (uint32_t*) (PADCONF_ADDR);
	*uart_ptr = 0x50000;
	uart_ptr = (uint32_t*) (PADCONF_ADDR+0x4);
	*uart_ptr = 0x10000;

	/* Initialize the console to provide early debug support */
	k3_console_setup();

	/* PLL8_HSDIV0 feeds A53, bump it up to 1.25GHz, VCO @2.5GHz */
	mmio_write_32(MAIN_PLL_MMR_BASE + MAIN_PLL_MMR_CFG_PLL8_HSDIV_CTRL0, 0x8001);
}

/******************************************************************************
 * Perform the very early platform specific architecture setup.  This only
 * does basic initialization. Later architectural setup (bl1_arch_setup())
 * does not do anything platform specific.
 *****************************************************************************/
void bl1_plat_arch_setup(void)
{
	int	ro_txt_size;
	int	ro_data_size;
	int 	rw_size;

	NOTICE("%s arch setup \n", __func__);
	ro_txt_size = (intptr_t) __RODATA_START__ - BL1_RO_BASE;
	ro_data_size = BL1_RW_BASE - (intptr_t) __RODATA_START__;
	rw_size = BL1_RW_LIMIT+1 - BL1_RW_BASE;
	const mmap_region_t bl_regions[] = {
		MAP_REGION_FLAT(BL1_RO_BASE,			ro_txt_size,		MT_CODE | MT_SECURE),
		MAP_REGION_FLAT(BL1_RO_BASE+ro_txt_size,	ro_data_size,		MT_MEMORY | MT_RW | MT_SECURE),
		MAP_REGION_FLAT(BL1_RW_BASE,           		rw_size,		MT_MEMORY | MT_RW | MT_SECURE),
		{ /* sentinel */ }
	};

	setup_page_tables(bl_regions, plat_k3_mmap);
	enable_mmu_el3(0);
}

struct
{
	uint16_t cmdid;         //!< The command ID
	uint8_t  hostid;        //!< Identifies the queue number used for replies to a received message
	uint8_t  seqnum;        //!< A sequence number used to detect lost messages
	uint32_t sizeandflags;  //!< If size is present, three bytes of flags, last byte of size
	uint32_t magicnum;              //!< Magic number to check this is indeed multistage context and should be applied
	uint32_t rsvd;              //!< Magic number to check this is indeed multistage context and should be applied
	union {
		uint32_t imageoffset[4];    //!< image offset for block/image mode
		char  filename[32];      //!< Filesystem file name for filesystem mode, max 32 chars
	} imagelocator;
}__packed a53_rom_msg_obj;

struct{
	struct ti_sci_secure_msg_hdr secure_hdr;
}__packed a53_tifs_msg_obj;

void k3_bl1_handoff(void)
{
	struct ti_sci_msg msg;

	a53_rom_msg_obj.cmdid = BL1_DONE_MSG_ID;
	a53_rom_msg_obj.hostid = 0;
	a53_rom_msg_obj.seqnum = 0;
	a53_rom_msg_obj.sizeandflags = 0x0c000000;

	a53_rom_msg_obj.magicnum = 0x11112222;
	a53_rom_msg_obj.rsvd = 0x0;
	a53_rom_msg_obj.imagelocator.imageoffset[0] = 0x0;
	a53_rom_msg_obj.imagelocator.imageoffset[1] = 0x0;
	a53_rom_msg_obj.imagelocator.imageoffset[2] = 0x0;
	a53_rom_msg_obj.imagelocator.imageoffset[3] = 0x0;

	if ((mmio_read_32(WKUP_BOOT_MODE) & 0xf8) == 0x40) {
		/* SD boot mode */
		memset(a53_rom_msg_obj.imagelocator.filename, 0, sizeof(a53_rom_msg_obj.imagelocator.filename));
		snprintf(a53_rom_msg_obj.imagelocator.filename, sizeof(a53_rom_msg_obj.imagelocator.filename), "%s%s", "\\","tispl.bin");
	} else {
		a53_rom_msg_obj.imagelocator.imageoffset[0] = K3_SPL_IMG_OFFSET;
	}

	msg.buf = (uint8_t*) &a53_rom_msg_obj;
	msg.len = sizeof(a53_rom_msg_obj);
	ti_sci_transport_send(0, &msg);
	NOTICE("%s ENTERING WFI - end of bl1 \n", __func__);
	asm ("wfi");
}

void bl1_platform_setup(void)
{
	if (k3_lpddr4_init() != 0U)
		NOTICE("%s DDR init failed \n", __func__);
	else {
		NOTICE("%s DDR init done\n", __func__);
		k3_bl1_handoff();
	}
}

struct image_desc * bl1_plat_get_image_desc(unsigned int image_id)
{
    return NULL;
}

void platform_mem_init(void)
{
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
                          uintptr_t *image_spec)
{
    return 0;
}
