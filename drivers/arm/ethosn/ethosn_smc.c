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

#if ETHOSN_NPU_TZMP1
#include "ethosn_big_fw.h"
#endif /* ETHOSN_NPU_TZMP1 */

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

#define ETHOSN_FW_VA_BASE              0x20000000UL
#define ETHOSN_WORKING_DATA_VA_BASE    0x40000000UL
#define ETHOSN_COMMAND_STREAM_VA_BASE  0x60000000UL

/* Reset timeout in us */
#define ETHOSN_RESET_TIMEOUT_US		U(10 * 1000 * 1000)
#define ETHOSN_RESET_WAIT_US		U(1)

#define ETHOSN_AUX_FEAT_LEVEL_IRQ	U(0x1)
#define ETHOSN_AUX_FEAT_STASHING	U(0x2)

#define SEC_AUXCTLR_REG			U(0x0024)
#define SEC_AUXCTLR_VAL			U(0x000ce080)
#define SEC_AUXCTLR_LEVEL_IRQ_VAL	U(0x04)
#define SEC_AUXCTLR_STASHING_VAL	U(0xA5000000)

#define SEC_DEL_REG			U(0x0004)
#if ETHOSN_NPU_TZMP1
#define SEC_DEL_VAL			U(0x808)
#else
#define SEC_DEL_VAL			U(0x80C)
#endif /* ETHOSN_NPU_TZMP1 */
#define SEC_DEL_EXCC_MASK		U(0x20)

#define SEC_SECCTLR_REG			U(0x0010)
/* Set bit[10] = 1 to workaround erratum 2838783 */
#define SEC_SECCTLR_VAL			U(0x403)

#define SEC_DEL_ADDR_EXT_REG            U(0x201C)
#define SEC_DEL_ADDR_EXT_VAL            U(0x1)

#define SEC_SYSCTRL0_REG		U(0x0018)
#define SEC_SYSCTRL0_CPU_WAIT		U(1)
#define SEC_SYSCTRL0_SLEEPING		U(1U << 4)
#define SEC_SYSCTRL0_INITVTOR_MASK	U(0x1FFFFF80)
#define SEC_SYSCTRL0_SOFT_RESET		U(1U << 29)
#define SEC_SYSCTRL0_HARD_RESET		U(1U << 31)

#define SEC_SYSCTRL1_REG		U(0x001C)
#define SEC_SYSCTRL1_VAL		U(0xe0180110)

#define SEC_NSAID_REG_BASE		U(0x3004)
#define SEC_NSAID_OFFSET		U(0x1000)

#define SEC_MMUSID_REG_BASE		U(0x3008)
#define SEC_MMUSID_OFFSET		U(0x1000)

#define SEC_ADDR_EXT_REG_BASE		U(0x3018)
#define SEC_ADDR_EXT_OFFSET		U(0x1000)
#define SEC_ADDR_EXT_SHIFT		U(0x14)
#define SEC_ADDR_EXT_MASK		U(0x1FFFFE00)

#define SEC_ATTR_CTLR_REG_BASE		U(0x3010)
#define SEC_ATTR_CTLR_OFFSET		U(0x1000)
#define SEC_ATTR_CTLR_NUM		U(9)
#define SEC_ATTR_CTLR_VAL		U(0x1)

#define SEC_NPU_ID_REG			U(0xF000)
#define SEC_NPU_ID_ARCH_VER_SHIFT	U(0X10)

#define FIRMWARE_STREAM_INDEX		U(0x0)
#define WORKING_STREAM_INDEX		U(0x1)
#define PLE_STREAM_INDEX		U(0x4)
#define INPUT_STREAM_INDEX		U(0x6)
#define INTERMEDIATE_STREAM_INDEX	U(0x7)
#define OUTPUT_STREAM_INDEX		U(0x8)

#define TO_EXTEND_ADDR(addr) \
	((addr >> SEC_ADDR_EXT_SHIFT) & SEC_ADDR_EXT_MASK)

#if ETHOSN_NPU_TZMP1
CASSERT(ETHOSN_NPU_FW_IMAGE_BASE > 0U, assert_ethosn_invalid_fw_image_base);
static const struct ethosn_big_fw *big_fw;

