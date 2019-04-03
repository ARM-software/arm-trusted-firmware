/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>

#include "rcar_def.h"
#include "cpg_registers.h"
#include "rcar_private.h"

/* DMA CHANNEL setting (0/16/32) */
#if RCAR_LSI == RCAR_V3M
#define	DMA_CH		16
#else
#define	DMA_CH		0
#endif

#if (DMA_CH == 0)
#define SYS_DMAC_BIT	((uint32_t)1U << 19U)
#define DMA_BASE	(0xE6700000U)
#elif (DMA_CH == 16)
#define SYS_DMAC_BIT	((uint32_t)1U << 18U)
#define DMA_BASE	(0xE7300000U)
#elif (DMA_CH == 32)
#define SYS_DMAC_BIT	((uint32_t)1U << 17U)
#define DMA_BASE	(0xE7320000U)
#else
#define SYS_DMAC_BIT	((uint32_t)1U << 19U)
#define DMA_BASE	(0xE6700000U)
#endif

/* DMA operation */
#define DMA_DMAOR	(DMA_BASE + 0x0060U)
/* DMA secure control */
#define	DMA_DMASEC	(DMA_BASE + 0x0030U)
/* DMA channel clear */
#define DMA_DMACHCLR	(DMA_BASE + 0x0080U)
/* DMA source address */
#define DMA_DMASAR	(DMA_BASE + 0x8000U)
/* DMA destination address */
#define DMA_DMADAR	(DMA_BASE + 0x8004U)
/* DMA transfer count */
#define DMA_DMATCR	(DMA_BASE + 0x8008U)
/* DMA channel control */
#define DMA_DMACHCR	(DMA_BASE + 0x800CU)
/* DMA fixed destination address */
#define DMA_DMAFIXDAR	(DMA_BASE + 0x8014U)

#define	DMA_USE_CHANNEL		(0x00000001U)
#define	DMAOR_INITIAL		(0x0301U)
#define	DMACHCLR_CH_ALL		(0x0000FFFFU)
#define	DMAFIXDAR_32BIT_SHIFT	(32U)
#define	DMAFIXDAR_DAR_MASK	(0x000000FFU)
#define	DMADAR_BOUNDARY_ADDR	(0x100000000ULL)
#define	DMATCR_CNT_SHIFT	(6U)
#define	DMATCR_MAX		(0x00FFFFFFU)
#define	DMACHCR_TRN_MODE	(0x00105409U)
#define	DMACHCR_DE_BIT		(0x00000001U)
#define	DMACHCR_TE_BIT		(0x00000002U)
#define	DMACHCR_CHE_BIT		(0x80000000U)

#define	DMA_SIZE_UNIT		FLASH_TRANS_SIZE_UNIT
#define	DMA_FRACTION_MASK	(0xFFU)
#define DMA_DST_LIMIT		(0x10000000000ULL)

/* transfer length limit */
#define DMA_LENGTH_LIMIT	((DMATCR_MAX * (1U << DMATCR_CNT_SHIFT)) \
				& ~DMA_FRACTION_MASK)

static void dma_enable(void)
{
	mstpcr_write(CPG_SMSTPCR2, CPG_MSTPSR2, SYS_DMAC_BIT);
}

static void dma_setup(void)
{
	mmio_write_16(DMA_DMAOR, 0);
	mmio_write_32(DMA_DMACHCLR, DMACHCLR_CH_ALL);
}

static void dma_start(uintptr_t dst, uint32_t src, uint32_t len)
{
	mmio_write_16(DMA_DMAOR, DMAOR_INITIAL);
	mmio_write_32(DMA_DMAFIXDAR, (dst >> DMAFIXDAR_32BIT_SHIFT) &
		      DMAFIXDAR_DAR_MASK);
	mmio_write_32(DMA_DMADAR, dst & UINT32_MAX);
	mmio_write_32(DMA_DMASAR, src);
	mmio_write_32(DMA_DMATCR, len >> DMATCR_CNT_SHIFT);
	mmio_write_32(DMA_DMASEC, DMA_USE_CHANNEL);
	mmio_write_32(DMA_DMACHCR, DMACHCR_TRN_MODE);
}

static void dma_end(void)
{
	while ((mmio_read_32(DMA_DMACHCR) & DMACHCR_TE_BIT) == 0) {
		if ((mmio_read_32(DMA_DMACHCR) & DMACHCR_CHE_BIT) != 0U) {
			ERROR("BL2: DMA - Channel Address Error\n");
			panic();
			break;
		}
	}
	/* DMA transfer Disable */
	mmio_clrbits_32(DMA_DMACHCR, DMACHCR_DE_BIT);
	while ((mmio_read_32(DMA_DMACHCR) & DMACHCR_DE_BIT) != 0)
		;

	mmio_write_32(DMA_DMASEC, 0);
	mmio_write_16(DMA_DMAOR, 0);
	mmio_write_32(DMA_DMACHCLR, DMA_USE_CHANNEL);
}

void rcar_dma_exec(uintptr_t dst, uint32_t src, uint32_t len)
{
	uint32_t dma_len = len;

	if (len & DMA_FRACTION_MASK)
		dma_len = (len + DMA_SIZE_UNIT) & ~DMA_FRACTION_MASK;

	if (!dma_len || dma_len > DMA_LENGTH_LIMIT) {
		ERROR("BL2: DMA - size invalid, length (0x%x)\n", dma_len);
		panic();
	}

	if (src & DMA_FRACTION_MASK) {
		ERROR("BL2: DMA - source address invalid (0x%x), "
			"length (0x%x)\n", src, dma_len);
		panic();
	}

	if ((dst & UINT32_MAX) + dma_len > DMADAR_BOUNDARY_ADDR	||
	    (dst + dma_len > DMA_DST_LIMIT)			||
	    (dst & DMA_FRACTION_MASK)) {
		ERROR("BL2: DMA - destination address invalid (0x%lx), "
		      "length (0x%x)\n", dst, dma_len);
		panic();
	}

	dma_start(dst, src, dma_len);
	dma_end();
}

void rcar_dma_init(void)
{
	dma_enable();
	dma_setup();
}
