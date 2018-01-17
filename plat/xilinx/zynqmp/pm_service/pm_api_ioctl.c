/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for ioctl.
 */

#include <arch_helpers.h>
#include <delay_timer.h>
#include <mmio.h>
#include <platform.h>
#include "pm_api_ioctl.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_common.h"
#include "pm_ipi.h"
#include "../zynqmp_def.h"

/**
 * pm_ioctl_get_rpu_oper_mode () - Get current RPU operation mode
 * @mode	Buffer to store value of oper mode(Split/Lock-step)
 *
 * This function provides current configured RPU operational mode.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ioctl_get_rpu_oper_mode(unsigned int *mode)
{
	unsigned int val;

	val = mmio_read_32(ZYNQMP_RPU_GLBL_CNTL);
	val &= ZYNQMP_SLSPLIT_MASK;
	if (val)
		*mode = PM_RPU_MODE_SPLIT;
	else
		*mode = PM_RPU_MODE_LOCKSTEP;

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_set_rpu_oper_mode () - Configure RPU operation mode
 * @mode	Value to set for oper mode(Split/Lock-step)
 *
 * This function configures RPU operational mode(Split/Lock-step).
 * It also sets TCM combined mode in RPU lock-step and TCM non-combined
 * mode for RPU split mode. In case of Lock step mode, RPU1's output is
 * clamped.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ioctl_set_rpu_oper_mode(unsigned int mode)
{
	unsigned int val;

	if (mmio_read_32(CRL_APB_RST_LPD_TOP) && CRL_APB_RPU_AMBA_RESET)
		return PM_RET_ERROR_ACCESS;

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
 * pm_ioctl_config_boot_addr() - Configure RPU boot address
 * @nid		Node ID of RPU
 * @value	Value to set for boot address (TCM/OCM)
 *
 * This function configures RPU boot address(memory).
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ioctl_config_boot_addr(enum pm_node_id nid,
						    unsigned int value)
{
	unsigned int rpu_cfg_addr, val;

	if (nid == NODE_RPU_0)
		rpu_cfg_addr = ZYNQMP_RPU0_CFG;
	else if (nid == NODE_RPU_1)
		rpu_cfg_addr = ZYNQMP_RPU1_CFG;
	else
		return PM_RET_ERROR_ARGS;

	val = mmio_read_32(rpu_cfg_addr);

	if (value == PM_RPU_BOOTMEM_LOVEC)
		val &= ~ZYNQMP_VINITHI_MASK;
	else if (value == PM_RPU_BOOTMEM_HIVEC)
		val |= ZYNQMP_VINITHI_MASK;
	else
		return PM_RET_ERROR_ARGS;

	mmio_write_32(rpu_cfg_addr, val);

	return PM_RET_SUCCESS;
}

/**
 * pm_ioctl_config_tcm_comb() - Configure TCM combined mode
 * @value	Value to set (Split/Combined)
 *
 * This function configures TCM to be in split mode or combined
 * mode.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ioctl_config_tcm_comb(unsigned int value)
{
	unsigned int val;

	val = mmio_read_32(ZYNQMP_RPU_GLBL_CNTL);

	if (value == PM_RPU_TCM_SPLIT)
		val &= ~ZYNQMP_TCM_COMB_MASK;
	else if (value == PM_RPU_TCM_COMB)
		val |= ZYNQMP_TCM_COMB_MASK;
	else
		return PM_RET_ERROR_ARGS;

	mmio_write_32(ZYNQMP_RPU_GLBL_CNTL, val);

	return PM_RET_SUCCESS;
}

/**
 * pm_api_ioctl() -  PM IOCTL API for device control and configs
 * @node_id	Node ID of the device
 * @ioctl_id	ID of the requested IOCTL
 * @arg1	Argument 1 to requested IOCTL call
 * @arg2	Argument 2 to requested IOCTL call
 * @value	Returned output value
 *
 * This function calls IOCTL to firmware for device control and configuration.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_api_ioctl(enum pm_node_id nid,
				unsigned int ioctl_id,
				unsigned int arg1,
				unsigned int arg2,
				unsigned int *value)
{
	int ret;

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
	default:
		ret = PM_RET_ERROR_NOTSUPPORTED;
	}

	return ret;
}
