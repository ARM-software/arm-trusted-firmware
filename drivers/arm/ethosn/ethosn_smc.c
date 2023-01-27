/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
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

#include <platform_def.h>

#if ARM_ETHOSN_NPU_TZMP1
#include "ethosn_big_fw.h"
#endif

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
/* Set bit[10] = 1 to workaround erratum 2838783 */
#define SEC_SECCTLR_VAL			U(0x403)

#define SEC_DEL_ADDR_EXT_REG		U(0x201C)
#define SEC_DEL_ADDR_EXT_VAL		U(0x15)

#define SEC_SYSCTRL0_REG		U(0x0018)
#define SEC_SYSCTRL0_SLEEPING		U(1U << 4)
#define SEC_SYSCTRL0_SOFT_RESET		U(3U << 29)
#define SEC_SYSCTRL0_HARD_RESET		U(1U << 31)

#define SEC_NSAID_REG_BASE		U(0x3004)
#define SEC_NSAID_OFFSET		U(0x1000)

#define SEC_MMUSID_REG_BASE		U(0x3008)
#define SEC_MMUSID_OFFSET		U(0x1000)

#define SEC_NPU_ID_REG			U(0xF000)
#define SEC_NPU_ID_ARCH_VER_SHIFT	U(0X10)

#define INPUT_STREAM_INDEX              U(0x6)
#define INTERMEDIATE_STREAM_INDEX       U(0x7)
#define OUTPUT_STREAM_INDEX             U(0x8)

#if ARM_ETHOSN_NPU_TZMP1
CASSERT(ARM_ETHOSN_NPU_FW_IMAGE_BASE > 0U, assert_ethosn_invalid_fw_image_base);
static const struct ethosn_big_fw *big_fw;
#endif

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

#if ARM_ETHOSN_NPU_TZMP1
static uint32_t ethosn_core_read_arch_version(uintptr_t core_addr)
{
	uint32_t npu_id = mmio_read_32(ETHOSN_CORE_SEC_REG(core_addr,
							   SEC_NPU_ID_REG));

	return (npu_id >> SEC_NPU_ID_ARCH_VER_SHIFT);
}

static void ethosn_configure_stream_nsaid(const struct ethosn_core_t *core,
					  bool is_protected)
{
	size_t i;
	uint32_t streams[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (is_protected) {
		streams[INPUT_STREAM_INDEX] = ARM_ETHOSN_NPU_PROT_DATA_NSAID;
		streams[INTERMEDIATE_STREAM_INDEX] =
			ARM_ETHOSN_NPU_PROT_DATA_NSAID;
		streams[OUTPUT_STREAM_INDEX] = ARM_ETHOSN_NPU_PROT_DATA_NSAID;
	}

	for (i = 0U; i < ARRAY_SIZE(streams); ++i) {
		const uintptr_t reg_addr = SEC_NSAID_REG_BASE +
			(SEC_NSAID_OFFSET * i);
		mmio_write_32(ETHOSN_CORE_SEC_REG(core->addr, reg_addr),
			      streams[i]);
	}
}
#endif

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

static int ethosn_core_is_sleeping(uintptr_t core_addr)
{
	const uintptr_t sysctrl0_reg =
		ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL0_REG);
	const uint32_t sleeping_mask = SEC_SYSCTRL0_SLEEPING;

	return ((mmio_read_32(sysctrl0_reg) & sleeping_mask) == sleeping_mask);
}

static bool ethosn_core_reset(uintptr_t core_addr, bool hard_reset)
{
	unsigned int timeout;
	const uintptr_t sysctrl0_reg =
		ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL0_REG);
	const uint32_t reset_val = hard_reset ? SEC_SYSCTRL0_HARD_RESET :
						SEC_SYSCTRL0_SOFT_RESET;

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

static int ethosn_core_full_reset(const struct ethosn_device_t *device,
				  const struct ethosn_core_t *core,
				  bool hard_reset,
				  u_register_t asset_alloc_idx,
				  u_register_t is_protected)
{
	if (!device->has_reserved_memory &&
	    asset_alloc_idx >= device->num_allocators) {
		WARN("ETHOSN: Unknown asset allocator index given to SMC call.\n");
		return ETHOSN_UNKNOWN_ALLOCATOR_IDX;
	}

	if (!ethosn_core_reset(core->addr, hard_reset)) {
		return ETHOSN_FAILURE;
	}

	if (!device->has_reserved_memory) {
		ethosn_configure_smmu_streams(device, core, asset_alloc_idx);

#if ARM_ETHOSN_NPU_TZMP1
		ethosn_configure_stream_nsaid(core, is_protected);
#endif
	}

	ethosn_delegate_to_ns(core->addr);

	return ETHOSN_SUCCESS;
}

