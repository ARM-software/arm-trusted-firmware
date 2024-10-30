/*
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for ioctl.
 */

#include <arch_helpers.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "pm_api_clock.h"
#include "pm_api_ioctl.h"
#include "pm_client.h"
#include "pm_common.h"
#include "pm_ipi.h"
#include <zynqmp_def.h>
#include "zynqmp_pm_api_sys.h"

/**
 * pm_ioctl_get_rpu_oper_mode () - Get current RPU operation mode.
 * @mode: Buffer to store value of oper mode(Split/Lock-step)
 *
 * This function provides current configured RPU operational mode.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_get_rpu_oper_mode(uint32_t *mode)
{
	uint32_t val;

	val = mmio_read_32(ZYNQMP_RPU_GLBL_CNTL);
	val &= ZYNQMP_SLSPLIT_MASK;
	if (val == 0U) {
		*mode = PM_RPU_MODE_LOCKSTEP;
	} else {
		*mode = PM_RPU_MODE_SPLIT;
	}

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_set_rpu_oper_mode () - Configure RPU operation mode.
 * @mode: Value to set for oper mode(Split/Lock-step).
 *
 * This function configures RPU operational mode(Split/Lock-step).
 * It also sets TCM combined mode in RPU lock-step and TCM non-combined
 * mode for RPU split mode. In case of Lock step mode, RPU1's output is
 * clamped.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_set_rpu_oper_mode(uint32_t mode)
{
	uint32_t val;

	if ((mmio_read_32(CRL_APB_RST_LPD_TOP) & CRL_APB_RPU_AMBA_RESET) != 0U) {
		return PM_RET_ERROR_ACCESS;
	}

	val = mmio_read_32(ZYNQMP_RPU_GLBL_CNTL);

	if (mode == PM_RPU_MODE_SPLIT) {
		val |= ZYNQMP_SLSPLIT_MASK;
		val &= ~ZYNQMP_TCM_COMB_MASK;
		val &= ~ZYNQMP_SLCLAMP_MASK;
	} else if (mode == PM_RPU_MODE_LOCKSTEP) {
		val &= ~ZYNQMP_SLSPLIT_MASK;
		val |= ZYNQMP_TCM_COMB_MASK;
		val |= ZYNQMP_SLCLAMP_MASK;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	mmio_write_32(ZYNQMP_RPU_GLBL_CNTL, val);

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_config_boot_addr() - Configure RPU boot address.
 * @nid: Node ID of RPU.
 * @value: Value to set for boot address (TCM/OCM).
 *
 * This function configures RPU boot address(memory).
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_config_boot_addr(enum pm_node_id nid,
						    uint32_t value)
{
	uint32_t rpu_cfg_addr, val;

	if (nid == NODE_RPU_0) {
		rpu_cfg_addr = ZYNQMP_RPU0_CFG;
	} else if (nid == NODE_RPU_1) {
		rpu_cfg_addr = ZYNQMP_RPU1_CFG;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	val = mmio_read_32(rpu_cfg_addr);

	if (value == PM_RPU_BOOTMEM_LOVEC) {
		val &= ~ZYNQMP_VINITHI_MASK;
	} else if (value == PM_RPU_BOOTMEM_HIVEC) {
		val |= ZYNQMP_VINITHI_MASK;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	mmio_write_32(rpu_cfg_addr, val);

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_config_tcm_comb() - Configure TCM combined mode.
 * @value: Value to set (Split/Combined).
 *
 * This function configures TCM to be in split mode or combined
 * mode.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_config_tcm_comb(uint32_t value)
{
	uint32_t val;

	val = mmio_read_32(ZYNQMP_RPU_GLBL_CNTL);

	if (value == PM_RPU_TCM_SPLIT) {
		val &= ~ZYNQMP_TCM_COMB_MASK;
	} else if (value == PM_RPU_TCM_COMB) {
		val |= ZYNQMP_TCM_COMB_MASK;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	mmio_write_32(ZYNQMP_RPU_GLBL_CNTL, val);

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_set_tapdelay_bypass() -  Enable/Disable tap delay bypass.
 * @type: Type of tap delay to enable/disable (e.g. QSPI).
 * @value: Enable/Disable.
 *
 * This function enable/disable tap delay bypass.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_set_tapdelay_bypass(uint32_t type,
						       uint32_t value)
{
	if ((((value != PM_TAPDELAY_BYPASS_ENABLE) &&
	     (value != PM_TAPDELAY_BYPASS_DISABLE)) || (type >= PM_TAPDELAY_MAX))) {
		return PM_RET_ERROR_ARGS;
	}

	return pm_mmio_write(IOU_TAPDLY_BYPASS, TAP_DELAY_MASK, value << type);
}

/**
 * pm_ioctl_sd_dll_reset() -  Reset DLL logic.
 * @nid: Node ID of the device.
 * @type: Reset type.
 *
 * This function resets DLL logic for the SD device.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_sd_dll_reset(enum pm_node_id nid,
						uint32_t type)
{
	uint32_t mask, val;
	enum pm_ret_status ret;

	if (nid == NODE_SD_0) {
		mask = ZYNQMP_SD0_DLL_RST_MASK;
		val = ZYNQMP_SD0_DLL_RST;
	} else if (nid == NODE_SD_1) {
		mask = ZYNQMP_SD1_DLL_RST_MASK;
		val = ZYNQMP_SD1_DLL_RST;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	switch (type) {
	case PM_DLL_RESET_ASSERT:
	case PM_DLL_RESET_PULSE:
		ret = pm_mmio_write(ZYNQMP_SD_DLL_CTRL, mask, val);
		if (ret != PM_RET_SUCCESS) {
			return ret;
		}

		if (type == PM_DLL_RESET_ASSERT) {
			break;
		}
		mdelay(1);
		/* Fallthrough */
	case PM_DLL_RESET_RELEASE:
		ret = pm_mmio_write(ZYNQMP_SD_DLL_CTRL, mask, 0);
		break;
	default:
		ret = PM_RET_ERROR_ARGS;
		break;
	}

	return ret;
}

