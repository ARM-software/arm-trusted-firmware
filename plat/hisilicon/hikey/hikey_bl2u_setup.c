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

#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <dw_mmc.h>
#include <dw_usb.h>
#include <emmc.h>
#include <errno.h>
#include <fastboot/fastboot.h>
#include <hi6220.h>
#include <mmio.h>
#include <partition/partition.h>
#include <platform.h>
#include <sp804_delay_timer.h>

#include "hikey_def.h"
#include "hikey_private.h"

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2U_RO_BASE (unsigned long)(&__RO_START__)
#define BL2U_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL2U_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2U_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

#define PHY_EYE_PATTERN			0x70533483

/*
 * 28 bytes for l-loader header.
 * 28 bytes for each entry header. There're 5 entries head at most.
 */
#define FIRST_ENTRY_OFFSET			28
#define MAX_ENTRY_NAME_LENGTH			8

typedef struct entry_head {
	char		magic[8];
	char		name[8];
	uint32_t	start;
	uint32_t	count;
	uint32_t	flag;
} loader_entry_t;

typedef enum entry_name {
	ENTRY_LOADER,
	ENTRY_BL1,
	ENTRY_BL2U,
	ENTRY_MAX = 5
} loader_entry_name_t;

static const char entry_name[ENTRY_MAX][MAX_ENTRY_NAME_LENGTH] = {
	"loader", "bl1", "bl2u"
};

void bl2u_early_platform_setup(struct meminfo *mem_layout,
			       void *plat_info)
{
	/* Initialize the console to provide early debug support */
	console_init(CONSOLE_BASE, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	sp804_timer_init(SP804_TIMER0_BASE, 10, 192);
	hikey_ddr_init();
}

void bl2u_plat_arch_setup(void)
{
	hikey_init_mmu_el1(BL2U_RO_LIMIT,
			   BL31_LIMIT - BL2U_RO_LIMIT,
			   BL2U_RO_BASE,
			   BL2U_RO_LIMIT,
			   BL2U_COHERENT_RAM_BASE,
			   BL2U_COHERENT_RAM_LIMIT);
}

static int hikey_check_entry_head(uintptr_t buf, int *num)
{
	char magic[8] = "ENTRYHDR";
	loader_entry_t *entry;
	int i;

	entry = (loader_entry_t *)buf;
	for (i = 0; i < ENTRY_MAX; i++, entry++) {
		if (strncmp(entry->magic, magic, 8) != 0)
			break;
	}
	if (i == 0) {
		/* The first entry isn't valid. */
		return -ENOENT;
	}
	*num = i;
	return 0;
}

static int hikey_flush_loader(void)
{
	loader_entry_t *entry, *first_entry;
	loader_entry_name_t i;
	uintptr_t dev_handle, image_handle, image_spec = 0;
	uintptr_t ddr_addr, entry_addr, emmc_addr, offset;
	size_t length, bytes_written;
	int result, entry_num = 0;

	/* Copy BL1&BL2U from SRAM to DDR. */
	ddr_addr = HIKEY_MMC_DATA_BASE + 0x400000;
	assert(ddr_addr < (HIKEY_MMC_DATA_BASE + HIKEY_MMC_DATA_SIZE));

	memcpy((void *)ddr_addr, (void *)LOADER_RAM_BASE,
	       BL2U_LIMIT - LOADER_RAM_BASE);

	entry_addr = ddr_addr + FIRST_ENTRY_OFFSET;
	result = hikey_check_entry_head(entry_addr, &entry_num);
	assert(result == 0);
	first_entry = (loader_entry_t *)entry_addr;
	entry = first_entry;

	result = plat_get_image_source(NS_BL1U_IMAGE_ID, &dev_handle,
				       &image_spec);
	assert(result == 0);

	result = io_open(dev_handle, image_spec, &image_handle);
	assert(result == 0);

	for (i = ENTRY_LOADER; i < entry_num; i++, entry++) {
		/* check the flag of boot partiton */
		assert(entry->flag == 1);
		emmc_addr = HIKEY_NSBL1U_BASE + (entry->start *	\
						 EMMC_BLOCK_SIZE);
		length = entry->count * EMMC_BLOCK_SIZE;
		/*
		 * ENTRY_LOADER entry only records the informations of followed
		 * binaries.
		 * ENTRY_BL1U & ENTRY_BL2U entries record the location
		 * information in RAM layout.
		 * Only loader & bl1 are flushed into eMMC RPMB. Drop bl2u
		 * since it could be downloaded into SRAM by recovery tool.
		 */
		if (i == ENTRY_LOADER) {
			offset = 0;
		} else if (i == ENTRY_BL1) {
			offset = (entry->start - first_entry->start) *
				 EMMC_BLOCK_SIZE;
		} else {
			break;
		}

		result = io_seek(image_handle, IO_SEEK_SET, emmc_addr);
		assert(result == 0);

		if (i == ENTRY_LOADER)
			continue;
		/*
		 * Note: At here, ddr_addr + offset is always aligned
		 * with EMMC_BLOCK_SIZE.
		 * Since ddr_addr is in the buffer range of emmc driver.
		 * If ddr_addr keeps align with EMMC_BLOCK_SIZE, we could
		 * avoid memory copy in block driver.
		 * If ddr_addr isn't align with EMMC_BLOCK_SIZE, block
		 * driver will try to copy it into buffer of emmc driver.
		 * And data may be corrupted since these two ranges are
		 * overlapped.
		 */
		result = io_write(image_handle, ddr_addr + offset, length,
				  &bytes_written);
		assert(result == 0);

		result = io_seek(image_handle, IO_SEEK_SET, emmc_addr);
		assert(result == 0);

		result = io_read(image_handle, ddr_addr + 0x08000000,
				 length, &bytes_written);
		assert(result == 0);
	}
	io_close(image_handle);
	return result;
}

static void hikey_usb_phy_init(void)
{
	unsigned int data;

	/* enable USB clock */
	mmio_write_32(PERI_SC_PERIPH_CLKEN0, PERI_CLK0_USBOTG);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_CLKSTAT0);
	} while ((data & PERI_CLK0_USBOTG) == 0);

	/* out of reset */
	mmio_write_32(PERI_SC_PERIPH_RSTDIS0,
		      PERI_RST0_USBOTG_BUS | PERI_RST0_POR_PICOPHY |
		      PERI_RST0_USBOTG | PERI_RST0_USBOTG_32K);
	do {
		data = mmio_read_32(PERI_SC_PERIPH_RSTSTAT0);
		data &= PERI_RST0_USBOTG_BUS | PERI_RST0_POR_PICOPHY |
			PERI_RST0_USBOTG | PERI_RST0_USBOTG_32K;
	} while (data);

	mmio_write_32(PERI_SC_PERIPH_CTRL8, PHY_EYE_PATTERN);

	/* configure USB PHY */
	data = mmio_read_32(PERI_SC_PERIPH_CTRL4);
	/* make PHY out of low power mode */
	data &= ~PERI_CTRL4_PICO_SIDDQ;
	/* detect VBUS by external circuit, switch D+ to 1.5KOhm pullup */
	data |= PERI_CTRL4_PICO_VBUSVLDEXTSEL | PERI_CTRL4_PICO_VBUSVLDEXT;
	data &= ~PERI_CTRL4_FPGA_EXT_PHY_SEL;
	/* select PHY */
	data &= ~PERI_CTRL4_OTG_PHY_SEL;
	mmio_write_32(PERI_SC_PERIPH_CTRL4, data);

	udelay(1000);

	data = mmio_read_32(PERI_SC_PERIPH_CTRL5);
	data &= ~PERI_CTRL5_PICOPHY_BC_MODE;
	mmio_write_32(PERI_SC_PERIPH_CTRL5, data);

	udelay(20000);
}

