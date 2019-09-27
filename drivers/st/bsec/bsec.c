/*
 * Copyright (c) 2017-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <limits.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#define BSEC_IP_VERSION_1_0	0x10
#define BSEC_COMPAT		"st,stm32mp15-bsec"

#define OTP_ACCESS_SIZE (round_up(OTP_MAX_SIZE, __WORD_BIT) / __WORD_BIT)

static uint32_t otp_nsec_access[OTP_ACCESS_SIZE] __unused;

static uint32_t bsec_power_safmem(bool power);

/* BSEC access protection */
static spinlock_t bsec_spinlock;
static uintptr_t bsec_base;

static void bsec_lock(void)
{
	if (stm32mp_lock_available()) {
		spin_lock(&bsec_spinlock);
	}
}

static void bsec_unlock(void)
{
	if (stm32mp_lock_available()) {
		spin_unlock(&bsec_spinlock);
	}
}

static int bsec_get_dt_node(struct dt_node_info *info)
{
	int node;

	node = dt_get_node(info, -1, BSEC_COMPAT);
	if (node < 0) {
		return -FDT_ERR_NOTFOUND;
	}

	return node;
}

#if defined(IMAGE_BL32)
static void enable_non_secure_access(uint32_t otp)
{
	otp_nsec_access[otp / __WORD_BIT] |= BIT(otp % __WORD_BIT);

	if (bsec_shadow_register(otp) != BSEC_OK) {
		panic();
	}
}

static bool non_secure_can_access(uint32_t otp)
{
	return (otp_nsec_access[otp / __WORD_BIT] &
		BIT(otp % __WORD_BIT)) != 0;
}

static int bsec_dt_otp_nsec_access(void *fdt, int bsec_node)
{
	int bsec_subnode;

	fdt_for_each_subnode(bsec_subnode, fdt, bsec_node) {
		const fdt32_t *cuint;
		uint32_t reg;
		uint32_t i;
		uint32_t size;
		uint8_t status;

		cuint = fdt_getprop(fdt, bsec_subnode, "reg", NULL);
		if (cuint == NULL) {
			panic();
		}

		reg = fdt32_to_cpu(*cuint) / sizeof(uint32_t);
		if (reg < STM32MP1_UPPER_OTP_START) {
			continue;
		}

		status = fdt_get_status(bsec_subnode);
		if ((status & DT_NON_SECURE) == 0U)  {
			continue;
		}

		size = fdt32_to_cpu(*(cuint + 1)) / sizeof(uint32_t);

		if ((fdt32_to_cpu(*(cuint + 1)) % sizeof(uint32_t)) != 0) {
			size++;
		}

		for (i = reg; i < (reg + size); i++) {
			enable_non_secure_access(i);
		}
	}

	return 0;
}
#endif

static uint32_t otp_bank_offset(uint32_t otp)
{
	assert(otp <= STM32MP1_OTP_MAX_ID);

	return ((otp & ~BSEC_OTP_MASK) >> BSEC_OTP_BANK_SHIFT) *
	       sizeof(uint32_t);
}

static uint32_t bsec_check_error(uint32_t otp)
{
	uint32_t bit = BIT(otp & BSEC_OTP_MASK);
	uint32_t bank = otp_bank_offset(otp);

	if ((mmio_read_32(bsec_base + BSEC_DISTURBED_OFF + bank) & bit) != 0U) {
		return BSEC_DISTURBED;
	}

	if ((mmio_read_32(bsec_base + BSEC_ERROR_OFF + bank) & bit) != 0U) {
		return BSEC_ERROR;
	}

	return BSEC_OK;
}

/*
 * bsec_probe: initialize BSEC driver.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_probe(void)
{
	void *fdt;
	int node;
	struct dt_node_info bsec_info;

	if (fdt_get_address(&fdt) == 0) {
		panic();
	}

	node = bsec_get_dt_node(&bsec_info);
	if (node < 0) {
		panic();
	}

	bsec_base = bsec_info.base;

#if defined(IMAGE_BL32)
	bsec_dt_otp_nsec_access(fdt, node);
#endif
	return BSEC_OK;
}

/*
 * bsec_get_base: return BSEC base address.
 */
