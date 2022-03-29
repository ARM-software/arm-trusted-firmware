/*
 * Copyright (c) 2017-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/etzpc.h>
#include <dt-bindings/soc/st,stm32-etzpc.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

/* Device Tree related definitions */
#define ETZPC_COMPAT			"st,stm32-etzpc"
#define ETZPC_LOCK_MASK			0x1U
#define ETZPC_MODE_SHIFT		8
#define ETZPC_MODE_MASK			GENMASK(1, 0)
#define ETZPC_ID_SHIFT			16
#define ETZPC_ID_MASK			GENMASK(7, 0)

/* ID Registers */
#define ETZPC_TZMA0_SIZE		0x000U
#define ETZPC_DECPROT0			0x010U
#define ETZPC_DECPROT_LOCK0		0x030U
#define ETZPC_HWCFGR			0x3F0U
#define ETZPC_VERR			0x3F4U

/* ID Registers fields */
#define ETZPC_TZMA0_SIZE_LOCK		BIT(31)
#define ETZPC_DECPROT0_MASK		GENMASK(1, 0)
#define ETZPC_HWCFGR_NUM_TZMA_SHIFT	0
#define ETZPC_HWCFGR_NUM_PER_SEC_SHIFT	8
#define ETZPC_HWCFGR_NUM_AHB_SEC_SHIFT	16
#define ETZPC_HWCFGR_CHUNCKS1N4_SHIFT	24

#define DECPROT_SHIFT			1
#define IDS_PER_DECPROT_REGS		16U
#define IDS_PER_DECPROT_LOCK_REGS	32U

/*
 * etzpc_instance.
 * base : register base address set during init given by user
 * chunk_size : supported TZMA size steps
 * num_tzma: number of TZMA zone read from register at init
 * num_ahb_sec : number of securable AHB master zone read from register
 * num_per_sec : number of securable AHB & APB Peripherals read from register
 * revision : IP revision read from register at init
 */
struct etzpc_instance {
	uintptr_t base;
	uint8_t chunck_size;
	uint8_t num_tzma;
	uint8_t num_per_sec;
	uint8_t num_ahb_sec;
	uint8_t revision;
};

/* Only 1 instance of the ETZPC is expected per platform */
static struct etzpc_instance etzpc_dev;

/*
 * Implementation uses uint8_t to store each securable DECPROT configuration.
 * When resuming from deep suspend, the DECPROT configurations are restored.
 */
#define PERIPH_LOCK_BIT		BIT(7)
#define PERIPH_ATTR_MASK	GENMASK(2, 0)

#if ENABLE_ASSERTIONS
static bool valid_decprot_id(unsigned int id)
{
	return id < (unsigned int)etzpc_dev.num_per_sec;
}

static bool valid_tzma_id(unsigned int id)
{
	return id < (unsigned int)etzpc_dev.num_tzma;
}
#endif

/*
 * etzpc_configure_decprot : Load a DECPROT configuration
 * decprot_id : ID of the IP
 * decprot_attr : Restriction access attribute
 */
void etzpc_configure_decprot(uint32_t decprot_id,
			     enum etzpc_decprot_attributes decprot_attr)
{
	uintptr_t offset = 4U * (decprot_id / IDS_PER_DECPROT_REGS);
	uint32_t shift = (decprot_id % IDS_PER_DECPROT_REGS) << DECPROT_SHIFT;
	uint32_t masked_decprot = (uint32_t)decprot_attr & ETZPC_DECPROT0_MASK;

	assert(valid_decprot_id(decprot_id));

	mmio_clrsetbits_32(etzpc_dev.base + ETZPC_DECPROT0 + offset,
			   (uint32_t)ETZPC_DECPROT0_MASK << shift,
			   masked_decprot << shift);
}

/*
 * etzpc_get_decprot : Get the DECPROT attribute
 * decprot_id : ID of the IP
 * return : Attribute of this DECPROT
 */
enum etzpc_decprot_attributes etzpc_get_decprot(uint32_t decprot_id)
{
	uintptr_t offset = 4U * (decprot_id / IDS_PER_DECPROT_REGS);
	uint32_t shift = (decprot_id % IDS_PER_DECPROT_REGS) << DECPROT_SHIFT;
	uintptr_t base_decprot = etzpc_dev.base + offset;
	uint32_t value;

	assert(valid_decprot_id(decprot_id));

	value = (mmio_read_32(base_decprot + ETZPC_DECPROT0) >> shift) &
		ETZPC_DECPROT0_MASK;

