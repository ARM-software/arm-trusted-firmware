/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CDN_NAND_H
#define CDN_NAND_H

#include <drivers/cadence/cdns_combo_phy.h>

// TBD: Move to common place
#define __bf_shf(x)			(__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

#define FIELD_PREP(_mask, _val)						\
	({ \
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

/* NAND flash device information */
typedef struct cnf_dev_info {
	uint8_t mfr_id;
	uint8_t dev_id;
	uint8_t type;
	uint8_t nluns;
	uint8_t sector_cnt;
	uint16_t npages_per_block;
	uint16_t sector_size;
	uint16_t last_sector_size;
	uint16_t page_size;
	uint16_t spare_size;
	uint32_t nblocks_per_lun;
	uint32_t block_size;
	unsigned long long total_size;
} cnf_dev_info_t;

/* Shared Macros */

/* Default values */
#define CNF_DEF_VOL_ID			0
#define CNF_DEF_DEVICE			0
#define CNF_DEF_TRD			0
#define CNF_READ_SINGLE_PAGE		1U
#define CNF_DEF_DELAY_US		500U
#define CNF_READ_INT_DELAY_US		10U

/* Number of micro seconds to complete the Device Discovery Process. */
#define CNF_DD_INIT_COMP_US		2000000U

/* Work modes */
#define CNF_WORK_MODE_CDMA		0U
#define CNF_WORK_MODE_PIO		1U

/* Command types */
#define CNF_CT_SET_FEATURE		0x0100
#define CNF_CT_RESET_ASYNC		0x1100
#define CNF_CT_RESET_SYNC		0x1101
#define CNF_CT_RESET_LUN		0x1102
#define CNF_CT_ERASE			0x1000
#define CNF_CT_PAGE_PROGRAM		0x2100
#define CNF_CT_PAGE_READ		0x2200

/* Interrupts enable or disable */
#define CNF_INT_EN			1U
#define CNF_INT_DIS			0U

/* Device types */
#define CNF_DT_UNKNOWN			0x00
#define CNF_DT_ONFI			0x01
#define CNF_DT_JEDEC			0x02
#define CNF_DT_LEGACY			0x03

/* Command and status registers */
#define CNF_CMDREG_REG_BASE		SOCFPGA_NAND_REG_BASE

/* DMA maximum burst size 0-127*/
#define CNF_DMA_BURST_SIZE_MAX		127U

/* DMA settings register field offsets */
#define CNF_DMA_SETTINGS_BURST		0U
#define CNF_DMA_SETTINGS_OTE		16U
#define CNF_DMA_SETTINGS_SDMA_ER	17U

#define CNF_DMA_MASTER_SEL		1U
#define CNF_DMA_SLAVE_SEL		0U

/* DMA FIFO trigger level register field offsets */
#define CNF_FIFO_TLEVEL_POS		0U
#define CNF_FIFO_TLEVEL_DMA_SIZE	16U
#define CNF_DMA_PREFETCH_SIZE		(1024 / 8)

#define CNF_GET_CTRL_BUSY(x)		(x & (1 << 8))
#define CNF_GET_INIT_COMP(x)		(x & (1 << 9))

/* Command register0 field offsets */
#define CNF_CMDREG0_CT			30U
#define CNF_CMDREG0_TRD			24U
#define CNF_CMDREG0_INTR		20U
#define CNF_CMDREG0_DMA			21U
#define CNF_CMDREG0_VOL			16U
#define CNF_CMDREG0_CMD			0U
#define CNF_CMDREG4_MEM			24U

/* Command status register field offsets */
#define CNF_ECMD			BIT(0)
#define CNF_EECC			BIT(1)
#define CNF_EMAX			BIT(2)
#define CNF_EDEV			BIT(12)
#define CNF_EDQS			BIT(13)
#define CNF_EFAIL			BIT(14)
#define CNF_CMPLT			BIT(15)
#define CNF_EBUS			BIT(16)
#define CNF_EDI				BIT(17)
#define CNF_EPAR			BIT(18)
#define CNF_ECTX			BIT(19)
#define CNF_EPRO			BIT(20)
#define CNF_EIDX			BIT(24)

#define CNF_CMDREG_CMD_REG0		0x00
#define CNF_CMDREG_CMD_REG1		0x04
#define CNF_CMDREG_CMD_REG2		0x08
#define CNF_CMDREG_CMD_REG3		0x0C
#define CNF_CMDREG_CMD_STAT_PTR		0x10
#define CNF_CMDREG_CMD_STAT		0x14
#define CNF_CMDREG_CMD_REG4		0x20
#define CNF_CMDREG_CTRL_STATUS		0x118
#define CNF_CMDREG_TRD_STATUS		0x120

#define CNF_CMDREG(_reg)		(CNF_CMDREG_REG_BASE \
						+ (CNF_CMDREG_##_reg))

/* Controller configuration registers */
#define CNF_LSB16_MASK			0xFFFF
#define CNF_GET_NPAGES_PER_BLOCK(x)	(x & CNF_LSB16_MASK)

#define CNF_GET_SCTR_SIZE(x)		(x & CNF_LSB16_MASK)
#define CNF_GET_LAST_SCTR_SIZE(x)	((x >> 16) & CNF_LSB16_MASK)

#define CNF_GET_PAGE_SIZE(x)		(x & CNF_LSB16_MASK)
#define CNF_GET_SPARE_SIZE(x)		((x >> 16) & CNF_LSB16_MASK)

#define CNF_CTRLCFG_REG_BASE		0x10B80400
#define CNF_CTRLCFG_TRANS_CFG0		0x00
#define CNF_CTRLCFG_TRANS_CFG1		0x04
#define CNF_CTRLCFG_LONG_POLL		0x08
#define CNF_CTRLCFG_SHORT_POLL		0x0C
#define CNF_CTRLCFG_RDST_CTRL_0		0x10
#define CNF_CTRLCFG_DEV_LAYOUT		0x24
#define CNF_CTRLCFG_ECC_CFG0		0x28
#define CNF_CTRLCFG_ECC_CFG1		0x2C
#define CNF_CTRLCFG_MULTIPLANE_CFG	0x34
#define CNF_CTRLCFG_CACHE_CFG		0x38
#define CNF_CTRLCFG_DMA_SETTINGS	0x3C
#define CNF_CTRLCFG_FIFO_TLEVEL		0x54

#define CNF_ECC_EN			BIT(0)
#define CNF_ECC_ERASE_DET_EN		BIT(1)
#define CNF_ECC_CORR_STR_MASK		GENMASK(10, 8)
#define CNF_ECC_CORR_STR		0x02

#define CNF_ECC_CFG0_VAL		((CNF_ECC_EN) | \
					 (CNF_ECC_ERASE_DET_EN) | \
					 (FIELD_PREP(CNF_ECC_CORR_STR_MASK, CNF_ECC_CORR_STR)))

/* Transfer configuration 0 and 1 register settings. */
#define CNF_SECTOR_CNT			0x04
#define CNF_SECTOR_OFFSET_MASK		GENMASK(31, 16)
#define CNF_SECTOR_OFFSET		0x00

#define CNF_TRANS_CFG0_VAL		((CNF_SECTOR_CNT) | \
					 (FIELD_PREP(CNF_SECTOR_OFFSET_MASK, CNF_SECTOR_OFFSET)))

#define CNF_SECTOR_SIZE			0x0800
#define CNF_LAST_SECTOR_MASK		GENMASK(31, 16)
#define CNF_LAST_SECTOR_SIZE		0x0828

#define CNF_TRANS_CFG1_VAL		((CNF_SECTOR_SIZE) | \
					 (FIELD_PREP(CNF_LAST_SECTOR_MASK, CNF_LAST_SECTOR_SIZE)))