uint32_t bsec_get_base(void)
{
	return bsec_base;
}

/*
 * bsec_set_config: enable and configure BSEC.
 * cfg: pointer to param structure used to set register.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_set_config(struct bsec_config *cfg)
{
	uint32_t value;
	int32_t result;

	value = ((((uint32_t)cfg->freq << BSEC_CONF_FRQ_SHIFT) &
						BSEC_CONF_FRQ_MASK) |
		 (((uint32_t)cfg->pulse_width << BSEC_CONF_PRG_WIDTH_SHIFT) &
						BSEC_CONF_PRG_WIDTH_MASK) |
		 (((uint32_t)cfg->tread << BSEC_CONF_TREAD_SHIFT) &
						BSEC_CONF_TREAD_MASK));

	bsec_lock();

	mmio_write_32(bsec_base + BSEC_OTP_CONF_OFF, value);

	bsec_unlock();

	result = bsec_power_safmem((bool)cfg->power &
				   BSEC_CONF_POWER_UP_MASK);
	if (result != BSEC_OK) {
		return result;
	}

	value = ((((uint32_t)cfg->upper_otp_lock << UPPER_OTP_LOCK_SHIFT) &
						UPPER_OTP_LOCK_MASK) |
		 (((uint32_t)cfg->den_lock << DENREG_LOCK_SHIFT) &
						DENREG_LOCK_MASK) |
		 (((uint32_t)cfg->prog_lock << GPLOCK_LOCK_SHIFT) &
						GPLOCK_LOCK_MASK));

	bsec_lock();

	mmio_write_32(bsec_base + BSEC_OTP_LOCK_OFF, value);

	bsec_unlock();

	return BSEC_OK;
}

/*
 * bsec_get_config: return config parameters set in BSEC registers.
 * cfg: config param return.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_get_config(struct bsec_config *cfg)
{
	uint32_t value;

	if (cfg == NULL) {
		return BSEC_INVALID_PARAM;
	}

	value = mmio_read_32(bsec_base + BSEC_OTP_CONF_OFF);
	cfg->power = (uint8_t)((value & BSEC_CONF_POWER_UP_MASK) >>
						BSEC_CONF_POWER_UP_SHIFT);
	cfg->freq = (uint8_t)((value & BSEC_CONF_FRQ_MASK) >>
						BSEC_CONF_FRQ_SHIFT);
	cfg->pulse_width = (uint8_t)((value & BSEC_CONF_PRG_WIDTH_MASK) >>
						BSEC_CONF_PRG_WIDTH_SHIFT);
	cfg->tread = (uint8_t)((value & BSEC_CONF_TREAD_MASK) >>
						BSEC_CONF_TREAD_SHIFT);

	value = mmio_read_32(bsec_base + BSEC_OTP_LOCK_OFF);
	cfg->upper_otp_lock = (uint8_t)((value & UPPER_OTP_LOCK_MASK) >>
						UPPER_OTP_LOCK_SHIFT);
	cfg->den_lock = (uint8_t)((value & DENREG_LOCK_MASK) >>
						DENREG_LOCK_SHIFT);
	cfg->prog_lock = (uint8_t)((value & GPLOCK_LOCK_MASK) >>
						GPLOCK_LOCK_SHIFT);

	return BSEC_OK;
}

/*
 * bsec_shadow_register: copy SAFMEM OTP to BSEC data.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_shadow_register(uint32_t otp)
{
	uint32_t result;
	bool power_up = false;

	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	/* Check if shadowing of OTP is locked */
	if (bsec_read_sr_lock(otp)) {
		VERBOSE("BSEC: OTP %i is locked and will not be refreshed\n",
			otp);
	}

	if ((bsec_get_status() & BSEC_MODE_PWR_MASK) == 0U) {
		result = bsec_power_safmem(true);

		if (result != BSEC_OK) {
			return result;
		}

		power_up = true;
	}

	bsec_lock();

	/* Set BSEC_OTP_CTRL_OFF and set ADDR with the OTP value */
	mmio_write_32(bsec_base + BSEC_OTP_CTRL_OFF, otp | BSEC_READ);

	while ((bsec_get_status() & BSEC_MODE_BUSY_MASK) != 0U) {
		;
	}

	result = bsec_check_error(otp);

	bsec_unlock();

	if (power_up) {
		if (bsec_power_safmem(false) != BSEC_OK) {
			panic();
		}
	}

	return result;
}

