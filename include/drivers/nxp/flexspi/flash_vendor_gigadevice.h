/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * GigaDevice-family FlexSPI NOR helpers. Opcode values and register
 * semantics come from the GD55LB02GF / GD55LB01GF / GD55LB04GF
 * family datasheet(s); other chips may share some of these (VL / NL
 * Register lock model is common across the GD55 series).
 */

#ifndef FLASH_VENDOR_GIGADEVICE_H
#define FLASH_VENDOR_GIGADEVICE_H

#include <stdint.h>

/*
 * Global Volatile-Lock Unlock (opcode 0x98).
 *
 * Clears every block's VL Register to 0 for the current power-up.
 * Needed when NVCR<02>.bit2=1 (factory default on GD55LB02GF), which
 * makes every block power-up VL-locked -- in that state the chip
 * silently drops SE/PP commands while still latching WEL, which
 * presents as "erase returns no-op, WEL stays 1, flash unchanged".
 *
 * Caller must have called fspi_init() and unlocked the FSPI LUT
 * region (this helper manages the LUT slot it uses internally).
 * Issues a WREN internally.
 *
 * @return 0 on success, negative on error (WREN failure, IP cmd
 *         engine error).
 */
int gd_global_vl_unlock(void);

/*
 * Read VL Register (opcode 0xE0) for the block at @offset.
 * One byte per block: 0x00 = Unprotected, 0xFF = Protected.
 *
 * @param[in]  offset  flash-relative 32-bit address inside the target block
 * @param[out] val     decoded VL byte on success
 * @return 0 on success, negative on error
 */
int gd_read_vl_register(uint32_t offset, uint8_t *val);

/*
 * Read NL Register (opcode 0xE2) for the block at @offset.
 * Non-volatile counterpart of VL. 0x00 = Unprotected, 0xFF = Protected.
 *
 * @param[in]  offset  flash-relative 32-bit address inside the target block
 * @param[out] val     decoded NL byte on success
 * @return 0 on success, negative on error
 */
int gd_read_nl_register(uint32_t offset, uint8_t *val);

#endif /* FLASH_VENDOR_GIGADEVICE_H */