void bl2u_platform_setup(void)
{
	dw_mmc_params_t emmc_params;
	dw_usb_params_t usb_params;
	fastboot_params_t fb_params;
	char response[16];

	memset(&emmc_params, 0, sizeof(dw_mmc_params_t));
	emmc_params.reg_base = DWMMC0_BASE;
	emmc_params.desc_base = HIKEY_MMC_DESC_BASE;
	emmc_params.desc_size = HIKEY_MMC_DESC_SIZE;
	emmc_params.clk_rate = 24 * 1000 * 1000;
	emmc_params.bus_width = EMMC_BUS_WIDTH_8;
	dw_mmc_init(&emmc_params);
	hikey_io_setup();
	partition_init(BL2U_IMAGE_ID);

	hikey_flush_loader();

	hikey_usb_phy_init();
	memset(&usb_params, 0, sizeof(dw_usb_params_t));
	usb_params.reg_base = DWUSB_BASE;
	usb_params.desc.base = HIKEY_USB_DESC_BASE;
	usb_params.desc.size = HIKEY_USB_DESC_SIZE;
	usb_params.buffer.base = HIKEY_USB_DATA_BASE;
	usb_params.buffer.size = HIKEY_USB_DATA_SIZE;
	usb_params.serialno = hikey_init_serialno();
	assert(usb_params.serialno != NULL);
	dw_usb_init(&usb_params);

	/* Initiailize the fastboot variable "max-download-size". */
	fb_params.buffer.base = HIKEY_FB_BUFFER_BASE;
	fb_params.buffer.size = HIKEY_FB_BUFFER_SIZE;
	fb_params.download.base = HIKEY_FB_DOWNLOAD_BASE;
	fb_params.download.size = HIKEY_FB_DOWNLOAD_SIZE;
	fb_params.image_id = BL2U_IMAGE_ID;
	sprintf(response, "0x%x", HIKEY_MMC_DATA_SIZE);
	fastboot_set_var(FASTBOOT_VAR_MAX_DOWNLOAD_SIZE, response, NULL);
	fastboot_set_var(FASTBOOT_VAR_VERSION, "1.0", NULL);
	fastboot_set_var(FASTBOOT_VAR_PRODUCT, "hikey", NULL);
	fastboot_set_var(FASTBOOT_VAR_PARTITION_TYPE, "",
			 hikey_get_partition_type);
	fastboot_set_var(FASTBOOT_VAR_PARTITION_SIZE, "",
			 hikey_get_partition_size);
	fastboot_register_command(FASTBOOT_COMMAND_REBOOT, hikey_reboot);
	fastboot_register_command(FASTBOOT_COMMAND_OEM, hikey_oem);
	fastboot_register_command(FASTBOOT_COMMAND_FLASH, hikey_flash);
	fastboot_register_command(FASTBOOT_COMMAND_ERASE, hikey_erase);
	fastboot_run(&fb_params);
}
