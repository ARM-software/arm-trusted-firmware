/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <smccc.h>
#include <smccc_helpers.h>
#include <spci_svc.h>
#include <spinlock.h>
#include <sprt_host.h>
#include <sprt_svc.h>
#include <string.h>
#include <utils.h>

#include "spm_private.h"

/*******************************************************************************
 * Macros to print UUIDs.
 ******************************************************************************/
#define PRINT_UUID_FORMAT	"%08x-%08x-%08x-%08x"
#define PRINT_UUID_ARGS(x)	x[0], x[1], x[2], x[3]

/*******************************************************************************
 * Array of structs that contains information about all handles of Secure
 * Services that are currently open.
 ******************************************************************************/
typedef enum spci_handle_status {
	HANDLE_STATUS_CLOSED = 0,
	HANDLE_STATUS_OPEN,
} spci_handle_status_t;

typedef struct spci_handle {
	/* 16-bit value used as reference in all SPCI calls */
	uint16_t handle;

	/* Client ID of the client that requested the handle */
	uint16_t client_id;

	/* Current status of the handle */
	spci_handle_status_t status;

	/*
	 * Context of the Secure Partition that provides the Secure Service
	 * referenced by this handle.
	 */
	sp_context_t *sp_ctx;

	/*
	 * The same handle might be used for multiple requests, keep a reference
	 * counter of them.
	 */
	unsigned int num_active_requests;
} spci_handle_t;

static spci_handle_t spci_handles[PLAT_SPCI_HANDLES_MAX_NUM];
static spinlock_t spci_handles_lock;

/*
 * Given a handle and a client ID, return the element of the spci_handles
 * array that contains the information of the handle. It can only return open
 * handles. It returns NULL if it couldn't find the element in the array.
 */
static spci_handle_t *spci_handle_info_get(uint16_t handle, uint16_t client_id)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(spci_handles); i++) {
		spci_handle_t *h = &(spci_handles[i]);

		/* Only check for open handles */
		if (h->status == HANDLE_STATUS_CLOSED) {
			continue;
		}

		/* Check if either the handle or the client ID are different */
		if ((h->handle != handle) || (h->client_id != client_id)) {
			continue;
		}

		return h;
	}

	return NULL;
}

/*
 * Returns a unique value for a handle. This function must be called while
 * spci_handles_lock is locked. It returns 0 on success, -1 on error.
 */
static int spci_create_handle_value(uint16_t *handle)
{
	/*
	 * Trivial implementation that relies on the fact that any handle will
	 * be closed before 2^16 more handles have been opened.
	 */
	static uint16_t handle_count;

	*handle = handle_count;

	handle_count++;

	return 0;
}

/*******************************************************************************
 * This function looks for a Secure Partition that has a Secure Service
 * identified by the given UUID. It returns a handle that the client can use to
 * access the service, and an SPCI_*** error code.
 ******************************************************************************/
static uint64_t spci_service_handle_open_poll(void *handle, u_register_t x1,
			u_register_t x2, u_register_t x3, u_register_t x4,
			u_register_t x5, u_register_t x6, u_register_t x7)
{
	unsigned int i;
	sp_context_t *sp_ptr;
	uint16_t service_handle;

	/* Bits 31:16 of w7 are reserved (MBZ). */
	assert((x7 & 0xFFFF0000U) == 0);

	uint16_t client_id = x7 & 0x0000FFFFU;
	uint32_t uuid[4] = { x1, x2, x3, x4 };

	/* Get pointer to the Secure Partition that handles this service */
	sp_ptr = spm_sp_get_by_uuid(&uuid);
	if (sp_ptr == NULL) {
		WARN("SPCI: Service requested by client 0x%04x not found\n",
		     client_id);
		WARN("SPCI:   UUID: " PRINT_UUID_FORMAT "\n",
		     PRINT_UUID_ARGS(uuid));

		SMC_RET2(handle, SPCI_NOT_PRESENT, 0);
	}

	/* Get lock of the array of handles */
	spin_lock(&spci_handles_lock);

	/*
	 * We need to record the client ID and Secure Partition that correspond
	 * to this handle. Look for the first free entry in the array.
	 */
	for (i = 0; i < PLAT_SPCI_HANDLES_MAX_NUM; i++) {
		if (spci_handles[i].status == HANDLE_STATUS_CLOSED) {
			break;
		}
	}

	if (i == PLAT_SPCI_HANDLES_MAX_NUM) {
		spin_unlock(&spci_handles_lock);

		WARN("SPCI: Can't open more handles. Client 0x%04x\n",
		     client_id);
		WARN("SPCI:   UUID: " PRINT_UUID_FORMAT "\n",
		     PRINT_UUID_ARGS(uuid));

		SMC_RET2(handle, SPCI_NO_MEMORY, 0);
	}

	/* Create new handle value */
	if (spci_create_handle_value(&service_handle) != 0) {
		spin_unlock(&spci_handles_lock);

		WARN("SPCI: Can't create a new handle value. Client 0x%04x\n",
		     client_id);
		WARN("SPCI:   UUID: " PRINT_UUID_FORMAT "\n",
		     PRINT_UUID_ARGS(uuid));

		SMC_RET2(handle, SPCI_NO_MEMORY, 0);
	}

	/* Save all information about this handle */
	spci_handles[i].status = HANDLE_STATUS_OPEN;
	spci_handles[i].client_id = client_id;
	spci_handles[i].handle = service_handle;
	spci_handles[i].num_active_requests = 0U;
	spci_handles[i].sp_ctx = sp_ptr;

	/* Release lock of the array of handles */
	spin_unlock(&spci_handles_lock);

	VERBOSE("SPCI: Service handle request by client 0x%04x: 0x%04x\n",
		client_id, service_handle);
	VERBOSE("SPCI:   UUID: " PRINT_UUID_FORMAT "\n", PRINT_UUID_ARGS(uuid));

	/* The handle is returned in the top 16 bits of x1 */
	SMC_RET2(handle, SPCI_SUCCESS, ((uint32_t)service_handle) << 16);
}