/*
 * bsec_read_otp: read an OTP data value.
 * val: read value.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_read_otp(uint32_t *val, uint32_t otp)
{
	uint32_t result;

	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	bsec_lock();

	*val = mmio_read_32(bsec_base + BSEC_OTP_DATA_OFF +
			    (otp * sizeof(uint32_t)));

	result = bsec_check_error(otp);

	bsec_unlock();

	return result;
}

/*
 * bsec_write_otp: write value in BSEC data register.
 * val: value to write.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_write_otp(uint32_t val, uint32_t otp)
{
	uint32_t result;

	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	/* Check if programming of OTP is locked */
	if (bsec_read_sw_lock(otp)) {
		VERBOSE("BSEC: OTP %i is locked and write will be ignored\n",
			otp);
	}

	bsec_lock();

	mmio_write_32(bsec_base + BSEC_OTP_DATA_OFF +
		      (otp * sizeof(uint32_t)), val);

	result = bsec_check_error(otp);

	bsec_unlock();

	return result;
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
	bool power_up = false;

	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	/* Check if programming of OTP is locked */
	if (bsec_read_sp_lock(otp)) {
		WARN("BSEC: OTP locked, prog will be ignored\n");
	}

	if ((mmio_read_32(bsec_base + BSEC_OTP_LOCK_OFF) &
	     BIT(BSEC_LOCK_PROGRAM)) != 0U) {
		WARN("BSEC: GPLOCK activated, prog will be ignored\n");
	}

	if ((bsec_get_status() & BSEC_MODE_PWR_MASK) == 0U) {
		result = bsec_power_safmem(true);

		if (result != BSEC_OK) {
			return result;
		}

		power_up = true;
	}

	bsec_lock();

	/* Set value in write register */
	mmio_write_32(bsec_base + BSEC_OTP_WRDATA_OFF, val);

	/* Set BSEC_OTP_CTRL_OFF and set ADDR with the OTP value */
	mmio_write_32(bsec_base + BSEC_OTP_CTRL_OFF, otp | BSEC_WRITE);

	while ((bsec_get_status() & BSEC_MODE_BUSY_MASK) != 0U) {
		;
	}

	if ((bsec_get_status() & BSEC_MODE_PROGFAIL_MASK) != 0U) {
		result = BSEC_PROG_FAIL;
	} else {
		result = bsec_check_error(otp);
	}

	bsec_unlock();

	if (power_up) {
		if (bsec_power_safmem(false) != BSEC_OK) {
			panic();
		}
	}

	return result;
}

