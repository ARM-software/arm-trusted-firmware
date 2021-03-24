/*
 * Copyright (c) 2015, 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SD_MMC_H
#define SD_MMC_H

#include <lib/mmio.h>

/* operating freq */
#define CARD_IDENTIFICATION_FREQ	400000
#define SD_SS_25MHZ	20000000
#define SD_HS_50MHZ	40000000
#define MMC_SS_20MHZ	15000000
#define MMC_HS_26MHZ	20000000
#define MMC_HS_52MHZ	40000000

/* Need to check this value ? */
#define MAX_PLATFORM_CLOCK	800000000

/* eSDHC system control register defines */
#define ESDHC_SYSCTL_DTOCV(t)		(((t) & 0xF) << 16)
#define ESDHC_SYSCTL_SDCLKFS(f)		(((f) & 0xFF) << 8)
#define ESDHC_SYSCTL_DVS(d)		(((d) & 0xF) << 4)
#define ESDHC_SYSCTL_SDCLKEN		(0x00000008)
#define ESDHC_SYSCTL_RSTA		(0x01000000)

/* Data timeout counter value. SDHC_CLK x 227 */
#define TIMEOUT_COUNTER_SDCLK_2_27	0xE
#define ESDHC_SYSCTL_INITA	0x08000000

/* eSDHC interrupt status enable register defines */
#define ESDHC_IRQSTATEN_CINS	0x00000040
#define ESDHC_IRQSTATEN_BWR	0x00000010

/* eSDHC interrupt status register defines */
#define ESDHC_IRQSTAT_DMAE	(0x10000000)
#define ESDHC_IRQSTAT_AC12E	(0x01000000)
#define ESDHC_IRQSTAT_DEBE	(0x00400000)
#define ESDHC_IRQSTAT_DCE	(0x00200000)
#define ESDHC_IRQSTAT_DTOE	(0x00100000)
#define ESDHC_IRQSTAT_CIE	(0x00080000)
#define ESDHC_IRQSTAT_CEBE	(0x00040000)
#define ESDHC_IRQSTAT_CCE	(0x00020000)
#define ESDHC_IRQSTAT_CTOE	(0x00010000)
#define ESDHC_IRQSTAT_CINT	(0x00000100)
#define ESDHC_IRQSTAT_CRM	(0x00000080)
#define ESDHC_IRQSTAT_CINS	(0x00000040)
#define ESDHC_IRQSTAT_BRR	(0x00000020)
#define ESDHC_IRQSTAT_BWR	(0x00000010)
#define ESDHC_IRQSTAT_DINT	(0x00000008)
#define ESDHC_IRQSTAT_BGE	(0x00000004)
#define ESDHC_IRQSTAT_TC	(0x00000002)
#define ESDHC_IRQSTAT_CC	(0x00000001)
#define ESDHC_IRQSTAT_CMD_ERR	(ESDHC_IRQSTAT_CIE |\
			ESDHC_IRQSTAT_CEBE |\
			ESDHC_IRQSTAT_CCE)
#define ESDHC_IRQSTAT_DATA_ERR	(ESDHC_IRQSTAT_DEBE |\
			ESDHC_IRQSTAT_DCE |\
			ESDHC_IRQSTAT_DTOE)
#define ESDHC_IRQSTAT_CLEAR_ALL	(0xFFFFFFFF)

/* eSDHC present state register defines */
#define ESDHC_PRSSTAT_CLSL	0x00800000
#define ESDHC_PRSSTAT_WPSPL	0x00080000
#define ESDHC_PRSSTAT_CDPL	0x00040000
#define ESDHC_PRSSTAT_CINS	0x00010000
#define ESDHC_PRSSTAT_BREN	0x00000800
#define ESDHC_PRSSTAT_BWEN	0x00000400
#define ESDHC_PRSSTAT_RTA	0x00000200
#define ESDHC_PRSSTAT_WTA	0x00000100
#define ESDHC_PRSSTAT_SDOFF	0x00000080
#define ESDHC_PRSSTAT_PEROFF	0x00000040
#define ESDHC_PRSSTAT_HCKOFF	0x00000020
#define ESDHC_PRSSTAT_IPGOFF	0x00000010
#define ESDHC_PRSSTAT_DLA	0x00000004
#define ESDHC_PRSSTAT_CDIHB	0x00000002
#define ESDHC_PRSSTAT_CIHB	0x00000001

