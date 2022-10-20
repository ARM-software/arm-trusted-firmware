/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
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
#include <lib/utils_def.h>
#include <plat/arm/common/fconf_ethosn_getter.h>

/*
 * Number of Arm(R) Ethos(TM)-N NPU (NPU) devices available
 */
#define ETHOSN_NUM_DEVICES \
	FCONF_GET_PROPERTY(hw_config, ethosn_config, num_devices)

#define ETHOSN_GET_DEVICE(dev_idx) \
	FCONF_GET_PROPERTY(hw_config, ethosn_device, dev_idx)

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

#define SEC_DEL_ADDR_EXT_REG		U(0x201C)
#define SEC_DEL_ADDR_EXT_VAL		U(0x15)

#define SEC_SYSCTRL0_REG		U(0x0018)
#define SEC_SYSCTRL0_SOFT_RESET		U(3U << 29)
#define SEC_SYSCTRL0_HARD_RESET		U(1U << 31)

#define SEC_MMUSID_REG_BASE		U(0x3008)
#define SEC_MMUSID_OFFSET		U(0x1000)

static bool ethosn_get_device_and_core(uintptr_t core_addr,
				       const struct ethosn_device_t **dev_match,
				       const struct ethosn_core_t **core_match)
{
	uint32_t dev_idx;
	uint32_t core_idx;

	for (dev_idx = 0U; dev_idx < ETHOSN_NUM_DEVICES; ++dev_idx) {
		const struct ethosn_device_t *dev = ETHOSN_GET_DEVICE(dev_idx);

		for (core_idx = 0U; core_idx < dev->num_cores; ++core_idx) {
			const struct ethosn_core_t *core = &(dev->cores[core_idx]);

			if (core->addr == core_addr) {
				*dev_match = dev;
				*core_match = core;
				return true;
			}
		}
	}

	WARN("ETHOSN: Unknown core address given to SMC call.\n");
	return false;
}

static void ethosn_configure_smmu_streams(const struct ethosn_device_t *device,
					  const struct ethosn_core_t *core,
					  uint32_t asset_alloc_idx)
{
	const struct ethosn_main_allocator_t *main_alloc =
		&(core->main_allocator);
	const struct ethosn_asset_allocator_t *asset_alloc =
		&(device->asset_allocators[asset_alloc_idx]);
	const uint32_t streams[9] = {
		main_alloc->firmware.stream_id,
		main_alloc->working_data.stream_id,
		asset_alloc->command_stream.stream_id,
		0U, /* Not used*/
		main_alloc->firmware.stream_id,
		asset_alloc->weight_data.stream_id,
		asset_alloc->buffer_data.stream_id,
		asset_alloc->intermediate_data.stream_id,
		asset_alloc->buffer_data.stream_id
	};
	size_t i;

	for (i = 0U; i < ARRAY_SIZE(streams); ++i) {
		const uintptr_t reg_addr = SEC_MMUSID_REG_BASE +
			(SEC_MMUSID_OFFSET * i);
		mmio_write_32(ETHOSN_CORE_SEC_REG(core->addr, reg_addr),
			      streams[i]);
	}
}

static void ethosn_delegate_to_ns(uintptr_t core_addr)
{
	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_SECCTLR_REG),
			SEC_SECCTLR_VAL);

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_REG),
			SEC_DEL_VAL);

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_ADDR_EXT_REG),
			SEC_DEL_ADDR_EXT_VAL);
}

static int ethosn_is_sec(uintptr_t core_addr)
{
	if ((mmio_read_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_DEL_REG))
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
			     u_register_t core_addr,
			     u_register_t asset_alloc_idx,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	int hard_reset = 0;
	const struct ethosn_device_t *device = NULL;
	const struct ethosn_core_t *core = NULL;
	const uint32_t fid = smc_fid & FUNCID_NUM_MASK;

	/* Only SiP fast calls are expected */
	if ((GET_SMC_TYPE(smc_fid) != SMC_TYPE_FAST) ||
		(GET_SMC_OEN(smc_fid) != OEN_SIP_START)) {
		SMC_RET1(handle, SMC_UNK);
	}

	/* Truncate parameters to 32-bits for SMC32 */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		core_addr &= 0xFFFFFFFF;
		asset_alloc_idx &= 0xFFFFFFFF;
		x3 &= 0xFFFFFFFF;
		x4 &= 0xFFFFFFFF;
	}

	if (!is_ethosn_fid(smc_fid) ||
	    (fid < ETHOSN_FNUM_VERSION || fid > ETHOSN_FNUM_SOFT_RESET)) {
		WARN("ETHOSN: Unknown SMC call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	/* Commands that do not require a valid core address */
	switch (fid) {
	case ETHOSN_FNUM_VERSION:
		SMC_RET2(handle, ETHOSN_VERSION_MAJOR, ETHOSN_VERSION_MINOR);
	}

	if (!ethosn_get_device_and_core(core_addr, &device, &core))  {
		SMC_RET1(handle, ETHOSN_UNKNOWN_CORE_ADDRESS);
	}

	/* Commands that require a valid core address */
	switch (fid) {
	case ETHOSN_FNUM_IS_SEC:
		SMC_RET1(handle, ethosn_is_sec(core->addr));
	}

	if (!device->has_reserved_memory &&
	    asset_alloc_idx >= device->num_allocators) {
		WARN("ETHOSN: Unknown asset allocator index given to SMC call.\n");
		SMC_RET1(handle, ETHOSN_UNKNOWN_ALLOCATOR_IDX);
	}

	/* Commands that require a valid device, core and asset allocator */
	switch (fid) {
	case ETHOSN_FNUM_HARD_RESET:
		hard_reset = 1;
		/* Fallthrough */
	case ETHOSN_FNUM_SOFT_RESET:
		if (!ethosn_reset(core->addr, hard_reset)) {
			SMC_RET1(handle, ETHOSN_FAILURE);
		}

		if (!device->has_reserved_memory) {
			ethosn_configure_smmu_streams(device, core,
						      asset_alloc_idx);
		}

		ethosn_delegate_to_ns(core->addr);
		SMC_RET1(handle, ETHOSN_SUCCESS);
	default:
		WARN("ETHOSN: Unimplemented SMC call: 0x%x\n", fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