/**
 * pm_ioctl_sd_set_tapdelay() -  Set tap delay for the SD device.
 * @nid: Node ID of the device.
 * @type: Type of tap delay to set (input/output).
 * @value: Value to set fot the tap delay.
 *
 * This function sets input/output tap delay for the SD device.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_sd_set_tapdelay(enum pm_node_id nid,
						   enum tap_delay_type type,
						   uint32_t value)
{
	uint32_t shift;
	enum pm_ret_status ret;
	uint32_t val, mask;

	if (nid == NODE_SD_0) {
		shift = 0;
		mask = ZYNQMP_SD0_DLL_RST_MASK;
	} else if (nid == NODE_SD_1) {
		shift = ZYNQMP_SD_TAP_OFFSET;
		mask = ZYNQMP_SD1_DLL_RST_MASK;
	} else {
		return PM_RET_ERROR_ARGS;
	}

	ret = pm_mmio_read(ZYNQMP_SD_DLL_CTRL, &val);
	if (ret != PM_RET_SUCCESS) {
		return ret;
	}

	if ((val & mask) == 0U) {
		ret = pm_ioctl_sd_dll_reset(nid, PM_DLL_RESET_ASSERT);
		if (ret != PM_RET_SUCCESS) {
			return ret;
		}
	}

	if (type == PM_TAPDELAY_INPUT) {
		ret = pm_mmio_write(ZYNQMP_SD_ITAP_DLY,
				    (ZYNQMP_SD_ITAPCHGWIN_MASK << shift),
				    (ZYNQMP_SD_ITAPCHGWIN << shift));

		if (ret != PM_RET_SUCCESS) {
			goto reset_release;
		}

		if (value == 0U) {
			ret = pm_mmio_write(ZYNQMP_SD_ITAP_DLY,
					    (ZYNQMP_SD_ITAPDLYENA_MASK <<
					     shift), 0);
		} else {
			ret = pm_mmio_write(ZYNQMP_SD_ITAP_DLY,
					    (ZYNQMP_SD_ITAPDLYENA_MASK <<
					    shift), (ZYNQMP_SD_ITAPDLYENA <<
					    shift));
		}

		if (ret != PM_RET_SUCCESS) {
			goto reset_release;
		}

		ret = pm_mmio_write(ZYNQMP_SD_ITAP_DLY,
				    (ZYNQMP_SD_ITAPDLYSEL_MASK << shift),
				    (value << shift));

		if (ret != PM_RET_SUCCESS) {
			goto reset_release;
		}

		ret = pm_mmio_write(ZYNQMP_SD_ITAP_DLY,
				    (ZYNQMP_SD_ITAPCHGWIN_MASK << shift), 0);
	} else if (type == PM_TAPDELAY_OUTPUT) {
		ret = pm_mmio_write(ZYNQMP_SD_OTAP_DLY,
				    (ZYNQMP_SD_OTAPDLYENA_MASK << shift), 0);

		if (ret != PM_RET_SUCCESS) {
			goto reset_release;
		}

		ret = pm_mmio_write(ZYNQMP_SD_OTAP_DLY,
				    (ZYNQMP_SD_OTAPDLYSEL_MASK << shift),
				    (value << shift));
	} else {
		ret = PM_RET_ERROR_ARGS;
	}

reset_release:
	if ((val & mask) == 0) {
		(void)pm_ioctl_sd_dll_reset(nid, PM_DLL_RESET_RELEASE);
	}

	return ret;
}

/**
 * pm_ioctl_set_pll_frac_mode() -  Ioctl function for setting pll mode.
 * @pll: PLL clock id.
 * @mode: Mode fraction/integar.
 *
 * This function sets PLL mode.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_set_pll_frac_mode
			(uint32_t pll, uint32_t mode)
{
	return pm_clock_set_pll_mode(pll, mode);
}

/**
 * pm_ioctl_get_pll_frac_mode() -  Ioctl function for getting pll mode.
 * @pll: PLL clock id.
 * @mode: Mode fraction/integar.
 *
 * This function return current PLL mode.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_get_pll_frac_mode
			(uint32_t pll, uint32_t *mode)
{
	return pm_clock_get_pll_mode(pll, mode);
}

/**
 * pm_ioctl_set_pll_frac_data() -  Ioctl function for setting pll fraction data.
 * @pll: PLL clock id.
 * @data: fraction data.
 *
 * This function sets fraction data.
 * It is valid for fraction mode only.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_set_pll_frac_data
			(uint32_t pll, uint32_t data)
{
	enum pm_node_id pll_nid;
	enum pm_ret_status status;

	/* Get PLL node ID using PLL clock ID */
	status = pm_clock_get_pll_node_id(pll, &pll_nid);
	if (status != PM_RET_SUCCESS) {
		return status;
	}

	return pm_pll_set_parameter(pll_nid, PM_PLL_PARAM_DATA, data);
}

