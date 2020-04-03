/*
 * Copyright (c) 2019-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FRU_H
#define FRU_H

#include <stdbool.h>
#include <stdint.h>

/* max string length */
#define FRU_MAX_STR_LEN      32

/* max number of DDR channels */
#define BCM_MAX_NR_DDR       3

/* max supported FRU table size */
#define BCM_MAX_FRU_LEN      512

/* FRU table starting offset */
#define BCM_FRU_TBL_OFFSET   0x300000

/* FRU time constants */
#define MINS_PER_DAY        1440
#define MINS_PER_HOUR       60
#define FRU_YEAR_START      1996
#define FRU_MONTH_START     1
#define FRU_DAY_START       1
#define MONTHS_PER_YEAR     12

/*
 * FRU areas based on the spec
 */
enum fru_area_name {
	FRU_AREA_INTERNAL = 0,
	FRU_AREA_CHASSIS_INFO,
	FRU_AREA_BOARD_INFO,
	FRU_AREA_PRODUCT_INFO,
	FRU_AREA_MRECORD_INFO,
	FRU_MAX_NR_AREAS
};

/*
 * FRU area information
 *
 * @use: indicate this area is being used
 * @version: format version
 * @offset: offset of this area from the beginning of the FRU table
 * @len: total length of the area
 */
struct fru_area_info {
	bool use;
	uint8_t version;
	unsigned int offset;
	unsigned int len;
};

/*
 * DDR MCB information
 *
 * @idx: DDR channel index
 * @size_mb: DDR size of this channel in MB
 * @ref_id: DDR MCB reference ID
 */
struct ddr_mcb {
	unsigned int idx;
	unsigned int size_mb;
	uint32_t ref_id;
};

/*
 * DDR information
 *
 * @ddr_info: array that contains MCB related info for each channel
 */
struct ddr_info {
	struct ddr_mcb mcb[BCM_MAX_NR_DDR];
};

/*
 * FRU board area information
 *
 * @lang: Language code
 * @mfg_date: Manufacturing date
 * @manufacturer: Manufacturer
 * @product_name: Product name
 * @serial_number: Serial number
 * @part_number: Part number
 * @file_id: FRU file ID
 */
struct fru_board_info {
	unsigned char lang;
	unsigned int mfg_date;
	unsigned char manufacturer[FRU_MAX_STR_LEN];
	unsigned char product_name[FRU_MAX_STR_LEN];
	unsigned char serial_number[FRU_MAX_STR_LEN];
	unsigned char part_number[FRU_MAX_STR_LEN];
	unsigned char file_id[FRU_MAX_STR_LEN];
};

/*
 * FRU manufacture date in human readable format
 */
struct fru_time {
	unsigned int min;
	unsigned int hour;
	unsigned int day;
	unsigned int month;
	unsigned int year;
};

#ifdef USE_FRU
int fru_validate(uint8_t *data, struct fru_area_info *fru_area);
int fru_parse_ddr(uint8_t *data, struct fru_area_info *area,
		  struct ddr_info *ddr);
int fru_parse_board(uint8_t *data, struct fru_area_info *area,
		    struct fru_board_info *board);
void fru_format_time(unsigned int min, struct fru_time *tm);
#else
static inline int fru_validate(uint8_t *data, struct fru_area_info *fru_area)
{
	return -1;
}

static inline int fru_parse_ddr(uint8_t *data, struct fru_area_info *area,
				struct ddr_info *ddr)
{
	return -1;
}

static inline int fru_parse_board(uint8_t *data, struct fru_area_info *area,
				  struct fru_board_info *board)
{
	return -1;
}

static inline void fru_format_time(unsigned int min, struct fru_time *tm)
{
}
#endif /* USE_FRU */

#endif /* FRU_H */
