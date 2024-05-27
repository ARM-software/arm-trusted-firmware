/*
 * Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EMI_MPU_H
#define EMI_MPU_H

#include <emi_mpu_priv.h>
#include <platform_def.h>

#define NO_PROTECTION			(0)
#define SEC_RW				(1)
#define SEC_RW_NSEC_R			(2)
#define SEC_RW_NSEC_W			(3)
#define SEC_R_NSEC_R			(4)
#define FORBIDDEN			(5)
#define SEC_R_NSEC_RW			(6)

#define LOCK				(1UL)
#define UNLOCK				(0)

#if (EMI_MPU_DGROUP_NUM == 1)
#define SET_ACCESS_PERMISSION(apc_ary, lock, d7, d6, d5, d4, d3, d2, d1, d0) \
do { \
	apc_ary[1] = 0; \
	apc_ary[0] = \
		(((unsigned int)  d7) << 21) | (((unsigned int)  d6) << 18) | \
		(((unsigned int)  d5) << 15) | (((unsigned int)  d4) << 12) | \
		(((unsigned int)  d3) <<  9) | (((unsigned int)  d2) <<  6) | \
		(((unsigned int)  d1) <<  3) |  ((unsigned int)  d0) | \
		((unsigned int) lock << 31); \
} while (0)
#elif (EMI_MPU_DGROUP_NUM == 2)
#define SET_ACCESS_PERMISSION(apc_ary, lock, d15, d14, d13, d12, d11, d10, \
				d9, d8, d7, d6, d5, d4, d3, d2, d1, d0) \
do { \
	apc_ary[1] = \
		(((unsigned int) d15) << 21) | (((unsigned int) d14) << 18) | \
		(((unsigned int) d13) << 15) | (((unsigned int) d12) << 12) | \
		(((unsigned int) d11) <<  9) | (((unsigned int) d10) <<  6) | \
		(((unsigned int)  d9) <<  3) |  ((unsigned int)  d8); \
	apc_ary[0] = \
		(((unsigned int)  d7) << 21) | (((unsigned int)  d6) << 18) | \
		(((unsigned int)  d5) << 15) | (((unsigned int)  d4) << 12) | \
		(((unsigned int)  d3) <<  9) | (((unsigned int)  d2) <<  6) | \
		(((unsigned int)  d1) <<  3) |  ((unsigned int)  d0) | \
		((unsigned int) lock << 31); \
} while (0)
#endif

struct emi_region_info_t {
	unsigned long long start;
	unsigned long long end;
	unsigned int region;
	unsigned int apc[EMI_MPU_DGROUP_NUM];
};

enum MPU_REQ_ORIGIN_ZONE_ID {
	MPU_REQ_ORIGIN_TEE_ZONE_SVP = 0,
	MPU_REQ_ORIGIN_TEE_ZONE_TUI = 1,
	MPU_REQ_ORIGIN_TEE_ZONE_WFD = 2,
	MPU_REQ_ORIGIN_TEE_ZONE_MAX = 3,
	MPU_REQ_ORIGIN_ZONE_INVALID = 0x7FFFFFFF,
};

int emi_mpu_init(void);
int emi_mpu_optee_handler(uint64_t encoded_addr, uint64_t zone_size,
						  uint64_t zone_info);
int emi_mpu_set_protection(struct emi_region_info_t *region_info);
int emi_mpu_clear_protection(unsigned int region);
void set_emi_mpu_regions(void);
int set_apu_emi_mpu_region(void);
#endif