static uintptr_t ethosn_smc_core_reset_handler(const struct ethosn_device_t *device,
					       const struct ethosn_core_t *core,
					       bool hard_reset,
					       u_register_t asset_alloc_idx,
					       u_register_t reset_type,
					       u_register_t is_protected,
					       void *handle)
{
	int ret;

	switch (reset_type) {
	case ETHOSN_RESET_TYPE_FULL:
		ret = ethosn_core_full_reset(device, core, hard_reset,
					     asset_alloc_idx, is_protected);
		break;
	case ETHOSN_RESET_TYPE_HALT:
		ret = ethosn_core_reset(core->addr, hard_reset) ? ETHOSN_SUCCESS : ETHOSN_FAILURE;
		break;
	default:
		WARN("ETHOSN: Invalid reset type given to SMC call.\n");
		ret = ETHOSN_INVALID_PARAMETER;
		break;
	}

	SMC_RET1(handle, ret);
}

static uintptr_t ethosn_smc_core_handler(uint32_t fid,
					 u_register_t core_addr,
					 u_register_t asset_alloc_idx,
					 u_register_t reset_type,
					 u_register_t is_protected,
					 void *handle)
{
	bool hard_reset = false;
	const struct ethosn_device_t *device = NULL;
	const struct ethosn_core_t *core = NULL;

	if (!ethosn_get_device_and_core(core_addr, &device, &core))  {
		SMC_RET1(handle, ETHOSN_UNKNOWN_CORE_ADDRESS);
	}

	switch (fid) {
	case ETHOSN_FNUM_IS_SEC:
		SMC_RET1(handle, ethosn_is_sec(core->addr));
	case ETHOSN_FNUM_IS_SLEEPING:
		SMC_RET1(handle, ethosn_core_is_sleeping(core->addr));
	case ETHOSN_FNUM_HARD_RESET:
		hard_reset = true;
		/* Fallthrough */
	case ETHOSN_FNUM_SOFT_RESET:
		return ethosn_smc_core_reset_handler(device, core,
						     hard_reset,
						     asset_alloc_idx,
						     reset_type,
						     is_protected,
						     handle);
	default:
		WARN("ETHOSN: Unimplemented SMC call: 0x%x\n", fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

uintptr_t ethosn_smc_handler(uint32_t smc_fid,
			     u_register_t x1,
			     u_register_t x2,
			     u_register_t x3,
			     u_register_t x4,
			     void *cookie,
			     void *handle,
			     u_register_t flags)
{
	const uint32_t fid = smc_fid & FUNCID_NUM_MASK;

	/* Only SiP fast calls are expected */
	if ((GET_SMC_TYPE(smc_fid) != SMC_TYPE_FAST) ||
		(GET_SMC_OEN(smc_fid) != OEN_SIP_START)) {
		SMC_RET1(handle, SMC_UNK);
	}

	/* Truncate parameters to 32-bits for SMC32 */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		x1 &= 0xFFFFFFFF;
		x2 &= 0xFFFFFFFF;
		x3 &= 0xFFFFFFFF;
		x4 &= 0xFFFFFFFF;
	}

	if (!is_ethosn_fid(smc_fid) || (fid > ETHOSN_FNUM_IS_SLEEPING)) {
		WARN("ETHOSN: Unknown SMC call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	if (fid == ETHOSN_FNUM_VERSION) {
		SMC_RET2(handle, ETHOSN_VERSION_MAJOR, ETHOSN_VERSION_MINOR);
	}

	return ethosn_smc_core_handler(fid, x1, x2, x3, x4, handle);
}

int ethosn_smc_setup(void)
{
#if ARM_ETHOSN_NPU_TZMP1
	struct ethosn_device_t *dev;
	uint32_t arch_ver;
#endif

	if (ETHOSN_NUM_DEVICES == 0U) {
		ERROR("ETHOSN: No NPU found\n");
		return ETHOSN_FAILURE;
	}

#if ARM_ETHOSN_NPU_TZMP1

	/* Only one NPU core is supported in the TZMP1 setup */
	if ((ETHOSN_NUM_DEVICES != 1U) ||
	    (ETHOSN_GET_DEVICE(0U)->num_cores != 1U)) {
		ERROR("ETHOSN: TZMP1 doesn't support multiple NPU cores\n");
		return ETHOSN_FAILURE;
	}

	dev = ETHOSN_GET_DEVICE(0U);
	arch_ver = ethosn_core_read_arch_version(dev->cores[0U].addr);
	big_fw = (struct ethosn_big_fw *)ARM_ETHOSN_NPU_FW_IMAGE_BASE;

	if (!ethosn_big_fw_verify_header(big_fw, arch_ver)) {
		return ETHOSN_FAILURE;
	}

	NOTICE("ETHOSN: TZMP1 setup succeeded with firmware version %u.%u.%u\n",
	       big_fw->fw_ver_major, big_fw->fw_ver_minor,
	       big_fw->fw_ver_patch);
#else
	NOTICE("ETHOSN: Setup succeeded\n");
#endif

	return 0;
}
