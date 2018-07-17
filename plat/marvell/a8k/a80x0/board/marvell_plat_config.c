/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>

/*
 * If bootrom is currently at BLE there's no need to include the memory
 * maps structure at this point
 */
#include <mvebu_def.h>
#ifndef IMAGE_BLE

/*****************************************************************************
 * AMB Configuration
 *****************************************************************************
 */
struct addr_map_win amb_memory_map[] = {
	/* CP1 SPI1 CS0 Direct Mode access */
	{0xf900,	0x1000000,	AMB_SPI1_CS0_ID},
};

int marvell_get_amb_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	*win = amb_memory_map;
	if (*win == NULL)
		*size = 0;
	else
		*size = ARRAY_SIZE(amb_memory_map);

	return 0;
}
#endif

/*****************************************************************************
 * IO WIN Configuration
 *****************************************************************************
 */
struct addr_map_win io_win_memory_map[] = {
	/* CP1 (MCI0) internal regs */
	{0x00000000f4000000,		0x2000000,  MCI_0_TID},
#ifndef IMAGE_BLE
	/* PCIe0 and SPI1_CS0 (RUNIT) on CP1*/
	{0x00000000f9000000,		0x2000000,  MCI_0_TID},
	/* PCIe1 on CP1*/
	{0x00000000fb000000,		0x1000000,  MCI_0_TID},
	/* PCIe2 on CP1*/
	{0x00000000fc000000,		0x1000000,  MCI_0_TID},
	/* MCI 0 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(0),	0x100000,  MCI_0_TID},
	/* MCI 1 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(1),	0x100000,  MCI_1_TID},
#endif
};

uint32_t marvell_get_io_win_gcr_target(int ap_index)
{
	return PIDI_TID;
}

int marvell_get_io_win_memory_map(int ap_index, struct addr_map_win **win,
				  uint32_t *size)
{
	*win = io_win_memory_map;
	if (*win == NULL)
		*size = 0;
	else
		*size = ARRAY_SIZE(io_win_memory_map);

	return 0;
}

#ifndef IMAGE_BLE
/*****************************************************************************
 * IOB Configuration
 *****************************************************************************
 */
struct addr_map_win iob_memory_map_cp0[] = {
	/* CP0 */
	/* PEX1_X1 window */
	{0x00000000f7000000,	0x1000000,	PEX1_TID},
	/* PEX2_X1 window */
	{0x00000000f8000000,	0x1000000,	PEX2_TID},
	/* PEX0_X4 window */
	{0x00000000f6000000,	0x1000000,	PEX0_TID},
	{0x00000000c0000000,	0x30000000,	PEX0_TID},
	{0x0000000800000000,	0x100000000,	PEX0_TID},
};

struct addr_map_win iob_memory_map_cp1[] = {
	/* CP1 */
	/* SPI1_CS0 (RUNIT) window */
	{0x00000000f9000000,	0x1000000,	RUNIT_TID},
	/* PEX1_X1 window */
	{0x00000000fb000000,	0x1000000,	PEX1_TID},
	/* PEX2_X1 window */
	{0x00000000fc000000,	0x1000000,	PEX2_TID},
	/* PEX0_X4 window */
	{0x00000000fa000000,	0x1000000,	PEX0_TID}
};

int marvell_get_iob_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	switch (base) {
	case MVEBU_CP_REGS_BASE(0):
		*win = iob_memory_map_cp0;
		*size = ARRAY_SIZE(iob_memory_map_cp0);
		return 0;
	case MVEBU_CP_REGS_BASE(1):
		*win = iob_memory_map_cp1;
		*size = ARRAY_SIZE(iob_memory_map_cp1);
		return 0;
	default:
		*size = 0;
		*win = 0;
		return 1;
	}
}
#endif

/*****************************************************************************
 * CCU Configuration
 *****************************************************************************
 */
struct addr_map_win ccu_memory_map[] = {
#ifdef IMAGE_BLE
	{0x00000000f2000000,	0x4000000,  IO_0_TID}, /* IO window */
#else
	{0x00000000f2000000,	0xe000000,  IO_0_TID}, /* IO window */
	{0x00000000c0000000,	0x30000000,  IO_0_TID}, /* IO window */
	{0x0000000800000000,	0x100000000,  IO_0_TID}, /* IO window */
#endif
};

uint32_t marvell_get_ccu_gcr_target(int ap)
{
	return DRAM_0_TID;
}

int marvell_get_ccu_memory_map(int ap, struct addr_map_win **win,
			       uint32_t *size)
{
	*win = ccu_memory_map;
	*size = ARRAY_SIZE(ccu_memory_map);

	return 0;
}

#ifndef IMAGE_BLE
/*****************************************************************************
 * SoC PM configuration
 *****************************************************************************
 */
/* CP GPIO should be used and the GPIOs should be within same GPIO register */
struct power_off_method pm_cfg = {
	.type = PMIC_GPIO,
	.cfg.gpio.pin_count = 1,
	.cfg.gpio.info = {{0, 35} },
	.cfg.gpio.step_count = 7,
	.cfg.gpio.seq = {1, 0, 1, 0, 1, 0, 1},
	.cfg.gpio.delay_ms = 10,
};

void *plat_marvell_get_pm_cfg(void)
{
	/* Return the PM configurations */
	return &pm_cfg;
}

/* In reference to #ifndef IMAGE_BLE, this part is used for BLE only. */
#else
/*****************************************************************************
 * SKIP IMAGE Configuration
 *****************************************************************************
 */
#if PLAT_RECOVERY_IMAGE_ENABLE
struct skip_image skip_im = {
	.detection_method = GPIO,
	.info.gpio.num = 33,
	.info.gpio.button_state = HIGH,
	.info.test.cp_ap = CP,
	.info.test.cp_index = 0,
};

void *plat_marvell_get_skip_image_data(void)
{
	/* Return the skip_image configurations */
	return &skip_im;
}
#endif
#endif
