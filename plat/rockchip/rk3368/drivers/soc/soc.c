/*
 * Copyright (C) 2015, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Tony.Xie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <rk3368_def.h>
#include <soc.h>

void secure_timer_init(void)
{
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT0, 0xffffffff);
	mmio_write_32(STIMER1_BASE + TIMER_LOADE_COUNT1, 0xffffffff);

	/* auto reload & enable the timer */
	mmio_write_32(STIMER1_BASE + TIMER_CONTROL_REG, 0x01);
}

void sgrf_init(void)
{
	/*
	 * setting all configurable ip into no-secure
	 */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(5), 0xffff0000);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(6), 0xffff0040);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(7), 0xffff0000);

	/*********************************************************************
	 * secure dma to no sesure
	 *********************************************************************/
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(0), 0xfffcfff8);
	mmio_write_32(SGRF_BASE + SGRF_BUSDMAC_CON(1), 0xffff0fff);
	dsb();
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1), 0x40004); /* dma1 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4), 0x10001); /* dma2 */

	dsb();

	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(1), 0x40000); /* dma1 */
	mmio_write_32(CRU_BASE + CRU_SOFTRSTS_CON(4), 0x10000); /* dma2 */

	/*********************************************************************
	 * HDCP
	 *********************************************************************/
	 /* PDFK */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(24), 0xFFFFeeff);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(25), 0xFFFFccdd);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(26), 0xFFFFaabb);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(27), 0xFFFF8899);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(28), 0xFFFF6677);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(29), 0xFFFF4455);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(30), 0xFFFF2233);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(31), 0xFFFF0011);
	/* DUK */
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(32), 0xFFFF1100);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(33), 0xFFFF3322);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(34), 0xFFFF5544);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(35), 0xFFFF7766);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(36), 0xFFFF9988);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(37), 0xFFFFbbaa);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(38), 0xFFFFddcc);
	mmio_write_32(SGRF_BASE + SGRF_SOC_CON(39), 0xFFFFeeff);
}

void platform_soc_init(void)
{
	secure_timer_init();
	sgrf_init();
}
