/*
 * Copyright (c) 2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <limits.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <drivers/st/bsec3_reg.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <libfdt.h>

#include <platform_def.h>

#define BSEC_IP_VERSION_1_0	U(0x10)
#define BSEC_IP_ID_3		U(0x100033)

#define MAX_NB_TRIES		U(3)

/*
 * IP configuration
 */
#define BSEC_OTP_MASK			GENMASK_32(4, 0)
#define BSEC_OTP_BANK_SHIFT		U(5)
#define BSEC_TIMEOUT_VALUE		U(0x800000) /* ~7sec @1.2GHz */

/* Magic use to indicated valid SHADOW = 'B' 'S' 'E' 'C' */
#define BSEC_MAGIC			U(0x42534543)

#define OTP_MAX_SIZE			(STM32MP2_OTP_MAX_ID + U(1))

struct bsec_shadow {
	uint32_t magic;
	uint32_t state;
	uint32_t value[OTP_MAX_SIZE];
	uint32_t status[OTP_MAX_SIZE];
};

static uint32_t otp_bank(uint32_t otp)
{
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	return (otp & ~BSEC_OTP_MASK) >> BSEC_OTP_BANK_SHIFT;
}

static uint32_t otp_bit_mask(uint32_t otp)
{
	return BIT(otp & BSEC_OTP_MASK);
}

/*
 * bsec_get_status: return status register value.
 */
static uint32_t bsec_get_status(void)
{
	return mmio_read_32(BSEC_BASE + BSEC_OTPSR);
}

/*
 * bsec_get_version: return BSEC version.
 */
static uint32_t bsec_get_version(void)
{
	return mmio_read_32(BSEC_BASE + BSEC_VERR) & BSEC_VERR_MASK;
}

/*
 * bsec_get_id: return BSEC ID.
 */
static uint32_t bsec_get_id(void)
{
	return mmio_read_32(BSEC_BASE + BSEC_IPIDR);
}

static bool is_fuse_shadowed(uint32_t otp)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);
	uint32_t bank_value;

	bank_value = mmio_read_32(BSEC_BASE + BSEC_SFSR(bank));

	if ((bank_value & otp_mask) != 0U) {
		return true;
	}

	return false;
}

static void poll_otp_status_busy(void)
{
	uint32_t timeout = BSEC_TIMEOUT_VALUE;

	while (((bsec_get_status() & BSEC_OTPSR_BUSY) != 0U) && (timeout != 0U)) {
		timeout--;
	}

	if ((bsec_get_status() & BSEC_OTPSR_BUSY) != 0U) {
		ERROR("BSEC timeout\n");
		panic();
	}
}

static uint32_t check_read_error(uint32_t otp)
{
	uint32_t status = bsec_get_status();

	if ((status & BSEC_OTPSR_SECF) != 0U) {
		VERBOSE("BSEC read %u single error correction detected\n", otp);
	}

	if ((status & BSEC_OTPSR_PPLF) != 0U) {
		VERBOSE("BSEC read %u permanent programming lock detected.\n", otp);
	}

	if ((status & BSEC_OTPSR_PPLMF) != 0U) {
		ERROR("BSEC read %u error 0x%x\n", otp, status);
		return BSEC_ERROR;
	}

	if ((status & (BSEC_OTPSR_DISTURBF | BSEC_OTPSR_DEDF | BSEC_OTPSR_AMEF)) != 0U) {
		ERROR("BSEC read %u error 0x%x with invalid FVR\n", otp, status);
		return BSEC_RETRY;
	}

	return BSEC_OK;
}

static uint32_t check_program_error(uint32_t otp)
{
	uint32_t status = bsec_get_status();

	if ((status & BSEC_OTPSR_PROGFAIL) != 0U) {
		ERROR("BSEC program %u error 0x%x\n", otp, status);
		return BSEC_RETRY;
	}

	return BSEC_OK;
}

