/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file for the MISC RPC implementation.
 *
 * @addtogroup MISC_SVC
 * @{
 */

#ifndef SC_MISC_RPC_H
#define SC_MISC_RPC_H

/* Includes */

/* Defines */

/*!
 * @name Defines for RPC MISC function calls
 */
/*@{*/
#define MISC_FUNC_UNKNOWN 0	/* Unknown function */
#define MISC_FUNC_SET_CONTROL 1U	/* Index for misc_set_control() RPC call */
#define MISC_FUNC_GET_CONTROL 2U	/* Index for misc_get_control() RPC call */
#define MISC_FUNC_SET_MAX_DMA_GROUP 4U	/* Index for misc_set_max_dma_group() RPC call */
#define MISC_FUNC_SET_DMA_GROUP 5U	/* Index for misc_set_dma_group() RPC call */
#define MISC_FUNC_SECO_IMAGE_LOAD 8U	/* Index for misc_seco_image_load() RPC call */
#define MISC_FUNC_SECO_AUTHENTICATE 9U	/* Index for misc_seco_authenticate() RPC call */
#define MISC_FUNC_SECO_FUSE_WRITE 20U	/* Index for misc_seco_fuse_write() RPC call */
#define MISC_FUNC_SECO_ENABLE_DEBUG 21U	/* Index for misc_seco_enable_debug() RPC call */
#define MISC_FUNC_SECO_FORWARD_LIFECYCLE 22U	/* Index for misc_seco_forward_lifecycle() RPC call */
#define MISC_FUNC_SECO_RETURN_LIFECYCLE 23U	/* Index for misc_seco_return_lifecycle() RPC call */
#define MISC_FUNC_SECO_BUILD_INFO 24U	/* Index for misc_seco_build_info() RPC call */
#define MISC_FUNC_SECO_CHIP_INFO 25U	/* Index for misc_seco_chip_info() RPC call */
#define MISC_FUNC_DEBUG_OUT 10U	/* Index for misc_debug_out() RPC call */
#define MISC_FUNC_WAVEFORM_CAPTURE 6U	/* Index for misc_waveform_capture() RPC call */
#define MISC_FUNC_BUILD_INFO 15U	/* Index for misc_build_info() RPC call */
#define MISC_FUNC_UNIQUE_ID 19U	/* Index for misc_unique_id() RPC call */
#define MISC_FUNC_SET_ARI 3U	/* Index for misc_set_ari() RPC call */
#define MISC_FUNC_BOOT_STATUS 7U	/* Index for misc_boot_status() RPC call */
#define MISC_FUNC_BOOT_DONE 14U	/* Index for misc_boot_done() RPC call */
#define MISC_FUNC_OTP_FUSE_READ 11U	/* Index for misc_otp_fuse_read() RPC call */
#define MISC_FUNC_OTP_FUSE_WRITE 17U	/* Index for misc_otp_fuse_write() RPC call */
#define MISC_FUNC_SET_TEMP 12U	/* Index for misc_set_temp() RPC call */
#define MISC_FUNC_GET_TEMP 13U	/* Index for misc_get_temp() RPC call */
#define MISC_FUNC_GET_BOOT_DEV 16U	/* Index for misc_get_boot_dev() RPC call */
#define MISC_FUNC_GET_BUTTON_STATUS 18U	/* Index for misc_get_button_status() RPC call */
/*@}*/

/* Types */

/* Functions */

/*!
 * This function dispatches an incoming MISC RPC request.
 *
 * @param[in]     caller_pt   caller partition
 * @param[in]     msg         pointer to RPC message
 */
void misc_dispatch(sc_rm_pt_t caller_pt, sc_rpc_msg_t *msg);

/*!
 * This function translates and dispatches an MISC RPC request.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     msg         pointer to RPC message
 */
void misc_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#endif				/* SC_MISC_RPC_H */

/**@}*/