/**
 * pm_ioctl_get_pll_frac_data() -  Ioctl function for getting pll fraction data.
 * @pll: PLL clock id.
 * @data: fraction data.
 *
 * This function returns fraction data value.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_get_pll_frac_data
			(uint32_t pll, uint32_t *data)
{
	enum pm_node_id pll_nid;
	enum pm_ret_status status;

	/* Get PLL node ID using PLL clock ID */
	status = pm_clock_get_pll_node_id(pll, &pll_nid);
	if (status != PM_RET_SUCCESS) {
		return status;
	}

	return pm_pll_get_parameter(pll_nid, PM_PLL_PARAM_DATA, data);
}

/**
 * pm_ioctl_write_ggs() - Ioctl function for writing global general storage
 *                        (ggs).
 * @index: GGS register index.
 * @value: Register value to be written.
 *
 * This function writes value to GGS register.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_write_ggs(uint32_t index,
					     uint32_t value)
{
	if (index >= GGS_NUM_REGS) {
		return PM_RET_ERROR_ARGS;
	}

	return pm_mmio_write(GGS_BASEADDR + (index << 2),
			     0xFFFFFFFFU, value);
}

/**
 * pm_ioctl_read_ggs() - Ioctl function for reading global general storage
 *                       (ggs).
 * @index: GGS register index.
 * @value: Register value.
 *
 * This function returns GGS register value.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_read_ggs(uint32_t index,
					    uint32_t *value)
{
	if (index >= GGS_NUM_REGS) {
		return PM_RET_ERROR_ARGS;
	}

	return pm_mmio_read(GGS_BASEADDR + (index << 2), value);
}

/**
 * pm_ioctl_write_pggs() - Ioctl function for writing persistent global general
 *                         storage (pggs).
 * @index: PGGS register index.
 * @value: Register value to be written.
 *
 * This function writes value to PGGS register.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_write_pggs(uint32_t index,
					      uint32_t value)
{
	if (index >= PGGS_NUM_REGS) {
		return PM_RET_ERROR_ARGS;
	}

	return pm_mmio_write(PGGS_BASEADDR + (index << 2),
			     0xFFFFFFFFU, value);
}

/**
 * pm_ioctl_afi() - Ioctl function for writing afi values.
 * @index: AFI register index.
 * @value: Register value to be written.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_afi(uint32_t index,
					      uint32_t value)
{
	uint32_t mask;
	const uint32_t regarr[] = {0xFD360000U,
				0xFD360014U,
				0xFD370000U,
				0xFD370014U,
				0xFD380000U,
				0xFD380014U,
				0xFD390000U,
				0xFD390014U,
				0xFD3a0000U,
				0xFD3a0014U,
				0xFD3b0000U,
				0xFD3b0014U,
				0xFF9b0000U,
				0xFF9b0014U,
				0xFD615000U,
				0xFF419000U,
				};

	if (index >= ARRAY_SIZE(regarr)) {
		return PM_RET_ERROR_ARGS;
	}

	if (index <= AFIFM6_WRCTRL) {
		mask = FABRIC_WIDTH;
	} else {
		mask = 0xf00;
	}

	return pm_mmio_write(regarr[index], mask, value);
}

/**
 * pm_ioctl_read_pggs() - Ioctl function for reading persistent global general
 *                        storage (pggs).
 * @index: PGGS register index.
 * @value: Register value.
 *
 * This function returns PGGS register value.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_read_pggs(uint32_t index,
					     uint32_t *value)
{
	if (index >= PGGS_NUM_REGS) {
		return PM_RET_ERROR_ARGS;
	}

	return pm_mmio_read(PGGS_BASEADDR + (index << 2), value);
}

/**
 * pm_ioctl_ulpi_reset() - Ioctl function for performing ULPI reset.
 *
 * Return: Returns status, either success or error+reason.
 *
 * This function peerforms the ULPI reset sequence for resetting
 * the ULPI transceiver.
 */
