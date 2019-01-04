/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * File containing client-side RPC functions for the PAD service. These
 * functions are ported to clients that communicate to the SC.
 *
 * @addtogroup PAD_SVC
 * @{
 */

/* Includes */

#include <stdlib.h>

#include <sci/sci_types.h>
#include <sci/svc/rm/sci_rm_api.h>
#include <sci/svc/pad/sci_pad_api.h>
#include <sci/sci_rpc.h>
#include "sci_pad_rpc.h"

/* Local Defines */

/* Local Types */

/* Local Functions */

sc_err_t sc_pad_set_mux(sc_ipc_t ipc, sc_pad_t pad,
			uint8_t mux, sc_pad_config_t config, sc_pad_iso_t iso)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_MUX;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_U8(&msg, 2U) = (uint8_t)mux;
	RPC_U8(&msg, 3U) = (uint8_t)config;
	RPC_U8(&msg, 4U) = (uint8_t)iso;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_mux(sc_ipc_t ipc, sc_pad_t pad,
			uint8_t *mux, sc_pad_config_t *config,
			sc_pad_iso_t *iso)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_MUX;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	if (mux != NULL) {
		*mux = RPC_U8(&msg, 0U);
	}

	if (config != NULL) {
		*config = RPC_U8(&msg, 1U);
	}

	if (iso != NULL) {
		*iso = RPC_U8(&msg, 2U);
	}

	return (sc_err_t)result;
}

sc_err_t sc_pad_set_gp(sc_ipc_t ipc, sc_pad_t pad, uint32_t ctrl)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_GP;
	RPC_U32(&msg, 0U) = (uint32_t)ctrl;
	RPC_U16(&msg, 4U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_gp(sc_ipc_t ipc, sc_pad_t pad, uint32_t *ctrl)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_GP;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (ctrl != NULL) {
		*ctrl = RPC_U32(&msg, 0U);
	}

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_set_wakeup(sc_ipc_t ipc, sc_pad_t pad, sc_pad_wakeup_t wakeup)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_WAKEUP;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_U8(&msg, 2U) = (uint8_t)wakeup;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_wakeup(sc_ipc_t ipc, sc_pad_t pad, sc_pad_wakeup_t *wakeup)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_WAKEUP;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	if (wakeup != NULL) {
		*wakeup = RPC_U8(&msg, 0U);
	}

	return (sc_err_t)result;
}

sc_err_t sc_pad_set_all(sc_ipc_t ipc, sc_pad_t pad, uint8_t mux,
			sc_pad_config_t config, sc_pad_iso_t iso, uint32_t ctrl,
			sc_pad_wakeup_t wakeup)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_ALL;
	RPC_U32(&msg, 0U) = (uint32_t)ctrl;
	RPC_U16(&msg, 4U) = (uint16_t)pad;
	RPC_U8(&msg, 6U) = (uint8_t)mux;
	RPC_U8(&msg, 7U) = (uint8_t)config;
	RPC_U8(&msg, 8U) = (uint8_t)iso;
	RPC_U8(&msg, 9U) = (uint8_t)wakeup;
	RPC_SIZE(&msg) = 4U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_all(sc_ipc_t ipc, sc_pad_t pad, uint8_t *mux,
			sc_pad_config_t *config, sc_pad_iso_t *iso,
			uint32_t *ctrl, sc_pad_wakeup_t *wakeup)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_ALL;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (ctrl != NULL) {
		*ctrl = RPC_U32(&msg, 0U);
	}

	result = RPC_R8(&msg);
	if (mux != NULL) {
		*mux = RPC_U8(&msg, 4U);
	}

	if (config != NULL) {
		*config = RPC_U8(&msg, 5U);
	}

	if (iso != NULL) {
		*iso = RPC_U8(&msg, 6U);
	}

	if (wakeup != NULL) {
		*wakeup = RPC_U8(&msg, 7U);
	}

	return (sc_err_t)result;
}

