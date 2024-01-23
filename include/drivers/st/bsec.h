/*
 * Copyright (c) 2017-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BSEC_H
#define BSEC_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/utils_def.h>

/*
 * Return status
 */
#define BSEC_OK				0U
#define BSEC_ERROR			0xFFFFFFFFU
#define BSEC_DISTURBED			0xFFFFFFFEU
#define BSEC_INVALID_PARAM		0xFFFFFFFCU
#define BSEC_PROG_FAIL			0xFFFFFFFBU
#define BSEC_LOCK_FAIL			0xFFFFFFFAU
#define BSEC_TIMEOUT			0xFFFFFFF9U
#define BSEC_RETRY			0xFFFFFFF8U
#define BSEC_NOT_SUPPORTED		0xFFFFFFF7U
#define BSEC_WRITE_LOCKED		0xFFFFFFF6U

/*
 * get BSEC global state: result for bsec_get_secure_state()
 * @state: global state
 *           [1:0] BSEC state
 *             00b: Sec Open
 *             01b: Sec Closed
 *             11b: Invalid
 *           [8]: Hardware Key set = 1b
 */
#define BSEC_STATE_SEC_OPEN		U(0x0)
#define BSEC_STATE_SEC_CLOSED		U(0x1)
#define BSEC_STATE_INVALID		U(0x3)
#define BSEC_STATE_MASK			GENMASK_32(1, 0)

uint32_t bsec_probe(void);

uint32_t bsec_read_otp(uint32_t *val, uint32_t otp);
uint32_t bsec_shadow_read_otp(uint32_t *val, uint32_t otp);
uint32_t bsec_write_otp(uint32_t val, uint32_t otp);
uint32_t bsec_program_otp(uint32_t val, uint32_t otp);

uint32_t bsec_read_debug_conf(void);

void bsec_write_scratch(uint32_t val);

/* Sticky lock support */
uint32_t bsec_set_sr_lock(uint32_t otp);
uint32_t bsec_read_sr_lock(uint32_t otp, bool *value);
uint32_t bsec_set_sw_lock(uint32_t otp);
uint32_t bsec_read_sw_lock(uint32_t otp, bool *value);
uint32_t bsec_set_sp_lock(uint32_t otp);
uint32_t bsec_read_sp_lock(uint32_t otp, bool *value);

uint32_t bsec_get_secure_state(void);
static inline bool bsec_mode_is_closed_device(void)
{
	return (bsec_get_secure_state() & BSEC_STATE_MASK) == BSEC_STATE_SEC_CLOSED;
}

#if defined(IMAGE_BL32)
uint32_t bsec_permanent_lock_otp(uint32_t otp);
uint32_t bsec_check_nsec_access_rights(uint32_t otp);
#endif

#endif /* BSEC_H */
