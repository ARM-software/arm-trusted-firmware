/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <common/bl_common.h>

#include <io_addr_dec.h>
#include <mvebu_def.h>

struct dec_win_config io_dec_win_conf[] = {
	/* dec_reg_base  win_attr  max_dram_win  max_remap  win_offset */
	{0xc000,         0x3d,     2,       0,        0x08}, /* USB */
	{0xc100,         0x3d,     3,       0,        0x10}, /* USB3 */
	{0xc200,         0x3d,     2,       0,        0x10}, /* DMA */
	{0xc300,         0x3d,     2,       0,        0x10}, /* NETA0 */
	{0xc400,         0x3d,     2,       0,        0x10}, /* NETA1 */
	{0xc500,         0x3d,     2,       0,        0x10}, /* PCIe */
	{0xc800,         0x3d,     3,       0,        0x10}, /* SATA */
	{0xca00,         0x3d,     3,       0,        0x08}, /* SD */
	{0xcb00,         0x3d,     3,       0,        0x10}, /* eMMC */
	{0xce00,	 0x3d,	   2,	    0,        0x08}, /* EIP97 */
};

int marvell_get_io_dec_win_conf(struct dec_win_config **win, uint32_t *size)
{
	*win = io_dec_win_conf;
	*size = sizeof(io_dec_win_conf)/sizeof(struct dec_win_config);

	return 0;
}

