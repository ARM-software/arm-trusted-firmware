/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* Those are parameters for xfi mode, which need to be tune for each board type.
 * For known DB boards the parameters was already calibrated and placed under
 * the plat/marvell/armada/a8k/<board_type>/board/phy-porting-layer.h
 */
struct xfi_params {
	uint8_t g1_ffe_res_sel;
	uint8_t g1_ffe_cap_sel;
	uint8_t align90;
	uint8_t g1_dfe_res;
	uint8_t g1_amp;
	uint8_t g1_emph;
	uint8_t g1_emph_en;
	uint8_t g1_tx_amp_adj;
	uint8_t g1_tx_emph_en;
	uint8_t g1_tx_emph;
	uint8_t g1_rx_selmuff;
	uint8_t g1_rx_selmufi;
	uint8_t g1_rx_selmupf;
	uint8_t g1_rx_selmupi;
	_Bool valid;
};

struct sata_params {
	uint8_t g1_amp;
	uint8_t g2_amp;
	uint8_t g3_amp;

	uint8_t g1_emph;
	uint8_t g2_emph;
	uint8_t g3_emph;

	uint8_t g1_emph_en;
	uint8_t g2_emph_en;
	uint8_t g3_emph_en;

	uint8_t g1_tx_amp_adj;
	uint8_t g2_tx_amp_adj;
	uint8_t g3_tx_amp_adj;

	uint8_t g1_tx_emph_en;
	uint8_t g2_tx_emph_en;
	uint8_t g3_tx_emph_en;

	uint8_t g1_tx_emph;
	uint8_t g2_tx_emph;
	uint8_t g3_tx_emph;

	uint8_t g3_dfe_res;

	uint8_t g3_ffe_res_sel;

	uint8_t g3_ffe_cap_sel;

	uint8_t align90;

	uint8_t g1_rx_selmuff;
	uint8_t g2_rx_selmuff;
	uint8_t g3_rx_selmuff;

	uint8_t g1_rx_selmufi;
	uint8_t g2_rx_selmufi;
	uint8_t g3_rx_selmufi;

	uint8_t g1_rx_selmupf;
	uint8_t g2_rx_selmupf;
	uint8_t g3_rx_selmupf;

	uint8_t g1_rx_selmupi;
	uint8_t g2_rx_selmupi;
	uint8_t g3_rx_selmupi;

	uint8_t polarity_invert;

	_Bool valid;
};

struct usb_params {
	uint8_t polarity_invert;
};

int mvebu_cp110_comphy_is_pll_locked(uint64_t comphy_base,
				     uint8_t comphy_index);
int mvebu_cp110_comphy_power_off(uint64_t comphy_base,
				 uint8_t comphy_index, uint64_t comphy_mode);
int mvebu_cp110_comphy_power_on(uint64_t comphy_base,
				uint8_t comphy_index, uint64_t comphy_mode);
int mvebu_cp110_comphy_xfi_rx_training(uint64_t comphy_base,
				       uint8_t comphy_index);
int mvebu_cp110_comphy_digital_reset(uint64_t comphy_base, uint8_t comphy_index,
				     uint32_t comphy_mode, uint32_t command);

#define COMPHY_POLARITY_NO_INVERT	0
#define COMPHY_POLARITY_TXD_INVERT	1
#define COMPHY_POLARITY_RXD_INVERT	2
