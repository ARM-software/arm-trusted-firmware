/*
 * Copyright (c) 2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DRIVERS_RAW_NAND_H
#define DRIVERS_RAW_NAND_H

#include <stdint.h>

#include <drivers/nand.h>

/* NAND ONFI default value mode 0 in picosecond */
#define NAND_TADL_MIN			400000UL
#define NAND_TALH_MIN			20000UL
#define NAND_TALS_MIN			50000UL
#define NAND_TAR_MIN			25000UL
#define NAND_TCCS_MIN			500000UL
#define NAND_TCEA_MIN			100000UL
#define NAND_TCEH_MIN			20000UL
#define NAND_TCH_MIN			20000UL
#define NAND_TCHZ_MAX			100000UL
#define NAND_TCLH_MIN			20000UL
#define NAND_TCLR_MIN			20000UL
#define NAND_TCLS_MIN			50000UL
#define NAND_TCOH_MIN			0UL
#define NAND_TCS_MIN			70000UL
#define NAND_TDH_MIN			20000UL
#define NAND_TDS_MIN			40000UL
#define NAND_TFEAT_MAX			1000000UL
#define NAND_TIR_MIN			10000UL
#define NAND_TITC_MIN			1000000UL
#define NAND_TR_MAX			200000000UL
#define NAND_TRC_MIN			100000UL
#define NAND_TREA_MAX			40000UL
#define NAND_TREH_MIN			30000UL
#define NAND_TRHOH_MIN			0UL
#define NAND_TRHW_MIN			200000UL
#define NAND_TRHZ_MAX			200000UL
#define NAND_TRLOH_MIN			0UL
#define NAND_TRP_MIN			50000UL
#define NAND_TRR_MIN			40000UL
#define NAND_TRST_MAX			250000000000ULL
#define NAND_TWB_MAX			200000UL
#define NAND_TWC_MIN			100000UL
#define NAND_TWH_MIN			30000UL
#define NAND_TWHR_MIN			120000UL
#define NAND_TWP_MIN			50000UL
#define NAND_TWW_MIN			100000UL

/* NAND request types */
#define NAND_REQ_CMD			0x0000U
#define NAND_REQ_ADDR			0x1000U
#define NAND_REQ_DATAIN			0x2000U
#define NAND_REQ_DATAOUT		0x3000U
#define NAND_REQ_WAIT			0x4000U
#define NAND_REQ_MASK			GENMASK(14, 12)
#define NAND_REQ_BUS_WIDTH_8		BIT(15)

#define PARAM_PAGE_SIZE			256

/* NAND ONFI commands */
#define NAND_CMD_READ_1ST		0x00U
#define NAND_CMD_CHANGE_1ST		0x05U
#define NAND_CMD_READID_SIG_ADDR	0x20U
#define NAND_CMD_READ_2ND		0x30U
#define NAND_CMD_STATUS			0x70U
#define NAND_CMD_READID			0x90U
#define NAND_CMD_CHANGE_2ND		0xE0U
#define NAND_CMD_READ_PARAM_PAGE	0xECU
#define NAND_CMD_RESET			0xFFU

#define ONFI_REV_21			BIT(3)
#define ONFI_FEAT_BUS_WIDTH_16		BIT(0)
#define ONFI_FEAT_EXTENDED_PARAM	BIT(7)

/* NAND ECC type */
#define NAND_ECC_NONE			U(0)
#define NAND_ECC_HW			U(1)
#define NAND_ECC_ONDIE			U(2)

/* NAND bus width */
#define NAND_BUS_WIDTH_8		U(0)
#define NAND_BUS_WIDTH_16		U(1)

struct nand_req {
	struct nand_device *nand;
	uint16_t type;
	uint8_t *addr;
	unsigned int length;
	unsigned int delay_ms;
	unsigned int inst_delay;
};

struct nand_param_page {
	/* Rev information and feature block */
	uint32_t page_sig;
	uint16_t rev;
	uint16_t features;
	uint16_t opt_cmd;
	uint8_t jtg;
	uint8_t train_cmd;
	uint16_t ext_param_length;
	uint8_t nb_param_pages;
	uint8_t reserved1[17];
	/* Manufacturer information */
	uint8_t manufacturer[12];
	uint8_t model[20];
	uint8_t manufacturer_id;
	uint16_t data_code;
	uint8_t reserved2[13];
	/* Memory organization */
	uint32_t bytes_per_page;
	uint16_t spare_per_page;
	uint32_t bytes_per_partial;
	uint16_t spare_per_partial;
	uint32_t num_pages_per_blk;
	uint32_t num_blk_in_lun;
	uint8_t num_lun;
	uint8_t num_addr_cycles;
	uint8_t bit_per_cell;
	uint16_t max_bb_per_lun;
	uint16_t blk_endur;
	uint8_t valid_blk_begin;
	uint16_t blk_enbur_valid;
	uint8_t nb_prog_page;
	uint8_t partial_prog_attr;
	uint8_t nb_ecc_bits;
	uint8_t plane_addr;
	uint8_t mplanes_ops;
	uint8_t ez_nand;
	uint8_t reserved3[12];
	/* Electrical parameters */
	uint8_t io_pin_cap_max;
	uint16_t sdr_timing_mode;
	uint16_t sdr_prog_cache_timing;
	uint16_t tprog;
	uint16_t tbers;
	uint16_t tr;
	uint16_t tccs;
	uint8_t nvddr_timing_mode;
	uint8_t nvddr2_timing_mode;
	uint8_t nvddr_features;
	uint16_t clk_input_cap_typ;
	uint16_t io_pin_cap_typ;
	uint16_t input_pin_cap_typ;
	uint8_t input_pin_cap_max;
	uint8_t drv_strength_support;
	uint16_t tr_max;
	uint16_t tadl;
	uint16_t tr_typ;
	uint8_t reserved4[6];
	/* Vendor block */
	uint16_t vendor_revision;
	uint8_t vendor[88];
	uint16_t crc16;
} __packed;

struct nand_ctrl_ops {
	int (*exec)(struct nand_req *req);
	void (*setup)(struct nand_device *nand);
};

struct rawnand_device {
	struct nand_device *nand_dev;
	const struct nand_ctrl_ops *ops;
};

int nand_raw_init(unsigned long long *size, unsigned int *erase_size);
int nand_wait_ready(unsigned long delay);
int nand_read_page_cmd(unsigned int page, unsigned int offset,
		       uintptr_t buffer, unsigned int len);
int nand_change_read_column_cmd(unsigned int offset, uintptr_t buffer,
				unsigned int len);
void nand_raw_ctrl_init(const struct nand_ctrl_ops *ops);

/*
 * Platform can implement this to override default raw NAND instance
 * configuration.
 *
 * @device: target raw NAND instance.
 * Return 0 on success, negative value otherwise.
 */
int plat_get_raw_nand_data(struct rawnand_device *device);

#endif	/* DRIVERS_RAW_NAND_H */
