/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <lib/bakery_lock.h>
#include <drivers/console.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat_dcm.h>
#include <plat_private.h>
#include <plat_dcm.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <mtk_plat_common.h>

#define PWR_STATUS                     (SPM_BASE + 0x180)

uint64_t plat_dcm_mcsi_a_addr;
uint32_t plat_dcm_mcsi_a_val;
static int plat_dcm_init_type;
static unsigned int dcm_big_core_cnt;
int plat_dcm_initiated;

#define PWR_STA_BIG_MP_MASK	(0x1 << 15)

DEFINE_BAKERY_LOCK(dcm_lock);

void dcm_lock_init(void)
{
	bakery_lock_init(&dcm_lock);
}

void dcm_lock_get(void)
{
	bakery_lock_get(&dcm_lock);
}

void dcm_lock_release(void)
{
	bakery_lock_release(&dcm_lock);
}

void plat_dcm_mcsi_a_backup(void)
{
}

void plat_dcm_mcsi_a_restore(void)
{
}

void plat_dcm_rgu_enable(void)
{
}

void plat_dcm_big_core_sync(short on)
{
	/* Check if Big cluster power is existed */
	if (!(mmio_read_32(PWR_STATUS) & PWR_STA_BIG_MP_MASK))
		return;

	if (on) {
		mmio_write_32(MP2_SYNC_DCM,
			      (mmio_read_32(MP2_SYNC_DCM) & ~MP2_SYNC_DCM_MASK)
			      | MP2_SYNC_DCM_ON);
		dcm_big_core_cnt++;
	} else
		mmio_write_32(MP2_SYNC_DCM,
			      (mmio_read_32(MP2_SYNC_DCM) & ~MP2_SYNC_DCM_MASK)
			      | MP2_SYNC_DCM_OFF);
}

void plat_dcm_restore_cluster_on(unsigned long mpidr)
{
	unsigned long cluster_id =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	switch (cluster_id) {
	case 0x1:
		dcm_lock_get();
		if (plat_dcm_init_type & BIG_CORE_DCM_TYPE)
			plat_dcm_big_core_sync(1);
		else
			plat_dcm_big_core_sync(0);
		dcm_lock_release();
		break;
	default:
		break;
	}
}

void plat_dcm_msg_handler(uint64_t x1)
{
	plat_dcm_init_type = x1 & ALL_DCM_TYPE;
}

unsigned long plat_dcm_get_enabled_cnt(uint64_t type)
{
	switch (type) {
	case BIG_CORE_DCM_TYPE:
		return dcm_big_core_cnt;
	default:
		return 0;
	}
}

void plat_dcm_init(void)
{
	dcm_lock_init();
}
