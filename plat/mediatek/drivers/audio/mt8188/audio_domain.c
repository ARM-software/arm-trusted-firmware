/*
 * Copyright (c) 2022, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <common/debug.h>

#include <audio.h>
#include <mt_audio_private.h>
#include <mtk_mmap_pool.h>
#include <platform_def.h>
#include <spm_reg.h>

#define MODULE_TAG "[AUDIO_DOMAIN]"

int32_t set_audio_domain_sidebands(void)
{
	uint32_t val = mmio_read_32(PWR_STATUS);

	if ((val & BIT(SPM_PWR_STATUS_AUDIO_BIT)) == 0) {
		ERROR("%s: %s, pwr_status=0x%x, w/o [%d]AUDIO!\n",
		      MODULE_TAG, __func__, val, SPM_PWR_STATUS_AUDIO_BIT);
		return -EIO;
	}

	mmio_write_32(AFE_SE_SECURE_CON, 0x0);

	mmio_write_32(AFE_SECURE_SIDEBAND0, 0x0);
	mmio_write_32(AFE_SECURE_SIDEBAND1, 0x0);
	mmio_write_32(AFE_SECURE_SIDEBAND2, 0x0);
	mmio_write_32(AFE_SECURE_SIDEBAND3, 0x0);

	VERBOSE("%s: %s, SE_SECURE_CON=0x%x, SIDEBAND0/1/2/3=0x%x/0x%x/0x%x/0x%x\n",
		MODULE_TAG, __func__,
		mmio_read_32(AFE_SE_SECURE_CON),
		mmio_read_32(AFE_SECURE_SIDEBAND0),
		mmio_read_32(AFE_SECURE_SIDEBAND1),
		mmio_read_32(AFE_SECURE_SIDEBAND2),
		mmio_read_32(AFE_SECURE_SIDEBAND3));

	return 0;
}
