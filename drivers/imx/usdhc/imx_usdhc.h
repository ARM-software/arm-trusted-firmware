/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_USDHC_H
#define IMX_USDHC_H

#include <drivers/mmc.h>

typedef struct imx_usdhc_params {
	uintptr_t	reg_base;
	int		clk_rate;
	int		bus_width;
	unsigned int	flags;
} imx_usdhc_params_t;

void imx_usdhc_init(imx_usdhc_params_t *params,
		    struct mmc_device_info *mmc_dev_info);

/* iMX MMC registers definition */
#define DSADDR			0x000
#define BLKATT			0x004
#define CMDARG			0x008
#define CMDRSP0			0x010
#define CMDRSP1			0x014
#define CMDRSP2			0x018
#define CMDRSP3			0x01c

#define XFERTYPE		0x00c
#define XFERTYPE_CMD(x)		(((x) & 0x3f) << 24)
#define XFERTYPE_CMDTYP_ABORT	(3 << 22)
#define XFERTYPE_DPSEL		BIT(21)
#define XFERTYPE_CICEN		BIT(20)
#define XFERTYPE_CCCEN		BIT(19)
#define XFERTYPE_RSPTYP_136	BIT(16)
#define XFERTYPE_RSPTYP_48	BIT(17)
#define XFERTYPE_RSPTYP_48_BUSY	(BIT(16) | BIT(17))

#define PSTATE			0x024
#define PSTATE_DAT0		BIT(24)
#define PSTATE_DLA		BIT(2)
#define PSTATE_CDIHB		BIT(1)
#define PSTATE_CIHB		BIT(0)

#define PROTCTRL		0x028
#define PROTCTRL_LE		BIT(5)
#define PROTCTRL_WIDTH_4	BIT(1)
#define PROTCTRL_WIDTH_8	BIT(2)
#define PROTCTRL_WIDTH_MASK	0x6

#define SYSCTRL			0x02c
#define SYSCTRL_RSTD		BIT(26)
#define SYSCTRL_RSTC		BIT(25)
#define SYSCTRL_RSTA		BIT(24)
#define SYSCTRL_CLOCK_MASK	0x0000fff0
#define SYSCTRL_TIMEOUT_MASK	0x000f0000
#define SYSCTRL_TIMEOUT(x)	((0xf & (x)) << 16)

#define INTSTAT			0x030
#define INTSTAT_DMAE		BIT(28)
#define INTSTAT_DEBE		BIT(22)
#define INTSTAT_DCE		BIT(21)
#define INTSTAT_DTOE		BIT(20)
#define INTSTAT_CIE		BIT(19)
#define INTSTAT_CEBE		BIT(18)
#define INTSTAT_CCE		BIT(17)
#define INTSTAT_DINT		BIT(3)
#define INTSTAT_BGE		BIT(2)
#define INTSTAT_TC		BIT(1)
#define INTSTAT_CC		BIT(0)
#define CMD_ERR			(INTSTAT_CIE | INTSTAT_CEBE | INTSTAT_CCE)
#define DATA_ERR		(INTSTAT_DMAE | INTSTAT_DEBE | INTSTAT_DCE | \
				 INTSTAT_DTOE)
#define DATA_COMPLETE		(INTSTAT_DINT | INTSTAT_TC)

#define INTSTATEN		0x034
#define INTSTATEN_DEBE		BIT(22)
#define INTSTATEN_DCE		BIT(21)
#define INTSTATEN_DTOE		BIT(20)
#define INTSTATEN_CIE		BIT(19)
#define INTSTATEN_CEBE		BIT(18)
#define INTSTATEN_CCE		BIT(17)
#define INTSTATEN_CTOE		BIT(16)
#define INTSTATEN_CINT		BIT(8)
#define INTSTATEN_BRR		BIT(5)
#define INTSTATEN_BWR		BIT(4)
#define INTSTATEN_DINT		BIT(3)
#define INTSTATEN_TC		BIT(1)
#define INTSTATEN_CC		BIT(0)
#define EMMC_INTSTATEN_BITS	(INTSTATEN_CC | INTSTATEN_TC | INTSTATEN_DINT | \
				 INTSTATEN_BWR | INTSTATEN_BRR | INTSTATEN_CINT | \
				 INTSTATEN_CTOE | INTSTATEN_CCE | INTSTATEN_CEBE | \
				 INTSTATEN_CIE | INTSTATEN_DTOE | INTSTATEN_DCE | \
				 INTSTATEN_DEBE)

#define INTSIGEN		0x038

#define WATERMARKLEV		0x044
#define WMKLV_RD_MASK		0xff
#define WMKLV_WR_MASK		0x00ff0000
#define WMKLV_MASK		(WMKLV_RD_MASK | WMKLV_WR_MASK)

#define MIXCTRL			0x048
#define MIXCTRL_MSBSEL		BIT(5)
#define MIXCTRL_DTDSEL		BIT(4)
#define MIXCTRL_DDREN		BIT(3)
#define MIXCTRL_AC12EN		BIT(2)
#define MIXCTRL_BCEN		BIT(1)
#define MIXCTRL_DMAEN		BIT(0)
#define MIXCTRL_DATMASK		0x7f

#define DLLCTRL			0x060

#define CLKTUNECTRLSTS		0x068

#define VENDSPEC		0x0c0
#define VENDSPEC_RSRV1		BIT(29)
#define VENDSPEC_CARD_CLKEN	BIT(14)
#define VENDSPEC_PER_CLKEN	BIT(13)
#define VENDSPEC_AHB_CLKEN	BIT(12)
#define VENDSPEC_IPG_CLKEN	BIT(11)
#define VENDSPEC_AC12_CHKBUSY	BIT(3)
#define VENDSPEC_EXTDMA		BIT(0)
#define VENDSPEC_INIT		(VENDSPEC_RSRV1	| VENDSPEC_CARD_CLKEN | \
				 VENDSPEC_PER_CLKEN | VENDSPEC_AHB_CLKEN | \
				 VENDSPEC_IPG_CLKEN | VENDSPEC_AC12_CHKBUSY | \
				 VENDSPEC_EXTDMA)

#define MMCBOOT			0x0c4

#define mmio_clrsetbits32(addr, clear, set)	mmio_write_32(addr, (mmio_read_32(addr) & ~(clear)) | (set))
#define mmio_clrbits32(addr, clear)		mmio_write_32(addr, mmio_read_32(addr) & ~(clear))
#define mmio_setbits32(addr, set)		mmio_write_32(addr, mmio_read_32(addr) | (set))

#endif /* IMX_USDHC_H */
