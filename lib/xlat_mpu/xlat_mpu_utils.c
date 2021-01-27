/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include "xlat_mpu_private.h"

#include <fvp_r_arch_helpers.h>
#include <platform_def.h>

#warning "xlat_mpu library is currently experimental and its API may change in future."


#if LOG_LEVEL < LOG_LEVEL_VERBOSE

void xlat_mmap_print(__unused const mmap_region_t *mmap)
{
	/* Empty */
}

void xlat_tables_print(__unused xlat_ctx_t *ctx)
{
	/* Empty */
}

#else /* if LOG_LEVEL >= LOG_LEVEL_VERBOSE */

static const char *invalid_descriptors_ommited =
		"%s(%d invalid descriptors omitted)\n";

void xlat_tables_print(xlat_ctx_t *ctx)
{
	const char *xlat_regime_str;
	int used_page_tables;

	if (ctx->xlat_regime == EL1_EL0_REGIME) {
		xlat_regime_str = "1&0";
	} else if (ctx->xlat_regime == EL2_REGIME) {
		xlat_regime_str = "2";
	} else {
		assert(ctx->xlat_regime == EL3_REGIME);
		xlat_regime_str = "3";
		/* If no EL3 and EL3 tables generated, then need to know. */
	}
	VERBOSE("Translation tables state:\n");
	VERBOSE("  Xlat regime:     EL%s\n", xlat_regime_str);
	VERBOSE("  Max allowed PA:  0x%llx\n", ctx->pa_max_address);
	VERBOSE("  Max allowed VA:  0x%lx\n", ctx->va_max_address);
	VERBOSE("  Max mapped PA:   0x%llx\n", ctx->max_pa);
	VERBOSE("  Max mapped VA:   0x%lx\n", ctx->max_va);

	VERBOSE("  Initial lookup level: %u\n", ctx->base_level);
	VERBOSE("  Entries @initial lookup level: %u\n",
		ctx->base_table_entries);

	xlat_tables_print_internal(ctx, 0U, ctx->base_table,
				   ctx->base_table_entries, ctx->base_level);
}

#endif /* LOG_LEVEL >= LOG_LEVEL_VERBOSE */
