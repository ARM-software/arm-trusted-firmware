/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdio.h>

#include <drivers/arm/rse_comms.h>
#include <plat/common/platform.h>
#include <rse_platform_api.h>
#include <tc_plat.h>

static void print_hex(const char *key_id_name, size_t key_size, const uint8_t *key_buf)
{
	printf("%s = ", key_id_name);
	for (int i = 0; i < key_size; i++) {
		printf("%02x", key_buf[i]);
	}
	printf("\n\n");
}

int rotpk_test(void)
{
	psa_status_t status;
	uint8_t key_buf[128];
	size_t key_size;

	struct key_id_info key_ids[3] = {
	       {.key_id = RSE_BUILTIN_KEY_ID_HOST_S_ROTPK,  .key_id_name = "Secure-ROTPK"},
	       {.key_id = RSE_BUILTIN_KEY_ID_HOST_NS_ROTPK,  .key_id_name = "NS-ROTPK"},
	       {.key_id = RSE_BUILTIN_KEY_ID_HOST_CCA_ROTPK,  .key_id_name = "CCA-ROTPK"}
	};

	status = rse_comms_init(PLAT_RSE_AP_SND_MHU_BASE, PLAT_RSE_AP_RCV_MHU_BASE);
	if (status != PSA_SUCCESS) {
		printf("Failed to initialize RSE communication channel - psa_status = %d\n", status);
		return -1;
	}

	for (int i = 0; i < ARRAY_SIZE(key_ids); i++) {
		status = rse_platform_key_read(key_ids[i].key_id, key_buf,
			       sizeof(key_buf), &key_size);
		if (status != PSA_SUCCESS) {
			printf("Failed to retrieve %s - psa_status = %d\n", key_ids[i].key_id_name, status);
			return -1;
		}
		print_hex(key_ids[i].key_id_name, key_size, key_buf);
	}

	printf("Passed rotpk_test\n");

	return 0;
}
