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

#if defined(NXP_XSPI_DIAG)
/*!
 * @details Read JEDEC flash identification bytes (RDID, opcode 0x9F).
 * Issues a command-only SPI transaction and fills @p id with up to @p len
 * bytes (manufacturer, memory type, memory capacity, extended). A typical
 * caller passes len=5 so @p id[0] is the JEDEC manufacturer ID.
 *
 * @param[out] id  destination buffer, at least @p len bytes
 * @param[in]  len number of ID bytes to read (1..8)
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_read_id(uint8_t *id, uint32_t len);

/*!
 * @details Read JEDEC Serial Flash Discoverable Parameters (SFDP,
 * JESD216, opcode 0x5A). Issues a 0x5A command with a 24-bit address
 * and 8 dummy cycles (SFDP is always legacy 3-byte addressing per
 * spec, regardless of the main array's 4-byte address mode), then
 * reads @p len bytes from SFDP offset @p sfdp_off into @p buf.
 *
 * Typical callers read 16 bytes at offset 0 to obtain the SFDP header
 * + first parameter-table header (whose "pointer" field locates the
 * Basic Flash Parameter Table), then read the BFPT with a second
 * call. Use for vendor-neutral geometry discovery: total density,
 * sector sizes + erase opcodes, page size.
 *
 * @param[in]  sfdp_off source offset within the SFDP address space
 * @param[out] buf      destination buffer, at least @p len bytes
 * @param[in]  len      number of bytes to read (1..FSPI RX IP buffer)
 *
 * @return XSPI_SUCCESS or error code
 */
int xspi_read_sfdp(uint32_t sfdp_off, uint8_t *buf, uint32_t len);

/*!
 * @details Put the attached flash chip into 4-byte address mode by
 * issuing the EN4B opcode (0xB7). Required on flashes whose power-up
 * default is 3-byte mode (ADP bit = 0) when the driver emits 4-byte
 * address opcodes for devices larger than 16 MiB. Without it, erase
 * and program commands are silently mis-targeted because the chip
 * latches 3 bytes of a 4-byte address.
 *
 * @return XSPI_SUCCESS on success, negative error code on IP cmd
 *         engine error.
 */
int fspi_enter_4byte_mode(void);

/*!
 * @details Read Status Register 1 (RDSR, opcode 0x05). One byte of
 * status: SRP0, BP[4:0], WEL, WIP. Typical use: verify WEL latched
 * after xspi_wren(), or probe block-protect bits during bring-up.
 *
 * @param[out] val  receives the SR1 byte on success
 * @return XSPI_SUCCESS on success, negative error code on IP error
 */
int fspi_read_sr1(uint8_t *val);

/*!
 * @details Read Status Register 3 (RDSR3 opcode 0x15). One byte of
 * status. Typical use: verify @ref fspi_enter_4byte_mode() took
 * effect by checking SR3 bit 19 (ADS = current address mode).
 *
 * @param[out] val  receives the SR3 byte on success
 * @return XSPI_SUCCESS on success, negative error code on IP error
 */
int fspi_read_sr3(uint8_t *val);

/*!
 * @details Ensure FSPI SCK stays at or below @p max_sck_hz by
 * programming FSPI_MCR0.SERCLKDIV appropriately. No-op if the
 * current divider already satisfies the limit. Use when a
 * bootrom-default divider would run SCK above the attached flash
 * chip's program/erase rating.
 *
 * The FSPI root clock feeding the divider is board-specific so the
 * caller passes it in. Actual SCK = fspi_root_hz / (SERCLKDIV + 1).
 *
 * @param[in] fspi_root_hz  FSPI root-clock frequency in Hz
 * @param[in] max_sck_hz    desired upper bound on SCK in Hz
 */
void fspi_set_serclk_max_hz(uint64_t fspi_root_hz, uint64_t max_sck_hz);

/*!
 * @details Pretty-print the current FSPI clock tree (MCR0.SERCLKDIV
 * + computed SCK) for diagnostics. Caller provides the FSPI root
 * clock because that depends on the board clock tree.
 *
 * @param[in] fspi_root_hz  FSPI root-clock frequency in Hz
 */
void fspi_print_speed(uint64_t fspi_root_hz);
#endif /* NXP_XSPI_DIAG */

/*!
 * Add test cases to confirm flash read/erase/write functionality.
 */
void fspi_test(uint32_t fspi_test_addr, uint32_t size, int extra);
#endif /* FSPI_API_H */
