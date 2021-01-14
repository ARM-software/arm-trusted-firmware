/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file  emmc_def.h
 * @brief eMMC boot is expecting this header file
 *
 */

#ifndef EMMC_DEF_H
#define EMMC_DEF_H

#include "emmc_std.h"

/* ************************ HEADER (INCLUDE) SECTION *********************** */

/* ***************** MACROS, CONSTANTS, COMPILATION FLAGS ****************** */
#define EMMC_POWER_ON		(1U)

/* ********************** STRUCTURES, TYPE DEFINITIONS ********************* */

/* ********************** DECLARATION OF EXTERNAL DATA ********************* */
extern st_mmc_base mmc_drv_obj;

/* ************************** FUNCTION PROTOTYPES ************************** */

/** @brief for assembler program
 */
uint32_t _rom_emmc_finalize(void);

/** @brief eMMC driver API
 */
EMMC_ERROR_CODE rcar_emmc_init(void);
EMMC_ERROR_CODE emmc_terminate(void);
EMMC_ERROR_CODE rcar_emmc_memcard_power(uint8_t mode);
EMMC_ERROR_CODE rcar_emmc_mount(void);
EMMC_ERROR_CODE emmc_set_request_mmc_clock(uint32_t *freq);
EMMC_ERROR_CODE emmc_send_idle_cmd(uint32_t arg);
EMMC_ERROR_CODE emmc_select_partition(EMMC_PARTITION_ID id);
EMMC_ERROR_CODE emmc_read_sector(uint32_t *buff_address_virtual,
				 uint32_t sector_number, uint32_t count,
				 uint32_t feature_flags);
EMMC_ERROR_CODE emmc_write_sector(uint32_t *buff_address_virtual,
				  uint32_t sector_number, uint32_t count,
				  uint32_t feature_flags);
EMMC_ERROR_CODE emmc_erase_sector(uint32_t *start_address,
				  uint32_t *end_address);
uint32_t emmc_bit_field(uint8_t *data, uint32_t top, uint32_t bottom);

/** @brief interrupt service
 */
uint32_t emmc_interrupt(void);

/** @brief DMA
 */

/** @brief send command API
 */
EMMC_ERROR_CODE emmc_exec_cmd(uint32_t error_mask, uint32_t *response);
void emmc_make_nontrans_cmd(HAL_MEMCARD_COMMAND cmd, uint32_t arg);
void emmc_make_trans_cmd(HAL_MEMCARD_COMMAND cmd, uint32_t arg,
			 uint32_t *buff_address_virtual, uint32_t len,
			 HAL_MEMCARD_OPERATION dir,
			 HAL_MEMCARD_DATA_TRANSFER_MODE transfer_mode);
EMMC_ERROR_CODE emmc_set_ext_csd(uint32_t arg);

/** @brief for error information
 */
void emmc_write_error_info(uint16_t func_no, EMMC_ERROR_CODE error_code);
void emmc_write_error_info_func_no(uint16_t func_no);

/* ********************************* CODE ********************************** */

#endif /* EMMC_DEF_H */
/* ******************************** END ************************************ */
