/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EMMC_H__
#define __EMMC_H__

#include <stdint.h>

#define EMMC_BLOCK_SIZE			512
#define EMMC_BLOCK_MASK			(EMMC_BLOCK_SIZE - 1)
#define EMMC_BOOT_CLK_RATE		(400 * 1000)

#define EMMC_CMD0			0
#define EMMC_CMD1			1
#define EMMC_CMD2			2
#define EMMC_CMD3			3
#define EMMC_CMD6			6
#define EMMC_CMD7			7
#define EMMC_CMD8			8
#define EMMC_CMD9			9
#define EMMC_CMD12			12
#define EMMC_CMD13			13
#define EMMC_CMD17			17
#define EMMC_CMD18			18
#define EMMC_CMD21			21
#define EMMC_CMD23			23
#define EMMC_CMD24			24
#define EMMC_CMD25			25
#define EMMC_CMD35			35
#define EMMC_CMD36			36
#define EMMC_CMD38			38

#define OCR_POWERUP			(1 << 31)
#define OCR_BYTE_MODE			(0 << 29)
#define OCR_SECTOR_MODE			(2 << 29)
#define OCR_ACCESS_MODE_MASK		(3 << 29)
#define OCR_VDD_MIN_2V7			(0x1ff << 15)
#define OCR_VDD_MIN_2V0			(0x7f << 8)
#define OCR_VDD_MIN_1V7			(1 << 7)

#define EMMC_RESPONSE_R1		1
#define EMMC_RESPONSE_R1B		1
#define EMMC_RESPONSE_R2		4
#define EMMC_RESPONSE_R3		1
#define EMMC_RESPONSE_R4		1
#define EMMC_RESPONSE_R5		1

#define EMMC_FIX_RCA			6	/* > 1 */
#define RCA_SHIFT_OFFSET		16

#define CMD_EXTCSD_PARTITION_CONFIG	179
#define CMD_EXTCSD_BUS_WIDTH		183
#define CMD_EXTCSD_HS_TIMING		185

#define PART_CFG_BOOT_PARTITION1_ENABLE	(1 << 3)
#define PART_CFG_PARTITION1_ACCESS	(1 << 0)

/* values in EXT CSD register */
#define EMMC_BUS_WIDTH_1		0
#define EMMC_BUS_WIDTH_4		1
#define EMMC_BUS_WIDTH_8		2
#define EMMC_BUS_WIDTH_DDR_4		5
#define EMMC_BUS_WIDTH_DDR_8		6
#define EMMC_BOOT_MODE_BACKWARD		(0 << 3)
#define EMMC_BOOT_MODE_HS_TIMING	(1 << 3)
#define EMMC_BOOT_MODE_DDR		(2 << 3)

#define EXTCSD_SET_CMD			(0 << 24)
#define EXTCSD_SET_BITS			(1 << 24)
#define EXTCSD_CLR_BITS			(2 << 24)
#define EXTCSD_WRITE_BYTES		(3 << 24)
#define EXTCSD_CMD(x)			(((x) & 0xff) << 16)
#define EXTCSD_VALUE(x)			(((x) & 0xff) << 8)

#define STATUS_CURRENT_STATE(x)		(((x) & 0xf) << 9)
#define STATUS_READY_FOR_DATA		(1 << 8)
#define STATUS_SWITCH_ERROR		(1 << 7)
#define EMMC_GET_STATE(x)		(((x) >> 9) & 0xf)
#define EMMC_STATE_IDLE			0
#define EMMC_STATE_READY		1
#define EMMC_STATE_IDENT		2
#define EMMC_STATE_STBY			3
#define EMMC_STATE_TRAN			4
#define EMMC_STATE_DATA			5
#define EMMC_STATE_RCV			6
#define EMMC_STATE_PRG			7
#define EMMC_STATE_DIS			8
#define EMMC_STATE_BTST			9
#define EMMC_STATE_SLP			10

#define EMMC_FLAG_CMD23			(1 << 0)

typedef struct emmc_cmd {
	unsigned int	cmd_idx;
	unsigned int	cmd_arg;
	unsigned int	resp_type;
	unsigned int	resp_data[4];
} emmc_cmd_t;

typedef struct emmc_ops {
	void (*init)(void);
	int (*send_cmd)(emmc_cmd_t *cmd);
	int (*set_ios)(int clk, int width);
	int (*prepare)(int lba, uintptr_t buf, size_t size);
	int (*read)(int lba, uintptr_t buf, size_t size);
	int (*write)(int lba, const uintptr_t buf, size_t size);
} emmc_ops_t;

typedef struct emmc_csd {
	unsigned int 	    not_used:		1;
	unsigned int   	    crc:			7;
	unsigned int   	    ecc:			2;
	unsigned int   	    file_format:		2;
	unsigned int   	    tmp_write_protect:	1;
	unsigned int   	    perm_write_protect:	1;
	unsigned int   	    copy:			1;
	unsigned int   	    file_format_grp:	1;

	unsigned int 		reserved_1:		5;
	unsigned int 		write_bl_partial:	1;
	unsigned int 		write_bl_len:		4;
	unsigned int 		r2w_factor:		3;
	unsigned int 		default_ecc:		2;
	unsigned int 		wp_grp_enable:		1;

	unsigned int		wp_grp_size:		5;
	unsigned int		erase_grp_mult:		5;
	unsigned int		erase_grp_size:		5;
	unsigned int		c_size_mult:		3;
	unsigned int		vdd_w_curr_max:		3;
	unsigned int		vdd_w_curr_min:		3;
	unsigned int		vdd_r_curr_max:		3;
	unsigned int		vdd_r_curr_min:		3;
	unsigned int		c_size_low:		2;

	unsigned int		c_size_high:		10;
	unsigned int		reserved_2:		2;
	unsigned int		dsr_imp:		1;
	unsigned int		read_blk_misalign:	1;
	unsigned int		write_blk_misalign:	1;
	unsigned int		read_bl_partial:	1;
	unsigned int		read_bl_len:		4;
	unsigned int		ccc:			12;

	unsigned int		tran_speed:		8;
	unsigned int		nsac:			8;
	unsigned int		taac:			8;
	unsigned int		reserved_3:		2;
	unsigned int		spec_vers:		4;
	unsigned int		csd_structure:		2;
} emmc_csd_t;

size_t emmc_read_blocks(int lba, uintptr_t buf, size_t size);
size_t emmc_write_blocks(int lba, const uintptr_t buf, size_t size);
size_t emmc_erase_blocks(int lba, size_t size);
size_t emmc_rpmb_read_blocks(int lba, uintptr_t buf, size_t size);
size_t emmc_rpmb_write_blocks(int lba, const uintptr_t buf, size_t size);
size_t emmc_rpmb_erase_blocks(int lba, size_t size);
void emmc_init(const emmc_ops_t *ops, int clk, int bus_width,
	       unsigned int flags);

#endif	/* __EMMC_H__ */
