/*
 * Copyright (c) 2015-2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#define	RCAR_CPG_BASE		0xE6150000U
#define	RCAR_CPG_CPGWPR		(RCAR_CPG_BASE + 0x0000U)
#define	RCAR_CPG_CPGWPCR	(RCAR_CPG_BASE + 0x0004U)

#define	RCAR_CPG_MSTPCR27	(RCAR_CPG_BASE + 0x2D6CU)
#define	RCAR_CPG_MSTPSR27	(RCAR_CPG_BASE + 0x2E6CU)

#define	RCAR_PTPRO_BASE		0xE6449000U
#define	RCAR_PTPSCR0		(RCAR_PTPRO_BASE + 0x780U)
#define	RCAR_PTPSCR1		(RCAR_PTPRO_BASE + 0x784U)

void rcar_ptp_setup(void)
{
#if PTP_NONSECURE_ACCESS
	mmio_clrbits_32(RCAR_CPG_MSTPCR27, BIT(23));
	while ((mmio_read_32(RCAR_CPG_MSTPSR27) & BIT(23)) != 0)
		;

	mmio_write_32(RCAR_PTPSCR0, 0x30003); /* PTPSCR0 */
	mmio_write_32(RCAR_PTPSCR1, 0x30003); /* PTPSCR1 */
#endif /* PTP_NONSECURE_ACCESS */
}