#define FW_INITVTOR_ADDR(big_fw) \
	((ETHOSN_FW_VA_BASE + big_fw->vector_table_offset) & \
	 SEC_SYSCTRL0_INITVTOR_MASK)

#define SYSCTRL0_INITVTOR_ADDR(value) \
	(value & SEC_SYSCTRL0_INITVTOR_MASK)

#endif /* ETHOSN_NPU_TZMP1 */

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

#if ETHOSN_NPU_TZMP1
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
	uint32_t streams[9] = {[0 ... 8] = ETHOSN_NPU_NS_RO_DATA_NSAID};

	streams[FIRMWARE_STREAM_INDEX] = ETHOSN_NPU_PROT_FW_NSAID;
	streams[PLE_STREAM_INDEX] = ETHOSN_NPU_PROT_FW_NSAID;

	streams[WORKING_STREAM_INDEX] = ETHOSN_NPU_NS_RW_DATA_NSAID;

	if (is_protected) {
		streams[INPUT_STREAM_INDEX] = ETHOSN_NPU_PROT_RO_DATA_NSAID;
		streams[INTERMEDIATE_STREAM_INDEX] =
			ETHOSN_NPU_PROT_RW_DATA_NSAID;
		streams[OUTPUT_STREAM_INDEX] = ETHOSN_NPU_PROT_RW_DATA_NSAID;
	} else {
		streams[INPUT_STREAM_INDEX] = ETHOSN_NPU_NS_RO_DATA_NSAID;
		streams[INTERMEDIATE_STREAM_INDEX] =
			ETHOSN_NPU_NS_RW_DATA_NSAID;
		streams[OUTPUT_STREAM_INDEX] = ETHOSN_NPU_NS_RW_DATA_NSAID;
	}

	for (i = 0U; i < ARRAY_SIZE(streams); ++i) {
		const uintptr_t reg_addr = SEC_NSAID_REG_BASE +
			(SEC_NSAID_OFFSET * i);
		mmio_write_32(ETHOSN_CORE_SEC_REG(core->addr, reg_addr),
			      streams[i]);
	}
}

static void ethosn_configure_vector_table(uintptr_t core_addr)
{
	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL0_REG),
			FW_INITVTOR_ADDR(big_fw));
}

#endif /* ETHOSN_NPU_TZMP1 */

static void ethosn_configure_events(uintptr_t core_addr)
{
	mmio_write_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL1_REG), SEC_SYSCTRL1_VAL);
}

