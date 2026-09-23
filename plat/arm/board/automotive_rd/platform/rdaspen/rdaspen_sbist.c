/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <services/pfdi.h>
#include <services/pfdi_svc.h>

#include "rdaspen_sbist.h"

#define SBIST_FAIL 0x4u

/* SBIST Controller registers structure type definition */
struct a720ae_sbist_state_t {
	volatile uint64_t fctlr;
	volatile uint64_t fpir;
	volatile uint64_t ffmir;
	volatile uint64_t sidr;
	volatile uint64_t sor;
};

static struct a720ae_sbist_state_t *sbist_base[PLATFORM_CORE_COUNT] __aligned(64);

static void sbist_error_update(unsigned int cpu_id)
{
	uint64_t fctlr = SBIST_FAIL;
	uint8_t cluster_id;

	cluster_id = cpu_id / PLAT_MAX_CPUS_PER_CLUSTER;
	sbist_base[cpu_id] = (struct a720ae_sbist_state_t *)
			     (SBIST_BASE(cluster_id) +
			     ((cpu_id % PLAT_MAX_CPUS_PER_CLUSTER) * SZ_64K));

	mmio_write_64((uintptr_t)&sbist_base[cpu_id]->fctlr, fctlr);
}

static int64_t plat_sbist_err_validation(uint32_t fid, int64_t error_id)
{
	unsigned int cpu = plat_my_core_pos();

	if (error_id == PFDI_RET_ERROR) {
		/* The SBIST only updates in the event of a runtime error */
		if (fid == PFDI_PE_TEST_RUN) {
			sbist_error_update(cpu);
			WARN("CPU%d: SBIST error injected\n", cpu);
		}
	}

	return error_id;
}

static const struct plat_pfdi_func_desc rdaspen_pfdi_desc = {
	.name = "RD_ASPEN",
	.force_plat_err = NULL,
	.check_plat_err = plat_sbist_err_validation,
	.post_run = NULL
};

void rdaspen_pfdi_setup(void)
{
	pfdi_register_plat_func_desc(&rdaspen_pfdi_desc);
}
