/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 * Copyright (c) 2022, Linaro.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <common/desc_image_load.h>

/*
 * Add dummy functions for measured boot for BL1.
 * In most of the SoC's, ROM/BL1 code is pre-built. So we are assumimg that
 * it doesn't have the capability to do measurements and extend eventlog.
 * hence these are dummy functions.
 */
void bl1_plat_mboot_init(void)
{
}

void bl1_plat_mboot_finish(void)
{
}

int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data)
{
	return 0;
}
