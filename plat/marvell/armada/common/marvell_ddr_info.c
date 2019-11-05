/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <platform_def.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <ddr_info.h>

#define DRAM_CH0_MMAP_LOW_REG(iface, cs, base)	\
	(base + DRAM_CH0_MMAP_LOW_OFFSET + (iface) * 0x10000 + (cs) * 0x8)
#define DRAM_CH0_MMAP_HIGH_REG(iface, cs, base)	\
	(DRAM_CH0_MMAP_LOW_REG(iface, cs, base) + 4)
#define DRAM_CS_VALID_ENABLED_MASK		0x1
#define DRAM_AREA_LENGTH_OFFS			16
#define DRAM_AREA_LENGTH_MASK			(0x1f << DRAM_AREA_LENGTH_OFFS)
#define DRAM_START_ADDRESS_L_OFFS		23
#define DRAM_START_ADDRESS_L_MASK		\
					(0x1ff << DRAM_START_ADDRESS_L_OFFS)
#define DRAM_START_ADDR_HTOL_OFFS		32

#define DRAM_MAX_CS_NUM				2

#define DRAM_CS_ENABLED(iface, cs, base) \
	(mmio_read_32(DRAM_CH0_MMAP_LOW_REG(iface, cs, base)) & \
	 DRAM_CS_VALID_ENABLED_MASK)
#define GET_DRAM_REGION_SIZE_CODE(iface, cs, base) \
	(mmio_read_32(DRAM_CH0_MMAP_LOW_REG(iface, cs, base)) & \
	DRAM_AREA_LENGTH_MASK) >> DRAM_AREA_LENGTH_OFFS

/* Mapping between DDR area length and real DDR size is specific and looks like
 * bellow:
 * 0 => 384 MB
 * 1 => 768 MB
 * 2 => 1536 MB
 * 3 => 3 GB
 * 4 => 6 GB
 *
 * 7 => 8 MB
 * 8 => 16 MB
 * 9 => 32 MB
 * 10 => 64 MB
 * 11 => 128 MB
 * 12 => 256 MB
 * 13 => 512 MB
 * 14 => 1 GB
 * 15 => 2 GB
 * 16 => 4 GB
 * 17 => 8 GB
 * 18 => 16 GB
 * 19 => 32 GB
 * 20 => 64 GB
 * 21 => 128 GB
 * 22 => 256 GB
 * 23 => 512 GB
 * 24 => 1 TB
 * 25 => 2 TB
 * 26 => 4 TB
 *
 * to calculate real size we need to use two different formulas:
 * -- GET_DRAM_REGION_SIZE_ODD for values 0-4 (DRAM_REGION_SIZE_ODD)
 * -- GET_DRAM_REGION_SIZE_EVEN for values 7-26 (DRAM_REGION_SIZE_EVEN)
 * using mentioned formulas we cover whole mapping between "Area length" value
 * and real size (see above mapping).
 */
#define DRAM_REGION_SIZE_EVEN(C)	(((C) >= 7) && ((C) <= 26))
#define GET_DRAM_REGION_SIZE_EVEN(C)	((uint64_t)1 << ((C) + 16))
#define DRAM_REGION_SIZE_ODD(C)		((C) <= 4)
#define GET_DRAM_REGION_SIZE_ODD(C)	((uint64_t)0x18000000 << (C))


uint64_t mvebu_get_dram_size(uint64_t ap_base_addr)
{
	uint64_t mem_size = 0;
	uint8_t region_code;
	uint8_t cs, iface;

	for (iface = 0; iface < DRAM_MAX_IFACE; iface++) {
		for (cs = 0; cs < DRAM_MAX_CS_NUM; cs++) {

			/* Exit loop on first disabled DRAM CS */
			if (!DRAM_CS_ENABLED(iface, cs, ap_base_addr))
				break;

			/* Decode area length for current CS
			 * from register value
			 */
			region_code =
				GET_DRAM_REGION_SIZE_CODE(iface, cs,
							  ap_base_addr);

			if (DRAM_REGION_SIZE_EVEN(region_code)) {
				mem_size +=
					GET_DRAM_REGION_SIZE_EVEN(region_code);
			} else if (DRAM_REGION_SIZE_ODD(region_code)) {
				mem_size +=
					GET_DRAM_REGION_SIZE_ODD(region_code);
			} else {
				WARN("%s: Invalid mem region (0x%x) CS#%d\n",
				      __func__, region_code, cs);
				return 0;
			}
		}
	}

	return mem_size;
}
