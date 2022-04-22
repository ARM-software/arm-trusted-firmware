/*
 * Copyright (c) 2024-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <stm32mp2_private.h>

/*
 * SYSCFG register offsets (base relative)
 */
#define SYSCFG_OCTOSPIAMCR		0x2C00U
#define SYSCFG_VDDIO3CCCR		0x4000U
#if !STM32MP21
#define SYSCFG_VDDIO4CCCR		0x4008U
#endif /* !STM32MP21 */
#define SYSCFG_VDDCCCR			0x4010U
#define SYSCFG_VDDIO2CCCR		0x4018U
#define SYSCFG_VDDIO1CCCR		0x4020U
#define SYSCFG_DEVICEID			0x6400U

/* SYSCFG_OCTOSPIAMCR Register */
#define SYSCFG_OCTOSPIAMCR_OAM_MASK	GENMASK_32(2, 0)

/* IO compensation CCR registers bit definition */
#define SYSCFG_CCCR_CS			BIT_32(9)
#define SYSCFG_CCCR_EN			BIT_32(8)
#define SYSCFG_CCCR_RAPSRC_MASK		GENMASK_32(7, 4)
#define SYSCFG_CCCR_RAPSRC_SHIFT	4U
#define SYSCFG_CCCR_RANSRC_MASK		GENMASK_32(3, 0)

/* IO compensation CCSR registers bit definition */
#define SYSCFG_CCSR_READY		BIT_32(8)
#define SYSCFG_CCSR_APSRC_MASK		GENMASK_32(7, 4)
#define SYSCFG_CCSR_ANSRC_MASK		GENMASK_32(3, 0)

#define SYSCFG_CCSR_READY_TIMEOUT_US	1000U

/* SYSCFG_DEVICEID Register */
#define SYSCFG_DEVICEID_DEV_ID_MASK	GENMASK_32(11, 0)

static uint32_t syscfg_cccr_offset[SYSFG_NB_IO_ID] = {
	[SYSFG_VDDIO1_ID] = SYSCFG_VDDIO1CCCR,
	[SYSFG_VDDIO2_ID] = SYSCFG_VDDIO2CCCR,
	[SYSFG_VDDIO3_ID] = SYSCFG_VDDIO3CCCR,
#if !STM32MP21
	[SYSFG_VDDIO4_ID] = SYSCFG_VDDIO4CCCR,
#endif /* !STM32MP21 */
	[SYSFG_VDD_IO_ID] = SYSCFG_VDDCCCR,
};

/*
 * @brief  Get device ID from SYSCFG registers.
 * @retval device ID (DEV_ID).
 */
uint32_t stm32mp_syscfg_get_chip_dev_id(void)
{
	return mmio_read_32(SYSCFG_BASE + SYSCFG_DEVICEID) & SYSCFG_DEVICEID_DEV_ID_MASK;
}

size_t stm32mp_syscfg_get_mm_size(uint8_t bank)
{
	uint32_t amcr = mmio_read_32(SYSCFG_BASE + SYSCFG_OCTOSPIAMCR);
	uint32_t oam = amcr & SYSCFG_OCTOSPIAMCR_OAM_MASK;
	size_t addr_mapping1;
	size_t addr_mapping2;

	if (oam > 3U) {
		addr_mapping2 = SZ_256M;
	} else {
		addr_mapping2 = oam * SZ_64M;
	}

	addr_mapping1 = SZ_256M - addr_mapping2;

	return (bank == 0U) ? addr_mapping1 : addr_mapping2;
}

/*
 * @brief  Enable IO compensation for an IO domain.
 * @param  id: IO compensation ID
 * @retval 0.
 */
void stm32mp_syscfg_enable_io_comp(enum syscfg_io_ids id)
{
	uintptr_t cccr_addr = SYSCFG_BASE + syscfg_cccr_offset[id];
	uintptr_t ccsr_addr = cccr_addr + 4U;
	uint64_t timeout_ref;

	VERBOSE("Enable IO comp for id %u\n", id);

	if ((mmio_read_32(ccsr_addr) & SYSCFG_CCSR_READY) != 0U) {
		return;
	}

	mmio_setbits_32(cccr_addr, SYSCFG_CCCR_EN);

	timeout_ref = timeout_init_us(SYSCFG_CCSR_READY_TIMEOUT_US);

	while ((mmio_read_32(ccsr_addr) & SYSCFG_CCSR_READY) == 0U)
		if (timeout_elapsed(timeout_ref)) {
			WARN("IO compensation cell not ready\n");
			/* Allow an almost silent failure here */
			break;
		}

	mmio_clrbits_32(cccr_addr, SYSCFG_CCCR_CS);
}

/*
 * @brief  Set fixed IO compensation for an IO domain.
 * @param  id: IO compensation ID
 * @param  pmos: pmos compensation code
 * @param  nmos: nmos compensation code
 * @retval 0.
 */
void stm32mp_syscfg_fixed_io_comp(enum syscfg_io_ids id, uint32_t pmos, uint32_t nmos)
{
	uintptr_t cccr_addr = SYSCFG_BASE + syscfg_cccr_offset[id];
	uint32_t value = 0U;

	VERBOSE("Set fixed IO comp for id %u\n", id);

	value = ((pmos << SYSCFG_CCCR_RAPSRC_SHIFT) & SYSCFG_CCCR_RAPSRC_MASK) |
		(nmos & SYSCFG_CCCR_RANSRC_MASK);
	mmio_write_32(cccr_addr, value);
}
