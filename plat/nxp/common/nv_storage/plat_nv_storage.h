/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_NV_STRG_H
#define PLAT_NV_STRG_H

#define DEFAULT_SET_VALUE 0xA1
#define READY_TO_WRITE_VALUE 0xFF

#ifndef NV_STORAGE_BASE_ADDR
#define NV_STORAGE_BASE_ADDR DEFAULT_NV_STORAGE_BASE_ADDR
#endif

typedef struct {
uint8_t warm_rst_flag;
uint8_t wdt_rst_flag;
uint8_t dummy[2];
} nv_app_data_t;


/*below enum and above structure should be in-sync. */
enum app_data_offset {
	WARM_RESET_FLAG_OFFSET,
	WDT_RESET_FLAG_OFFSET,
	APP_DATA_MAX_OFFSET,
};

int read_nv_app_data(void);

int wr_nv_app_data(int data_offset,
			uint8_t *data,
			int data_size);

const nv_app_data_t *get_nv_data(void);

#endif /* PLAT_NV_STRG_H */
