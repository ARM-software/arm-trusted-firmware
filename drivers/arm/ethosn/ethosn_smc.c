/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>

#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/arm/ethosn.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/arm/common/fconf_ethosn_getter.h>

/* Arm Ethos-N NPU (NPU) status */
#define ETHOSN_STATUS \
	FCONF_GET_PROPERTY(hw_config, ethosn_config, status)

/* Number of NPU cores available */
#define ETHOSN_NUM_CORES \
	FCONF_GET_PROPERTY(hw_config, ethosn_config, num_cores)

/* Address to an NPU core  */
#define ETHOSN_CORE_ADDR(core_idx) \
	FCONF_GET_PROPERTY(hw_config, ethosn_core_addr, core_idx)

/* NPU core sec registry address */
#define ETHOSN_CORE_SEC_REG(core_addr, reg_offset) \
	(core_addr + reg_offset)

/* Reset timeout in us */
#define ETHOSN_RESET_TIMEOUT_US		U(10 * 1000 * 1000)
#define ETHOSN_RESET_WAIT_US		U(1)

#define SEC_DEL_REG			U(0x0004)
#define SEC_DEL_VAL			U(0x81C)
#define SEC_DEL_EXCC_MASK		U(0x20)

#define SEC_SECCTLR_REG			U(0x0010)
#define SEC_SECCTLR_VAL			U(0x3)

#define SEC_DEL_MMUSID_REG		U(0x2008)
#define SEC_DEL_MMUSID_VAL		U(0x3FFFF)

#define SEC_DEL_ADDR_EXT_REG		U(0x201C)
#define SEC_DEL_ADDR_EXT_VAL		U(0x15)

#define SEC_SYSCTRL0_REG		U(0x0018)
#define SEC_SYSCTRL0_SOFT_RESET		U(3U << 29)
#define SEC_SYSCTRL0_HARD_RESET		U(1U << 31)

static void ethosn_delegate_to_ns(uintptr_t core_addr)
{
	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_SECCTLR_REG),
			SEC_SECCTLR_VAL);

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_REG),
			SEC_DEL_VAL);

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_MMUSID_REG),
			SEC_DEL_MMUSID_VAL);

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_ADDR_EXT_REG),
			SEC_DEL_ADDR_EXT_VAL);
}

static int ethosn_is_sec(void)
{
	if ((mmio_read_32(ETHOSN_CORE_SEC_REG(ETHOSN_CORE_ADDR(0), SEC_DEL_REG))
		& SEC_DEL_EXCC_MASK) != 0U) {
		return 0;
	}

	return 1;
}

static bool ethosn_reset(uintptr_t core_addr, int hard_reset)
{
	unsigned int timeout;
	const uintptr_t sysctrl0_reg =
		ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL0_REG);
	const uint32_t reset_val = (hard_reset != 0) ? SEC_SYSCTRL0_HARD_RESET
						    : SEC_SYSCTRL0_SOFT_RESET;

	mmio_write_32(sysctrl0_reg, reset_val);

	/* Wait for reset to complete */
	for (timeout = 0U; timeout < ETHOSN_RESET_TIMEOUT_US;
			   timeout += ETHOSN_RESET_WAIT_US) {

		if ((mmio_read_32(sysctrl0_reg) & reset_val) == 0U) {
			break;
		}

		udelay(ETHOSN_RESET_WAIT_US);
	}

	return timeout < ETHOSN_RESET_TIMEOUT_US;
}

uintptr_t ethosn_smc_handler(uint32_t smc_fid,
			     u_register_t core_idx,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	uintptr_t core_addr;
	int hard_reset = 0;

	/* Only SiP fast calls are expected */
	if ((GET_SMC_TYPE(smc_fid) != SMC_TYPE_FAST) ||
		(GET_SMC_OEN(smc_fid) != OEN_SIP_START)) {
		SMC_RET1(handle, SMC_UNK);
	}

	/* Truncate parameters to 32-bits for SMC32 */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		core_idx &= 0xFFFFFFFF;
		x2 &= 0xFFFFFFFF;
		x3 &= 0xFFFFFFFF;
		x4 &= 0xFFFFFFFF;
	}

	if (!is_ethosn_fid(smc_fid)) {
		SMC_RET1(handle, SMC_UNK);
	}

	if (ETHOSN_STATUS == ETHOSN_STATUS_DISABLED) {
		WARN("ETHOSN: Arm Ethos-N NPU not available\n");
		SMC_RET1(handle, ETHOSN_NOT_SUPPORTED);
	}

	switch (smc_fid & FUNCID_NUM_MASK) {
	case ETHOSN_FNUM_VERSION:
		SMC_RET2(handle, ETHOSN_VERSION_MAJOR, ETHOSN_VERSION_MINOR);
	case ETHOSN_FNUM_IS_SEC:
		SMC_RET1(handle, ethosn_is_sec());
	case ETHOSN_FNUM_HARD_RESET:
		hard_reset = 1;
		/* Fallthrough */
	case ETHOSN_FNUM_SOFT_RESET:
		if (core_idx >= ETHOSN_NUM_CORES) {
			WARN("ETHOSN: core index out of range\n");
			SMC_RET1(handle, ETHOSN_CORE_IDX_OUT_OF_RANGE);
		}

		core_addr = ETHOSN_CORE_ADDR(core_idx);

		if (!ethosn_reset(core_addr, hard_reset)) {
			SMC_RET1(handle, ETHOSN_FAILURE);
		}

		ethosn_delegate_to_ns(core_addr);

		SMC_RET1(handle, ETHOSN_SUCCESS);
	default:
		SMC_RET1(handle, SMC_UNK);
	}
}
