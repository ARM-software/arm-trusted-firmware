/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * File containing client-side RPC functions for the MISC service. These
 * functions are ported to clients that communicate to the SC.
 *
 * @addtogroup MISC_SVC
 * @{
 */

/* Includes */

#include <sci/sci_types.h>
#include <sci/svc/rm/sci_rm_api.h>
#include <sci/svc/misc/sci_misc_api.h>
#include <sci/sci_rpc.h>
#include <stdlib.h>
#include "sci_misc_rpc.h"

/* Local Defines */

/* Local Types */

/* Local Functions */

sc_err_t sc_misc_set_control(sc_ipc_t ipc, sc_rsrc_t resource,
			     sc_ctrl_t ctrl, uint32_t val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SET_CONTROL;
	RPC_U32(&msg, 0U) = (uint32_t)ctrl;
	RPC_U32(&msg, 4U) = (uint32_t)val;
	RPC_U16(&msg, 8U) = (uint16_t)resource;
	RPC_SIZE(&msg) = 4U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_get_control(sc_ipc_t ipc, sc_rsrc_t resource,
			     sc_ctrl_t ctrl, uint32_t *val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_GET_CONTROL;
	RPC_U32(&msg, 0U) = (uint32_t)ctrl;
	RPC_U16(&msg, 4U) = (uint16_t)resource;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (val != NULL)
		*val = RPC_U32(&msg, 0U);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_set_max_dma_group(sc_ipc_t ipc, sc_rm_pt_t pt,
				   sc_misc_dma_group_t max)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SET_MAX_DMA_GROUP;
	RPC_U8(&msg, 0U) = (uint8_t)pt;
	RPC_U8(&msg, 1U) = (uint8_t)max;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_set_dma_group(sc_ipc_t ipc, sc_rsrc_t resource,
			       sc_misc_dma_group_t group)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SET_DMA_GROUP;
	RPC_U16(&msg, 0U) = (uint16_t)resource;
	RPC_U8(&msg, 2U) = (uint8_t)group;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_image_load(sc_ipc_t ipc, sc_faddr_t addr_src,
				 sc_faddr_t addr_dst, uint32_t len,
				 sc_bool_t fw)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_IMAGE_LOAD;
	RPC_U32(&msg, 0U) = (uint32_t)(addr_src >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)addr_src;
	RPC_U32(&msg, 8U) = (uint32_t)(addr_dst >> 32U);
	RPC_U32(&msg, 12U) = (uint32_t)addr_dst;
	RPC_U32(&msg, 16U) = (uint32_t)len;
	RPC_U8(&msg, 20U) = (uint8_t)fw;
	RPC_SIZE(&msg) = 7U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_authenticate(sc_ipc_t ipc,
				   sc_misc_seco_auth_cmd_t cmd, sc_faddr_t addr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_AUTHENTICATE;
	RPC_U32(&msg, 0U) = (uint32_t)(addr >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)addr;
	RPC_U8(&msg, 8U) = (uint8_t)cmd;
	RPC_SIZE(&msg) = 4U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_fuse_write(sc_ipc_t ipc, sc_faddr_t addr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_FUSE_WRITE;
	RPC_U32(&msg, 0U) = (uint32_t)(addr >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)addr;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_enable_debug(sc_ipc_t ipc, sc_faddr_t addr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_ENABLE_DEBUG;
	RPC_U32(&msg, 0U) = (uint32_t)(addr >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)addr;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_forward_lifecycle(sc_ipc_t ipc, uint32_t lifecycle)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_FORWARD_LIFECYCLE;
	RPC_U32(&msg, 0U) = (uint32_t)lifecycle;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_seco_return_lifecycle(sc_ipc_t ipc, sc_faddr_t addr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_RETURN_LIFECYCLE;
	RPC_U32(&msg, 0U) = (uint32_t)(addr >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)addr;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

void sc_misc_seco_build_info(sc_ipc_t ipc, uint32_t *version, uint32_t *commit)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_BUILD_INFO;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (version != NULL)
		*version = RPC_U32(&msg, 0U);

	if (commit != NULL)
		*commit = RPC_U32(&msg, 4U);
}

sc_err_t sc_misc_seco_chip_info(sc_ipc_t ipc, uint16_t *lc,
				uint16_t *monotonic, uint32_t *uid_l,
				uint32_t *uid_h)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SECO_CHIP_INFO;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (uid_l != NULL)
		*uid_l = RPC_U32(&msg, 0U);

	if (uid_h != NULL)
		*uid_h = RPC_U32(&msg, 4U);

	if (lc != NULL)
		*lc = RPC_U16(&msg, 8U);

	if (monotonic != NULL)
		*monotonic = RPC_U16(&msg, 10U);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

void sc_misc_debug_out(sc_ipc_t ipc, uint8_t ch)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_DEBUG_OUT;
	RPC_U8(&msg, 0U) = (uint8_t)ch;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);
}

sc_err_t sc_misc_waveform_capture(sc_ipc_t ipc, sc_bool_t enable)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_WAVEFORM_CAPTURE;
	RPC_U8(&msg, 0U) = (uint8_t)enable;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

void sc_misc_build_info(sc_ipc_t ipc, uint32_t *build, uint32_t *commit)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_BUILD_INFO;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (build != NULL)
		*build = RPC_U32(&msg, 0U);

	if (commit != NULL)
		*commit = RPC_U32(&msg, 4U);
}

void sc_misc_unique_id(sc_ipc_t ipc, uint32_t *id_l, uint32_t *id_h)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_UNIQUE_ID;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (id_l != NULL)
		*id_l = RPC_U32(&msg, 0U);

	if (id_h != NULL)
		*id_h = RPC_U32(&msg, 4U);
}

sc_err_t sc_misc_set_ari(sc_ipc_t ipc, sc_rsrc_t resource,
			 sc_rsrc_t resource_mst, uint16_t ari, sc_bool_t enable)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SET_ARI;
	RPC_U16(&msg, 0U) = (uint16_t)resource;
	RPC_U16(&msg, 2U) = (uint16_t)resource_mst;
	RPC_U16(&msg, 4U) = (uint16_t)ari;
	RPC_U8(&msg, 6U) = (uint8_t)enable;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

void sc_misc_boot_status(sc_ipc_t ipc, sc_misc_boot_status_t status)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_BOOT_STATUS;
	RPC_U8(&msg, 0U) = (uint8_t)status;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_TRUE);
}

