/*
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_CPUSPEC_H
#define ERRATA_CPUSPEC_H

#include <stdint.h>
#include <arch.h>
#include <arch_helpers.h>

#if __aarch64__
#include <cortex_a710.h>
#include <cortex_a78.h>
#include <cortex_a78_ae.h>
#include <cortex_a78c.h>
#include <cortex_x2.h>
#include <cortex_x3.h>
#include <cortex_x4.h>
#include <neoverse_n2.h>
#include <neoverse_v1.h>
#include <neoverse_v2.h>
#endif

/* Max number of platform based errata with no workaround in EL3 */
#define MAX_PLAT_CPU_ERRATA_ENTRIES	2

#define ERRATA_LIST_END		(MAX_PLAT_CPU_ERRATA_ENTRIES - 1)

/* Default values for unused memory in the array */
#define UNDEF_ERRATA		{UINT_MAX, UCHAR_MAX, UCHAR_MAX}

#define RXPX_RANGE(x, y, z)	(((x >= y) && (x <= z)) ? true : false)

/*
 * CPU specific values for errata handling
 */
struct em_cpu{
	unsigned int em_errata_id;
	unsigned char em_rxpx_lo;	/* lowest revision of errata applicable for the cpu */
	unsigned char em_rxpx_hi;	/* highest revision of errata applicable for the cpu */
};

struct em_cpu_list{
	unsigned long cpu_partnumber;	/* cpu specific part number defined in midr reg */
	struct   em_cpu cpu_errata_list[MAX_PLAT_CPU_ERRATA_ENTRIES];
};

int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag);
#endif /* ERRATA_CPUSPEC_H */
