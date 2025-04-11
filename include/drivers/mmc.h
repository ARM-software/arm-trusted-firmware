/*
 * Copyright (c) 2021-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MMC_H
#define MMC_H

#include <stdint.h>

#include <lib/utils_def.h>

#define MMC_BLOCK_SIZE			U(512)
#define MMC_BLOCK_MASK			(MMC_BLOCK_SIZE - U(1))
#define MMC_BOOT_CLK_RATE		(400 * 1000)

#define MMC_CMD(_x)			U(_x)

#define MMC_ACMD(_x)			U(_x)

#define OCR_POWERUP			BIT_32(31U)
#define OCR_HCS				BIT_32(30U)
#define OCR_BYTE_MODE			(U(0) << 29U)
#define OCR_SECTOR_MODE			(U(2) << 29U)
#define OCR_ACCESS_MODE_MASK		(U(3) << 29U)
#define OCR_3_5_3_6			BIT_32(23U)
#define OCR_3_4_3_5			BIT_32(22U)
#define OCR_3_3_3_4			BIT_32(21U)
#define OCR_3_2_3_3			BIT_32(20U)
#define OCR_3_1_3_2			BIT_32(19U)
#define OCR_3_0_3_1			BIT_32(18U)
#define OCR_2_9_3_0			BIT_32(17U)
#define OCR_2_8_2_9			BIT_32(16U)
#define OCR_2_7_2_8			BIT_32(15U)
#define OCR_VDD_MIN_2V7			GENMASK_32(23U, 15U)
#define OCR_VDD_MIN_2V0			GENMASK_32(14U, 8U)
#define OCR_VDD_MIN_1V7			BIT_32(7U)

#define MMC_RSP_48			BIT_32(0U)
#define MMC_RSP_136			BIT_32(1U)		/* 136 bit response */
#define MMC_RSP_CRC			BIT_32(2U)		/* expect valid crc */
#define MMC_RSP_CMD_IDX			BIT_32(3U)		/* response contains cmd idx */
#define MMC_RSP_BUSY			BIT_32(4U)		/* device may be busy */

/* JEDEC 4.51 chapter 6.12 */
#define MMC_RESPONSE_R1			(MMC_RSP_48 | MMC_RSP_CMD_IDX | MMC_RSP_CRC)
#define MMC_RESPONSE_R1B		(MMC_RESPONSE_R1 | MMC_RSP_BUSY)
#define MMC_RESPONSE_R2			(MMC_RSP_48 | MMC_RSP_136 | MMC_RSP_CRC)
#define MMC_RESPONSE_R3			(MMC_RSP_48)
#define MMC_RESPONSE_R4			(MMC_RSP_48)
#define MMC_RESPONSE_R5			(MMC_RSP_48 | MMC_RSP_CRC | MMC_RSP_CMD_IDX)
#define MMC_RESPONSE_R6			(MMC_RSP_48 | MMC_RSP_CRC | MMC_RSP_CMD_IDX)
#define MMC_RESPONSE_R7			(MMC_RSP_48 | MMC_RSP_CRC | MMC_RSP_CMD_IDX)

/* Value randomly chosen for eMMC RCA, it should be > 1 */
#define MMC_FIX_RCA			6
#define RCA_SHIFT_OFFSET		16

#define CMD_EXTCSD_PARTITION_CONFIG	179
#define CMD_EXTCSD_BUS_WIDTH		183
#define CMD_EXTCSD_HS_TIMING		185
#define CMD_EXTCSD_PART_SWITCH_TIME	199
#define CMD_EXTCSD_SEC_CNT		212
#define CMD_EXTCSD_BOOT_SIZE_MULT	226

#define EXT_CSD_PART_CONFIG_ACC_MASK	GENMASK(2, 0)
#define PART_CFG_BOOT_PARTITION1_ENABLE	(U(1) << 3)
#define PART_CFG_BOOT_PARTITION1_ACCESS (U(1) << 0)
#define PART_CFG_BOOT_PARTITION_NO_ACCESS	U(0)
#define PART_CFG_BOOT_PART_EN_MASK		GENMASK(5, 3)
#define PART_CFG_BOOT_PART_EN_SHIFT		3
#define PART_CFG_CURRENT_BOOT_PARTITION(x)	(((x) & PART_CFG_BOOT_PART_EN_MASK) >> \
	PART_CFG_BOOT_PART_EN_SHIFT)

/* Values in EXT CSD register */
#define MMC_BUS_WIDTH_1			U(0)
#define MMC_BUS_WIDTH_4			U(1)
#define MMC_BUS_WIDTH_8			U(2)
#define MMC_BUS_WIDTH_DDR_4		U(5)
#define MMC_BUS_WIDTH_DDR_8		U(6)
#define MMC_BOOT_MODE_BACKWARD		(U(0) << 3)
#define MMC_BOOT_MODE_HS_TIMING		(U(1) << 3)
#define MMC_BOOT_MODE_DDR		(U(2) << 3)

#define EXTCSD_SET_CMD			(U(0) << 24)
#define EXTCSD_SET_BITS			(U(1) << 24)
#define EXTCSD_CLR_BITS			(U(2) << 24)
#define EXTCSD_WRITE_BYTES		(U(3) << 24)
#define EXTCSD_CMD(x)			(((x) & 0xff) << 16)
#define EXTCSD_VALUE(x)			(((x) & 0xff) << 8)
#define EXTCSD_CMD_SET_NORMAL		U(1)

#define CSD_TRAN_SPEED_UNIT_MASK	GENMASK(2, 0)
#define CSD_TRAN_SPEED_MULT_MASK	GENMASK(6, 3)
#define CSD_TRAN_SPEED_MULT_SHIFT	3