static void check_reset_error(void)
{
	uint32_t status = bsec_get_status();

	/* check initial status reporting */
	if ((status & BSEC_OTPSR_BUSY) != 0U) {
		VERBOSE("BSEC reset and busy when OTPSR read\n");
	}
	if ((status & BSEC_OTPSR_HIDEUP) != 0U) {
		VERBOSE("BSEC upper fuse are not accessible (HIDEUP)\n");
	}
	if ((status & BSEC_OTPSR_OTPSEC) != 0U) {
		VERBOSE("BSEC reset single error correction detected\n");
	}
	if ((status & BSEC_OTPSR_OTPNVIR) == 0U) {
		VERBOSE("BSEC reset first fuse word 0 is detected zero\n");
	}
	if ((status & BSEC_OTPSR_OTPERR) != 0U) {
		ERROR("BSEC reset critical error 0x%x\n", status);
		panic();
	}
	if ((status & BSEC_OTPSR_FUSEOK) != BSEC_OTPSR_FUSEOK) {
		ERROR("BSEC reset critical error 0x%x\n", status);
		panic();
	}
}

static bool is_bsec_write_locked(void)
{
	return (mmio_read_32(BSEC_BASE + BSEC_LOCKR) & BSEC_LOCKR_GWLOCK_MASK) != 0U;
}

/*
 * bsec_probe: initialize BSEC driver.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_probe(void)
{
	uint32_t version = bsec_get_version();
	uint32_t id = bsec_get_id();

	if ((version != BSEC_IP_VERSION_1_0) || (id != BSEC_IP_ID_3)) {
		EARLY_ERROR("%s: version = 0x%x, id = 0x%x\n", __func__, version, id);
		panic();
	}

	check_reset_error();

	return BSEC_OK;
}

/*
 * bsec_shadow_register: copy SAFMEM OTP to BSEC data.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
static uint32_t bsec_shadow_register(uint32_t otp)
{
	uint32_t result;
	uint32_t i;
	bool value;

	result = bsec_read_sr_lock(otp, &value);
	if (result != BSEC_OK) {
		WARN("BSEC: %u Sticky-read bit read Error %u\n", otp, result);
	} else if (value) {
		VERBOSE("BSEC: OTP %u is locked and will not be refreshed\n", otp);
	}

	for (i = 0U; i < MAX_NB_TRIES; i++) {
		mmio_write_32(BSEC_BASE + BSEC_OTPCR, otp);

		poll_otp_status_busy();

		result = check_read_error(otp);
		if (result != BSEC_RETRY) {
			break;
		}
	}

	return result;
}

/*
 * bsec_write_otp: write a value in shadow OTP.
 * val: value to program.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_write_otp(uint32_t val, uint32_t otp)
{
	bool state;
	uint32_t result;

	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	if (!is_fuse_shadowed(otp)) {
		return BSEC_ERROR;
	}

	if (is_bsec_write_locked()) {
		return BSEC_WRITE_LOCKED;
	}

	result = bsec_read_sw_lock(otp, &state);
	if (result != BSEC_OK) {
		WARN("Shadow register is SW locked\n");
		return result;
	}

	mmio_write_32(BSEC_BASE + BSEC_FVR(otp), val);

	return BSEC_OK;
}

/*
 * bsec_program_otp: program a bit in SAFMEM after the prog.
 *	The OTP data is not refreshed.
 * val: value to program.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_program_otp(uint32_t val, uint32_t otp)
{
	uint32_t result;
	uint32_t i;
	bool value;

	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	if (is_bsec_write_locked() == true) {
		return BSEC_WRITE_LOCKED;
	}

	result = bsec_read_sp_lock(otp, &value);
	if (result != BSEC_OK) {
		WARN("BSEC: %u Sticky-prog bit read Error %u\n", otp, result);
	} else if (value) {
		WARN("BSEC: OTP locked, prog will be ignored\n");
		return BSEC_WRITE_LOCKED;
	}

	mmio_write_32(BSEC_BASE + BSEC_WDR, val);

	for (i = 0U; i < MAX_NB_TRIES; i++) {
		mmio_write_32(BSEC_BASE + BSEC_OTPCR, otp | BSEC_OTPCR_PROG);

		poll_otp_status_busy();

		result = check_program_error(otp);
		if (result != BSEC_RETRY) {
			break;
		}
	}

	return result;
}

/*
 * bsec_read_debug_conf: read debug configuration.
 */
uint32_t bsec_read_debug_conf(void)
{
	return mmio_read_32(BSEC_BASE + BSEC_DENR);
}

