/*
 * Copyright (c) 2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef STM32MP1_IMG_DEF_H
#define STM32MP1_IMG_DEF_H

#include <export/common/tbbr/tbbr_img_def_exp.h>

/* Undef the existing values */
#undef BL32_EXTRA1_IMAGE_ID
#undef BL32_EXTRA2_IMAGE_ID
#undef TOS_FW_CONFIG_ID
#undef TRUSTED_BOOT_FW_CERT_ID
#undef FWU_METADATA_IMAGE_ID
#undef BKUP_FWU_METADATA_IMAGE_ID
#undef FW_CONFIG_ID
#undef HW_CONFIG_ID
#undef GPT_IMAGE_ID
#undef ENC_IMAGE_ID

/* Define the STM32MP1 used ID */
#define FW_CONFIG_ID			U(1)
#define HW_CONFIG_ID			U(2)
#define GPT_IMAGE_ID			U(3)
#define ENC_IMAGE_ID			U(6)
#define BL32_EXTRA1_IMAGE_ID		U(8)
#define BL32_EXTRA2_IMAGE_ID		U(9)
#define FWU_METADATA_IMAGE_ID		U(12)
#define BKUP_FWU_METADATA_IMAGE_ID	U(13)
#define TOS_FW_CONFIG_ID		U(16)
#define STM32MP_CONFIG_CERT_ID		U(17)

/* Increase the MAX_NUMBER_IDS to match the authentication pool required */
#define MAX_NUMBER_IDS			U(19)

#endif	/* STM32MP1_IMG_DEF_H */
