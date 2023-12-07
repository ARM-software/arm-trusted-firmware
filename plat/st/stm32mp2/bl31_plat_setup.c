/*
 * Copyright (c) 2023-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <common/bl_common.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/stm32_console.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_def.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl_params_t *params_from_bl2;
	int ret;

	/*
	 * Invalidate remaining data from second half of SYSRAM (used by BL2) as this area will
	 * be later used as non-secure.
	 */
	inv_dcache_range(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
			 STM32MP_SYSRAM_SIZE / 2U);

	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/*
	 * Map soc_fw_config device tree with secure property, i.e. default region.
	 * DDR region definitions will be finalized at BL32 level.
	 */
	mmap_add_region(arg1, arg1, STM32MP_SOC_FW_CONFIG_MAX_SIZE, MT_RO_DATA | MT_SECURE);

#if USE_COHERENT_MEM
	/* Map coherent memory */
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	configure_mmu();

	ret = dt_open_and_check(arg1);
	if (ret < 0) {
		EARLY_ERROR("%s: failed to open DT (%d)\n", __func__, ret);
		panic();
	}

	ret = stm32mp2_clk_init();
	if (ret < 0) {
		EARLY_ERROR("%s: failed init clocks (%d)\n", __func__, ret);
		panic();
	}

	generic_delay_timer_init();

	stm32_tamp_nvram_init();

	(void)stm32mp_uart_console_setup();

	/*
	 * Map upper SYSRAM where bl_params_t are stored in BL2
	 */
	ret = mmap_add_dynamic_region(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
				      STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
				      STM32MP_SYSRAM_SIZE / 2U, MT_RO_DATA | MT_SECURE);
	if (ret < 0) {
		ERROR("BL2 params area mapping: %d\n", ret);
		panic();
	}

	assert(arg0 != 0UL);
	params_from_bl2 = (bl_params_t *)arg0;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	while (bl_params != NULL) {
		/*
		 * Copy BL33 entry point information.
		 * They are stored in Secure RAM, in BL2's address space.
		 */
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			/*
			 *  Check if hw_configuration is given to BL32 and
			 *  share it to BL33
			 */
			if (arg2 != 0U) {
				bl33_image_ep_info.args.arg0 = 0U;
				bl33_image_ep_info.args.arg1 = 0U;
				bl33_image_ep_info.args.arg2 = arg2;
			}
		}

		if (bl_params->image_id == BL32_IMAGE_ID) {
			bl32_image_ep_info = *bl_params->ep_info;

			if (arg2 != 0U) {
				bl32_image_ep_info.args.arg3 = arg2;
			}
		}

		bl_params = bl_params->next_params_info;
	}

	ret = mmap_remove_dynamic_region(STM32MP_SYSRAM_BASE + STM32MP_SYSRAM_SIZE / 2U,
					 STM32MP_SYSRAM_SIZE / 2U);
	if (ret < 0) {
		ERROR("BL2 params area unmapping: %d\n", ret);
		panic();
	}
}

void bl31_plat_arch_setup(void)
{
	stm32mp_gic_init();
}

void bl31_platform_setup(void)
{
}

void bl31_plat_runtime_setup(void)
{
	/* We reinit the nvram driver Optee should have update the register access configuration
	 * based on the device tree
	 */
	int ret = stm32_tamp_nvram_update_rights();

	if (ret != 0) {
		ERROR("Failed to update TAMP backup registers rights\n");
		panic();
	}
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(unsigned int type)
{
	entry_point_info_t *next_image_info = NULL;

	assert(sec_state_is_valid(type));

	switch (type) {
	case NON_SECURE:
		next_image_info = &bl33_image_ep_info;
		break;

	case SECURE:
		next_image_info = &bl32_image_ep_info;
		break;

	default:
		break;
	}

	/* None of the next images on ST platforms can have 0x0 as the entrypoint */
	if ((next_image_info == NULL) || (next_image_info->pc == 0UL)) {
		return NULL;
	}

	return next_image_info;
}

/* Only support system reset for serial boot and no low power feature to reduce BL31 size */
#if !STM32MP_SUPPORT_PM
static void __dead2 stm32_system_reset(void)
{
	stm32mp_system_reset();
}

static const plat_psci_ops_t stm32_psci_ops = {
	.system_reset = stm32_system_reset,
};

/* Stub PSCI platform functions */
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	return 0U;
}

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	*psci_ops = &stm32_psci_ops;
	return 0;
}
#endif /* STM32MP_SUPPORT_PM */
