/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file for the RM RPC implementation.
 *
 * @addtogroup RM_SVC
 * @{
 */

#ifndef SCI_RM_RPC_H
#define SCI_RM_RPC_H

/* Includes */

/* Defines */

/*!
 * @name Defines for RPC RM function calls
 */
/*@{*/
#define RM_FUNC_UNKNOWN 0	/* Unknown function */
#define RM_FUNC_PARTITION_ALLOC 1U	/* Index for rm_partition_alloc() RPC call */
#define RM_FUNC_SET_CONFIDENTIAL 31U	/* Index for rm_set_confidential() RPC call */
#define RM_FUNC_PARTITION_FREE 2U	/* Index for rm_partition_free() RPC call */
#define RM_FUNC_GET_DID 26U	/* Index for rm_get_did() RPC call */
#define RM_FUNC_PARTITION_STATIC 3U	/* Index for rm_partition_static() RPC call */
#define RM_FUNC_PARTITION_LOCK 4U	/* Index for rm_partition_lock() RPC call */
#define RM_FUNC_GET_PARTITION 5U	/* Index for rm_get_partition() RPC call */
#define RM_FUNC_SET_PARENT 6U	/* Index for rm_set_parent() RPC call */
#define RM_FUNC_MOVE_ALL 7U	/* Index for rm_move_all() RPC call */
#define RM_FUNC_ASSIGN_RESOURCE 8U	/* Index for rm_assign_resource() RPC call */
#define RM_FUNC_SET_RESOURCE_MOVABLE 9U	/* Index for rm_set_resource_movable() RPC call */
#define RM_FUNC_SET_SUBSYS_RSRC_MOVABLE 28U	/* Index for rm_set_subsys_rsrc_movable() RPC call */
#define RM_FUNC_SET_MASTER_ATTRIBUTES 10U	/* Index for rm_set_master_attributes() RPC call */
#define RM_FUNC_SET_MASTER_SID 11U	/* Index for rm_set_master_sid() RPC call */
#define RM_FUNC_SET_PERIPHERAL_PERMISSIONS 12U	/* Index for rm_set_peripheral_permissions() RPC call */
#define RM_FUNC_IS_RESOURCE_OWNED 13U	/* Index for rm_is_resource_owned() RPC call */
#define RM_FUNC_IS_RESOURCE_MASTER 14U	/* Index for rm_is_resource_master() RPC call */
#define RM_FUNC_IS_RESOURCE_PERIPHERAL 15U	/* Index for rm_is_resource_peripheral() RPC call */
#define RM_FUNC_GET_RESOURCE_INFO 16U	/* Index for rm_get_resource_info() RPC call */
#define RM_FUNC_MEMREG_ALLOC 17U	/* Index for rm_memreg_alloc() RPC call */
#define RM_FUNC_MEMREG_SPLIT 29U	/* Index for rm_memreg_split() RPC call */
#define RM_FUNC_MEMREG_FREE 18U	/* Index for rm_memreg_free() RPC call */
#define RM_FUNC_FIND_MEMREG 30U	/* Index for rm_find_memreg() RPC call */
#define RM_FUNC_ASSIGN_MEMREG 19U	/* Index for rm_assign_memreg() RPC call */
#define RM_FUNC_SET_MEMREG_PERMISSIONS 20U	/* Index for rm_set_memreg_permissions() RPC call */
#define RM_FUNC_IS_MEMREG_OWNED 21U	/* Index for rm_is_memreg_owned() RPC call */
#define RM_FUNC_GET_MEMREG_INFO 22U	/* Index for rm_get_memreg_info() RPC call */
#define RM_FUNC_ASSIGN_PAD 23U	/* Index for rm_assign_pad() RPC call */
#define RM_FUNC_SET_PAD_MOVABLE 24U	/* Index for rm_set_pad_movable() RPC call */
#define RM_FUNC_IS_PAD_OWNED 25U	/* Index for rm_is_pad_owned() RPC call */
#define RM_FUNC_DUMP 27U	/* Index for rm_dump() RPC call */
/*@}*/

/* Types */

/* Functions */

/*!
 * This function dispatches an incoming RM RPC request.
 *
 * @param[in]     caller_pt   caller partition
 * @param[in]     msg         pointer to RPC message
 */
void rm_dispatch(sc_rm_pt_t caller_pt, sc_rpc_msg_t *msg);

/*!
 * This function translates and dispatches an RM RPC request.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     msg         pointer to RPC message
 */
void rm_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#endif /* SCI_RM_RPC_H */

/**@}*/
