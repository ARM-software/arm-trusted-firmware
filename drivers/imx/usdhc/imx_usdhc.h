/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 * Copyright 2025 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_USDHC_H
#define IMX_USDHC_H

#include <drivers/mmc.h>

typedef struct imx_usdhc_params {
	uintptr_t	reg_base;
	unsigned int	clk_rate;
	unsigned int	bus_width;
	unsigned int	flags;
} imx_usdhc_params_t;

void imx_usdhc_init(imx_usdhc_params_t *params,
		    struct mmc_device_info *mmc_dev_info);

/* iMX MMC registers definition */
#define DSADDR			0x000U
#define BLKATT			0x004U
#define BLKATT_BLKCNT(x)	(((x) << 16U) & GENMASK_32(31U, 16U))
#define BLKATT_BLKSIZE(x)	((x) & GENMASK_32(12U, 0U))
#define CMDARG			0x008U
#define CMDRSP0			0x010U
#define CMDRSP1			0x014U
#define CMDRSP2			0x018U
#define CMDRSP3			0x01cU

#define XFERTYPE		0x00cU
#define XFERTYPE_CMD(x)		(((x) & 0x3fU) << 24U)
#define XFERTYPE_GET_CMD(x)	(((x) & GENMASK_32(29U, 24U)) >> 24U)
#define XFERTYPE_CMDTYP_ABORT	(3U << 22U)
#define XFERTYPE_DPSEL		BIT_32(21U)
#define XFERTYPE_CICEN		BIT_32(20U)
#define XFERTYPE_CCCEN		BIT_32(19U)
#define XFERTYPE_RSPTYP_136	BIT_32(16U)
#define XFERTYPE_RSPTYP_48	BIT_32(17U)
#define XFERTYPE_RSPTYP_48_BUSY	(BIT_32(16U) | BIT_32(17U))

#define PSTATE			0x024U
#define PSTATE_DAT0		BIT_32(24U)
#define PSTATE_SDSTB		BIT_32(3U)
#define PSTATE_DLA		BIT_32(2U)
#define PSTATE_CDIHB		BIT_32(1U)
#define PSTATE_CIHB		BIT_32(0U)

#define PROTCTRL		0x028U
#define PROTCTRL_LE		BIT_32(5U)
#define PROTCTRL_WIDTH_4	BIT_32(1U)
#define PROTCTRL_WIDTH_8	BIT_32(2U)
#define PROTCTRL_WIDTH_MASK	0x6U

#define SYSCTRL			0x02cU
#define SYSCTRL_RSTD		BIT_32(26U)
#define SYSCTRL_RSTC		BIT_32(25U)
#define SYSCTRL_RSTA		BIT_32(24U)
#define SYSCTRL_CLOCK_MASK	GENMASK_32(15U, 4U)
#define SYSCTRL_TIMEOUT_MASK	GENMASK_32(19U, 16U)
#define SYSCTRL_TIMEOUT(x)	((0xfU & (x)) << 16U)

#define INTSTAT			0x030U
#define INTSTAT_DMAE		BIT_32(28U)
#define INTSTAT_DEBE		BIT_32(22U)
#define INTSTAT_DCE		BIT_32(21U)
#define INTSTAT_DTOE		BIT_32(20U)
#define INTSTAT_CIE		BIT_32(19U)
#define INTSTAT_CEBE		BIT_32(18U)
#define INTSTAT_CCE		BIT_32(17U)
#define INTSTAT_DINT		BIT_32(3U)
#define INTSTAT_BGE		BIT_32(2U)
#define INTSTAT_TC		BIT_32(1U)
#define INTSTAT_CC		BIT_32(0U)
#define CMD_ERR			(INTSTAT_CIE | INTSTAT_CEBE | INTSTAT_CCE)
#define DATA_ERR		(INTSTAT_DMAE | INTSTAT_DEBE | INTSTAT_DCE | \
				 INTSTAT_DTOE)
#define DATA_COMPLETE		(INTSTAT_DINT | INTSTAT_TC)

#define INTSTATEN		0x034U
#define INTSTATEN_DEBE		BIT_32(22U)
#define INTSTATEN_DCE		BIT_32(21U)
#define INTSTATEN_DTOE		BIT_32(20U)
#define INTSTATEN_CIE		BIT_32(19U)
#define INTSTATEN_CEBE		BIT_32(18U)
#define INTSTATEN_CCE		BIT_32(17U)
#define INTSTATEN_CTOE		BIT_32(16U)
#define INTSTATEN_CINT		BIT_32(8U)
#define INTSTATEN_BRR		BIT_32(5U)
#define INTSTATEN_BWR		BIT_32(4U)
#define INTSTATEN_DINT		BIT_32(3U)
#define INTSTATEN_TC		BIT_32(1U)
#define INTSTATEN_CC		BIT_32(0U)
#define EMMC_INTSTATEN_BITS	(INTSTATEN_CC | INTSTATEN_TC | INTSTATEN_DINT | \
				 INTSTATEN_BWR | INTSTATEN_BRR | INTSTATEN_CINT | \
				 INTSTATEN_CTOE | INTSTATEN_CCE | INTSTATEN_CEBE | \
				 INTSTATEN_CIE | INTSTATEN_DTOE | INTSTATEN_DCE | \
				 INTSTATEN_DEBE)

#define INTSIGEN		0x038U

#define WATERMARKLEV		0x044U
#define WMKLV_RD_MASK		GENMASK_32(7U, 0U)
#define WMKLV_WR_MASK		GENMASK_32(23U, 16U)
#define WMKLV_MASK		(WMKLV_RD_MASK | WMKLV_WR_MASK)

#define MIXCTRL			0x048U
#define MIXCTRL_MSBSEL		BIT_32(5U)
#define MIXCTRL_DTDSEL		BIT_32(4U)
#define MIXCTRL_DDREN		BIT_32(3U)
#define MIXCTRL_AC12EN		BIT_32(2U)
#define MIXCTRL_BCEN		BIT_32(1U)
#define MIXCTRL_DMAEN		BIT_32(0U)
#define MIXCTRL_DATMASK		0x7fU

#define DLLCTRL			0x060U

#define CLKTUNECTRLSTS		0x068U

#define VENDSPEC		0x0c0U
#define VENDSPEC_RSRV1		BIT_32(29U)
#define VENDSPEC_CARD_CLKEN	BIT_32(14U)
#define VENDSPEC_PER_CLKEN	BIT_32(13U)
#define VENDSPEC_AHB_CLKEN	BIT_32(12U)
#define VENDSPEC_IPG_CLKEN	BIT_32(11U)
#define VENDSPEC_AC12_CHKBUSY	BIT_32(3U)
#define VENDSPEC_EXTDMA		BIT_32(0U)
#define VENDSPEC_INIT		(VENDSPEC_RSRV1	| VENDSPEC_CARD_CLKEN | \
				 VENDSPEC_PER_CLKEN | VENDSPEC_AHB_CLKEN | \
				 VENDSPEC_IPG_CLKEN | VENDSPEC_AC12_CHKBUSY | \
				 VENDSPEC_EXTDMA)

#define MMCBOOT			0x0c4U

#define mmio_clrsetbits32(addr, clear, set)	mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)) | (set))
#define mmio_clrbits32(addr, clear)		mmio_write_32(addr, mmio_read_32(addr) & ~(clear))
#define mmio_setbits32(addr, set)		mmio_write_32(addr, mmio_read_32(addr) | (set))

#endif /* IMX_USDHC_H */