sc_err_t sc_pad_set(sc_ipc_t ipc, sc_pad_t pad, uint32_t val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET;
	RPC_U32(&msg, 0U) = (uint32_t)val;
	RPC_U16(&msg, 4U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get(sc_ipc_t ipc, sc_pad_t pad, uint32_t *val)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (val != NULL) {
		*val = RPC_U32(&msg, 0U);
	}

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_set_gp_28fdsoi(sc_ipc_t ipc, sc_pad_t pad,
			       sc_pad_28fdsoi_dse_t dse, sc_pad_28fdsoi_ps_t ps)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_GP_28FDSOI;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_U8(&msg, 2U) = (uint8_t)dse;
	RPC_U8(&msg, 3U) = (uint8_t)ps;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_gp_28fdsoi(sc_ipc_t ipc, sc_pad_t pad,
			       sc_pad_28fdsoi_dse_t *dse,
			       sc_pad_28fdsoi_ps_t *ps)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_GP_28FDSOI;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	if (dse != NULL) {
		*dse = RPC_U8(&msg, 0U);
	}

	if (ps != NULL) {
		*ps = RPC_U8(&msg, 1U);
	}

	return (sc_err_t)result;
}

sc_err_t sc_pad_set_gp_28fdsoi_hsic(sc_ipc_t ipc, sc_pad_t pad,
				    sc_pad_28fdsoi_dse_t dse, sc_bool_t hys,
				    sc_pad_28fdsoi_pus_t pus, sc_bool_t pke,
				    sc_bool_t pue)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_GP_28FDSOI_HSIC;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_U8(&msg, 2U) = (uint8_t)dse;
	RPC_U8(&msg, 3U) = (uint8_t)pus;
	RPC_U8(&msg, 4U) = (uint8_t)hys;
	RPC_U8(&msg, 5U) = (uint8_t)pke;
	RPC_U8(&msg, 6U) = (uint8_t)pue;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_gp_28fdsoi_hsic(sc_ipc_t ipc, sc_pad_t pad,
				    sc_pad_28fdsoi_dse_t *dse, sc_bool_t *hys,
				    sc_pad_28fdsoi_pus_t *pus, sc_bool_t *pke,
				    sc_bool_t *pue)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_GP_28FDSOI_HSIC;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	if (dse != NULL) {
		*dse = RPC_U8(&msg, 0U);
	}

	if (pus != NULL) {
		*pus = RPC_U8(&msg, 1U);
	}

	if (hys != NULL) {
		*hys = RPC_U8(&msg, 2U);
	}

	if (pke != NULL) {
		*pke = RPC_U8(&msg, 3U);
	}

	if (pue != NULL) {
		*pue = RPC_U8(&msg, 4U);
	}

	return (sc_err_t)result;
}

sc_err_t sc_pad_set_gp_28fdsoi_comp(sc_ipc_t ipc, sc_pad_t pad,
				    uint8_t compen, sc_bool_t fastfrz,
				    uint8_t rasrcp, uint8_t rasrcn,
				    sc_bool_t nasrc_sel, sc_bool_t psw_ovr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_SET_GP_28FDSOI_COMP;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_U8(&msg, 2U) = (uint8_t)compen;
	RPC_U8(&msg, 3U) = (uint8_t)rasrcp;
	RPC_U8(&msg, 4U) = (uint8_t)rasrcn;
	RPC_U8(&msg, 5U) = (uint8_t)fastfrz;
	RPC_U8(&msg, 6U) = (uint8_t)nasrc_sel;
	RPC_U8(&msg, 7U) = (uint8_t)psw_ovr;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_pad_get_gp_28fdsoi_comp(sc_ipc_t ipc, sc_pad_t pad,
				    uint8_t *compen, sc_bool_t *fastfrz,
				    uint8_t *rasrcp, uint8_t *rasrcn,
				    sc_bool_t *nasrc_sel, sc_bool_t *compok,
				    uint8_t *nasrc, sc_bool_t *psw_ovr)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_PAD;
	RPC_FUNC(&msg) = (uint8_t)PAD_FUNC_GET_GP_28FDSOI_COMP;
	RPC_U16(&msg, 0U) = (uint16_t)pad;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	if (compen != NULL) {
		*compen = RPC_U8(&msg, 0U);
	}

	if (rasrcp != NULL) {
		*rasrcp = RPC_U8(&msg, 1U);
	}

	if (rasrcn != NULL) {
		*rasrcn = RPC_U8(&msg, 2U);
	}

	if (nasrc != NULL) {
		*nasrc = RPC_U8(&msg, 3U);
	}

	if (fastfrz != NULL) {
		*fastfrz = RPC_U8(&msg, 4U);
	}

	if (nasrc_sel != NULL) {
		*nasrc_sel = RPC_U8(&msg, 5U);
	}

	if (compok != NULL) {
		*compok = RPC_U8(&msg, 6U);
	}

	if (psw_ovr != NULL) {
		*psw_ovr = RPC_U8(&msg, 7U);
	}

	return (sc_err_t)result;
}

/**@}*/
