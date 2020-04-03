/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	CSL_SD_PROT_H
#define	CSL_SD_PROT_H

#define SD_CARD_UNKNOWN		0	/* bad type or unrecognized */
#define SD_CARD_SD		1	/* IO only card */
#define SD_CARD_SDIO		2	/* memory only card */
#define SD_CARD_COMBO		3	/* IO and memory combo card */
#define SD_CARD_MMC		4	/* memory only card */
#define SD_CARD_CEATA		5	/* IO and memory combo card */

#define SD_IO_FIXED_ADDRESS	0	/* fix Address */
#define SD_IO_INCREMENT_ADDRESS	1

#define SD_HIGH_CAPACITY_CARD	0x40000000

#define MMC_CMD_IDLE_RESET_ARG	0xF0F0F0F0

/* Supported operating voltages are 3.2-3.3 and 3.3-3.4 */
#define MMC_OCR_OP_VOLT			0x00300000
/* Enable sector access mode */
#define MMC_OCR_SECTOR_ACCESS_MODE	0x40000000

/* command index */
#define SD_CMD_GO_IDLE_STATE		0	/* mandatory for SDIO */
#define SD_CMD_SEND_OPCOND		1
#define SD_CMD_ALL_SEND_CID		2
#define SD_CMD_MMC_SET_RCA		3
#define SD_CMD_MMC_SET_DSR		4
#define SD_CMD_IO_SEND_OP_COND		5	/* mandatory for SDIO */
#define SD_ACMD_SET_BUS_WIDTH		6
#define SD_CMD_SWITCH_FUNC		6
#define SD_CMD_SELECT_DESELECT_CARD	7
#define SD_CMD_READ_EXT_CSD		8
#define SD_CMD_SEND_CSD			9
#define SD_CMD_SEND_CID			10
#define SD_CMD_STOP_TRANSMISSION	12
#define SD_CMD_SEND_STATUS		13
#define SD_ACMD_SD_STATUS		13
#define SD_CMD_GO_INACTIVE_STATE	15
#define SD_CMD_SET_BLOCKLEN		16
#define SD_CMD_READ_SINGLE_BLOCK	17
#define SD_CMD_READ_MULTIPLE_BLOCK	18
#define SD_CMD_WRITE_BLOCK		24
#define SD_CMD_WRITE_MULTIPLE_BLOCK	25
#define SD_CMD_PROGRAM_CSD		27
#define SD_CMD_SET_WRITE_PROT		28
#define SD_CMD_CLR_WRITE_PROT		29
#define SD_CMD_SEND_WRITE_PROT		30
#define SD_CMD_ERASE_WR_BLK_START	32
#define SD_CMD_ERASE_WR_BLK_END		33
#define SD_CMD_ERASE_GROUP_START	35
#define SD_CMD_ERASE_GROUP_END		36
#define SD_CMD_ERASE			38
#define SD_CMD_LOCK_UNLOCK		42
#define SD_CMD_IO_RW_DIRECT		52	/* mandatory for SDIO */
#define SD_CMD_IO_RW_EXTENDED		53	/* mandatory for SDIO */
#define SD_CMD_APP_CMD			55
#define SD_CMD_GEN_CMD			56
#define SD_CMD_READ_OCR			58
#define SD_CMD_CRC_ON_OFF		59	/* mandatory for SDIO */
#define SD_ACMD_SEND_NUM_WR_BLOCKS	22
#define SD_ACMD_SET_WR_BLOCK_ERASE_CNT	23
#define SD_ACMD_SD_SEND_OP_COND		41
#define SD_ACMD_SET_CLR_CARD_DETECT	42
#define SD_ACMD_SEND_SCR		51

/* response parameters */
#define SD_RSP_NO_NONE	0
#define SD_RSP_NO_1	1
#define SD_RSP_NO_2	2
#define SD_RSP_NO_3	3
#define SD_RSP_NO_4	4
#define SD_RSP_NO_5	5
#define SD_RSP_NO_6	6

/* Modified R6 response (to CMD3) */
#define SD_RSP_MR6_COM_CRC_ERROR	0x8000
#define SD_RSP_MR6_ILLEGAL_COMMAND	0x4000
#define SD_RSP_MR6_ERROR		0x2000

/* Modified R1 in R4 Response (to CMD5) */
#define SD_RSP_MR1_SBIT			0x80
#define SD_RSP_MR1_PARAMETER_ERROR	0x40
#define SD_RSP_MR1_RFU5			0x20
#define SD_RSP_MR1_FUNC_NUM_ERROR	0x10
#define SD_RSP_MR1_COM_CRC_ERROR	0x80
#define SD_RSP_MR1_ILLEGAL_COMMAND	0x40
#define SD_RSP_MR1_RFU1			0x20
#define SD_RSP_MR1_IDLE_STATE		0x01