static enum pm_ret_status pm_ioctl_ulpi_reset(void)
{
	enum pm_ret_status ret;

	ret = pm_mmio_write(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOT_PIN_MASK,
			    ZYNQMP_ULPI_RESET_VAL_HIGH);
	if (ret != PM_RET_SUCCESS) {
		return ret;
	}

	/* Drive ULPI assert for atleast 1ms */
	mdelay(1);

	ret = pm_mmio_write(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOT_PIN_MASK,
			    ZYNQMP_ULPI_RESET_VAL_LOW);
	if (ret != PM_RET_SUCCESS) {
		return ret;
	}

	/* Drive ULPI de-assert for atleast 1ms */
	mdelay(1);

	ret = pm_mmio_write(CRL_APB_BOOT_PIN_CTRL, CRL_APB_BOOT_PIN_MASK,
			    ZYNQMP_ULPI_RESET_VAL_HIGH);

	return ret;
}

/**
 * pm_ioctl_set_boot_health_status() - Ioctl for setting healthy boot status.
 * @value: Value to write.
 *
 * This function sets healthy bit value to indicate boot health status
 * to firmware.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ioctl_set_boot_health_status(uint32_t value)
{
	return pm_mmio_write(PMU_GLOBAL_GEN_STORAGE4,
			     PM_BOOT_HEALTH_STATUS_MASK, value);
}

/**
 * pm_api_ioctl() -  PM IOCTL API for device control and configs.
 * @nid: Node ID of the device.
 * @ioctl_id: ID of the requested IOCTL.
 * @arg1: Argument 1 to requested IOCTL call.
 * @arg2: Argument 2 to requested IOCTL call.
 * @value: Returned output value.
 *
 * This function calls IOCTL to firmware for device control and configuration.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_api_ioctl(enum pm_node_id nid,
				uint32_t ioctl_id,
				uint32_t arg1,
				uint32_t arg2,
				uint32_t *value)
{
	enum pm_ret_status ret;
	uint32_t payload[PAYLOAD_ARG_CNT];

	switch (ioctl_id) {
	case IOCTL_GET_RPU_OPER_MODE:
		ret = pm_ioctl_get_rpu_oper_mode(value);
		break;
	case IOCTL_SET_RPU_OPER_MODE:
		ret = pm_ioctl_set_rpu_oper_mode(arg1);
		break;
	case IOCTL_RPU_BOOT_ADDR_CONFIG:
		ret = pm_ioctl_config_boot_addr(nid, arg1);
		break;
	case IOCTL_TCM_COMB_CONFIG:
		ret = pm_ioctl_config_tcm_comb(arg1);
		break;
	case IOCTL_SET_TAPDELAY_BYPASS:
		ret = pm_ioctl_set_tapdelay_bypass(arg1, arg2);
		break;
	case IOCTL_SD_DLL_RESET:
		ret = pm_ioctl_sd_dll_reset(nid, arg1);
		break;
	case IOCTL_SET_SD_TAPDELAY:
		ret = pm_ioctl_sd_set_tapdelay(nid, arg1, arg2);
		break;
	case IOCTL_SET_PLL_FRAC_MODE:
		ret = pm_ioctl_set_pll_frac_mode(arg1, arg2);
		break;
	case IOCTL_GET_PLL_FRAC_MODE:
		ret = pm_ioctl_get_pll_frac_mode(arg1, value);
		break;
	case IOCTL_SET_PLL_FRAC_DATA:
		ret = pm_ioctl_set_pll_frac_data(arg1, arg2);
		break;
	case IOCTL_GET_PLL_FRAC_DATA:
		ret = pm_ioctl_get_pll_frac_data(arg1, value);
		break;
	case IOCTL_WRITE_GGS:
		ret = pm_ioctl_write_ggs(arg1, arg2);
		break;
	case IOCTL_READ_GGS:
		ret = pm_ioctl_read_ggs(arg1, value);
		break;
	case IOCTL_WRITE_PGGS:
		ret = pm_ioctl_write_pggs(arg1, arg2);
		break;
	case IOCTL_READ_PGGS:
		ret = pm_ioctl_read_pggs(arg1, value);
		break;
	case IOCTL_ULPI_RESET:
		ret = pm_ioctl_ulpi_reset();
		break;
	case IOCTL_SET_BOOT_HEALTH_STATUS:
		ret = pm_ioctl_set_boot_health_status(arg1);
		break;
	case IOCTL_AFI:
		ret = pm_ioctl_afi(arg1, arg2);
		break;
	default:
		/* Send request to the PMU */
		PM_PACK_PAYLOAD5(payload, PM_IOCTL, nid, ioctl_id, arg1, arg2);

		ret = pm_ipi_send_sync(primary_proc, payload, value, 1);
		break;
	}

	return ret;
}