/*******************************************************************************
 * This function closes a handle that a specific client uses to access a Secure
 * Service. It returns a SPCI_*** error code.
 ******************************************************************************/
static uint64_t spci_service_handle_close(void *handle, u_register_t x1)
{
	spci_handle_t *handle_info;
	uint16_t client_id = x1 & 0x0000FFFFU;
	uint16_t service_handle = (x1 >> 16) & 0x0000FFFFU;

	spin_lock(&spci_handles_lock);

	handle_info = spci_handle_info_get(service_handle, client_id);

	if (handle_info == NULL) {
		spin_unlock(&spci_handles_lock);

		WARN("SPCI: Tried to close invalid handle 0x%04x by client 0x%04x\n",
		     service_handle, client_id);

		SMC_RET1(handle, SPCI_INVALID_PARAMETER);
	}

	if (handle_info->status != HANDLE_STATUS_OPEN) {
		spin_unlock(&spci_handles_lock);

		WARN("SPCI: Tried to close handle 0x%04x by client 0x%04x in status %d\n",
			service_handle, client_id, handle_info->status);

		SMC_RET1(handle, SPCI_INVALID_PARAMETER);
	}

	if (handle_info->num_active_requests != 0U) {
		spin_unlock(&spci_handles_lock);

		/* A handle can't be closed if there are requests left */
		WARN("SPCI: Tried to close handle 0x%04x by client 0x%04x with %d requests left\n",
			service_handle, client_id,
			handle_info->num_active_requests);

		SMC_RET1(handle, SPCI_BUSY);
	}

	memset(handle_info, 0, sizeof(spci_handle_t));

	handle_info->status = HANDLE_STATUS_CLOSED;

	spin_unlock(&spci_handles_lock);

	VERBOSE("SPCI: Closed handle 0x%04x by client 0x%04x.\n",
		service_handle, client_id);

	SMC_RET1(handle, SPCI_SUCCESS);
}

/*******************************************************************************
 * This function requests a Secure Service from a given handle and client ID.
 ******************************************************************************/
