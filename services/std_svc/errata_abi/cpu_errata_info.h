/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_CPUSPEC_H
#define ERRATA_CPUSPEC_H

#include <stdint.h>
#include <arch_helpers.h>

#if __aarch64__
#include <cortex_a35.h>
#include <cortex_a510.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <cortex_a55.h>
#include <cortex_a710.h>
#include <cortex_a72.h>
#include <cortex_a73.h>
#include <cortex_a75.h>
#include <cortex_a76.h>
#include <cortex_a77.h>
#include <cortex_a78.h>
#include <cortex_a78_ae.h>
#include <cortex_a78c.h>
#include <cortex_a715.h>
#include <cortex_x1.h>
#include <cortex_x2.h>
#include <cortex_x3.h>
#include <neoverse_n1.h>
#include <neoverse_n2.h>
#include <neoverse_v1.h>
#include <neoverse_v2.h>
#else
#include <cortex_a15.h>
#include <cortex_a17.h>
#include <cortex_a57.h>
#include <cortex_a9.h>
#endif

#define MAX_ERRATA_ENTRIES	32

#define ERRATA_LIST_END		(MAX_ERRATA_ENTRIES - 1)

/* Default values for unused memory in the array */
#define UNDEF_ERRATA		{UINT_MAX, UCHAR_MAX, UCHAR_MAX, false, false}

#define EXTRACT_PARTNUM(x)	((x >> MIDR_PN_SHIFT) & MIDR_PN_MASK)

#define RXPX_RANGE(x, y, z)	(((x >= y) && (x <= z)) ? true : false)

/*
 * CPU specific values for errata handling
 */
struct em_cpu{
	unsigned int em_errata_id;
	unsigned char em_rxpx_lo;	/* lowest revision of errata applicable for the cpu */
	unsigned char em_rxpx_hi;	/* highest revision of errata applicable for the cpu */
	bool errata_enabled;		/* indicate if errata enabled */
	/* flag to indicate if errata query is based out of non-arm interconnect */
	bool non_arm_interconnect;
};

struct em_cpu_list{
	/* field to hold cpu specific part number defined in midr reg */
	unsigned long cpu_partnumber;
	struct   em_cpu cpu_errata_list[MAX_ERRATA_ENTRIES];
};

int32_t verify_errata_implemented(uint32_t errata_id, uint32_t forward_flag);
#endif /* ERRATA_CPUSPEC_H */