/* eSDHC protocol control register defines */
#define ESDHC_PROCTL_EMODE_LE	0x00000020
#define ESDHC_PROCTL_DTW_1BIT	0x00000000
#define ESDHC_PROCTL_DTW_4BIT	0x00000002
#define ESDHC_PROCTL_DTW_8BIT	0x00000004

/* Watermark Level Register (WML) */
#define ESDHC_WML_RD_WML(w)	((w) & 0x7F)
#define ESDHC_WML_WR_WML(w)	(((w) & 0x7F) << 16)
#define ESDHC_WML_RD_BRST(w)	(((w) & 0xF) << 8)
#define ESDHC_WML_WR_BRST(w)	(((w) & 0xF) << 24)
#define ESDHC_WML_WR_BRST_MASK	(0x0F000000)
#define ESDHC_WML_RD_BRST_MASK	(0x00000F00)
#define ESDHC_WML_RD_WML_MASK	(0x0000007F)
#define ESDHC_WML_WR_WML_MASK	(0x007F0000)
#define WML_512_BYTES		(0x0)
#define BURST_128_BYTES	(0x0)

/* eSDHC control register define */
#define ESDHC_DCR_SNOOP		0x00000040

/* ESDHC Block attributes register */
#define ESDHC_BLKATTR_BLKCNT(c)	(((c) & 0xffff) << 16)
#define ESDHC_BLKATTR_BLKSZE(s)	((s) & 0xfff)

/* Transfer Type Register */
#define ESDHC_XFERTYP_CMD(c)	(((c) & 0x3F) << 24)
#define ESDHC_XFERTYP_CMDTYP_NORMAL	(0x0)
#define ESDHC_XFERTYP_CMDTYP_SUSPEND	(0x00400000)
#define ESDHC_XFERTYP_CMDTYP_RESUME	(0x00800000)
#define ESDHC_XFERTYP_CMDTYP_ABORT	(0x00C00000)
#define ESDHC_XFERTYP_DPSEL	(0x00200000)
#define ESDHC_XFERTYP_CICEN	(0x00100000)
#define ESDHC_XFERTYP_CCCEN	(0x00080000)
#define ESDHC_XFERTYP_RSPTYP_NONE	(0x0)
#define ESDHC_XFERTYP_RSPTYP_136	(0x00010000)
#define ESDHC_XFERTYP_RSPTYP_48	(0x00020000)
#define ESDHC_XFERTYP_RSPTYP_48_BUSY	(0x00030000)
#define ESDHC_XFERTYP_MSBSEL	(0x00000020)
#define ESDHC_XFERTYP_DTDSEL	(0x00000010)
#define ESDHC_XFERTYP_AC12EN	(0x00000004)
#define ESDHC_XFERTYP_BCEN	(0x00000002)
#define ESDHC_XFERTYP_DMAEN	(0x00000001)

#define MMC_VDD_HIGH_VOLTAGE	0x00000100

/* command index */
#define CMD0	0
#define CMD1	1
#define CMD2	2
#define CMD3	3
#define CMD5	5
#define CMD6	6
#define CMD7	7
#define CMD8	8
#define CMD9	9
#define CMD12	12
#define CMD13	13
#define CMD14	14
#define CMD16	16
#define CMD17	17
#define CMD18	18
#define CMD19	19
#define CMD24	24
#define CMD41	41
#define CMD42	42
#define CMD51	51
#define CMD55	55
#define CMD56	56
#define ACMD6	CMD6
#define ACMD13	CMD13
#define ACMD41	CMD41
#define ACMD42	CMD42
#define ACMD51	CMD51