static uint32_t bsec_lock_register_set(uint32_t offset, uint32_t mask)
{
	uint32_t value = mmio_read_32(BSEC_BASE + offset);

	/* The lock is already set */
	if ((value & mask) != 0U) {
		return BSEC_OK;
	}

	if (is_bsec_write_locked()) {
		return BSEC_WRITE_LOCKED;
	}

	value |= mask;

	mmio_write_32(BSEC_BASE + offset, value);

	return BSEC_OK;
}

static bool bsec_lock_register_get(uint32_t offset, uint32_t mask)
{
	uint32_t value = mmio_read_32(BSEC_BASE + offset);

	return (value & mask) != 0U;
}

/*
 * bsec_set_sr_lock: set shadow-read lock.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_set_sr_lock(uint32_t otp)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	return bsec_lock_register_set(BSEC_SRLOCK(bank), otp_mask);
}

/*
 * bsec_read_sr_lock: read shadow-read lock.
 * otp: OTP number.
 * value: read value (true or false).
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_read_sr_lock(uint32_t otp, bool *value)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	assert(value != NULL);
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	*value = bsec_lock_register_get(BSEC_SRLOCK(bank), otp_mask);

	return BSEC_OK;
}

/*
 * bsec_set_sw_lock: set shadow-write lock.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_set_sw_lock(uint32_t otp)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	return bsec_lock_register_set(BSEC_SWLOCK(bank), otp_mask);
}

/*
 * bsec_read_sw_lock: read shadow-write lock.
 * otp: OTP number.
 * value: read value (true or false).
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_read_sw_lock(uint32_t otp, bool *value)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	assert(value != NULL);
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	*value = bsec_lock_register_get(BSEC_SWLOCK(bank), otp_mask);

	return BSEC_OK;
}

/*
 * bsec_set_sp_lock: set shadow-program lock.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_set_sp_lock(uint32_t otp)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	return bsec_lock_register_set(BSEC_SPLOCK(bank), otp_mask);
}

/*
 * bsec_read_sp_lock: read shadow-program lock.
 * otp: OTP number.
 * value: read value (true or false).
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_read_sp_lock(uint32_t otp, bool *value)
{
	uint32_t bank = otp_bank(otp);
	uint32_t otp_mask = otp_bit_mask(otp);

	assert(value != NULL);
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	*value = bsec_lock_register_get(BSEC_SPLOCK(bank), otp_mask);

	return BSEC_OK;
}

/*
 * bsec_get_secure_state: read state in BSEC status register.
 * return: secure state
 */
uint32_t bsec_get_secure_state(void)
{
	uint32_t state = BSEC_STATE_INVALID;
	uint32_t status = bsec_get_status();
	uint32_t bsec_sr = mmio_read_32(BSEC_BASE + BSEC_SR);

	if ((status & BSEC_OTPSR_FUSEOK) == BSEC_OTPSR_FUSEOK) {
		/* NVSTATE is only valid if FUSEOK */
		uint32_t nvstates = (bsec_sr & BSEC_SR_NVSTATE_MASK) >> BSEC_SR_NVSTATE_SHIFT;

		if (nvstates == BSEC_SR_NVSTATE_OPEN) {
			state = BSEC_STATE_SEC_OPEN;
		} else if (nvstates == BSEC_SR_NVSTATE_CLOSED) {
			state = BSEC_STATE_SEC_CLOSED;
		} else {
			VERBOSE("%s nvstates = %u\n", __func__, nvstates);
		}
	}

	return state;
}

/*
 * bsec_shadow_read_otp: Load OTP from SAFMEM and provide its value
 * val: read value.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_shadow_read_otp(uint32_t *val, uint32_t otp)
{
	assert(val != NULL);
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	*val = 0U;

	if (is_bsec_write_locked()) {
		return BSEC_WRITE_LOCKED;
	}

	if (!is_fuse_shadowed(otp)) {
		uint32_t result = bsec_shadow_register(otp);

		if (result != BSEC_OK) {
			ERROR("BSEC: %u Shadowing Error %u\n", otp, result);
			return result;
		}
	}

	*val = mmio_read_32(BSEC_BASE + BSEC_FVR(otp));

	return BSEC_OK;
}

/*
 * bsec_read_otp: read an OTP data value.
 * val: read value.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_read_otp(uint32_t *val, uint32_t otp)
{
	assert(val != NULL);
	if (otp > STM32MP2_OTP_MAX_ID) {
		panic();
	}

	return bsec_shadow_read_otp(val, otp);
}