/* R5 response (to CMD52 and CMD53) */
#define SD_RSP_R5_COM_CRC_ERROR		0x80
#define SD_RSP_R5_ILLEGAL_COMMAND	0x40
#define SD_RSP_R5_IO_CURRENTSTATE1	0x20
#define SD_RSP_R5_IO_CURRENTSTATE0	0x10
#define SD_RSP_R5_ERROR			0x80
#define SD_RSP_R5_RFU			0x40
#define SD_RSP_R5_FUNC_NUM_ERROR	0x20
#define SD_RSP_R5_OUT_OF_RANGE		0x01

/* argument for SD_CMD_IO_RW_DIRECT and SD_CMD_IO_RW_EXTENDED */
#define SD_OP_READ			0 /* Read_Write */
#define SD_OP_WRITE			1 /* Read_Write */

#define SD_RW_NORMAL			0 /* no RAW */
#define SD_RW_RAW			1 /* RAW */

#define SD_BYTE_MODE			0 /* Byte Mode */
#define SD_BLOCK_MODE			1 /* BlockMode */

#define SD_FIXED_ADDRESS		0 /* fix Address */
#define SD_INCREMENT_ADDRESS		1 /* IncrementAddress */

#define SD_CMD5_ARG_IO_OCR_MASK		0x00FFFFFF
#define SD_CMD5_ARG_IO_OCR_SHIFT	0
#define SD_CMD55_ARG_RCA_SHIFT		16
#define SD_CMD59_ARG_CRC_OPTION_MASK	0x01
#define SD_CMD59_ARG_CRC_OPTION_SHIFT	0

/* SD_CMD_IO_RW_DIRECT Argument */
#define SdioIoRWDirectArg(rw, raw, func, addr, data) \
		(((rw & 1) << 31) | ((func & 0x7) << 28) | \
		((raw & 1) << 27) | ((addr & 0x1FFFF) << 9) | \
		(data & 0xFF))

/* build SD_CMD_IO_RW_EXTENDED Argument */
#define SdioIoRWExtArg(rw, blk, func, addr, inc_addr, count) \
		(((rw & 1) << 31) | ((func & 0x7) << 28) | \
		((blk & 1) << 27) | ((inc_addr & 1) << 26) | \
		((addr & 0x1FFFF) << 9) | (count & 0x1FF))

/*
 * The Common I/O area shall be implemented on all SDIO cards and
 * is accessed the the host via I/O reads and writes to function 0,
 * the registers within the CIA are provided to enable/disable
 * the operationo fthe i/o funciton.
 */

/* cccr_sdio_rev */
#define SDIO_REV_SDIOID_MASK		0xf0 /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK		0x0f /* CCCR format version number */

/* sd_rev */
#define SDIO_REV_PHY_MASK	    0x0f /* SD format version number */
#define SDIO_FUNC_ENABLE_1	    0x02 /* function 1 I/O enable */
#define SDIO_FUNC_READY_1	    0x02 /* function 1 I/O ready */
#define SDIO_INTR_CTL_FUNC1_EN	    0x2  /* interrupt enable for function 1 */
#define SDIO_INTR_CTL_MASTER_EN	    0x1  /* interrupt enable master */
#define SDIO_INTR_STATUS_FUNC1	    0x2  /* interrupt pending for function 1 */
#define SDIO_IO_ABORT_RESET_ALL	    0x08 /* I/O card reset */
#define SDIO_IO_ABORT_FUNC_MASK	    0x07 /* abort selection: function x */
#define SDIO_BUS_CARD_DETECT_DIS    0x80 /* Card Detect disable */
#define SDIO_BUS_SPI_CONT_INTR_CAP  0x40 /* support continuous SPI interrupt */
#define SDIO_BUS_SPI_CONT_INTR_EN   0x20 /* continuous SPI interrupt enable */
#define SDIO_BUS_DATA_WIDTH_MASK    0x03 /* bus width mask */
#define SDIO_BUS_DATA_WIDTH_4BIT    0x02 /* bus width 4-bit mode */
#define SDIO_BUS_DATA_WIDTH_1BIT    0x00 /* bus width 1-bit mode */