/*
 * bsec_permanent_lock_otp: permanent lock of OTP in SAFMEM.
 * otp: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_permanent_lock_otp(uint32_t otp)
{
	uint32_t result;
	bool power_up = false;
	uint32_t data;
	uint32_t addr;

	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	if ((bsec_get_status() & BSEC_MODE_PWR_MASK) == 0U) {
		result = bsec_power_safmem(true);

		if (result != BSEC_OK) {
			return result;
		}

		power_up = true;
	}

	if (otp < STM32MP1_UPPER_OTP_START) {
		addr = otp >> ADDR_LOWER_OTP_PERLOCK_SHIFT;
		data = DATA_LOWER_OTP_PERLOCK_BIT <<
		       ((otp & DATA_LOWER_OTP_PERLOCK_MASK) << 1U);
	} else {
		addr = (otp >> ADDR_UPPER_OTP_PERLOCK_SHIFT) + 2U;
		data = DATA_UPPER_OTP_PERLOCK_BIT <<
		       (otp & DATA_UPPER_OTP_PERLOCK_MASK);
	}

	bsec_lock();

	/* Set value in write register */
	mmio_write_32(bsec_base + BSEC_OTP_WRDATA_OFF, data);

	/* Set BSEC_OTP_CTRL_OFF and set ADDR with the OTP value */
	mmio_write_32(bsec_base + BSEC_OTP_CTRL_OFF,
		      addr | BSEC_WRITE | BSEC_LOCK);

	while ((bsec_get_status() & BSEC_MODE_BUSY_MASK) != 0U) {
		;
	}

	if ((bsec_get_status() & BSEC_MODE_PROGFAIL_MASK) != 0U) {
		result = BSEC_PROG_FAIL;
	} else {
		result = bsec_check_error(otp);
	}

	bsec_unlock();

	if (power_up) {
		if (bsec_power_safmem(false) != BSEC_OK) {
			panic();
		}
	}

	return result;
}

/*
 * bsec_write_debug_conf: write value in debug feature
 *	to enable/disable debug service.
 * val: value to write.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_write_debug_conf(uint32_t val)
{
	uint32_t result = BSEC_ERROR;
	uint32_t masked_val = val & BSEC_DEN_ALL_MSK;

	bsec_lock();

	mmio_write_32(bsec_base + BSEC_DEN_OFF, masked_val);

	if ((mmio_read_32(bsec_base + BSEC_DEN_OFF) ^ masked_val) == 0U) {
		result = BSEC_OK;
	}

	bsec_unlock();

	return result;
}

/*
 * bsec_read_debug_conf: read debug configuration.
 */
uint32_t bsec_read_debug_conf(void)
{
	return mmio_read_32(bsec_base + BSEC_DEN_OFF);
}

/*
 * bsec_get_status: return status register value.
 */
uint32_t bsec_get_status(void)
{
	return mmio_read_32(bsec_base + BSEC_OTP_STATUS_OFF);
}

/*
 * bsec_get_hw_conf: return hardware configuration.
 */
uint32_t bsec_get_hw_conf(void)
{
	return mmio_read_32(bsec_base + BSEC_IPHW_CFG_OFF);
}

/*
 * bsec_get_version: return BSEC version.
 */
uint32_t bsec_get_version(void)
{
	return mmio_read_32(bsec_base + BSEC_IPVR_OFF);
}

/*
 * bsec_get_id: return BSEC ID.
 */
uint32_t bsec_get_id(void)
{
	return mmio_read_32(bsec_base + BSEC_IP_ID_OFF);
}

/*
 * bsec_get_magic_id: return BSEC magic number.
 */
uint32_t bsec_get_magic_id(void)
{
	return mmio_read_32(bsec_base + BSEC_IP_MAGIC_ID_OFF);
}

/*
 * bsec_write_sr_lock: write shadow-read lock.
 * otp: OTP number.
 * value: value to write in the register.
 *	Must be always 1.
 * return: true if OTP is locked, else false.
 */
bool bsec_write_sr_lock(uint32_t otp, uint32_t value)
{
	bool result = false;
	uint32_t bank = otp_bank_offset(otp);
	uint32_t bank_value;
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);

	bsec_lock();

	bank_value = mmio_read_32(bsec_base + BSEC_SRLOCK_OFF + bank);

	if ((bank_value & otp_mask) == value) {
		/*
		 * In case of write don't need to write,
		 * the lock is already set.
		 */
		if (value != 0U) {
			result = true;
		}
	} else {
		if (value != 0U) {
			bank_value = bank_value | otp_mask;
		} else {
			bank_value = bank_value & ~otp_mask;
		}

		/*
		 * We can write 0 in all other OTP
		 * if the lock is activated in one of other OTP.
		 * Write 0 has no effect.
		 */
		mmio_write_32(bsec_base + BSEC_SRLOCK_OFF + bank, bank_value);
		result = true;
	}

	bsec_unlock();

	return result;
}