static bool ethosn_configure_aux_features(const struct ethosn_device_t *device,
					  uintptr_t core_addr,
					  uint32_t features)
{
	uint32_t val = SEC_AUXCTLR_VAL;

	if (features & ETHOSN_AUX_FEAT_LEVEL_IRQ) {
		val |= SEC_AUXCTLR_LEVEL_IRQ_VAL;
	}

	if (features & ETHOSN_AUX_FEAT_STASHING) {
		/* Stashing can't be used with reserved memory */
		if (device->has_reserved_memory) {
			return false;
		}

		val |= SEC_AUXCTLR_STASHING_VAL;
	}

	mmio_setbits_32(ETHOSN_CORE_SEC_REG(core_addr, SEC_AUXCTLR_REG), val);

	return true;
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

static void ethosn_configure_stream_addr_extends(const struct ethosn_device_t *device,
						 uintptr_t core_addr)
{
	uint32_t addr_extends[3] = { 0 };
	size_t i;

	if (device->has_reserved_memory) {
		const uint32_t addr = TO_EXTEND_ADDR(device->reserved_memory_addr);

		addr_extends[0] = addr;
		addr_extends[1] = addr;
		addr_extends[2] = addr;
	} else {
		addr_extends[0] = TO_EXTEND_ADDR(ETHOSN_FW_VA_BASE);
		addr_extends[1] = TO_EXTEND_ADDR(ETHOSN_WORKING_DATA_VA_BASE);
		addr_extends[2] = TO_EXTEND_ADDR(ETHOSN_COMMAND_STREAM_VA_BASE);
	}

	for (i = 0U; i < ARRAY_SIZE(addr_extends); ++i) {
		const uintptr_t reg_addr = SEC_ADDR_EXT_REG_BASE +
			(SEC_ADDR_EXT_OFFSET * i);
		mmio_write_32(ETHOSN_CORE_SEC_REG(core_addr, reg_addr),
			      addr_extends[i]);
	}
}

static void ethosn_configure_stream_attr_ctlr(uintptr_t core_addr)
{
	size_t i;

	for (i = 0U; i < SEC_ATTR_CTLR_NUM; ++i) {
		const uintptr_t reg_addr = SEC_ATTR_CTLR_REG_BASE +
			(SEC_ATTR_CTLR_OFFSET * i);
		mmio_write_32(ETHOSN_CORE_SEC_REG(core_addr, reg_addr),
			      SEC_ATTR_CTLR_VAL);
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

static int ethosn_core_boot_fw(uintptr_t core_addr)
{
#if ETHOSN_NPU_TZMP1
	const uintptr_t sysctrl0_reg = ETHOSN_CORE_SEC_REG(core_addr, SEC_SYSCTRL0_REG);
	const uint32_t sysctrl0_val = mmio_read_32(sysctrl0_reg);
	const bool waiting = (sysctrl0_val & SEC_SYSCTRL0_CPU_WAIT);

	if (!waiting) {
		WARN("ETHOSN: Firmware is already running.\n");
		return ETHOSN_INVALID_STATE;
	}

	if (SYSCTRL0_INITVTOR_ADDR(sysctrl0_val) != FW_INITVTOR_ADDR(big_fw)) {
		WARN("ETHOSN: Unknown vector table won't boot firmware.\n");
		return ETHOSN_INVALID_CONFIGURATION;
	}

	mmio_clrbits_32(sysctrl0_reg, SEC_SYSCTRL0_CPU_WAIT);

	return ETHOSN_SUCCESS;
#else
	return ETHOSN_NOT_SUPPORTED;
#endif /* ETHOSN_NPU_TZMP1 */
}

static int ethosn_core_full_reset(const struct ethosn_device_t *device,
				  const struct ethosn_core_t *core,
				  bool hard_reset,
				  u_register_t asset_alloc_idx,
				  u_register_t is_protected,
				  u_register_t aux_features)
{
	if (!device->has_reserved_memory &&
	    asset_alloc_idx >= device->num_allocators) {
		WARN("ETHOSN: Unknown asset allocator index given to SMC call.\n");
		return ETHOSN_UNKNOWN_ALLOCATOR_IDX;
	}

	if (!ethosn_core_reset(core->addr, hard_reset)) {
		return ETHOSN_FAILURE;
	}

	if (!ethosn_configure_aux_features(device, core->addr, aux_features)) {
		return ETHOSN_INVALID_CONFIGURATION;
	}

	ethosn_configure_events(core->addr);

	if (!device->has_reserved_memory) {
		ethosn_configure_smmu_streams(device, core, asset_alloc_idx);

#if ETHOSN_NPU_TZMP1
		ethosn_configure_stream_nsaid(core, is_protected);
#endif /* ETHOSN_NPU_TZMP1 */
	}

	ethosn_configure_stream_addr_extends(device, core->addr);
	ethosn_configure_stream_attr_ctlr(core->addr);

#if ETHOSN_NPU_TZMP1
	ethosn_configure_vector_table(core->addr);
#endif /* ETHOSN_NPU_TZMP1 */

	ethosn_delegate_to_ns(core->addr);

	return ETHOSN_SUCCESS;
}

static uintptr_t ethosn_smc_core_reset_handler(const struct ethosn_device_t *device,
					       const struct ethosn_core_t *core,
					       bool hard_reset,
					       u_register_t asset_alloc_idx,
					       u_register_t reset_type,
					       u_register_t is_protected,
					       u_register_t aux_features,
					       void *handle)
{
	int ret;

	switch (reset_type) {
	case ETHOSN_RESET_TYPE_FULL:
		ret = ethosn_core_full_reset(device, core, hard_reset,
					     asset_alloc_idx, is_protected,
					     aux_features);
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
					 u_register_t aux_features,
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
						     aux_features,
						     handle);
	case ETHOSN_FNUM_BOOT_FW:
		SMC_RET1(handle, ethosn_core_boot_fw(core->addr));
	default:
		WARN("ETHOSN: Unimplemented SMC call: 0x%x\n", fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

static uintptr_t ethosn_smc_fw_prop_handler(u_register_t fw_property,
					    void *handle)
{
#if ETHOSN_NPU_TZMP1
	switch (fw_property) {
	case ETHOSN_FW_PROP_VERSION:
		SMC_RET4(handle, ETHOSN_SUCCESS,
			 big_fw->fw_ver_major,
			 big_fw->fw_ver_minor,
			 big_fw->fw_ver_patch);
	case ETHOSN_FW_PROP_MEM_INFO:
		SMC_RET3(handle, ETHOSN_SUCCESS,
			 ((void *)big_fw) + big_fw->offset,
			 big_fw->size);
	case ETHOSN_FW_PROP_OFFSETS:
		SMC_RET3(handle, ETHOSN_SUCCESS,
			 big_fw->ple_offset,
			 big_fw->unpriv_stack_offset);
	case ETHOSN_FW_PROP_VA_MAP:
		SMC_RET4(handle, ETHOSN_SUCCESS,
			 ETHOSN_FW_VA_BASE,
			 ETHOSN_WORKING_DATA_VA_BASE,
			 ETHOSN_COMMAND_STREAM_VA_BASE);
	default:
		WARN("ETHOSN: Unknown firmware property\n");
		SMC_RET1(handle, ETHOSN_INVALID_PARAMETER);
	}
#else
	SMC_RET1(handle, ETHOSN_NOT_SUPPORTED);
#endif /* ETHOSN_NPU_TZMP1 */
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

	if (!is_ethosn_fid(smc_fid) || (fid > ETHOSN_FNUM_BOOT_FW)) {
		WARN("ETHOSN: Unknown SMC call: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}

	switch (fid) {
	case ETHOSN_FNUM_VERSION:
		SMC_RET2(handle, ETHOSN_VERSION_MAJOR, ETHOSN_VERSION_MINOR);
	case ETHOSN_FNUM_GET_FW_PROP:
		return ethosn_smc_fw_prop_handler(x1, handle);
	}

	return ethosn_smc_core_handler(fid, x1, x2, x3, x4,
				       SMC_GET_GP(handle, CTX_GPREG_X5),
				       handle);
}

int ethosn_smc_setup(void)
{
#if ETHOSN_NPU_TZMP1
	struct ethosn_device_t *dev;
	uint32_t arch_ver;
#endif /* ETHOSN_NPU_TZMP1 */

	if (ETHOSN_NUM_DEVICES == 0U) {
		ERROR("ETHOSN: No NPU found\n");
		return ETHOSN_FAILURE;
	}

#if ETHOSN_NPU_TZMP1

	/* Only one NPU core is supported in the TZMP1 setup */
	if ((ETHOSN_NUM_DEVICES != 1U) ||
	    (ETHOSN_GET_DEVICE(0U)->num_cores != 1U)) {
		ERROR("ETHOSN: TZMP1 doesn't support multiple NPU cores\n");
		return ETHOSN_FAILURE;
	}

	dev = ETHOSN_GET_DEVICE(0U);
	if (dev->has_reserved_memory) {
		ERROR("ETHOSN: TZMP1 doesn't support using reserved memory\n");
		return ETHOSN_FAILURE;
	}

	arch_ver = ethosn_core_read_arch_version(dev->cores[0U].addr);
	big_fw = (struct ethosn_big_fw *)ETHOSN_NPU_FW_IMAGE_BASE;

	if (!ethosn_big_fw_verify_header(big_fw, arch_ver)) {
		return ETHOSN_FAILURE;
	}

	NOTICE("ETHOSN: TZMP1 setup succeeded with firmware version %u.%u.%u\n",
	       big_fw->fw_ver_major, big_fw->fw_ver_minor,
	       big_fw->fw_ver_patch);
#else
	NOTICE("ETHOSN: Setup succeeded\n");
#endif /* ETHOSN_NPU_TZMP1 */

	return 0;
}
