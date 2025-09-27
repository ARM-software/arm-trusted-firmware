/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include "rcar_def.h"
#include "rcar_private.h"

#define RCAR_CNTCVL_OFF				(0x08U)
#define RCAR_CNTCVU_OFF				(0x0CU)

static uint64_t rcar_pwrc_saved_cntpct_el0;
static uint32_t rcar_pwrc_saved_cntfid;

void rcar_pwrc_save_timer_state(void)
{
	rcar_pwrc_saved_cntpct_el0 = read_cntpct_el0();

	rcar_pwrc_saved_cntfid =
		mmio_read_32((uintptr_t)(RCAR_CNTC_BASE + CNTFID_OFF));
}

void rcar_pwrc_restore_timer_state(void)
{
	/* Stop timer before restoring counter value */
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + CNTCR_OFF), 0U);

	/* restore lower counter value */
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCVL_OFF),
		(uint32_t)(rcar_pwrc_saved_cntpct_el0 & 0xFFFFFFFFU));
	/* restore upper counter value */
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + RCAR_CNTCVU_OFF),
		(uint32_t)(rcar_pwrc_saved_cntpct_el0 >> 32U));
	/* restore counter frequency setting */
	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + CNTFID_OFF),
		rcar_pwrc_saved_cntfid);

	/* Start generic timer back */
	write_cntfrq_el0((u_register_t)plat_get_syscnt_freq2());

	mmio_write_32((uintptr_t)(RCAR_CNTC_BASE + CNTCR_OFF),
			CNTCR_FCREQ((uint32_t)(0)) | CNTCR_EN);
}