static uint64_t spci_service_request_blocking(void *handle,
			uint32_t smc_fid, u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4, u_register_t x5,
			u_register_t x6, u_register_t x7)
{
	spci_handle_t *handle_info;
	sp_context_t *sp_ctx;
	cpu_context_t *cpu_ctx;
	uint32_t rx0;
	u_register_t rx1, rx2, rx3;
	uint16_t request_handle, client_id;

	/* Get handle array lock */
	spin_lock(&spci_handles_lock);

	/* Get pointer to struct of this open handle and client ID. */
	request_handle = (x7 >> 16U) & 0x0000FFFFU;
	client_id = x7 & 0x0000FFFFU;

	handle_info = spci_handle_info_get(request_handle, client_id);
	if (handle_info == NULL) {
		spin_unlock(&spci_handles_lock);

		WARN("SPCI_SERVICE_TUN_REQUEST_BLOCKING: Not found.\n");
		WARN("  Handle 0x%04x. Client ID 0x%04x\n", request_handle,
		     client_id);

		SMC_RET1(handle, SPCI_BUSY);
	}

	/* Get pointer to the Secure Partition that handles the service */
	sp_ctx = handle_info->sp_ctx;
	assert(sp_ctx != NULL);
	cpu_ctx = &(sp_ctx->cpu_ctx);

	/* Blocking requests are only allowed if the queue is empty */
	if (handle_info->num_active_requests > 0) {
		spin_unlock(&spci_handles_lock);

		SMC_RET1(handle, SPCI_BUSY);
	}

	/* Prevent this handle from being closed */
	handle_info->num_active_requests += 1;

	/* Release handle lock */
	spin_unlock(&spci_handles_lock);

	/* Save the Normal world context */
	cm_el1_sysregs_context_save(NON_SECURE);

	/* Wait until the Secure Partition is idle and set it to busy. */
	sp_state_wait_switch(sp_ctx, SP_STATE_IDLE, SP_STATE_BUSY);

	/* Pass arguments to the Secure Partition */
	struct sprt_queue_entry_message message = {
		.type = SPRT_MSG_TYPE_SERVICE_TUN_REQUEST,
		.client_id = client_id,
		.service_handle = request_handle,
		.session_id = x6,
		.token = 0, /* No token needed for blocking requests */
		.args = {smc_fid, x1, x2, x3, x4, x5}
	};

	spin_lock(&(sp_ctx->spm_sp_buffer_lock));
	int rc = sprt_push_message((void *)sp_ctx->spm_sp_buffer_base, &message,
				   SPRT_QUEUE_NUM_BLOCKING);
	spin_unlock(&(sp_ctx->spm_sp_buffer_lock));
	if (rc != 0) {
		/*
		 * This shouldn't happen, blocking requests can only be made if
		 * the request queue is empty.
		 */
		assert(rc == -ENOMEM);
		ERROR("SPCI_SERVICE_TUN_REQUEST_BLOCKING: Queue is full.\n");
		panic();
	}

	/* Jump to the Secure Partition. */
	rx0 = spm_sp_synchronous_entry(sp_ctx);

	/* Verify returned value */
	if (rx0 != SPRT_PUT_RESPONSE_AARCH64) {
		ERROR("SPM: %s: Unexpected x0 value 0x%x\n", __func__, rx0);
		panic();
	}

	rx1 = read_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X3);
	rx2 = read_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X4);
	rx3 = read_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X5);

	/* Flag Secure Partition as idle. */
	assert(sp_ctx->state == SP_STATE_BUSY);
	sp_state_set(sp_ctx, SP_STATE_IDLE);

	/* Decrease count of requests. */
	spin_lock(&spci_handles_lock);
	handle_info->num_active_requests -= 1;
	spin_unlock(&spci_handles_lock);

	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	SMC_RET4(handle, SPCI_SUCCESS, rx1, rx2, rx3);
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPCI.
 ******************************************************************************/
uint64_t spci_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie, void *handle,
			  uint64_t flags)
{
	uint32_t spci_fid;

	/* SPCI only supported from the Non-secure world for now */
	if (is_caller_non_secure(flags) == SMC_FROM_SECURE) {
		SMC_RET1(handle, SMC_UNK);
	}

	if ((smc_fid & SPCI_FID_TUN_FLAG) == 0) {

		/* Miscellaneous calls */

		spci_fid = (smc_fid >> SPCI_FID_MISC_SHIFT) & SPCI_FID_MISC_MASK;

		switch (spci_fid) {

		case SPCI_FID_VERSION:
			SMC_RET1(handle, SPCI_VERSION_COMPILED);

		case SPCI_FID_SERVICE_HANDLE_OPEN:
		{
			if ((smc_fid & SPCI_SERVICE_HANDLE_OPEN_NOTIFY_BIT) != 0) {
				/* Not supported for now */
				WARN("SPCI_SERVICE_HANDLE_OPEN_NOTIFY not supported.\n");
				SMC_RET1(handle, SPCI_INVALID_PARAMETER);
			}

			uint64_t x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
			uint64_t x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
			uint64_t x7 = SMC_GET_GP(handle, CTX_GPREG_X7);

			return spci_service_handle_open_poll(handle, x1, x2, x3,
							     x4, x5, x6, x7);
		}
		case SPCI_FID_SERVICE_HANDLE_CLOSE:
			return spci_service_handle_close(handle, x1);

		case SPCI_FID_SERVICE_REQUEST_BLOCKING:
		{
			uint64_t x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
			uint64_t x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
			uint64_t x7 = SMC_GET_GP(handle, CTX_GPREG_X7);

			return spci_service_request_blocking(handle,
					smc_fid, x1, x2, x3, x4, x5, x6, x7);
		}

		default:
			break;
		}

	} else {

		/* Tunneled calls */

	}

	WARN("SPCI: Unsupported call 0x%08x\n", smc_fid);
	SMC_RET1(handle, SPCI_NOT_SUPPORTED);
}
