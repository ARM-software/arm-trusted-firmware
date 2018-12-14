/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <hi6220.h>

#define MCU_SECTION_MAX		30

enum MCU_IMAGE_SEC_TYPE_ENUM {
	MCU_IMAGE_SEC_TYPE_TEXT = 0,	/* text section */
	MCU_IMAGE_SEC_TYPE_DATA,	/* data section */
	MCU_IMAGE_SEC_TYPE_BUTT
};

enum MCU_IMAGE_SEC_LOAD_ENUM {
	MCU_IMAGE_SEC_LOAD_STATIC = 0,
	MCU_IMAGE_SEC_LOAD_DYNAMIC,
	MCU_IMAGE_SEC_LOAD_BUFFER,
	MCU_IMAGE_SEC_LOAD_MODEM_ENTRY,
	MCU_IMAGE_SEC_LOAD_BUTT
};

struct mcu_image_sec {
	unsigned short serial;
	char type;
	char load_attr;
	uint32_t src_offset;		/* offset in image */
	uint32_t dst_offset;		/* offset in memory */
	uint32_t size;
};

struct mcu_image_head {
	char time_stamp[24];
	uint32_t image_size;
	uint32_t secs_num;
	struct mcu_image_sec secs[MCU_SECTION_MAX];
};

#define SOC_SRAM_M3_BASE_ADDR		(0xF6000000)

#define MCU_SRAM_SIZE			(0x0000C000)
#define MCU_CACHE_SIZE			(0x00004000)
#define MCU_CODE_SIZE			(MCU_SRAM_SIZE - MCU_CACHE_SIZE)

#define MCU_SYS_MEM_ADDR		(0x05E00000)
#define MCU_SYS_MEM_SIZE		(0x00100000)

static uint32_t mcu2ap_addr(uint32_t mcu_addr)
{
	if (mcu_addr < MCU_CODE_SIZE)
		return (mcu_addr + SOC_SRAM_M3_BASE_ADDR);
	else if ((mcu_addr >= MCU_SRAM_SIZE) &&
		 (mcu_addr < MCU_SRAM_SIZE + MCU_SYS_MEM_SIZE))
		return mcu_addr - MCU_SRAM_SIZE + MCU_SYS_MEM_ADDR;
	else
		return mcu_addr;
}

static int is_binary_header_invalid(struct mcu_image_head *head,
				    unsigned int length)
{
	/* invalid cases */
	if ((head->image_size == 0) ||
	    (head->image_size > length) ||
	    (head->secs_num > MCU_SECTION_MAX) ||
	    (head->secs_num == 0))
		return 1;

	return 0;
}

static int is_binary_section_invalid(struct mcu_image_sec *sec,
				     struct mcu_image_head *head)
{
	unsigned long ap_dst_offset = 0;

	if ((sec->serial >= head->secs_num) ||
	    (sec->src_offset + sec->size > head->image_size))
		return 1;

	if ((sec->type >= MCU_IMAGE_SEC_TYPE_BUTT) ||
	    (sec->load_attr >= MCU_IMAGE_SEC_LOAD_BUTT))
		return 1;

	ap_dst_offset = mcu2ap_addr(sec->dst_offset);
	if ((ap_dst_offset >= SOC_SRAM_M3_BASE_ADDR) &&
	    (ap_dst_offset < SOC_SRAM_M3_BASE_ADDR + 0x20000 - sec->size))
		return 0;
	else if ((ap_dst_offset >= MCU_SYS_MEM_ADDR) &&
		 (ap_dst_offset < MCU_SYS_MEM_ADDR + MCU_SYS_MEM_SIZE - sec->size))
		return 0;
	else if ((ap_dst_offset >= 0xfff8e000) &&
		 (ap_dst_offset < 0xfff91c00 - sec->size))
		return 0;

	ERROR("%s: mcu destination address invalid.\n", __func__);
	ERROR("%s: number=%d, dst offset=%d size=%d\n",
		__func__, sec->serial, sec->dst_offset, sec->size);
	return 1;
}