/* commands abbreviations */
#define CMD_GO_IDLE_STATE	CMD0
#define CMD_MMC_SEND_OP_COND	CMD1
#define CMD_ALL_SEND_CID	CMD2
#define CMD_SEND_RELATIVE_ADDR	CMD3
#define CMD_SET_DSR	CMD4
#define CMD_SWITCH_FUNC	CMD6
#define CMD_SELECT_CARD	CMD7
#define CMD_DESELECT_CARD	CMD7
#define CMD_SEND_IF_COND	CMD8
#define CMD_MMC_SEND_EXT_CSD	CMD8
#define CMD_SEND_CSD	CMD9
#define CMD_SEND_CID	CMD10
#define CMD_STOP_TRANSMISSION	CMD12
#define CMD_SEND_STATUS	CMD13
#define CMD_BUS_TEST_R	CMD14
#define CMD_GO_INACTIVE_STATE	CMD15
#define CMD_SET_BLOCKLEN	CMD16
#define CMD_READ_SINGLE_BLOCK	CMD17
#define CMD_READ_MULTIPLE_BLOCK	CMD18
#define CMD_WRITE_SINGLE_BLOCK	CMD24
#define CMD_BUS_TEST_W	CMD19
#define CMD_APP_CMD	CMD55
#define CMD_GEN_CMD	CMD56
#define CMD_SET_BUS_WIDTH	ACMD6
#define CMD_SD_STATUS	ACMD13
#define CMD_SD_SEND_OP_COND	ACMD41
#define CMD_SET_CLR_CARD_DETECT	ACMD42
#define CMD_SEND_SCR	ACMD51

/* MMC card spec version */
#define MMC_CARD_VERSION_1_2	0
#define MMC_CARD_VERSION_1_4	1
#define MMC_CARD_VERSION_2_X	2
#define MMC_CARD_VERSION_3_X	3
#define MMC_CARD_VERSION_4_X	4

/* SD Card Spec Version */
/* May need to add version 3 here? */
#define SD_CARD_VERSION_1_0	0
#define SD_CARD_VERSION_1_10	1
#define SD_CARD_VERSION_2_0	2

/* card types */
#define MMC_CARD	0
#define SD_CARD		1
#define NOT_SD_CARD	MMC_CARD

/* Card rca */
#define SD_MMC_CARD_RCA	0x1
#define BLOCK_LEN_512	512

/* card state */
#define STATE_IDLE	0
#define STATE_READY	1
#define STATE_IDENT	2
#define STATE_STBY	3
#define STATE_TRAN	4
#define STATE_DATA	5
#define STATE_RCV	6
#define STATE_PRG	7
#define STATE_DIS	8

/* Card OCR register */
/* VDD voltage window 1,65 to 1.95 */
#define MMC_OCR_VDD_165_195	0x00000080
/* VDD voltage window 2.7-2.8 */
#define MMC_OCR_VDD_FF8	0x00FF8000
#define MMC_OCR_CCS	0x40000000/* Card Capacity */
#define MMC_OCR_BUSY	0x80000000/* busy bit */
#define SD_OCR_HCS	0x40000000/* High capacity host */
#define MMC_OCR_SECTOR_MODE	0x40000000/* Access Mode as Sector */

/* mmc Switch function */
#define SET_EXT_CSD_HS_TIMING	0x03B90100/* set High speed */

/* check supports switching or not */
#define SD_SWITCH_FUNC_CHECK_MODE	0x00FFFFF1
#define SD_SWITCH_FUNC_SWITCH_MODE	0x80FFFFF1/* switch */
#define SD_SWITCH_FUNC_HIGH_SPEED	0x02/* HIGH SPEED FUNC */
#define SWITCH_ERROR		0x00000080