	return (enum etzpc_decprot_attributes)value;
}

/*
 * etzpc_lock_decprot : Lock access to the DECPROT attribute
 * decprot_id : ID of the IP
 */
void etzpc_lock_decprot(uint32_t decprot_id)
{
	uintptr_t offset = 4U * (decprot_id / IDS_PER_DECPROT_LOCK_REGS);
	uint32_t shift = BIT(decprot_id % IDS_PER_DECPROT_LOCK_REGS);
	uintptr_t base_decprot = etzpc_dev.base + offset;

	assert(valid_decprot_id(decprot_id));

	mmio_write_32(base_decprot + ETZPC_DECPROT_LOCK0, shift);
}

/*
 * etzpc_configure_tzma : Configure the target TZMA read only size
 * tzma_id : ID of the memory
 * tzma_value : read-only size
 */
void etzpc_configure_tzma(uint32_t tzma_id, uint16_t tzma_value)
{
	assert(valid_tzma_id(tzma_id));

	mmio_write_32(etzpc_dev.base + ETZPC_TZMA0_SIZE +
		      (sizeof(uint32_t) * tzma_id), tzma_value);
}

/*
 * etzpc_get_tzma : Get the target TZMA read only size
 * tzma_id : TZMA ID
 * return : Size of read only size
 */
uint16_t etzpc_get_tzma(uint32_t tzma_id)
{
	assert(valid_tzma_id(tzma_id));

	return (uint16_t)mmio_read_32(etzpc_dev.base + ETZPC_TZMA0_SIZE +
				      (sizeof(uint32_t) * tzma_id));
}

/*
 * etzpc_lock_tzma : Lock the target TZMA
 * tzma_id : TZMA ID
 */
void etzpc_lock_tzma(uint32_t tzma_id)
{
	assert(valid_tzma_id(tzma_id));

	mmio_setbits_32(etzpc_dev.base + ETZPC_TZMA0_SIZE +
			(sizeof(uint32_t) * tzma_id), ETZPC_TZMA0_SIZE_LOCK);
}

/*
 * etzpc_get_lock_tzma : Return the lock status of the target TZMA
 * tzma_id : TZMA ID
 * return : True if TZMA is locked, false otherwise
 */
bool etzpc_get_lock_tzma(uint32_t tzma_id)
{
	uint32_t tzma_size;

	assert(valid_tzma_id(tzma_id));

	tzma_size = mmio_read_32(etzpc_dev.base + ETZPC_TZMA0_SIZE +
				 (sizeof(uint32_t) * tzma_id));

	return (tzma_size & ETZPC_TZMA0_SIZE_LOCK) != 0;
}

/*
 * etzpc_get_num_per_sec : Return the DECPROT ID limit value
 */
uint8_t etzpc_get_num_per_sec(void)
{
	return etzpc_dev.num_per_sec;
}

/*
 * etzpc_get_revision : Return the ETZPC IP revision
 */
uint8_t etzpc_get_revision(void)
{
	return etzpc_dev.revision;
}

/*
 * etzpc_get_base_address : Return the ETZPC IP base address
 */
uintptr_t etzpc_get_base_address(void)
{
	return etzpc_dev.base;
}

/*
 * etzpc_init : Initialize the ETZPC driver
 * Return 0 on success and a negative errno on failure
 */
int etzpc_init(void)
{
	uint32_t hwcfg;

	etzpc_dev.base = STM32MP1_ETZPC_BASE;

	hwcfg = mmio_read_32(etzpc_dev.base + ETZPC_HWCFGR);

	etzpc_dev.num_tzma = (uint8_t)(hwcfg >> ETZPC_HWCFGR_NUM_TZMA_SHIFT);
	etzpc_dev.num_per_sec = (uint8_t)(hwcfg >>
					  ETZPC_HWCFGR_NUM_PER_SEC_SHIFT);
	etzpc_dev.num_ahb_sec = (uint8_t)(hwcfg >>
					  ETZPC_HWCFGR_NUM_AHB_SEC_SHIFT);
	etzpc_dev.chunck_size = (uint8_t)(hwcfg >>
					  ETZPC_HWCFGR_CHUNCKS1N4_SHIFT);

	etzpc_dev.revision = mmio_read_8(etzpc_dev.base + ETZPC_VERR);

	VERBOSE("ETZPC version 0x%x", etzpc_dev.revision);

	return 0;
}