/*
 * bsec_read_sr_lock: read shadow-read lock.
 * otp: OTP number.
 * return: true if otp is locked, else false.
 */
bool bsec_read_sr_lock(uint32_t otp)
{
	uint32_t bank = otp_bank_offset(otp);
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);
	uint32_t bank_value = mmio_read_32(bsec_base + BSEC_SRLOCK_OFF + bank);

	return (bank_value & otp_mask) != 0U;
}

/*
 * bsec_write_sw_lock: write shadow-write lock.
 * otp: OTP number.
 * value: Value to write in the register.
 *	Must be always 1.
 * return: true if OTP is locked, else false.
 */
bool bsec_write_sw_lock(uint32_t otp, uint32_t value)
{
	bool result = false;
	uint32_t bank = otp_bank_offset(otp);
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);
	uint32_t bank_value;

	bsec_lock();

	bank_value = mmio_read_32(bsec_base + BSEC_SWLOCK_OFF + bank);

	if ((bank_value & otp_mask) == value) {
		/*
		 * In case of write don't need to write,
		 * the lock is already set.
		 */
		if (value != 0U) {
			result = true;
		}
	} else {
		if (value != 0U) {
			bank_value = bank_value | otp_mask;
		} else {
			bank_value = bank_value & ~otp_mask;
		}

		/*
		 * We can write 0 in all other OTP
		 * if the lock is activated in one of other OTP.
		 * Write 0 has no effect.
		 */
		mmio_write_32(bsec_base + BSEC_SWLOCK_OFF + bank, bank_value);
		result = true;
	}

	bsec_unlock();

	return result;
}

/*
 * bsec_read_sw_lock: read shadow-write lock.
 * otp: OTP number.
 * return: true if OTP is locked, else false.
 */
bool bsec_read_sw_lock(uint32_t otp)
{
	uint32_t bank = otp_bank_offset(otp);
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);
	uint32_t bank_value = mmio_read_32(bsec_base + BSEC_SWLOCK_OFF + bank);

	return (bank_value & otp_mask) != 0U;
}

/*
 * bsec_write_sp_lock: write shadow-program lock.
 * otp: OTP number.
 * value: Value to write in the register.
 *	Must be always 1.
 * return: true if OTP is locked, else false.
 */
bool bsec_write_sp_lock(uint32_t otp, uint32_t value)
{
	bool result = false;
	uint32_t bank = otp_bank_offset(otp);
	uint32_t bank_value;
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);

	bsec_lock();

	bank_value = mmio_read_32(bsec_base + BSEC_SPLOCK_OFF + bank);

	if ((bank_value & otp_mask) == value) {
		/*
		 * In case of write don't need to write,
		 * the lock is already set.
		 */
		if (value != 0U) {
			result = true;
		}
	} else {
		if (value != 0U) {
			bank_value = bank_value | otp_mask;
		} else {
			bank_value = bank_value & ~otp_mask;
		}

		/*
		 * We can write 0 in all other OTP
		 * if the lock is activated in one of other OTP.
		 * Write 0 has no effect.
		 */
		mmio_write_32(bsec_base + BSEC_SPLOCK_OFF + bank, bank_value);
		result = true;
	}

	bsec_unlock();

	return result;
}

/*
 * bsec_read_sp_lock: read shadow-program lock.
 * otp: OTP number.
 * return: true if OTP is locked, else false.
 */
bool bsec_read_sp_lock(uint32_t otp)
{
	uint32_t bank = otp_bank_offset(otp);
	uint32_t otp_mask = BIT(otp & BSEC_OTP_MASK);
	uint32_t bank_value = mmio_read_32(bsec_base + BSEC_SPLOCK_OFF + bank);

	return (bank_value & otp_mask) != 0U;
}

/*
 * bsec_wr_lock: Read permanent lock status.
 * otp: OTP number.
 * return: true if OTP is locked, else false.
 */
