/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


/*!
 * @file	fspi_api.h
 * @brief	This file contains the FlexSPI/FSPI API to communicate
 *		to attached Slave device.
 * @addtogroup	FSPI_API
 * @{
 */

#ifndef FSPI_API_H
#define FSPI_API_H

#if DEBUG_FLEXSPI
#define SZ_57M			0x3900000u
#endif

/*!
 * Basic set of APIs.
 */

/*!
 * @details AHB read/IP Read, decision to be internal to API
 * Minimum Read size = 1Byte
 * @param[in] src_off source offset from where data to read from flash
 * @param[out] des Destination location where data needs to be copied
 * @param[in] len length in Bytes,where 1-word=4-bytes/32-bits
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_read(uint32_t src_off, uint32_t *des, uint32_t len);
/*!
 * @details Sector erase, Minimum size
 * 256KB(0x40000)/128KB(0x20000)/64K(0x10000)/4K(0x1000)
 * depending upon flash, Calls xspi_wren() internally
 * @param[out] erase_offset Destination erase location on flash which
 * has to be erased, needs to be multiple of 0x40000/0x20000/0x10000
 * @param[in] erase_len length in bytes in Hex like 0x100000 for 1MB, minimum
 * erase size is 1 sector(0x40000/0x20000/0x10000)
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_sector_erase(uint32_t erase_offset, uint32_t erase_len);
/*!
 * @details IP write, For writing data to flash, calls xspi_wren() internally.
 * Single/multiple page write can start @any offset, but performance will be low
 * due to ERRATA
 * @param[out] dst_off Destination location on flash where data needs to
 * be written
 * @param[in] src source offset from where data to be read
 * @param[in] len length in bytes,where 1-word=4-bytes/32-bits
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_write(uint32_t dst_off, void *src, uint32_t len);
/*!
 * @details fspi_init, Init function.
 * @param[in] uint32_t base_reg_addr
 * @param[in] uint32_t flash_start_addr
 *
 * @return XSPI_SUCCESS or error code
 */
int fspi_init(uint32_t base_reg_addr, uint32_t flash_start_addr);
/*!
 * @details is_flash_busy, Check if any erase or write or lock is
 * pending on flash/slave
 * @param[in] void
 *
 * @return TRUE/FLASE
 */
bool is_flash_busy(void);

/*!
 * Advanced set of APIs.
 */

/*!
 * @details Write enable, to be used by advance users only.
 * Step 1 for sending write commands to flash.
 * @param[in] dst_off destination offset where data will be written
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_wren(uint32_t dst_off);
/*!
 * @details AHB read, meaning direct memory mapped access to flash,
 * Minimum Read size = 1Byte
 * @param[in] src_off source offset from where data to read from flash,
 * needs to be word aligned
 * @param[out] des Destination location where data needs to be copied
 * @param[in] len length in Bytes,where 1-word=4-bytes/32-bits
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_ahb_read(uint32_t src_off, uint32_t *des, uint32_t len);
/*!
 * @details IP read, READ via RX buffer from flash, minimum READ size = 1Byte
 * @param[in] src_off source offset from where data to be read from flash
 * @param[out] des Destination location where data needs to be copied
 * @param[in] len length in Bytes,where 1-word=4-bytes/32-bits
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_ip_read(uint32_t src_off, uint32_t *des, uint32_t len);
/*!
 * @details CHIP erase, Erase complete chip in one go
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_bulk_erase(void);

/*!
 * Add test cases to confirm flash read/erase/write functionality.
 */
void fspi_test(uint32_t fspi_test_addr, uint32_t size, int extra);
#endif /* FSPI_API_H */
