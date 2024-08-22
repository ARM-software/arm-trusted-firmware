/*
 * Copyright 2023-2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

typedef struct {
	uint8_t flag_sm;
	uint8_t flag_ap;
	uint8_t flag_unused1;
	uint8_t flag_unused2;
	uint32_t turn;
} sm_ap_semaphore_t;

/* Used for GIC_WAKER sync between AP and SM. */
void request_sm_ap_sema(void)
{
	volatile sm_ap_semaphore_t *sm_ap_sema = (sm_ap_semaphore_t *)SM_AP_SEMA_ADDR;

	sm_ap_sema->flag_ap = 1U;
	sm_ap_sema->turn = 0U;

	while (sm_ap_sema->flag_sm && sm_ap_sema->turn == 0U) {
		/* Wait for SM to be done in the critical section. */
		;
	}
}

/* Used for GIC_WAKER sync between AP and SM. */
void release_sm_ap_sema(void)
{
	sm_ap_semaphore_t *sm_ap_sema = (sm_ap_semaphore_t *)SM_AP_SEMA_ADDR;

	sm_ap_sema->flag_ap = 0U;
}

