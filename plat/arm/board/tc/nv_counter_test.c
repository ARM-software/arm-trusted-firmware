/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>

#include <drivers/arm/rss_comms.h>
#include <plat/common/platform.h>
#include "rss_platform_api.h"

#include <platform_def.h>

int nv_counter_test(void)
{
	psa_status_t status;
	uint32_t old_val;
	uint32_t new_val;
	uint32_t id;

	status = rss_comms_init(PLAT_RSS_AP_SND_MHU_BASE, PLAT_RSS_AP_RCV_MHU_BASE);
	if (status != PSA_SUCCESS) {
		printf("Failed to initialize RSS communication channel - psa_status = %d\n", status);
		return -1;
	}

	for (id = 0; id < 3; id++) {
		status = rss_platform_nv_counter_read(id, sizeof(old_val), (uint8_t *)&old_val);
		if (status != PSA_SUCCESS) {
			printf("Failed during first id=(%d) rss_platform_nv_counter_read - psa_status = %d\n",
				       id, status);
			return -1;
		}

		status = rss_platform_nv_counter_increment(id);
		if (status != PSA_SUCCESS) {
			printf("Failed during id=(%d) rss_platform_nv_counter_increment - psa_status = %d\n",
					id, status);
			return -1;
		}

		status = rss_platform_nv_counter_read(id, sizeof(new_val), (uint8_t *)&new_val);
		if (status != PSA_SUCCESS) {
			printf("Failed during second id=(%d) rss_platform_nv_counter_read - psa_status = %d\n",
					id, status);
			return -1;
		}

		if (old_val + 1 != new_val) {
			printf("Failed nv_counter_test: old_val (%d) + 1 != new_val (%d)\n",
					old_val, new_val);
			return -1;
		}
	}
	printf("Passed nv_counter_test\n");

	return 0;
}