void hisi_mcu_enable_sram(void)
{
	mmio_write_32(AO_SC_PERIPH_CLKEN4,
		      AO_SC_PERIPH_CLKEN4_HCLK_IPC_S |
		      AO_SC_PERIPH_CLKEN4_HCLK_IPC_NS);

	/* set register to enable dvfs which is used by mcu */
	mmio_write_32(PERI_SC_RESERVED8_ADDR, 0x0A001022);

	/* mcu mem is powered on, need de-assert reset */
	mmio_write_32(AO_SC_PERIPH_RSTDIS4,
		      AO_SC_PERIPH_RSTDIS4_RESET_MCU_ECTR_N);

	/* enable mcu hclk */
	mmio_write_32(AO_SC_PERIPH_CLKEN4,
		      AO_SC_PERIPH_CLKEN4_HCLK_MCU |
		      AO_SC_PERIPH_CLKEN4_CLK_MCU_DAP);
}

void hisi_mcu_start_run(void)
{
	unsigned int val;

	/* set mcu ddr remap configuration */
	mmio_write_32(AO_SC_MCU_SUBSYS_CTRL2, MCU_SYS_MEM_ADDR);

	/* de-assert reset for mcu and to run */
	mmio_write_32(AO_SC_PERIPH_RSTDIS4,
		AO_SC_PERIPH_RSTDIS4_RESET_MCU_ECTR_N |
		AO_SC_PERIPH_RSTDIS4_RESET_MCU_SYS_N |
		AO_SC_PERIPH_RSTDIS4_RESET_MCU_POR_N |
		AO_SC_PERIPH_RSTDIS4_RESET_MCU_DAP_N);

	val = mmio_read_32(AO_SC_SYS_CTRL2);
	mmio_write_32(AO_SC_SYS_CTRL2,
		val | AO_SC_SYS_CTRL2_GLB_SRST_STAT_CLEAR);

	INFO("%s: AO_SC_SYS_CTRL2=%x\n", __func__,
		mmio_read_32(AO_SC_SYS_CTRL2));
}

int hisi_mcu_load_image(uintptr_t image_base, uint32_t image_size)
{
	unsigned int i;
	struct mcu_image_head *head;
	char *buf;

	head = (struct mcu_image_head *)image_base;
	if (is_binary_header_invalid(head, image_size)) {
		ERROR("Invalid %s image header.\n", head->time_stamp);
		return -1;
	}

	buf = (char *)head;
	for (i = 0; i < head->secs_num; i++) {

		int *src, *dst;

		/* check the sections */
		if (is_binary_section_invalid(&head->secs[i], head)) {
			ERROR("Invalid mcu section.\n");
			return -1;
		}

		/* check if the section is static-loaded */
		if (head->secs[i].load_attr != MCU_IMAGE_SEC_LOAD_STATIC)
			continue;

		/* copy the sections */
		src = (int *)(intptr_t)(buf + head->secs[i].src_offset);
		dst = (int *)(intptr_t)mcu2ap_addr(head->secs[i].dst_offset);

		memcpy((void *)dst, (void *)src, head->secs[i].size);

		INFO("%s: mcu sections %d:\n", __func__, i);
		INFO("%s:  src  = 0x%x\n",
		     __func__, (unsigned int)(uintptr_t)src);
		INFO("%s:  dst  = 0x%x\n",
		     __func__, (unsigned int)(uintptr_t)dst);
		INFO("%s:  size = %d\n", __func__, head->secs[i].size);

		INFO("%s:  [SRC 0x%x] 0x%x 0x%x 0x%x 0x%x\n",
		     __func__, (unsigned int)(uintptr_t)src,
		     src[0], src[1], src[2], src[3]);
		INFO("%s:  [DST 0x%x] 0x%x 0x%x 0x%x 0x%x\n",
		     __func__, (unsigned int)(uintptr_t)dst,
		     dst[0], dst[1], dst[2], dst[3]);
	}

	return 0;
}