/* capability */
#define SDIO_CAP_4BLS  0x80 /* 4-bit support for low speed card */
#define SDIO_CAP_LSC   0x40 /* low speed card */
#define SDIO_CAP_E4MI  0x20 /* enable int between block in 4-bit mode */
#define SDIO_CAP_S4MI  0x10 /* support int between block in 4-bit mode */
#define SDIO_CAP_SBS   0x08 /* support suspend/resume */
#define SDIO_CAP_SRW   0x04 /* support read wait */
#define SDIO_CAP_SMB   0x02 /* support multi-block transfer */
#define SDIO_CAP_SDC   0x01 /* Support Direct cmd during multi-uint8 transfer */

/* CIA FBR1 registers */
#define SDIO_FUNC1_INFO           0x100 /* basic info for function 1 */
#define SDIO_FUNC1_EXT            0x101 /* extension of standard I/O device */
#define SDIO_CIS_FUNC1_BASE_LOW   0x109 /* function 1 cis address bit 0-7 */
#define SDIO_CIS_FUNC1_BASE_MID   0x10A /* function 1 cis address bit 8-15 */
#define SDIO_CIS_FUNC1_BASE_HIGH  0x10B /* function 1 cis address bit 16 */
#define SDIO_CSA_BASE_LOW         0x10C /* CSA base address uint8_t 0 */
#define SDIO_CSA_BASE_MID         0x10D /* CSA base address uint8_t 1 */
#define SDIO_CSA_BASE_HIGH        0x10E /* CSA base address uint8_t 2 */
#define SDIO_CSA_DATA_OFFSET      0x10F /* CSA data register */
#define SDIO_IO_BLK_SIZE_LOW      0x110 /* I/O block size uint8_t 0 */
#define SDIO_IO_BLK_SIZE_HIGH     0x111 /* I/O block size uint8_t 1 */

/* SD_SDIO_FUNC1_INFO bits */
#define SDIO_FUNC1_INFO_DIC     0x0f	/* device interface code */
#define SDIO_FUNC1_INFO_CSA     0x40	/* CSA support flag */
#define SDIO_FUNC1_INFO_CSA_EN  0x80	/* CSA enabled */

/* SD_SDIO_FUNC1_EXT bits */
#define SDIO_FUNC1_EXT_SHP		0x03	/* support high power */
#define SDIO_FUNC1_EXT_EHP		0x04	/* enable high power */

/* devctr */
/* I/O device interface code */
#define SDIO_DEVCTR_DEVINTER		0x0f
/* support CSA */
#define SDIO_DEVCTR_CSA_SUP		0x40
/* enable CSA */
#define SDIO_DEVCTR_CSA_EN		0x80

/* ext_dev */
/* supports high-power mask */
#define SDIO_HIGHPWR_SUPPORT_M		0x3
/* enable high power */
#define SDIO_HIGHPWR_EN			0x4
/* standard power function(up to 200mA */
#define SDIO_HP_STD			0
/* need high power to operate */
#define SDIO_HP_REQUIRED		0x2
/* can work with standard power, but prefer high power */
#define SDIO_HP_DESIRED			0x3

/* misc define */
/* macro to calculate fbr register base */
#define FBR_REG_BASE(n)			(n*0x100)
#define SDIO_FUNC_0			0
#define SDIO_FUNC_1			1
#define SDIO_FUNC_2			2
#define SDIO_FUNC_3			3
#define SDIO_FUNC_4			4
#define SDIO_FUNC_5			5
#define SDIO_FUNC_6			6
#define SDIO_FUNC_7			7

/* maximum block size for block mode operation */
#define SDIO_MAX_BLOCK_SIZE		2048
/* minimum block size for block mode operation */
#define SDIO_MIN_BLOCK_SIZE		1

/* Card registers: status bit position */
#define SDIO_STATUS_OUTOFRANGE		31
#define SDIO_STATUS_COMCRCERROR		23
#define SDIO_STATUS_ILLEGALCOMMAND	22
#define SDIO_STATUS_ERROR		19
#define SDIO_STATUS_IOCURRENTSTATE3	12
#define SDIO_STATUS_IOCURRENTSTATE2	11
#define SDIO_STATUS_IOCURRENTSTATE1	10
#define SDIO_STATUS_IOCURRENTSTATE0	9
#define SDIO_STATUS_FUN_NUM_ERROR	4

#define GET_SDIOCARD_STATUS(x)		((x >> 9) & 0x0f)
#define SDIO_STATUS_STATE_IDLE		0
#define SDIO_STATUS_STATE_READY		1
#define SDIO_STATUS_STATE_IDENT		2
#define SDIO_STATUS_STATE_STBY		3
#define SDIO_STATUS_STATE_TRAN		4
#define SDIO_STATUS_STATE_DATA		5
#define SDIO_STATUS_STATE_RCV		6
#define SDIO_STATUS_STATE_PRG		7
#define SDIO_STATUS_STATE_DIS		8