bool bsec_wr_lock(uint32_t otp)
{
	uint32_t bank = otp_bank_offset(otp);
	uint32_t lock_bit = BIT(otp & BSEC_OTP_MASK);

	if ((mmio_read_32(bsec_base + BSEC_WRLOCK_OFF + bank) &
	     lock_bit) != 0U) {
		/*
		 * In case of write don't need to write,
		 * the lock is already set.
		 */
		return true;
	}

	return false;
}

/*
 * bsec_otp_lock: Lock Upper OTP or Global programming or debug enable
 * service: Service to lock see header file.
 * value: Value to write must always set to 1 (only use for debug purpose).
 * return: BSEC_OK if succeed.
 */
uint32_t bsec_otp_lock(uint32_t service, uint32_t value)
{
	uintptr_t reg = bsec_base + BSEC_OTP_LOCK_OFF;

	switch (service) {
	case BSEC_LOCK_UPPER_OTP:
		mmio_write_32(reg, value << BSEC_LOCK_UPPER_OTP);
		break;
	case BSEC_LOCK_DEBUG:
		mmio_write_32(reg, value << BSEC_LOCK_DEBUG);
		break;
	case BSEC_LOCK_PROGRAM:
		mmio_write_32(reg, value << BSEC_LOCK_PROGRAM);
		break;
	default:
		return BSEC_INVALID_PARAM;
	}

	return BSEC_OK;
}

/*
 * bsec_power_safmem: Activate or deactivate SAFMEM power.
 * power: true to power up, false to power down.
 * return: BSEC_OK if succeed.
 */
static uint32_t bsec_power_safmem(bool power)
{
	uint32_t register_val;
	uint32_t timeout = BSEC_TIMEOUT_VALUE;

	bsec_lock();

	register_val = mmio_read_32(bsec_base + BSEC_OTP_CONF_OFF);

	if (power) {
		register_val |= BSEC_CONF_POWER_UP_MASK;
	} else {
		register_val &= ~BSEC_CONF_POWER_UP_MASK;
	}

	mmio_write_32(bsec_base + BSEC_OTP_CONF_OFF, register_val);

	/* Waiting loop */
	if (power) {
		while (((bsec_get_status() & BSEC_MODE_PWR_MASK) == 0U) &&
		       (timeout != 0U)) {
			timeout--;
		}
	} else {
		while (((bsec_get_status() & BSEC_MODE_PWR_MASK) != 0U) &&
		       (timeout != 0U)) {
			timeout--;
		}
	}

	bsec_unlock();

	if (timeout == 0U) {
		return BSEC_TIMEOUT;
	}

	return BSEC_OK;
}

/*
 * bsec_shadow_read_otp: Load OTP from SAFMEM and provide its value
 * otp_value: read value.
 * word: OTP number.
 * return value: BSEC_OK if no error.
 */
uint32_t bsec_shadow_read_otp(uint32_t *otp_value, uint32_t word)
{
	uint32_t result;

	result = bsec_shadow_register(word);
	if (result != BSEC_OK) {
		ERROR("BSEC: %u Shadowing Error %i\n", word, result);
		return result;
	}

	result = bsec_read_otp(otp_value, word);
	if (result != BSEC_OK) {
		ERROR("BSEC: %u Read Error %i\n", word, result);
	}

	return result;
}

/*
 * bsec_check_nsec_access_rights: check non-secure access rights to target OTP.
 * otp: OTP number.
 * return: BSEC_OK if authorized access.
 */
uint32_t bsec_check_nsec_access_rights(uint32_t otp)
{
#if defined(IMAGE_BL32)
	if (otp > STM32MP1_OTP_MAX_ID) {
		return BSEC_INVALID_PARAM;
	}

	if (otp >= STM32MP1_UPPER_OTP_START) {
		/* Check if BSEC is in OTP-SECURED closed_device state. */
		if (stm32mp_is_closed_device()) {
			if (!non_secure_can_access(otp)) {
				return BSEC_ERROR;
			}
		}
	}
#endif

	return BSEC_OK;
}

