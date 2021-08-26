/*
 * Copyright (C) 2018 Marvell International Ltd.
 * Copyright (C) 2021 Semihalf.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <armada_common.h>
#include <mvebu_def.h>

/*
 * If bootrom is currently at BLE there's no need to include the memory
 * maps structure at this point
 */
#ifndef IMAGE_BLE

/*****************************************************************************
 * AMB Configuration
 *****************************************************************************
 */
struct addr_map_win amb_memory_map_cp0[] = {
	/* CP0 SPI1 CS0 Direct Mode access */
	{0xef00,	0x1000000,	AMB_SPI1_CS0_ID},
};

struct addr_map_win amb_memory_map_cp1[] = {
	/* CP1 SPI1 CS0 Direct Mode access */
	{0xe800,	0x1000000,	AMB_SPI1_CS0_ID},
};

int marvell_get_amb_memory_map(struct addr_map_win **win, uint32_t *size,
			       uintptr_t base)
{
	switch (base) {
	case MVEBU_CP_REGS_BASE(0):
		*win = amb_memory_map_cp0;
		*size = ARRAY_SIZE(amb_memory_map_cp0);
		return 0;
	case MVEBU_CP_REGS_BASE(1):
		*win = amb_memory_map_cp1;
		*size = ARRAY_SIZE(amb_memory_map_cp1);
		return 0;
	case MVEBU_CP_REGS_BASE(2):
	default:
		*size = 0;
		*win = 0;
		return 1;
	}
}
#endif

/*****************************************************************************
 * IO WIN Configuration
 *****************************************************************************
 */
struct addr_map_win io_win_memory_map[] = {
#if (CP_COUNT > 1)
	/* SB (MCi0) internal regs */
	{0x00000000f4000000,		0x2000000,	MCI_0_TID},
	/* SB (MCi0) PCIe0-2 on CP1 */
	{0x00000000e2000000,		0x7000000,	MCI_0_TID},
	/*
	 * Due to lack of sufficient number of IO windows registers,
	 * below CP1 PCIE configuration must be performed in the
	 * later firmware stages. It should replace the MCI 0 indirect
	 * window, which becomes no longer needed.
	 */
	/* {0x0000000890000000,		0x30000000,	MCI_0_TID}, */
#if (CP_COUNT > 2)
	/* SB (MCi1) internal regs */
	{0x00000000f6000000,		0x2000000,	MCI_1_TID},
	/* SB (MCi1) PCIe0-2 on CP2 */
	{0x00000000e9000000,		0x6000000,	MCI_1_TID},
	/*
	 * Due to lack of sufficient number of IO windows registers,
	 * below CP2 PCIE configuration must be performed in the
	 * later firmware stages. It should replace the MCI 1 indirect
	 * window, which becomes no longer needed.
	 */
	/* {0x00000008c0000000,		0x30000000,	MCI_1_TID}, */
#endif
#endif
#ifndef IMAGE_BLE
	/* MCI 0 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(0),	0x100000,	MCI_0_TID},
	/* MCI 1 indirect window */
	{MVEBU_MCI_REG_BASE_REMAP(1),	0x100000,	MCI_1_TID},
#endif
};

/* Global Control Register - window default target */
uint32_t marvell_get_io_win_gcr_target(int ap_index)
{
	/*
	 * PIDI == iMCIP AP to SB internal MoChi connection.
	 * In other words CP0
	 */
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
	/* SPI1_CS0 (RUNIT) window */
	{0x00000000ef000000,	0x1000000,	RUNIT_TID},
	/* PEX2_X1 window */
	{0x00000000e1000000,	0x1000000,	PEX2_TID},
	/* PEX1_X1 window */
	{0x00000000e0000000,	0x1000000,	PEX1_TID},
	/* PEX0_X4 window */
	{0x00000000c0000000,	0x20000000,	PEX0_TID},
	{0x0000000800000000,	0x90000000,	PEX0_TID},
};

struct addr_map_win iob_memory_map_cp1[] = {
	/* SPI1_CS0 (RUNIT) window */
	{0x00000000e8000000,	0x1000000,	RUNIT_TID},
	/* PEX2_X1 window */
	{0x00000000e6000000,	0x2000000,	PEX2_TID},
	{0x00000008b0000000,	0x10000000,	PEX2_TID},
	/* PEX1_X1 window */
	{0x00000000e4000000,	0x2000000,	PEX1_TID},
	{0x00000008a0000000,	0x10000000,	PEX1_TID},
	/* PEX0_X2 window */
	{0x00000000e2000000,	0x2000000,	PEX0_TID},
	{0x0000000890000000,	0x10000000,	PEX0_TID},
};

struct addr_map_win iob_memory_map_cp2[] = {

	/* PEX2_X1 window */
	{0x00000000ed000000,	0x2000000,	PEX2_TID},
	{0x00000008e0000000,	0x10000000,	PEX2_TID},
	/* PEX1_X1 window */
	{0x00000000eb000000,	0x2000000,	PEX1_TID},
	{0x00000008d0000000,	0x10000000,	PEX1_TID},
	/* PEX0_X1 window */
	{0x00000000e9000000,	0x2000000,	PEX0_TID},
	{0x00000008c0000000,	0x10000000,	PEX0_TID},
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
	case MVEBU_CP_REGS_BASE(2):
		*win = iob_memory_map_cp2;
		*size = ARRAY_SIZE(iob_memory_map_cp2);
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
struct addr_map_win ccu_memory_map[] = {	/* IO window */
#ifdef IMAGE_BLE
	{0x00000000f2000000,	0x6000000,	IO_0_TID}, /* IO window */
#else
#if LLC_SRAM
	{PLAT_MARVELL_LLC_SRAM_BASE, PLAT_MARVELL_LLC_SRAM_SIZE, DRAM_0_TID},
#endif
	{0x00000000f2000000,	0xe000000,	IO_0_TID}, /* IO window */
	{0x00000000c0000000,	0x30000000,	IO_0_TID}, /* IO window */
	{0x0000000800000000,	0x100000000,    IO_0_TID}, /* IO window */
	{0x0000002000000000,	0x70e000000,	IO_0_TID}, /* IO for CV-OS */
#endif
};

uint32_t marvell_get_ccu_gcr_target(int ap)
{
	return DRAM_0_TID;
}

int marvell_get_ccu_memory_map(int ap_index, struct addr_map_win **win,
			       uint32_t *size)
{
	*win = ccu_memory_map;
	*size = ARRAY_SIZE(ccu_memory_map);

	return 0;
}

#ifdef IMAGE_BLE
/*****************************************************************************
 * SKIP IMAGE Configuration
 *****************************************************************************
 */
void *plat_get_skip_image_data(void)
{
	/* No recovery button on CN-9130 board? */
	return NULL;
}
#endif