/* sprom */
#define SBSDIO_SPROM_CS        0x10000	/* command and status */
#define SBSDIO_SPROM_INFO      0x10001	/* info register */
#define SBSDIO_SPROM_DATA_LOW  0x10002	/* indirect access data uint8_t 0 */
#define SBSDIO_SPROM_DATA_HIGH 0x10003	/* indirect access data uint8_t 1 */
#define SBSDIO_SPROM_ADDR_LOW  0x10004	/* indirect access addr uint8_t 0 */
#define SBSDIO_SPROM_ADDR_HIGH 0x10005	/* indirect access addr uint8_t 0 */
#define SBSDIO_CHIP_CTRL_DATA  0x10006	/* xtal_pu data output */
#define SBSDIO_CHIP_CTRL_EN    0x10007	/* xtal_pu enable */
#define SBSDIO_WATERMARK       0x10008	/* retired in rev 7 */
#define SBSDIO_DEVICE_CTL      0x10009	/* control busy signal generation */

#define SBSDIO_SPROM_IDLE      0
#define SBSDIO_SPROM_WRITE     1
#define SBSDIO_SPROM_READ      2
#define SBSDIO_SPROM_WEN       4
#define SBSDIO_SPROM_WDS       7
#define SBSDIO_SPROM_DONE      8

/* SBSDIO_SPROM_INFO */
#define SBSDIO_SROM_SZ_MASK		0x03	/* SROM size, 1: 4k, 2: 16k */
#define SBSDIO_SROM_BLANK		0x04	/* depreciated in corerev 6 */
#define	SBSDIO_SROM_OTP			0x80	/* OTP present */

/* SBSDIO_CHIP_CTRL */
/* or'd with onchip xtal_pu, 1: power on oscillator */
#define SBSDIO_CHIP_CTRL_XTAL		0x01

/* SBSDIO_WATERMARK */
/* number of bytes minus 1 for sd device to wait before sending data to host */
#define SBSDIO_WATERMARK_MASK		0x3f

/* SBSDIO_DEVICE_CTL */
/* 1: device will assert busy signal when receiving CMD53 */
#define SBSDIO_DEVCTL_SETBUSY		0x01
/* 1: assertion of sdio interrupt is synchronous to the sdio clock */
#define SBSDIO_DEVCTL_SPI_INTR_SYNC	0x02

/* function 1 OCP space */
/* sb offset addr is <= 15 bits, 32k */
#define SBSDIO_SB_OFT_ADDR_MASK		0x07FFF
#define SBSDIO_SB_OFT_ADDR_LIMIT	0x08000
/* sdsdio function 1 OCP space has 16/32 bit section */
#define SBSDIO_SB_ACCESS_2_4B_FLAG	0x08000

/* direct(mapped) cis space */
/* MAPPED common CIS address */
#define SBSDIO_CIS_BASE_COMMON		0x1000
/* function 0(common) cis size in bytes */
#define SBSDIO_CIS_FUNC0_LIMIT		0x020
/* funciton 1 cis size in bytes */
#define SBSDIO_CIS_SIZE_LIMIT		0x200
/* cis offset addr is < 17 bits */
#define SBSDIO_CIS_OFT_ADDR_MASK	0x1FFFF
/* manfid tuple length, include tuple, link bytes */
#define SBSDIO_CIS_MANFID_TUPLE_LEN	6

/* indirect cis access (in sprom) */
/* 8 control bytes first, CIS starts from 8th uint8_t */
#define SBSDIO_SPROM_CIS_OFFSET		0x8
/* sdio uint8_t mode: maximum length of one data comamnd */
#define SBSDIO_BYTEMODE_DATALEN_MAX	64
/* 4317 supports less */
#define SBSDIO_BYTEMODE_DATALEN_MAX_4317	52
/* sdio core function one address mask */
#define SBSDIO_CORE_ADDR_MASK	0x1FFFF

/* CEATA defines */
#define CEATA_EXT_CSDBLOCK_SIZE         512
#define CEATA_FAST_IO                   39
#define CEATA_MULTIPLE_REGISTER_RW      60
#define CEATA_MULTIPLE_BLOCK_RW         61

