/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <emi_mpu.h>

int is_4GB(void)
{
	return 0; /* 8183 doesn't use 4GB */
}

/*
 * emi_mpu_set_region_protection: protect a region.
 * @start: start address of the region
 * @end: end address of the region
 * @region: EMI MPU region id
 * @access_permission: EMI MPU access permission
 * Return 0 for success, otherwise negative status code.
 */
int emi_mpu_set_region_protection(
	unsigned long start, unsigned long end,
	int region,
	unsigned int access_permission)
{
	int ret = 0;

	if (end <= start) {
		ERROR("[EMI][MTEE][MPU] Invalid address!.\n");
		return -1;
	}

	if (is_4GB()) {
		/* 4GB mode: emi_addr = phy_addr & 0xffff */
		start = EMI_PHY_OFFSET & 0xffff;
		end = EMI_PHY_OFFSET & 0xffff;
	} else {
		/* non-4GB mode: emi_addr = phy_addr - MEM_OFFSET */
		start = start - EMI_PHY_OFFSET;
		end = end - EMI_PHY_OFFSET;
	}

	/*Address 64KB alignment*/
	start = start >> 16;
	end = end >> 16;

	switch (region) {
	case 0:
		mmio_write_32(EMI_MPU_APC0, 0);
		mmio_write_32(EMI_MPU_SA0, start);
		mmio_write_32(EMI_MPU_EA0, end);
		mmio_write_32(EMI_MPU_APC0, access_permission);
		break;

	case 1:
		mmio_write_32(EMI_MPU_APC1, 0);
		mmio_write_32(EMI_MPU_SA1, start);
		mmio_write_32(EMI_MPU_EA1, end);
		mmio_write_32(EMI_MPU_APC1, access_permission);
		break;

	case 2:
		mmio_write_32(EMI_MPU_APC2, 0);
		mmio_write_32(EMI_MPU_SA2, start);
		mmio_write_32(EMI_MPU_EA2, end);
		mmio_write_32(EMI_MPU_APC2, access_permission);
		break;

	case 3:
		mmio_write_32(EMI_MPU_APC3, 0);
		mmio_write_32(EMI_MPU_SA3, start);
		mmio_write_32(EMI_MPU_EA3, end);
		mmio_write_32(EMI_MPU_APC3, access_permission);
		break;

	case 4:
		mmio_write_32(EMI_MPU_APC4, 0);
		mmio_write_32(EMI_MPU_SA4, start);
		mmio_write_32(EMI_MPU_EA4, end);
		mmio_write_32(EMI_MPU_APC4, access_permission);
		break;

	case 5:
		mmio_write_32(EMI_MPU_APC5, 0);
		mmio_write_32(EMI_MPU_SA5, start);
		mmio_write_32(EMI_MPU_EA5, end);
		mmio_write_32(EMI_MPU_APC5, access_permission);
		break;

	case 6:
		mmio_write_32(EMI_MPU_APC6, 0);
		mmio_write_32(EMI_MPU_SA6, start);
		mmio_write_32(EMI_MPU_EA6, end);
		mmio_write_32(EMI_MPU_APC6, access_permission);
		break;

	case 7:
		mmio_write_32(EMI_MPU_APC7, 0);
		mmio_write_32(EMI_MPU_SA7, start);
		mmio_write_32(EMI_MPU_EA7, end);
		mmio_write_32(EMI_MPU_APC7, access_permission);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

void dump_emi_mpu_regions(void)
{
	unsigned int apc, sa, ea;
	unsigned int apc_addr = EMI_MPU_APC0;
	unsigned int sa_addr = EMI_MPU_SA0;
	unsigned int ea_addr = EMI_MPU_EA0;
	int i;

	for (i = 0; i < 8; ++i) {
		apc = mmio_read_32(apc_addr + i * 4);
		sa = mmio_read_32(sa_addr + i * 4);
		ea = mmio_read_32(ea_addr + i * 4);
		WARN("region %d:\n", i);
		WARN("\tapc:0x%x, sa:0x%x, ea:0x%x\n", apc, sa, ea);
	}
}

void emi_mpu_init(void)
{
	/* Set permission */
	emi_mpu_set_region_protection(0x40000000UL, 0x4FFFFFFFUL, 0,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x50000000UL, 0x528FFFFFUL, 1,
				(FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x52900000UL, 0x5FFFFFFFUL, 2,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x60000000UL, 0x7FFFFFFFUL, 3,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0x80000000UL, 0x9FFFFFFFUL, 4,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0xA0000000UL, 0xBFFFFFFFUL, 5,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0xC0000000UL, 0xDFFFFFFFUL, 6,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	emi_mpu_set_region_protection(0xE0000000UL, 0xFFFFFFFFUL, 7,
				(FORBIDDEN << 3 | FORBIDDEN << 6));
	dump_emi_mpu_regions();
}