#define CNF_CTRLCFG(_reg)		(CNF_CTRLCFG_REG_BASE \
					+ (CNF_CTRLCFG_##_reg))

/* Data integrity registers */
#define CNF_DI_PAR_EN			0U
#define CNF_DI_CRC_EN			1U

#define CNF_DI_REG_BASE			0x10B80700
#define CNF_DI_CONTROL			0x00
#define CNF_DI_INJECT0			0x04
#define CNF_DI_INJECT1			0x08
#define CNF_DI_ERR_REG_ADDR		0x0C
#define CNF_DI_INJECT2			0x10

#define CNF_DI(_reg)			(CNF_DI_REG_BASE \
					+ (CNF_DI_##_reg))

/* Controller parameter registers */
#define CNF_NTHREADS_MASK		0x07
#define CNF_GET_NLUNS(x)		(x & 0xFF)
#define CNF_GET_DEV_TYPE(x)		((x >> 30) & 0x03)
#define CNF_GET_NTHREADS(x)		(1 << (x & CNF_NTHREADS_MASK))

#define CNF_CTRLPARAM_REG_BASE		0x10B80800
#define CNF_CTRLPARAM_VERSION		0x00
#define CNF_CTRLPARAM_FEATURE		0x04
#define CNF_CTRLPARAM_MFR_ID		0x08
#define CNF_CTRLPARAM_DEV_AREA		0x0C
#define CNF_CTRLPARAM_DEV_PARAMS0	0x10
#define CNF_CTRLPARAM_DEV_PARAMS1	0x14
#define CNF_CTRLPARAM_DEV_FEATUERS	0x18
#define CNF_CTRLPARAM_DEV_BLOCKS_PLUN	0x1C

#define CNF_MFR_ID_MASK			GENMASK(7, 0)
#define CNF_DEV_ID_MASK			GENMASK(23, 16)

#define CNF_CTRLPARAM(_reg)		(CNF_CTRLPARAM_REG_BASE \
					+ (CNF_CTRLPARAM_##_reg))

/* Protection mechanism registers */
#define CNF_PROT_REG_BASE		0x10B80900
#define CNF_PROT_CTRL0			0x00
#define CNF_PROT_DOWN0			0x04
#define CNF_PROT_UP0			0x08
#define CNF_PROT_CTRL1			0x10
#define CNF_PROT_DOWN1			0x14
#define CNF_PROT_UP1			0x18

#define CNF_PROT(_reg)			(CNF_PROT_REG_BASE \
					+ (CNF_PROT_##_reg))

/* Mini controller registers */
#define CNF_MINICTRL_REG_BASE		0x10B81000

/* Operation work modes */
#define CNF_OPR_WORK_MODE_SDR		0U
#define CNF_OPR_WORK_MODE_NVDDR		1U
#define CNF_OPR_WORK_MODE_RES		3U

/* Mini controller common settings register field offsets */
#define CNF_CMN_SETTINGS_OPR_MASK	0x00000003
#define CNF_CMN_SETTINGS_WR_WUP		20U
#define CNF_CMN_SETTINGS_RD_WUP		16U
#define CNF_CMN_SETTINGS_DEV16		8U
#define CNF_CMN_SETTINGS_OPR		0U

/* Async mode register field offsets */
#define CNF_ASYNC_TIMINGS_TRH		24U
#define CNF_ASYNC_TIMINGS_TRP		16U
#define CNF_ASYNC_TIMINGS_TWH		8U
#define CNF_ASYNC_TIMINGS_TWP		0U

/*
 * The number of clock cycles (nf_clk) the mini controller needs to
 * assert/de-assert WE# and assert/de-assert RE# signals in SDR asynchronous mode.
 */
#define CNF_ASYNC_NCLOCK_CYCLES		0x09
#define CNF_ASYNC_TOGGLE_TIMINGS_VAL	((CNF_ASYNC_NCLOCK_CYCLES << CNF_ASYNC_TIMINGS_TRH) | \
					(CNF_ASYNC_NCLOCK_CYCLES << CNF_ASYNC_TIMINGS_TRP)  | \
					(CNF_ASYNC_NCLOCK_CYCLES << CNF_ASYNC_TIMINGS_TWH)  | \
					(CNF_ASYNC_NCLOCK_CYCLES << CNF_ASYNC_TIMINGS_TWP))

/*
 * Default value from the datasheet: The number of clock cycles (nf_clk) between
 * the de-assertion of the DLL update request and resuming traffic to the PHY.
 */
#define RESYNC_IDLE_CNT			0x07
#define RESYNC_IDLE_CNT_MASK		GENMASK(7, 0)

/*
 * Default value from the datasheet: The number of clock cycles (nf_clk) for which
 * the DLL update request has to be asserted, to resynchronize the DLLs and read
 * and write FIFO pointers.
 */
#define RESYNC_HIGH_CNT			0x07
#define RESYNC_HIGH_CNT_MASK		GENMASK(11, 8)

#define CNF_DLL_PHY_RST_N		BIT(24)
#define CNF_DLL_PHY_EXT_WR_MODE		BIT(17)
#define CNF_DLL_PHY_EXT_RD_MODE		BIT(16)

#define CNF_DLL_PHY_CTRL_VAL		((CNF_DLL_PHY_RST_N) | \
					(CNF_DLL_PHY_EXT_WR_MODE) | \
					(CNF_DLL_PHY_EXT_RD_MODE) | \
					(FIELD_PREP(RESYNC_IDLE_CNT_MASK, RESYNC_IDLE_CNT)) | \
					(FIELD_PREP(RESYNC_HIGH_CNT_MASK, RESYNC_HIGH_CNT)))

/* Global timings configurations */
#define CNF_MINICTRL_TIMINGS0_VAL	0x4f631727
#define CNF_MINICTRL_TIMINGS1_VAL	0x28300063
#define CNF_MINICTRL_TIMINGS2_VAL	0x00c7030d

/* Mini controller DLL PHY controller register field offsets */
#define CNF_MINICTRL_WP_SETTINGS	0x00
#define CNF_MINICTRL_RBN_SETTINGS	0x04
#define CNF_MINICTRL_CMN_SETTINGS	0x08
#define CNF_MINICTRL_SKIP_BYTES_CFG	0x0C
#define CNF_MINICTRL_SKIP_BYTES_OFFSET	0x10
#define CNF_MINICTRL_TOGGLE_TIMINGS0	0x14
#define CNF_MINICTRL_TOGGLE_TIMINGS1	0x18
#define CNF_MINICTRL_ASYNC_TOGGLE_TIMINGS 0x1C
#define CNF_MINICTRL_SYNC_TIMINGS	0x20
#define CNF_MINICTRL_TIMINGS0		0x24
#define CNF_MINICTRL_TIMINGS1		0x28
#define CNF_MINICTRL_TIMINGS2		0x2C
#define CNF_MINICTRL_DLL_PHY_UPDATE_CNT	0x30
#define CNF_MINICTRL_DLL_PHY_CTRL	0x34

/*
 * Number of bytes to skip from offset of block.
 * The bytes are written with the value programmed in the marker field.
 */
#define CNF_NSKIP_BYTES			0x02
#define CNF_MARKER_VAL			0xFFFF
#define CNF_MARKER_MASK			GENMASK(31, 16)
#define CNF_SKIP_BYTES_CFG_VAL		((CNF_NSKIP_BYTES) | \
					(FIELD_PREP(CNF_MARKER_MASK, CNF_MARKER_VAL)))

/* Offset after which the controller starts sending the dummy bytest to the device. */
#define CNF_SKIP_BYTES_OFFSET_VAL	(0x00002000)

#define CNF_MINICTRL(_reg)		(CNF_MINICTRL_REG_BASE \
					+ (CNF_MINICTRL_##_reg))

/*
 * @brief Nand IO MTD initialization routine
 *
 * @total_size: [out] Total size of the NAND flash device
 * @erase_size: [out] Minimum erase size of the NAND flash device
 * Return: 0 on success, a negative errno on failure
 */
int cdns_nand_init_mtd(unsigned long long *total_size, unsigned int *erase_size);

/*
 * @brief Read bytes from the NAND flash device
 *
 * @offset: Byte offset to read from in device
 * @buffer: [out] Bytes read from device
 * @length: Number of bytes to read
 * @out_length: [out] Number of bytes read from device
 * Return: 0 on success, a negative errno on failure
 */
int cdns_nand_read(unsigned int offset, uintptr_t buffer, size_t length, size_t *out_length);

/*
 * @brief NAND Flash Controller/Host initialization
 * Return: 0 on success, a negative errno on failure
 */
int cdns_nand_host_init(void);

#endif
