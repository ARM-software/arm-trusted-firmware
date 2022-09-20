/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <lib/mmio.h>

enum mtk_audio_smc_call_op {
	MTK_AUDIO_SMC_OP_INIT = 0,
	MTK_AUDIO_SMC_OP_DRAM_REQUEST,
	MTK_AUDIO_SMC_OP_DRAM_RELEASE,
	MTK_AUDIO_SMC_OP_SRAM_REQUEST,
	MTK_AUDIO_SMC_OP_SRAM_RELEASE,
	MTK_AUDIO_SMC_OP_ADSP_REQUEST,
	MTK_AUDIO_SMC_OP_ADSP_RELEASE,
	MTK_AUDIO_SMC_OP_DOMAIN_SIDEBANDS,
	MTK_AUDIO_SMC_OP_BTCVSD_WRITE,
	MTK_AUDIO_SMC_OP_BTCVSD_UPDATE_CTRL_CLEAR,
	MTK_AUDIO_SMC_OP_BTCVSD_UPDATE_CTRL_UNDERFLOW,
	MTK_AUDIO_SMC_OP_NUM,
};

int32_t set_audio_domain_sidebands(void);

#endif /* AUDIO_H */
