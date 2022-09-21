/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <lib/fconf/fconf.h>

#include <platform_def.h>
#include <stm32mp_fconf_getter.h>

static int fconf_populate_stm32mp_fuse_provisioning(uintptr_t config)
{
	int node, child;

	/* As libfdt uses void *, we can't avoid this cast */
	const void *fdt = (void *)config;

	/* Assert the node offset point to "st,provisioning" compatible property */
	const char *compatible_str = "st,provisioning";

	node = fdt_node_offset_by_compatible(fdt, -1, compatible_str);
	if (node < 0) {
		/* The node is not mandatory, just return */
		return 0;
	}

	fdt_for_each_subnode(child, fdt, node) {
		int rc;
		const fdt32_t *cuint;
		int shadow_len;
		uint32_t phandle;
		uint32_t otp_id;
		uint32_t otp_len;
		uint32_t otp_len_word;
		unsigned int i;

		cuint = fdt_getprop(fdt, child, "nvmem-cells", NULL);
		if (cuint == NULL) {
			panic();
		}

		phandle = fdt32_to_cpu(*cuint);
		rc = dt_get_otp_by_phandle(phandle, &otp_id, &otp_len);
		if (rc != 0) {
			WARN("Phandle not found\n");
			panic();
		}

		cuint = fdt_getprop(fdt, child, "st,shadow-value", &shadow_len);
		if (cuint == NULL || shadow_len < 0 || (shadow_len % sizeof(uint32_t) != 0U)) {
			panic();
		}

		otp_len_word = otp_len / sizeof(uint32_t);
		if (otp_len % sizeof(uint32_t) != 0U) {
			otp_len_word++;
		}

		if (otp_len_word != (shadow_len / sizeof(uint32_t))) {
			WARN("Invalid OTP size\n");
			panic();
		}

		for (i = 0U; i < otp_len_word; i++) {
			uint32_t shadow_val;
			uint32_t otp_val = 0U;
			bool check_state = false;

			if (stm32_otp_read_sw_lock(otp_id + i, &check_state) != BSEC_OK) {
				panic();
			}

			if (check_state) {
				WARN("Shadow write lock\n");
				panic();
			}

			shadow_val = fdt32_to_cpu(*cuint++);

			if (stm32_otp_shadow_read(&otp_val, otp_id + i) != BSEC_OK) {
				panic();
			}

			if (otp_val != 0U) {
				INFO("Override the OTP %u initial value\n", otp_id);
			}

			if (stm32_otp_write(otp_val | shadow_val, otp_id + i) != BSEC_OK) {
				panic();
			}
		}
	}

	return 0;
}

FCONF_REGISTER_POPULATOR(TB_FW, stm32mp_fuse, fconf_populate_stm32mp_fuse_provisioning)