/* errors in sending commands */
#define RESP_TIMEOUT	0x1
#define COMMAND_ERROR	0x2
/* error in response */
#define R1_ERROR	(1 << 19)
#define R1_CURRENT_STATE(x)	(((x) & 0x00001E00) >> 9)

/* Host Controller Capabilities */
#define ESDHC_HOSTCAPBLT_DMAS           (0x00400000)


/* SD/MMC memory map */
struct esdhc_regs {
	uint32_t dsaddr;	/* dma system address */
	uint32_t blkattr;	/* Block attributes */
	uint32_t cmdarg;	/* Command argument */
	uint32_t xfertyp;	/* Command transfer type */
	uint32_t cmdrsp[4];	/* Command response0,1,2,3 */
	uint32_t datport;	/* Data buffer access port */
	uint32_t prsstat;	/* Present state */
	uint32_t proctl;	/* Protocol control */
	uint32_t sysctl;	/* System control */
	uint32_t irqstat;	/* Interrupt status */
	uint32_t irqstaten;	/* Interrupt status enable */
	uint32_t irqsigen;	/* Interrupt signal enable */
	uint32_t autoc12err;	/* Auto CMD12 status */
	uint32_t hostcapblt;	/* Host controller capabilities */
	uint32_t wml;	/* Watermark level */
	uint32_t res1[2];
	uint32_t fevt;	/* Force event */
	uint32_t res2;
	uint32_t adsaddrl;
	uint32_t adsaddrh;
	uint32_t res3[39];
	uint32_t hostver;	/* Host controller version */
	uint32_t res4;
	uint32_t dmaerr;	/* DMA error address */
	uint32_t dmaerrh;	/* DMA error address high */
	uint32_t dmaerrattr; /* DMA error atrribute */
	uint32_t res5;
	uint32_t hostcapblt2;/* Host controller capabilities2 */
	uint32_t res6[2];
	uint32_t tcr;	/* Tuning control */
	uint32_t res7[7];
	uint32_t dirctrl;	/* Direction control */
	uint32_t ccr;	/* Clock control */
	uint32_t res8[177];
	uint32_t ctl;	/* Control register */
};

/* SD/MMC card attributes */
struct card_attributes {
	uint32_t type;	/* sd or mmc card */
	uint32_t version;	/* version */
	uint32_t block_len;	/* block length */
	uint32_t bus_freq;	/* sdhc bus frequency */
	uint16_t rca;	/* relative card address */
	uint8_t is_high_capacity;	/* high capacity */
};

struct mmc {
	struct esdhc_regs *esdhc_regs;
	struct card_attributes card;

	uint32_t block_len;
	uint32_t voltages_caps;	/* supported voltaes */
	uint32_t dma_support;	/* DMA support */
};

enum cntrl_num {
	SDHC1 = 0,
	SDHC2
};

int sd_emmc_init(uintptr_t *block_dev_spec,
			uintptr_t nxp_esdhc_addr,
			size_t nxp_sd_block_offset,
			size_t nxp_sd_block_size,
			bool card_detect);

int esdhc_emmc_init(struct mmc *mmc, bool card_detect);
int esdhc_read(struct mmc *mmc, uint32_t src_offset, uintptr_t dst,
	       size_t size);
int esdhc_write(struct mmc *mmc, uintptr_t src, uint32_t dst_offset,
		size_t size);

#ifdef NXP_ESDHC_BE
#define esdhc_in32(a)           bswap32(mmio_read_32((uintptr_t)(a)))
#define esdhc_out32(a, v)       mmio_write_32((uintptr_t)(a), bswap32(v))
#elif defined(NXP_ESDHC_LE)
#define esdhc_in32(a)           mmio_read_32((uintptr_t)(a))
#define esdhc_out32(a, v)       mmio_write_32((uintptr_t)(a), (v))
#else
#error Please define CCSR ESDHC register endianness
#endif

#endif /*SD_MMC_H*/