#define STATUS_CURRENT_STATE(x)		(((x) & 0xf) << 9)
#define STATUS_READY_FOR_DATA		BIT(8)
#define STATUS_SWITCH_ERROR		BIT(7)
#define MMC_GET_STATE(x)		(((x) >> 9) & 0xf)
#define MMC_STATE_IDLE			0
#define MMC_STATE_READY			1
#define MMC_STATE_IDENT			2
#define MMC_STATE_STBY			3
#define MMC_STATE_TRAN			4
#define MMC_STATE_DATA			5
#define MMC_STATE_RCV			6
#define MMC_STATE_PRG			7
#define MMC_STATE_DIS			8
#define MMC_STATE_BTST			9
#define MMC_STATE_SLP			10

#define MMC_FLAG_CMD23			(U(1) << 0)
#define MMC_FLAG_SD_CMD6		(U(1) << 1)

#define CMD8_CHECK_PATTERN		U(0xAA)
#define VHS_2_7_3_6_V			BIT(8)

#define SD_SCR_BUS_WIDTH_1		BIT(8)
#define SD_SCR_BUS_WIDTH_4		BIT(10)

#define SD_SWITCH_FUNC_CHECK		0U
#define SD_SWITCH_FUNC_SWITCH		BIT(31)
#define SD_SWITCH_ALL_GROUPS_MASK	GENMASK(23, 0)

struct mmc_cmd {
	unsigned int	cmd_idx;
	unsigned int	cmd_arg;
	unsigned int	resp_type;
	unsigned int	resp_data[4];
};

struct mmc_ops {
	void (*init)(void);
	int (*send_cmd)(struct mmc_cmd *cmd);
	int (*set_ios)(unsigned int clk, unsigned int width);
	int (*prepare)(int lba, uintptr_t buf, size_t size);
	int (*read)(int lba, uintptr_t buf, size_t size);
	int (*write)(int lba, const uintptr_t buf, size_t size);
};

struct mmc_csd_emmc {
	unsigned int		not_used:		1;
	unsigned int		crc:			7;
	unsigned int		ecc:			2;
	unsigned int		file_format:		2;
	unsigned int		tmp_write_protect:	1;
	unsigned int		perm_write_protect:	1;
	unsigned int		copy:			1;
	unsigned int		file_format_grp:	1;

	unsigned int		reserved_1:		5;
	unsigned int		write_bl_partial:	1;
	unsigned int		write_bl_len:		4;
	unsigned int		r2w_factor:		3;
	unsigned int		default_ecc:		2;
	unsigned int		wp_grp_enable:		1;

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
};

struct mmc_csd_sd_v2 {
	unsigned int		not_used:		1;
	unsigned int		crc:			7;
	unsigned int		reserved_1:		2;
	unsigned int		file_format:		2;
	unsigned int		tmp_write_protect:	1;
	unsigned int		perm_write_protect:	1;
	unsigned int		copy:			1;
	unsigned int		file_format_grp:	1;

	unsigned int		reserved_2:		5;
	unsigned int		write_bl_partial:	1;
	unsigned int		write_bl_len:		4;
	unsigned int		r2w_factor:		3;
	unsigned int		reserved_3:		2;
	unsigned int		wp_grp_enable:		1;

	unsigned int		wp_grp_size:		7;
	unsigned int		sector_size:		7;
	unsigned int		erase_block_en:		1;
	unsigned int		reserved_4:		1;
	unsigned int		c_size_low:		16;

	unsigned int		c_size_high:		6;
	unsigned int		reserved_5:		6;
	unsigned int		dsr_imp:		1;
	unsigned int		read_blk_misalign:	1;
	unsigned int		write_blk_misalign:	1;
	unsigned int		read_bl_partial:	1;
	unsigned int		read_bl_len:		4;
	unsigned int		ccc:			12;

	unsigned int		tran_speed:		8;
	unsigned int		nsac:			8;
	unsigned int		taac:			8;
	unsigned int		reserved_6:		6;
	unsigned int		csd_structure:		2;
};

struct sd_switch_status {
	unsigned short		max_current;
	unsigned short		support_g6;
	unsigned short		support_g5;
	unsigned short		support_g4;
	unsigned short		support_g3;
	unsigned short		support_g2;
	unsigned short		support_g1;
	unsigned char		sel_g6_g5;
	unsigned char		sel_g4_g3;
	unsigned char		sel_g2_g1;
	unsigned char		data_struct_ver;
	unsigned short		busy_g6;
	unsigned short		busy_g5;
	unsigned short		busy_g4;
	unsigned short		busy_g3;
	unsigned short		busy_g2;
	unsigned short		busy_g1;
	unsigned short		reserved[17];
};

enum mmc_device_type {
	MMC_IS_EMMC,
	MMC_IS_SD,
	MMC_IS_SD_HC,
};

struct mmc_device_info {
	unsigned long long	device_size;	/* Size of device in bytes */
	unsigned int		block_size;	/* Block size in bytes */
	unsigned int		max_bus_freq;	/* Max bus freq in Hz */
	unsigned int		ocr_voltage;	/* OCR voltage */
	enum mmc_device_type	mmc_dev_type;	/* Type of MMC */
};

size_t mmc_read_blocks(int lba, uintptr_t buf, size_t size);
size_t mmc_write_blocks(int lba, const uintptr_t buf, size_t size);
size_t mmc_erase_blocks(int lba, size_t size);
int mmc_part_switch_current_boot(void);
int mmc_part_switch_user(void);
size_t mmc_boot_part_size(void);
size_t mmc_boot_part_read_blocks(int lba, uintptr_t buf, size_t size);
int mmc_init(const struct mmc_ops *ops_ptr, unsigned int clk,
	     unsigned int width, unsigned int flags,
	     struct mmc_device_info *device_info);

#endif /* MMC_H */