sc_err_t sc_misc_boot_done(sc_ipc_t ipc, sc_rsrc_t cpu)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_BOOT_DONE;
	RPC_U16(&msg, 0U) = (uint16_t)cpu;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_otp_fuse_read(sc_ipc_t ipc, uint32_t word, uint32_t *val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_OTP_FUSE_READ;
	RPC_U32(&msg, 0U) = (uint32_t)word;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (val != NULL)
		*val = RPC_U32(&msg, 0U);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_otp_fuse_write(sc_ipc_t ipc, uint32_t word, uint32_t val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_OTP_FUSE_WRITE;
	RPC_U32(&msg, 0U) = (uint32_t)word;
	RPC_U32(&msg, 4U) = (uint32_t)val;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_set_temp(sc_ipc_t ipc, sc_rsrc_t resource,
			  sc_misc_temp_t temp, int16_t celsius, int8_t tenths)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_SET_TEMP;
	RPC_U16(&msg, 0U) = (uint16_t)resource;
	RPC_I16(&msg, 2U) = (int16_t) celsius;
	RPC_U8(&msg, 4U) = (uint8_t)temp;
	RPC_I8(&msg, 5U) = (int8_t) tenths;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_misc_get_temp(sc_ipc_t ipc, sc_rsrc_t resource,
			  sc_misc_temp_t temp, int16_t *celsius,
			  int8_t *tenths)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_GET_TEMP;
	RPC_U16(&msg, 0U) = (uint16_t)resource;
	RPC_U8(&msg, 2U) = (uint8_t)temp;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (celsius != NULL)
		*celsius = RPC_I16(&msg, 0U);

	result = RPC_R8(&msg);
	if (tenths != NULL)
		*tenths = RPC_I8(&msg, 2U);

	return (sc_err_t)result;
}

void sc_misc_get_boot_dev(sc_ipc_t ipc, sc_rsrc_t *dev)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_GET_BOOT_DEV;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (dev != NULL)
		*dev = RPC_U16(&msg, 0U);
}

void sc_misc_get_button_status(sc_ipc_t ipc, sc_bool_t *status)
{
	sc_rpc_msg_t msg;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_MISC;
	RPC_FUNC(&msg) = (uint8_t)MISC_FUNC_GET_BUTTON_STATUS;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (status != NULL)
		*status = RPC_U8(&msg, 0U);
}

/**@}*/