/* defines CE ATA task file registers */
#define CEATA_SCT_CNT_EXP_REG           0x02
#define CEATA_LBA_LOW_EXP_REG           0x03
#define CEATA_LBA_MID_EXP_REG           0x04
#define CEATA_LBA_HIGH_EXP_REG          0x05
#define CEATA_CNTRL_REG                 0x06
#define CEATA_FEATURE_REG               0x09	/* write */
#define CEATA_ERROR_REG                 0x09	/* read */
#define CEATA_SCT_CNT_REG               0x0A
#define CEATA_LBA_LOW_REG               0x0B
#define CEATA_LBA_MID_REG               0x0C
#define CEATA_LBA_HIGH_REG              0x0D
#define CEATA_DEV_HEAD_REG              0x0E
#define CEATA_STA_REG                   0x0F	/* read */
#define CEATA_CMD_REG                   0x0F	/* write */

/* defines CEATA control and status registers for ce ata client driver */
#define CEATA_SCR_TEMPC_REG             0x80
#define CEATA_SCR_TEMPMAXP_REG          0x84
#define CEATA_TEMPMINP_REG              0x88
#define CEATA_SCR_STATUS_REG            0x8C
#define CEATA_SCR_REALLOCSA_REG         0x90
#define CEATA_SCR_ERETRACTSA_REG        0x94
#define CEATA_SCR_CAPABILITIES_REG      0x98
#define CEATA_SCR_CONTROL_REG           0xC0

/* defines for SCR capabilities register bits for ce ata client driver */
#define CEATA_SCR_CAP_512               0x00000001
#define CEATA_SCR_CAP_1K                0x00000002
#define CEATA_SCR_CAP_4K                0x00000004

/* defines CE ATA Control reg bits for ce ata client driver */
#define CEATA_CNTRL_ENABLE_INTR         0x00
#define CEATA_CNTRL_DISABLE_INTR        0x02
#define CEATA_CNTRL_SRST                0x04
#define CEATA_CNTRL_RSRST               0x00

/* define CE ATA Status reg bits for ce ata client driver */
#define CEATA_STA_ERROR_BIT             0x01
#define CEATA_STA_OVR_BIT               0x02
#define CEATA_STA_SPT_BIT               0x04
#define CEATA_STA_DRQ_BIT               0x08
#define CEATA_STA_DRDY_BIT              0x40
#define CEATA_STA_BSY_BIT               0x80

/* define CE ATA Error reg bits for ce ata client driver */
#define CEATA_ERROR_ABORTED_BIT         0x04
#define CEATA_ERROR_IDNF_BIT            0x10
#define CEATA_ERROR_UNCORRECTABLE_BIT   0x40
#define CEATA_ERROR_ICRC_BIT            0x80

/* define CE ATA Commands for ce ata client driver */
#define CEATA_CMD_IDENTIFY_DEVICE       0xEC
#define CEATA_CMD_READ_DMA_EXT          0x25
#define CEATA_CMD_WRITE_DMA_EXT         0x35
#define CEATA_CMD_STANDBY_IMMEDIATE     0xE0
#define CEATA_CMD_FLUSH_CACHE_EXT       0xEA

struct csd_mmc {
	uint32_t padding:8;
	uint32_t structure:2;
	uint32_t csdSpecVer:4;
	uint32_t reserved1:2;
	uint32_t taac:8;
	uint32_t nsac:8;
	uint32_t speed:8;
	uint32_t classes:12;
	uint32_t rdBlkLen:4;
	uint32_t rdBlkPartial:1;
	uint32_t wrBlkMisalign:1;
	uint32_t rdBlkMisalign:1;
	uint32_t dsr:1;
	uint32_t reserved2:2;
	uint32_t size:12;
	uint32_t vddRdCurrMin:3;
	uint32_t vddRdCurrMax:3;
	uint32_t vddWrCurrMin:3;
	uint32_t vddWrCurrMax:3;
	uint32_t devSizeMulti:3;
	uint32_t eraseGrpSize:5;
	uint32_t eraseGrpSizeMulti:5;
	uint32_t wrProtGroupSize:5;
	uint32_t wrProtGroupEnable:1;
	uint32_t manuDefEcc:2;
	uint32_t wrSpeedFactor:3;
	uint32_t wrBlkLen:4;
	uint32_t wrBlkPartial:1;
	uint32_t reserved5:4;
	uint32_t protAppl:1;
	uint32_t fileFormatGrp:1;
	uint32_t copyFlag:1;
	uint32_t permWrProt:1;
	uint32_t tmpWrProt:1;
	uint32_t fileFormat:2;
	uint32_t eccCode:2;
};

/* CSD register*/
union sd_csd {
	uint32_t csd[4];
	struct csd_mmc mmc;
};

struct sd_card_data {
	union sd_csd csd;
};
#endif /* CSL_SD_PROT_H */