/**
 * tfa_ioctl_bitmask() -  API to get supported IOCTL ID mask.
 * @bit_mask: Returned bit mask of supported IOCTL IDs.
 *
 * Return: 0 success, negative value for errors.
 *
 */
enum pm_ret_status tfa_ioctl_bitmask(uint32_t *bit_mask)
{
	const uint8_t supported_ids[] = {
		IOCTL_GET_RPU_OPER_MODE,
		IOCTL_SET_RPU_OPER_MODE,
		IOCTL_RPU_BOOT_ADDR_CONFIG,
		IOCTL_TCM_COMB_CONFIG,
		IOCTL_SET_TAPDELAY_BYPASS,
		IOCTL_SD_DLL_RESET,
		IOCTL_SET_SD_TAPDELAY,
		IOCTL_SET_PLL_FRAC_MODE,
		IOCTL_GET_PLL_FRAC_MODE,
		IOCTL_SET_PLL_FRAC_DATA,
		IOCTL_GET_PLL_FRAC_DATA,
		IOCTL_WRITE_GGS,
		IOCTL_READ_GGS,
		IOCTL_WRITE_PGGS,
		IOCTL_READ_PGGS,
		IOCTL_ULPI_RESET,
		IOCTL_SET_BOOT_HEALTH_STATUS,
		IOCTL_AFI,
	};
	uint8_t i, ioctl_id;
	int32_t ret;

	for (i = 0U; i < ARRAY_SIZE(supported_ids); i++) {
		ioctl_id = supported_ids[i];
		if (ioctl_id >= 64U) {
			return PM_RET_ERROR_NOTSUPPORTED;
		}
		ret = check_api_dependency(ioctl_id);
		if (ret == PM_RET_SUCCESS) {
			bit_mask[ioctl_id / 32U] |= BIT(ioctl_id % 32U);
		}
	}

	return PM_RET_SUCCESS;
}
