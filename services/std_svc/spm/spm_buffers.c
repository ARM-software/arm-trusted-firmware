/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <platform_def.h>

#include "./spm_private.h"

/*******************************************************************************
 * Secure Service response global array. All the responses to the requests done
 * to the Secure Partition are stored here. They are removed from the array as
 * soon as their value is read.
 ******************************************************************************/
struct sprt_response {
	int is_valid;
	uint32_t token;
	uint16_t client_id, handle;
	u_register_t x1, x2, x3;
};

static struct sprt_response responses[PLAT_SPM_RESPONSES_MAX];

static spinlock_t responses_lock;

/* Add response to the global response buffer. Returns 0 on success else -1. */
int spm_response_add(uint16_t client_id, uint16_t handle, uint32_t token,
		     u_register_t x1, u_register_t x2, u_register_t x3)
{
	spin_lock(&responses_lock);

	/* Make sure that there isn't any other response with the same token. */
	for (unsigned int i = 0U; i < ARRAY_SIZE(responses); i++) {
		struct sprt_response *resp = &(responses[i]);

		if ((resp->is_valid == 1) && (resp->token == token)) {
			spin_unlock(&responses_lock);

			return -1;
		}
	}

	for (unsigned int i = 0U; i < ARRAY_SIZE(responses); i++) {
		struct sprt_response *resp = &(responses[i]);

		if (resp->is_valid == 0) {
			resp->token = token;
			resp->client_id = client_id;
			resp->handle = handle;
			resp->x1 = x1;
			resp->x2 = x2;
			resp->x3 = x3;

			dmbish();

			resp->is_valid = 1;

			spin_unlock(&responses_lock);

			return 0;
		}
	}

	spin_unlock(&responses_lock);

	return -1;
}

/*
 * Returns a response from the requests array and removes it from it. Returns 0
 * on success, -1 if it wasn't found.
 */
int spm_response_get(uint16_t client_id, uint16_t handle, uint32_t token,
		     u_register_t *x1, u_register_t *x2, u_register_t *x3)
{
	spin_lock(&responses_lock);

	for (unsigned int i = 0U; i < ARRAY_SIZE(responses); i++) {
		struct sprt_response *resp = &(responses[i]);

		/* Ignore invalid entries */
		if (resp->is_valid == 0) {
			continue;
		}

		/* Make sure that all the information matches the stored one */
		if ((resp->token != token) || (resp->client_id != client_id) ||
		    (resp->handle != handle)) {
			continue;
		}

		*x1 = resp->x1;
		*x2 = resp->x2;
		*x3 = resp->x3;

		dmbish();

		resp->is_valid = 0;

		spin_unlock(&responses_lock);

		return 0;
	}

	spin_unlock(&responses_lock);

	return -1;
}
