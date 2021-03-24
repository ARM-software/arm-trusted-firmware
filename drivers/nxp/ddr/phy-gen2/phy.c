/*
 * Copyright 2021 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/debug.h>
#include "csr.h"
#include <ddr.h>
#include "ddr4fw.h"
#include <drivers/delay_timer.h>
#ifdef NXP_WARM_BOOT
#include <fspi_api.h>
#endif
#include "input.h"
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#ifdef DDR_PHY_DEBUG
#include "messages.h"
#endif
#ifdef NXP_WARM_BOOT
#include "phy.h"
#endif
#include "pie.h"

#define TIMEOUTDEFAULT 500
#define MAP_PHY_ADDR(pstate, n, instance, offset, c) \
		((((pstate * n) + instance + c) << 12) + offset)

static uint32_t map_phy_addr_space(uint32_t addr)
{
	/* 23 bit addressing */
	uint32_t pstate =     (addr & U(0x700000)) >> 20U; /* bit 22:20 */
	uint32_t block_type = (addr & U(0x0f0000)) >> 16U; /* bit 19:16 */
	uint32_t instance =   (addr & U(0x00f000)) >> 12U; /* bit 15:12 */
	uint32_t offset =     (addr & U(0x000fff));        /* bit 11:0 */

	switch (block_type) {
	case 0x0: /* 0x0 : ANIB */
		return MAP_PHY_ADDR(pstate, 12, instance, offset, 0);
	case 0x1: /* 0x1 : DBYTE */
		return MAP_PHY_ADDR(pstate, 10, instance, offset, 0x30);
	case 0x2: /* 0x2 : MASTER */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x58);
	case 0x4: /* 0x4 : ACSM */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x5c);
	case 0x5: /* 0x5 : μCTL Memory */
		return MAP_PHY_ADDR(pstate, 0, instance, offset, 0x60);
	case 0x7: /* 0x7 : PPGC */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x68);
	case 0x9: /* 0x9 : INITENG */
		return MAP_PHY_ADDR(pstate, 1, 0, offset, 0x69);
	case 0xc: /* 0xC : DRTUB */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6d);
	case 0xd: /* 0xD : APB Only */
		return MAP_PHY_ADDR(pstate, 0, 0, offset, 0x6e);
	default:
		printf("ERR: Invalid block_type = 0x%x\n", block_type);
		return 0;
	}
}

static inline uint16_t *phy_io_addr(void *phy, uint32_t addr)
{
	return phy + (map_phy_addr_space(addr) << 2);
}

static inline void phy_io_write16(uint16_t *phy, uint32_t addr, uint16_t data)
{
	mmio_write_16((uintptr_t)phy_io_addr(phy, addr), data);
#ifdef DEBUG_PHY_IO
	printf("0x%06x,0x%x\n", addr, data);
#endif
}

static inline uint16_t phy_io_read16(uint16_t *phy, uint32_t addr)
{
	uint16_t reg = mmio_read_16((uintptr_t) phy_io_addr(phy, addr));

#ifdef DEBUG_PHY_IO
	printf("R: 0x%06x,0x%x\n", addr, reg);
#endif

	return reg;
}

#ifdef NXP_APPLY_MAX_CDD

#define CDD_VAL_READ_ADDR (0x054012)
#define CDD_DATA_LEN    (60)

static void read_phy_reg(uint16_t *phy, uint32_t addr,
		uint16_t *buf, uint32_t len)
{
	uint32_t i = 0U;

	for (i = 0U; i < len/2; i++) {
		buf[i] = phy_io_read16(phy, (addr + i));
	}
}

static uint32_t findrank(uint32_t cs_in_use)
{
	uint32_t val = 0U;

	switch (cs_in_use) {
	case U(0xf):
		val = 4U;
		break;
	case U(0x3):
		val = 2U;
		break;
	case U(0x1):
		val = 1U;
		break;
	default:
		printf("Error - Invalid cs_in_use value\n");
	}
	return val;
}

static uint8_t findmax(uint8_t *buf, uint32_t len)
{
	uint8_t max = 0U;
	uint32_t i = 0U;

	for (i = 0U; i < len; i++) {
		if (buf[i] > max) {
			max = buf[i];
		}
	}

	return max;
}

static void get_cdd_val(uint16_t **phy_ptr, uint32_t rank, uint32_t freq,
		uint32_t *tcfg0, uint32_t *tcfg4)
{
	uint8_t cdd[CDD_DATA_LEN+4] = {0U};
	uint32_t i, val = 0U;
	uint16_t *phy;
	uint8_t buf[16] = {U(0x0)};
	uint8_t trr = 0U, tww = 0U, trw = 0U, twr = 0U;
	uint8_t rrmax = 0U, wwmax = 0U, rwmax = 0U, wrmax = 0U;
	uint8_t tmp = U(0x0);
	uint8_t *c =  NULL;

	for (i = 0U; i < NUM_OF_DDRC; i++) {

		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}

		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, U(0x0));

		read_phy_reg(phy, CDD_VAL_READ_ADDR,
				(uint16_t *)&cdd, CDD_DATA_LEN);

		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, U(0x1));

	/* CDD values and address
	 *
	 *   0x054012    0x24    cdd[0]  CDD[X][X]
	 *   0x054012    0x25    cdd[1]  RR[3][2]
	 *   0x054013    0x26    cdd[2]  RR[3][1]
	 *   0x054013    0x27    cdd[3]  RR[3][0]
	 *   0x054014    0x28    cdd[4]  RR[2][3]
	 *   0x054014    0x29    cdd[5]  RR[2][1]
	 *   0x054015    0x2a    cdd[6]  RR[2][0]
	 *   0x054015    0x2b    cdd[7]  RR[1][3]
	 *   0x054016    0x2c    cdd[8]  RR[1][2]
	 *   0x054016    0x2d    cdd[9]  RR[1][0]
	 *   0x054017    0x2e    cdd[10] RR[0][3]
	 *   0x054017    0x2f    cdd[11] RR[0][2]
	 *   0x054018    0x30    cdd[12] RR[0][1]

	 *   0x054018    0x31    cdd[13] WW[3][2]
	 *   0x054019    0x32    cdd[14] WW[3][1]
	 *   0x054019    0x33    cdd[15] WW[3][0]
	 *   0x05401a    0x34    cdd[16] WW[2][3]
	 *   0x05401a    0x35    cdd[17] WW[2][1]
	 *   0x05401b    0x36    cdd[18] WW[2][0]
	 *   0x05401b    0x37    cdd[19] WW[1][3]
	 *   0x05401c    0x38    cdd[20] WW[1][2]
	 *   0x05401c    0x39    cdd[21] WW[1][0]
	 *   0x05401d    0x3a    cdd[22] WW[0][3]
	 *   0x05401d    0x3b    cdd[23] WW[0][2]
	 *   0x05401e    0x3c    cdd[24] WW[0][1]

	 *   0x05401e    0x3d    cdd[25] RW[3][3]
	 *   0x05401f    0x3e    cdd[26] RW[3][2]
	 *   0x05401f    0x3f    cdd[27] RW[3][1]
	 *   0x054020    0x40    cdd[28] RW[3][0]
	 *   0x054020    0x41    cdd[29] RW[2][3]
	 *   0x054021    0x42    cdd[30] RW[2][2]
	 *   0x054021    0x43    cdd[31] RW[2][1]
	 *   0x054022    0x44    cdd[32] RW[2][0]
	 *   0x054022    0x45    cdd[33] RW[1][3]
	 *   0x054023    0x46    cdd[34] RW[1][2]
	 *   0x054023    0x47    cdd[35] RW[1][1]
	 *   0x054024    0x48    cdd[36] RW[1][0]
	 *   0x054024    0x49    cdd[37] RW[0][3]
	 *   0x054025    0x4a    cdd[38] RW[0][2]
	 *   0x054025    0x4b    cdd[39] RW[0][1]
	 *   0x054026    0x4c    cdd[40] RW[0][0]

	 *   0x054026    0x4d    cdd[41] WR[3][3]
	 *   0x054027    0x4e    cdd[42] WR[3][2]
	 *   0x054027    0x4f    cdd[43] WR[3][1]
	 *   0x054028    0x50    cdd[44] WR[3][0]
	 *   0x054028    0x51    cdd[45] WR[2][3]
	 *   0x054029    0x52    cdd[46] WR[2][2]
	 *   0x054029    0x53    cdd[47] WR[2][1]
	 *   0x05402a    0x54    cdd[48] WR[2][0]
	 *   0x05402a    0x55    cdd[49] WR[1][3]
	 *   0x05402b    0x56    cdd[50] WR[1][2]
	 *   0x05402b    0x57    cdd[51] WR[1][1]
	 *   0x05402c    0x58    cdd[52] WR[1][0]
	 *   0x05402c    0x59    cdd[53] WR[0][3]
	 *   0x05402d    0x5a    cdd[54] WR[0][2]
	 *   0x05402d    0x5b    cdd[55] WR[0][1]
	 *   0x05402e    0x5c    cdd[56] WR[0][0]
	 *   0x05402e    0x5d    cdd[57] CDD[Y][Y]
	 */

		switch (rank) {
		case 1U:
			tmp = rwmax;
			rwmax = cdd[40];
			if (tmp > rwmax) {
				rwmax = tmp;
			}

			tmp = wrmax;
			wrmax = cdd[56];
			if (tmp > wrmax) {
				wrmax = tmp;
			}

			break;

		case 2U:
			buf[0] = cdd[12];
			buf[1] = cdd[9];
			tmp = rrmax;
			rrmax = findmax(buf, 2U);
			if (tmp > rrmax) {
				rrmax = tmp;
			}

			buf[0] = cdd[24];
			buf[1] = cdd[21];
			tmp = wwmax;
			wwmax = findmax(buf, 2U);
			if (tmp > wwmax) {
				wwmax = tmp;
			}

			buf[0] = cdd[40];
			buf[1] = cdd[39];
			buf[2] = cdd[36];
			buf[3] = cdd[35];
			tmp = rwmax;
			rwmax = findmax(buf, 4U);
			if (tmp > rwmax) {
				rwmax = tmp;
			}

			buf[0] = cdd[56];
			buf[1] = cdd[55];
			buf[2] = cdd[52];
			buf[3] = cdd[51];
			tmp = wrmax;
			wrmax = findmax(buf, 4U);
			if (tmp > wrmax) {
				wrmax = tmp;
			}

			break;

		case 4U:
			tmp = rrmax;
			c = &cdd[1];
			rrmax = findmax(c, 12U);
			if (tmp > rrmax) {
				rrmax = tmp;
			}

			tmp = wwmax;
			c = &cdd[13];
			wwmax = findmax(c, 12U);
			if (tmp > wwmax) {
				wwmax = tmp;
			}

			tmp = rwmax;
			c = &cdd[25];
			rwmax = findmax(c, 16U);
			if (tmp > rwmax) {
				rwmax = tmp;
			}

			tmp = wrmax;
			c = &cdd[41];
			wrmax = findmax(c, 16U);
			if (tmp > wrmax) {
				wrmax = tmp;
			}

			break;

		}
	}

	rrmax += 3U;
	wwmax += 4U;

	if (wwmax > 7U) {
		wwmax = 7U;
	}

	if (rrmax > 7U) {
		rrmax = 7U;
	}

	if (wrmax > U(0xf)) {
		wrmax = 0U;
	}

	if (rwmax > U(0x7)) {
		rwmax = U(0x7);
	}

	val = *tcfg0;
	tww = (val >> 24U) & U(0x3);
	trr = (val >> 26U) & U(0x3);
	twr = (val >> 28U) & U(0x3);
	trw = (val >> 30U) & U(0x3);

	val = *tcfg4;
	tww = tww | (((val >> 8U) & U(0x1)) << 2U);
	trr = trr | (((val >> 10U) & U(0x1)) << 2U);
	twr = twr | (((val >> 12U) & U(0x1)) << 2U);
	trw = trw | (((val >> 14U) & U(0x3)) << 2U);

	if (trr > rrmax) {
		rrmax = trr;
	}

	if (tww > wwmax) {
		wwmax = tww;
	}

	if (trw > rwmax) {
		rwmax = trw;
	}

	if (twr > wrmax) {
		wrmax = twr;
	}

	debug("CDD rrmax %x wwmax %x rwmax %x wrmax %x\n",
			rrmax, wwmax, rwmax, wrmax);

	val = ((wwmax & U(0x3)) << 24U)
		| ((rrmax & U(0x3)) << 26U)
		| ((wrmax & U(0x3)) << 28U)
		| ((rwmax & U(0x3)) << 30U);

	*tcfg0 = (*tcfg0 & U(0x00FFFFFF)) | (val);

	val = (((wwmax >> 2U) & U(0x1)) << 8U)
		| (((rrmax >> 2U) & U(0x1)) << 10U)
		| (((wrmax >> 2U) & U(0x1)) << 12U)
		| (((rwmax >> 2U) & U(0x3)) << 14U);

	*tcfg4 = (*tcfg4 & U(0xffff00ff)) | val;
}
#endif

#ifdef NXP_WARM_BOOT
int save_phy_training_values(uint16_t **phy_ptr, uint32_t address_to_store,
		uint32_t num_of_phy, int train2d)
{
	uint16_t *phy = NULL, value = 0x0;
	uint32_t size = 1U, num_of_regs = 1U, phy_store = 0U;
	int i = 0, j = 0, ret = -EINVAL;

	ret = xspi_sector_erase(address_to_store, PHY_ERASE_SIZE);
	if (ret != 0) {
		return -EINVAL;
	}

	for (j = 0; j < num_of_phy; j++) {
		/* Save training values of all PHYs */
		phy = phy_ptr[j];
		size = sizeof(training_1D_values);
		num_of_regs = ARRAY_SIZE(training_1D_values);

		/* Enable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x0);
		/* Enable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x3);
		if (train2d != 0) {
		/* Address to store training values is
		 * to be appended for next PHY
		 */
			phy_store = address_to_store + (j *
					(sizeof(training_1D_values) +
					 sizeof(training_2D_values)));
		} else {
			phy_store = address_to_store + (j *
					(sizeof(training_1D_values)));
		}
		debug("Saving 1D Training reg val at: %d\n", phy_store);
		for (i = 0; i < num_of_regs; i++) {
			value = phy_io_read16(phy, training_1D_values[i].addr);
#ifdef DEBUG_WARM_RESET
			debug("%d. Reg: %x, value: %x PHY: %p\n", i,
					training_1D_values[i].addr, value,
					phy_io_addr(phy,
						training_1D_values[i].addr));
#endif
			training_1D_values[i].data = value;
		}
		/* Storing 1D training values on flash */
		ret = xspi_write(phy_store, (void *)training_1D_values, size);
		if (train2d != 0) {
			phy_store = phy_store+size;
			size = sizeof(training_2D_values);
			num_of_regs = ARRAY_SIZE(training_2D_values);
			debug("Saving 2D Training reg val at:%d\n", phy_store);
			for (i = 0; i < num_of_regs; i++) {
				value = phy_io_read16(phy,
						training_2D_values[i].addr);
				training_2D_values[i].data = value;
#ifdef DEBUG_WARM_RESET
				debug("%d.2D addr:0x%x,val:0x%x,PHY:0x%p\n",
						i, training_2D_values[i].addr,
						value, phy_io_addr(phy,
						training_2D_values[i].addr));
#endif
			}
			/* Storing 2D training values on flash */
			ret = xspi_write(phy_store, training_2D_values,
					size);
		}
		/* Disable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x0);
		/* Disable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x1);
	}
	if (ret != 0) {
		return -EINVAL;
	}

	return 0;
}

int restore_phy_training_values(uint16_t **phy_ptr, uint32_t address_to_restore,
		uint32_t num_of_phy, int train2d)
{
	uint16_t *phy = NULL;
	uint32_t size = 1U, num_of_regs = 1U, phy_store = 0U;
	int i = 0, j = 0, ret = -EINVAL;

	debug("Restoring Training register values\n");
	for (j = 0; j < num_of_phy; j++) {
		phy = phy_ptr[j];
		size = sizeof(training_1D_values);
		num_of_regs = ARRAY_SIZE(training_1D_values);
		if (train2d != 0) {
		/* The address to restore training values is
		 * to be appended for next PHY
		 */
			phy_store = address_to_restore + (j *
					(sizeof(training_1D_values) +
					 sizeof(training_2D_values)));
		} else {
			phy_store = address_to_restore + (j *
					(sizeof(training_1D_values)));
		}
		/* Enable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x0);
		/* Enable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x3);

		/* Reading 1D training values from flash*/
		ret = xspi_read(phy_store, (uint32_t *)training_1D_values,
				size);
		debug("Restoring 1D Training reg val at:%08x\n", phy_store);
		for (i = 0; i < num_of_regs; i++) {
			phy_io_write16(phy, training_1D_values[i].addr,
					training_1D_values[i].data);
#ifdef DEBUG_WARM_RESET
			debug("%d. Reg: %x, value: %x PHY: %p\n", i,
					training_1D_values[i].addr,
					training_1D_values[i].data,
					phy_io_addr(phy,
						training_1D_values[i].addr));
#endif
		}
		if (train2d != 0) {
			phy_store = phy_store + size;
			size = sizeof(training_2D_values);
			num_of_regs = ARRAY_SIZE(training_2D_values);
			/* Reading 2D training values from flash */
			ret = xspi_read(phy_store,
					(uint32_t *)training_2D_values,	size);
			debug("Restoring 2D Training reg val at:%08x\n",
					phy_store);
			for (i = 0; i < num_of_regs; i++) {
				phy_io_write16(phy, training_2D_values[i].addr,
						training_2D_values[i].data);
#ifdef DEBUG_WARM_RESET
				debug("%d. Reg: %x, value: %x PHY: %p\n", i,
						training_2D_values[i].addr,
						training_2D_values[i].data,
						phy_io_addr(phy,
						training_1D_values[i].addr));
#endif
			}
		}
		/* Disable clocks in case they were disabled. */
		phy_io_write16(phy, t_drtub |
				csr_ucclk_hclk_enables_addr, 0x0);
		/* Disable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly |
				csr_micro_cont_mux_sel_addr, 0x1);
	}
	if (ret != 0) {
		return -EINVAL;
	}
	return 0;
}
#endif

static void load_pieimage(uint16_t *phy,
			  enum dimm_types dimm_type)
{
	int i;
	int size;
	const struct pie *image = NULL;

	switch (dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		image = pie_udimm;
		size = ARRAY_SIZE(pie_udimm);
		break;
	case RDIMM:
		image = pie_rdimm;
		size = ARRAY_SIZE(pie_rdimm);
		break;
	case LRDIMM:
		image = pie_lrdimm;
		size = ARRAY_SIZE(pie_lrdimm);
		break;
	default:
		printf("Unsupported DIMM type\n");
		break;
	}

	if (image != NULL) {
		for (i = 0; i < size; i++)
			phy_io_write16(phy, image[i].addr, image[i].data);
	}
}

static void prog_acsm_playback(uint16_t *phy,
			       const struct input *input, const void *msg)
{
	int vec;
	const struct ddr4r1d *msg_blk;
	uint16_t acsmplayback[2][3];
	uint32_t f0rc0a;
	uint32_t f0rc3x;
	uint32_t f0rc5x;

	if (input->basic.dimm_type != RDIMM) {
		return;
	}

	msg_blk = msg;
	f0rc0a = (msg_blk->f0rc0a_d0 & U(0xf)) | U(0xa0);
	f0rc3x = (msg_blk->f0rc3x_d0 & U(0xff)) | U(0x300);
	f0rc5x = (input->adv.phy_gen2_umctl_f0rc5x & U(0xff)) | U(0x500);

	acsmplayback[0][0] = U(0x3ff) & f0rc0a;
	acsmplayback[1][0] = (U(0x1c00) & f0rc0a) >> 10U;
	acsmplayback[0][1] = U(0x3ff) & f0rc3x;
	acsmplayback[1][1] = (U(0x1c00) & f0rc3x) >> 10U;
	acsmplayback[0][2] = U(0x3ff) & f0rc5x;
	acsmplayback[1][2] = (U(0x1c00) & f0rc5x) >> 10U;
	for (vec = 0; vec < 3; vec++) {
		phy_io_write16(phy, t_acsm | (csr_acsm_playback0x0_addr +
			       (vec << 1)), acsmplayback[0][vec]);
		phy_io_write16(phy, t_acsm | (csr_acsm_playback1x0_addr +
			       (vec << 1)), acsmplayback[1][vec]);
	}
}

static void prog_acsm_ctr(uint16_t *phy,
			  const struct input *input)
{
	if (input->basic.dimm_type != RDIMM) {
		return;
	}

	phy_io_write16(phy, t_acsm | csr_acsm_ctrl13_addr,
		       0xf << csr_acsm_cke_enb_lsb);

	phy_io_write16(phy, t_acsm | csr_acsm_ctrl0_addr,
		       csr_acsm_par_mode_mask | csr_acsm_2t_mode_mask);
}

static void prog_cal_rate_run(uint16_t *phy,
			  const struct input *input)
{
	int cal_rate;
	int cal_interval;
	int cal_once;
	uint32_t addr;

	cal_interval = input->adv.cal_interval;
	cal_once = input->adv.cal_once;
	cal_rate = 0x1 << csr_cal_run_lsb		|
			cal_once << csr_cal_once_lsb	|
			cal_interval << csr_cal_interval_lsb;
	addr = t_master | csr_cal_rate_addr;
	phy_io_write16(phy, addr, cal_rate);
}

static void prog_seq0bdly0(uint16_t *phy,
		    const struct input *input)
{
	int ps_count[4];
	int frq;
	uint32_t addr;
	int lower_freq_opt = 0;

	__unused const soc_info_t *soc_info;

	frq = input->basic.frequency >> 1;
	ps_count[0] = frq >> 3; /* 0.5 * frq / 4*/
	if (input->basic.frequency < 400) {
		lower_freq_opt = (input->basic.dimm_type == RDIMM) ? 7 : 3;
	} else if (input->basic.frequency < 533) {
		lower_freq_opt = (input->basic.dimm_type == RDIMM) ? 14 : 11;
	}

	/* 1.0 * frq / 4 - lower_freq */
	ps_count[1] = (frq >> 2) - lower_freq_opt;
	ps_count[2] = (frq << 1) +  (frq >> 1); /* 10.0 * frq / 4 */

#ifdef DDR_PLL_FIX
	soc_info = get_soc_info();
	if (soc_info->maj_ver == 1) {
		ps_count[0] = 0x520; /* seq0bdly0 */
		ps_count[1] = 0xa41; /* seq0bdly1 */
		ps_count[2] = 0x668a; /* seq0bdly2 */
	}
#endif
	if (frq > 266) {
		ps_count[3] = 44;
	} else if (frq > 200) {
		ps_count[3] = 33;
	} else {
		ps_count[3] = 16;
	}

	addr = t_master | csr_seq0bdly0_addr;
	phy_io_write16(phy, addr, ps_count[0]);

	debug("seq0bdly0 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly1_addr;
	phy_io_write16(phy, addr, ps_count[1]);

	debug("seq0bdly1 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly2_addr;
	phy_io_write16(phy, addr, ps_count[2]);

	debug("seq0bdly2 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_seq0bdly3_addr;
	phy_io_write16(phy, addr, ps_count[3]);

	debug("seq0bdly3 = 0x%x\n", phy_io_read16(phy, addr));
}

/* Only RDIMM requires msg_blk */
static void i_load_pie(uint16_t **phy_ptr,
		       const struct input *input,
		       const void *msg)
{
	int i;
	uint16_t *phy;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}

		phy_io_write16(phy,
			       t_apbonly | csr_micro_cont_mux_sel_addr,
			       0U);

		load_pieimage(phy, input->basic.dimm_type);

		prog_seq0bdly0(phy, input);
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag0_addr,
			       U(0x0000));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag1_addr,
			       U(0x0173));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag2_addr,
			       U(0x0060));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag3_addr,
			       U(0x6110));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag4_addr,
			       U(0x2152));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag5_addr,
			       U(0xdfbd));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag6_addr,
			       input->basic.dimm_type == RDIMM &&
			       input->adv.phy_gen2_umctl_opt == 1U ?
			       U(0x6000) : U(0xffff));
		phy_io_write16(phy, t_initeng | csr_seq0bdisable_flag7_addr,
			       U(0x6152));
		prog_acsm_playback(phy, input, msg);		/* rdimm */
		prog_acsm_ctr(phy, input);			/* rdimm */

		phy_io_write16(phy, t_master | csr_cal_zap_addr, U(0x1));
		prog_cal_rate_run(phy, input);

		phy_io_write16(phy, t_drtub | csr_ucclk_hclk_enables_addr,
			       input->basic.dimm_type == RDIMM ? U(0x2) : 0U);

		phy_io_write16(phy, t_apbonly | csr_micro_cont_mux_sel_addr, 1U);
	}
}

static void phy_gen2_init_input(struct input *input)
{
	int i;

	input->adv.dram_byte_swap		= 0;
	input->adv.ext_cal_res_val		= 0;
	input->adv.tx_slew_rise_dq		= 0xf;
	input->adv.tx_slew_fall_dq		= 0xf;
	input->adv.tx_slew_rise_ac		= 0xf;
	input->adv.tx_slew_fall_ac		= 0xf;
	input->adv.mem_alert_en			= 0;
	input->adv.mem_alert_puimp		= 5;
	input->adv.mem_alert_vref_level		= 0x29;
	input->adv.mem_alert_sync_bypass	= 0;
	input->adv.cal_interval			= 0x9;
	input->adv.cal_once			= 0;
	input->adv.dis_dyn_adr_tri		= 0;
	input->adv.is2ttiming			= 0;
	input->adv.d4rx_preamble_length		= 0;
	input->adv.d4tx_preamble_length		= 0;

	for (i = 0; i < 7; i++) {
		debug("mr[%d] = 0x%x\n", i, input->mr[i]);
	}

	debug("input->cs_d0 = 0x%x\n", input->cs_d0);
	debug("input->cs_d1 = 0x%x\n", input->cs_d1);
	debug("input->mirror = 0x%x\n", input->mirror);
	debug("PHY ODT impedance = %d ohm\n", input->adv.odtimpedance);
	debug("PHY DQ driver impedance = %d ohm\n", input->adv.tx_impedance);
	debug("PHY Addr driver impedance = %d ohm\n", input->adv.atx_impedance);

	for (i = 0; i < 4; i++) {
		debug("odt[%d] = 0x%x\n", i, input->odt[i]);
	}

	if (input->basic.dimm_type == RDIMM) {
		for (i = 0; i < 16; i++) {
			debug("input->rcw[%d] = 0x%x\n", i, input->rcw[i]);
		}
		debug("input->rcw3x = 0x%x\n", input->rcw3x);
	}
}

/*
 * All protocols share the same base structure of message block.
 * RDIMM and LRDIMM have more entries defined than UDIMM.
 * Create message blocks for 1D and 2D training.
 * Update len with message block size.
 */
static int phy_gen2_msg_init(void *msg_1d,
			     void *msg_2d,
			     const struct input *input)
{
	struct ddr4u1d *msg_blk = msg_1d;
	struct ddr4u2d *msg_blk_2d = msg_2d;
	struct ddr4r1d *msg_blk_r;
	struct ddr4lr1d *msg_blk_lr;

	switch (input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		msg_blk->dram_type	= U(0x2);
		break;
	case RDIMM:
		msg_blk->dram_type	= U(0x4);
		break;
	case LRDIMM:
		msg_blk->dram_type	= U(0x5);
		break;
	default:
		ERROR("Unsupported DIMM type\n");
		return -EINVAL;
	}
	msg_blk->pstate			= 0U;

	/*Enable quickRd2D, a substage of read deskew, to 1D training.*/
	msg_blk->reserved00             = U(0x20);

	/*Enable High-Effort WrDQ1D.*/
	msg_blk->reserved00             |= U(0x40);

	/* Enable 1D extra effort training.*/
	msg_blk->reserved1c[3]		= U(0x3);

	if (input->basic.dimm_type == LRDIMM) {
		msg_blk->sequence_ctrl	= U(0x3f1f);
	} else {
		msg_blk->sequence_ctrl	= U(0x031f);
	}
	msg_blk->phy_config_override	= 0U;
#ifdef DDR_PHY_DEBUG
	msg_blk->hdt_ctrl		= U(0x5);
#else
	msg_blk->hdt_ctrl		= U(0xc9);
#endif
	msg_blk->msg_misc		= U(0x0);
	msg_blk->dfimrlmargin		= U(0x1);
	msg_blk->phy_vref		= input->vref ? input->vref : U(0x61);
	msg_blk->cs_present		= input->cs_d0 | input->cs_d1;
	msg_blk->cs_present_d0		= input->cs_d0;
	msg_blk->cs_present_d1		= input->cs_d1;
	if (input->mirror != 0) {
		msg_blk->addr_mirror	= U(0x0a);	/* odd CS are mirrored */
	}
	msg_blk->share2dvref_result	= 1U;

	msg_blk->acsm_odt_ctrl0		= input->odt[0];
	msg_blk->acsm_odt_ctrl1		= input->odt[1];
	msg_blk->acsm_odt_ctrl2		= input->odt[2];
	msg_blk->acsm_odt_ctrl3		= input->odt[3];
	msg_blk->enabled_dqs = (input->basic.num_active_dbyte_dfi0 +
				input->basic.num_active_dbyte_dfi1) * 8;
	msg_blk->x16present		= input->basic.dram_data_width == 0x10 ?
					  msg_blk->cs_present : 0;
	msg_blk->d4misc			= U(0x1);
	msg_blk->cs_setup_gddec		= U(0x1);
	msg_blk->rtt_nom_wr_park0	= 0U;
	msg_blk->rtt_nom_wr_park1	= 0U;
	msg_blk->rtt_nom_wr_park2	= 0U;
	msg_blk->rtt_nom_wr_park3	= 0U;
	msg_blk->rtt_nom_wr_park4	= 0U;
	msg_blk->rtt_nom_wr_park5	= 0U;
	msg_blk->rtt_nom_wr_park6	= 0U;
	msg_blk->rtt_nom_wr_park7	= 0U;
	msg_blk->mr0			= input->mr[0];
	msg_blk->mr1			= input->mr[1];
	msg_blk->mr2			= input->mr[2];
	msg_blk->mr3			= input->mr[3];
	msg_blk->mr4			= input->mr[4];
	msg_blk->mr5			= input->mr[5];
	msg_blk->mr6			= input->mr[6];
	if ((msg_blk->mr4 & U(0x1c0)) != 0U) {
		ERROR("Setting DRAM CAL mode is not supported\n");
	}

	msg_blk->alt_cas_l		= 0U;
	msg_blk->alt_wcas_l		= 0U;

	msg_blk->dramfreq		= input->basic.frequency * 2U;
	msg_blk->pll_bypass_en		= input->basic.pll_bypass;
	msg_blk->dfi_freq_ratio		= input->basic.dfi_freq_ratio == 0U ? 1U :
					  input->basic.dfi_freq_ratio == 1U ? 2U :
					  4U;
	msg_blk->bpznres_val		= input->adv.ext_cal_res_val;
	msg_blk->disabled_dbyte		= 0U;

	debug("msg_blk->dram_type = 0x%x\n", msg_blk->dram_type);
	debug("msg_blk->sequence_ctrl = 0x%x\n", msg_blk->sequence_ctrl);
	debug("msg_blk->phy_cfg = 0x%x\n", msg_blk->phy_cfg);
	debug("msg_blk->x16present = 0x%x\n", msg_blk->x16present);
	debug("msg_blk->dramfreq = 0x%x\n", msg_blk->dramfreq);
	debug("msg_blk->pll_bypass_en = 0x%x\n", msg_blk->pll_bypass_en);
	debug("msg_blk->dfi_freq_ratio = 0x%x\n", msg_blk->dfi_freq_ratio);
	debug("msg_blk->phy_odt_impedance = 0x%x\n",
						msg_blk->phy_odt_impedance);
	debug("msg_blk->phy_drv_impedance = 0x%x\n",
						msg_blk->phy_drv_impedance);
	debug("msg_blk->bpznres_val = 0x%x\n", msg_blk->bpznres_val);
	debug("msg_blk->enabled_dqs = 0x%x\n", msg_blk->enabled_dqs);
	debug("msg_blk->acsm_odt_ctrl0 = 0x%x\n", msg_blk->acsm_odt_ctrl0);
	debug("msg_blk->acsm_odt_ctrl1 = 0x%x\n", msg_blk->acsm_odt_ctrl1);
	debug("msg_blk->acsm_odt_ctrl2 = 0x%x\n", msg_blk->acsm_odt_ctrl2);
	debug("msg_blk->acsm_odt_ctrl3 = 0x%x\n", msg_blk->acsm_odt_ctrl3);

	/* RDIMM only */
	if (input->basic.dimm_type == RDIMM ||
	    input->basic.dimm_type == LRDIMM) {
		msg_blk_r = (struct ddr4r1d *)msg_blk;
		if (msg_blk_r->cs_present_d0 != 0U) {
			msg_blk_r->f0rc00_d0 = input->rcw[0];
			msg_blk_r->f0rc01_d0 = input->rcw[1];
			msg_blk_r->f0rc02_d0 = input->rcw[2];
			msg_blk_r->f0rc03_d0 = input->rcw[3];
			msg_blk_r->f0rc04_d0 = input->rcw[4];
			msg_blk_r->f0rc05_d0 = input->rcw[5];
			msg_blk_r->f0rc06_d0 = input->rcw[6];
			msg_blk_r->f0rc07_d0 = input->rcw[7];
			msg_blk_r->f0rc08_d0 = input->rcw[8];
			msg_blk_r->f0rc09_d0 = input->rcw[9];
			msg_blk_r->f0rc0a_d0 = input->rcw[10];
			msg_blk_r->f0rc0b_d0 = input->rcw[11];
			msg_blk_r->f0rc0c_d0 = input->rcw[12];
			msg_blk_r->f0rc0d_d0 = input->rcw[13];
			msg_blk_r->f0rc0e_d0 = input->rcw[14];
			msg_blk_r->f0rc0f_d0 = input->rcw[15];
			msg_blk_r->f0rc3x_d0 = input->rcw3x;
		}
		if (msg_blk_r->cs_present_d1 != 0) {
			msg_blk_r->f0rc00_d1 = input->rcw[0];
			msg_blk_r->f0rc01_d1 = input->rcw[1];
			msg_blk_r->f0rc02_d1 = input->rcw[2];
			msg_blk_r->f0rc03_d1 = input->rcw[3];
			msg_blk_r->f0rc04_d1 = input->rcw[4];
			msg_blk_r->f0rc05_d1 = input->rcw[5];
			msg_blk_r->f0rc06_d1 = input->rcw[6];
			msg_blk_r->f0rc07_d1 = input->rcw[7];
			msg_blk_r->f0rc08_d1 = input->rcw[8];
			msg_blk_r->f0rc09_d1 = input->rcw[9];
			msg_blk_r->f0rc0a_d1 = input->rcw[10];
			msg_blk_r->f0rc0b_d1 = input->rcw[11];
			msg_blk_r->f0rc0c_d1 = input->rcw[12];
			msg_blk_r->f0rc0d_d1 = input->rcw[13];
			msg_blk_r->f0rc0e_d1 = input->rcw[14];
			msg_blk_r->f0rc0f_d1 = input->rcw[15];
			msg_blk_r->f0rc3x_d1 = input->rcw3x;
		}
		if (input->basic.dimm_type == LRDIMM) {
			msg_blk_lr = (struct ddr4lr1d *)msg_blk;
			msg_blk_lr->bc0a_d0 = msg_blk_lr->f0rc0a_d0;
			msg_blk_lr->bc0a_d1 = msg_blk_lr->f0rc0a_d1;
			msg_blk_lr->f0bc6x_d0 = msg_blk_lr->f0rc3x_d0;
			msg_blk_lr->f0bc6x_d1 = msg_blk_lr->f0rc3x_d1;
		}
	}

	/* below is different for 1D and 2D message block */
	if (input->basic.train2d != 0) {
		memcpy(msg_blk_2d, msg_blk, sizeof(struct ddr4u1d));
		/*High-Effort WrDQ1D is applicable to 2D traning also*/
		msg_blk_2d->reserved00          |= U(0x40);
		msg_blk_2d->sequence_ctrl	= U(0x0061);
		msg_blk_2d->rx2d_train_opt	= 0U;
		msg_blk_2d->tx2d_train_opt	= 0U;
		msg_blk_2d->share2dvref_result	= 1U;
		msg_blk_2d->delay_weight2d	= U(0x20);
		msg_blk_2d->voltage_weight2d	= U(0x80);
		debug("rx2d_train_opt %d, tx2d_train_opt %d\n",
				msg_blk_2d->rx2d_train_opt,
				msg_blk_2d->tx2d_train_opt);
	}

	msg_blk->phy_cfg = (((msg_blk->mr3 & U(0x8)) != 0U) ||
				((msg_blk_2d->mr3 & 0x8) != 0U)) ? 0U
				: input->adv.is2ttiming;

	return 0;
}

static void prog_tx_pre_drv_mode(uint16_t *phy,
				 const struct input *input)
{
	int lane, byte, b_addr, c_addr, p_addr;
	int tx_slew_rate, tx_pre_p, tx_pre_n;
	int tx_pre_drv_mode = 0x2;
	uint32_t addr;

	/* Program TxPreDrvMode with 0x2 */
	/* FIXME: TxPreDrvMode depends on DramType? */
	tx_pre_p = input->adv.tx_slew_rise_dq;
	tx_pre_n = input->adv.tx_slew_fall_dq;
	tx_slew_rate = tx_pre_drv_mode << csr_tx_pre_drv_mode_lsb	|
		     tx_pre_p << csr_tx_pre_p_lsb			|
		     tx_pre_n << csr_tx_pre_n_lsb;
	p_addr = 0;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = p_addr | t_dbyte | c_addr | b_addr |
					csr_tx_slew_rate_addr;
			phy_io_write16(phy, addr, tx_slew_rate);
		}
	}
}

static void prog_atx_pre_drv_mode(uint16_t *phy,
				  const struct input *input)
{
	int anib, c_addr;
	int atx_slew_rate, atx_pre_p, atx_pre_n, atx_pre_drv_mode,
		ck_anib_inst[2];
	uint32_t addr;

	atx_pre_n = input->adv.tx_slew_fall_ac;
	atx_pre_p = input->adv.tx_slew_rise_ac;

	if (input->basic.num_anib == 8) {
		ck_anib_inst[0] = 1;
		ck_anib_inst[1] = 1;
	} else if (input->basic.num_anib == 10 || input->basic.num_anib == 12 ||
	    input->basic.num_anib == 13) {
		ck_anib_inst[0] = 4;
		ck_anib_inst[1] = 5;
	} else {
		ERROR("Invalid number of aNIBs: %d\n", input->basic.num_anib);
		return;
	}

	for (anib = 0; anib < input->basic.num_anib; anib++) {
		c_addr = anib << 12;
		if (anib == ck_anib_inst[0] || anib == ck_anib_inst[1]) {
			atx_pre_drv_mode = 0;
		} else {
			atx_pre_drv_mode = 3;
		}
		atx_slew_rate = atx_pre_drv_mode << csr_atx_pre_drv_mode_lsb |
				atx_pre_n << csr_atx_pre_n_lsb		     |
				atx_pre_p << csr_atx_pre_p_lsb;
		addr = t_anib | c_addr | csr_atx_slew_rate_addr;
		phy_io_write16(phy, addr, atx_slew_rate);
	}
}

static void prog_enable_cs_multicast(uint16_t *phy,
				     const struct input *input)
{
	uint32_t addr = t_master | csr_enable_cs_multicast_addr;

	if (input->basic.dimm_type != RDIMM &&
	    input->basic.dimm_type != LRDIMM) {
		return;
	}

	phy_io_write16(phy, addr, input->adv.cast_cs_to_cid);
}

static void prog_dfi_rd_data_cs_dest_map(uint16_t *phy,
					 unsigned int ip_rev,
					 const struct input *input,
					 const struct ddr4lr1d *msg)
{
	const struct ddr4lr1d *msg_blk;
	uint16_t dfi_xxdestm0 = 0U;
	uint16_t dfi_xxdestm1 = 0U;
	uint16_t dfi_xxdestm2 = 0U;
	uint16_t dfi_xxdestm3 = 0U;
	uint16_t dfi_rd_data_cs_dest_map;
	uint16_t dfi_wr_data_cs_dest_map;
	__unused const soc_info_t *soc_info;

#ifdef ERRATA_DDR_A011396
	/* Only apply to DDRC 5.05.00 */
	soc_info = get_soc_info(NXP_DCFG_ADDR);
	if ((soc_info->maj_ver == 1U) && (ip_rev == U(0x50500))) {
		phy_io_write16(phy,
				t_master | csr_dfi_rd_data_cs_dest_map_addr,
				0U);
		return;
	}
#endif

	msg_blk = msg;

	switch (input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		if ((msg_blk->msg_misc & U(0x40)) != 0U) {
			dfi_rd_data_cs_dest_map = U(0xa0);
			dfi_wr_data_cs_dest_map = U(0xa0);

			phy_io_write16(phy,
				t_master | csr_dfi_rd_data_cs_dest_map_addr,
				dfi_rd_data_cs_dest_map);
			phy_io_write16(phy,
				t_master | csr_dfi_wr_data_cs_dest_map_addr,
				dfi_wr_data_cs_dest_map);
		}
		break;
	case LRDIMM:
		if (msg->cs_present_d1 != 0U) {
			dfi_xxdestm2 = 1U;
			dfi_xxdestm3 = 1U;
		}

		dfi_rd_data_cs_dest_map =
			dfi_xxdestm0 << csr_dfi_rd_destm0_lsb	|
			dfi_xxdestm1 << csr_dfi_rd_destm1_lsb	|
			dfi_xxdestm2 << csr_dfi_rd_destm2_lsb	|
			dfi_xxdestm3 << csr_dfi_rd_destm3_lsb;
		dfi_wr_data_cs_dest_map =
			dfi_xxdestm0 << csr_dfi_wr_destm0_lsb	|
			dfi_xxdestm1 << csr_dfi_wr_destm1_lsb	|
			dfi_xxdestm2 << csr_dfi_wr_destm2_lsb	|
			dfi_xxdestm3 << csr_dfi_wr_destm3_lsb;
		phy_io_write16(phy, t_master | csr_dfi_rd_data_cs_dest_map_addr,
				dfi_rd_data_cs_dest_map);
		phy_io_write16(phy, t_master | csr_dfi_wr_data_cs_dest_map_addr,
				dfi_wr_data_cs_dest_map);

		break;
	default:
		break;
	}
}

static void prog_pll_ctrl(uint16_t *phy,
			   const struct input *input)
{
	uint32_t addr;
	int pll_ctrl1 = 0x21; /* 000100001b */
	int pll_ctrl4 = 0x17f; /* 101111111b */
	int pll_test_mode = 0x24; /* 00100100b */

	addr = t_master | csr_pll_ctrl1_addr;
	phy_io_write16(phy, addr, pll_ctrl1);

	debug("pll_ctrl1 = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_pll_test_mode_addr;
	phy_io_write16(phy, addr, pll_test_mode);

	debug("pll_test_mode = 0x%x\n", phy_io_read16(phy, addr));

	addr = t_master | csr_pll_ctrl4_addr;
	phy_io_write16(phy, addr, pll_ctrl4);

	debug("pll_ctrl4 = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_pll_ctrl2(uint16_t *phy,
			   const struct input *input)
{
	int pll_ctrl2;
	uint32_t addr = t_master | csr_pll_ctrl2_addr;

	if (input->basic.frequency / 2 < 235) {
		pll_ctrl2 = 0x7;
	} else if (input->basic.frequency / 2 < 313) {
		pll_ctrl2 = 0x6;
	} else if (input->basic.frequency / 2 < 469) {
		pll_ctrl2 = 0xb;
	} else if (input->basic.frequency / 2 < 625) {
		pll_ctrl2 = 0xa;
	} else if (input->basic.frequency / 2 < 938) {
		pll_ctrl2 = 0x19;
	} else if (input->basic.frequency / 2 < 1067) {
		pll_ctrl2 = 0x18;
	} else {
		pll_ctrl2 = 0x19;
	}

	phy_io_write16(phy, addr, pll_ctrl2);

	debug("pll_ctrl2 = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_dll_lck_param(uint16_t *phy, const struct input *input)
{
	uint32_t addr = t_master | csr_dll_lockparam_addr;

	phy_io_write16(phy, addr, U(0x212));
	debug("dll_lck_param = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_dll_gain_ctl(uint16_t *phy, const struct input *input)
{
	uint32_t addr = t_master | csr_dll_gain_ctl_addr;

	phy_io_write16(phy, addr, U(0x61));
	debug("dll_gain_ctl = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_pll_pwr_dn(uint16_t *phy,
			   const struct input *input)
{
	uint32_t addr;

	addr = t_master | csr_pll_pwr_dn_addr;
	phy_io_write16(phy, addr, 0U);

	debug("pll_pwrdn = 0x%x\n", phy_io_read16(phy, addr));
}

static void prog_ard_ptr_init_val(uint16_t *phy,
				  const struct input *input)
{
	int ard_ptr_init_val;
	uint32_t addr = t_master | csr_ard_ptr_init_val_addr;

	if (input->basic.frequency >= 933) {
		ard_ptr_init_val = 0x2;
	} else {
		ard_ptr_init_val = 0x1;
	}

	phy_io_write16(phy, addr, ard_ptr_init_val);
}

static void prog_dqs_preamble_control(uint16_t *phy,
				      const struct input *input)
{
	int data;
	uint32_t addr = t_master | csr_dqs_preamble_control_addr;
	const int wdqsextension = 0;
	const int lp4sttc_pre_bridge_rx_en = 0;
	const int lp4postamble_ext = 0;
	const int lp4tgl_two_tck_tx_dqs_pre = 0;
	const int position_dfe_init = 2;
	const int dll_rx_preamble_mode = 1;
	int two_tck_tx_dqs_pre = input->adv.d4tx_preamble_length;
	int two_tck_rx_dqs_pre = input->adv.d4rx_preamble_length;

	data = wdqsextension << csr_wdqsextension_lsb			|
	       lp4sttc_pre_bridge_rx_en << csr_lp4sttc_pre_bridge_rx_en_lsb |
	       lp4postamble_ext << csr_lp4postamble_ext_lsb		|
	       lp4tgl_two_tck_tx_dqs_pre << csr_lp4tgl_two_tck_tx_dqs_pre_lsb |
	       position_dfe_init << csr_position_dfe_init_lsb		|
	       two_tck_tx_dqs_pre << csr_two_tck_tx_dqs_pre_lsb		|
	       two_tck_rx_dqs_pre << csr_two_tck_rx_dqs_pre_lsb;
	phy_io_write16(phy, addr, data);

	data = dll_rx_preamble_mode << csr_dll_rx_preamble_mode_lsb;
	addr = t_master | csr_dbyte_dll_mode_cntrl_addr;
	phy_io_write16(phy, addr, data);
}

static void prog_proc_odt_time_ctl(uint16_t *phy,
				   const struct input *input)
{
	int proc_odt_time_ctl;
	uint32_t addr = t_master | csr_proc_odt_time_ctl_addr;

	if (input->adv.wdqsext != 0) {
		proc_odt_time_ctl = 0x3;
	} else if (input->basic.frequency <= 933) {
		proc_odt_time_ctl = 0xa;
	} else if (input->basic.frequency <= 1200) {
		if (input->adv.d4rx_preamble_length == 1) {
			proc_odt_time_ctl = 0x2;
		} else {
			proc_odt_time_ctl = 0x6;
		}
	} else {
		if (input->adv.d4rx_preamble_length == 1) {
			proc_odt_time_ctl = 0x3;
		} else {
			proc_odt_time_ctl = 0x7;
		}
	}
	phy_io_write16(phy, addr, proc_odt_time_ctl);
}

static const struct impedance_mapping map[] = {
	{	29,	0x3f	},
	{	31,	0x3e	},
	{	33,	0x3b	},
	{	36,	0x3a	},
	{	39,	0x39	},
	{	42,	0x38	},
	{	46,	0x1b	},
	{	51,	0x1a	},
	{	57,	0x19	},
	{	64,	0x18	},
	{	74,	0x0b	},
	{	88,	0x0a	},
	{	108,	0x09	},
	{	140,	0x08	},
	{	200,	0x03	},
	{	360,	0x02	},
	{	481,	0x01	},
	{}
};

static int map_impedance(int strength)
{
	const struct impedance_mapping *tbl = map;
	int val = 0;

	if (strength == 0) {
		return 0;
	}

	while (tbl->ohm != 0U) {
		if (strength < tbl->ohm) {
			val = tbl->code;
			break;
		}
		tbl++;
	}

	return val;
}

static int map_odtstren_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0) {
			val = 0;
		} else if (strength == 120) {
			val = 0x8;
		} else if (strength == 60) {
			val = 0x18;
		} else if (strength == 40) {
			val = 0x38;
		} else {
			printf("error: unsupported ODTStrenP %d\n", strength);
		}
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static void prog_tx_odt_drv_stren(uint16_t *phy,
				  const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int tx_odt_drv_stren;
	int odtstren_p, odtstren_n;
	uint32_t addr;

	odtstren_p = map_odtstren_p(input->adv.odtimpedance,
				input->basic.hard_macro_ver);
	if (odtstren_p < 0) {
		return;
	}

	odtstren_n = 0;	/* always high-z */
	tx_odt_drv_stren = odtstren_n << csr_odtstren_n_lsb | odtstren_p;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
				csr_tx_odt_drv_stren_addr;
			phy_io_write16(phy, addr, tx_odt_drv_stren);
		}
	}
}

static int map_drvstren_fsdq_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0) {
			val = 0x07;
		} else if (strength == 120) {
			val = 0x0F;
		} else if (strength == 60) {
			val = 0x1F;
		} else if (strength == 40) {
			val = 0x3F;
		} else {
			printf("error: unsupported drv_stren_fSDq_p %d\n",
			       strength);
		}
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static int map_drvstren_fsdq_n(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 0) {
			val = 0x00;
		} else if (strength == 120) {
			val = 0x08;
		} else if (strength == 60) {
			val = 0x18;
		} else if (strength == 40) {
			val = 0x38;
		} else {
			printf("error: unsupported drvStrenFSDqN %d\n",
			       strength);
		}
	} else {
		val = map_impedance(strength);
	}

	return val;
}

static void prog_tx_impedance_ctrl1(uint16_t *phy,
				    const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int tx_impedance_ctrl1;
	int drv_stren_fsdq_p, drv_stren_fsdq_n;
	uint32_t addr;

	drv_stren_fsdq_p = map_drvstren_fsdq_p(input->adv.tx_impedance,
					input->basic.hard_macro_ver);
	drv_stren_fsdq_n = map_drvstren_fsdq_n(input->adv.tx_impedance,
					input->basic.hard_macro_ver);
	tx_impedance_ctrl1 = drv_stren_fsdq_n << csr_drv_stren_fsdq_n_lsb |
			   drv_stren_fsdq_p << csr_drv_stren_fsdq_p_lsb;

	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
				csr_tx_impedance_ctrl1_addr;
			phy_io_write16(phy, addr, tx_impedance_ctrl1);
		}
	}
}

static int map_adrv_stren_p(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 120) {
			val = 0x1c;
		} else if (strength == 60) {
			val = 0x1d;
		} else if (strength == 40) {
			val = 0x1f;
		} else {
			printf("error: unsupported aDrv_stren_p %d\n",
			       strength);
		}
	} else {
		if (strength == 120) {
			val = 0x00;
		} else if (strength == 60) {
			val = 0x01;
		} else if (strength == 40) {
			val = 0x03;
		} else if (strength == 30) {
			val = 0x07;
		} else if (strength == 24) {
			val = 0x0f;
		} else if (strength == 20) {
			val = 0x1f;
		} else {
			printf("error: unsupported aDrv_stren_p %d\n",
			       strength);
		}
	}

	return val;
}

static int map_adrv_stren_n(int strength, int hard_macro_ver)
{
	int val = -1;

	if (hard_macro_ver == 4) {
		if (strength == 120) {
			val = 0x00;
		} else if (strength == 60) {
			val = 0x01;
		} else if (strength == 40) {
			val = 0x03;
		} else {
			printf("Error: unsupported ADrvStrenP %d\n", strength);
		}
	} else {
		if (strength == 120) {
			val = 0x00;
		} else if (strength == 60) {
			val = 0x01;
		} else if (strength == 40) {
			val = 0x03;
		} else if (strength == 30) {
			val = 0x07;
		} else if (strength == 24) {
			val = 0x0f;
		} else if (strength == 20) {
			val = 0x1f;
		} else {
			printf("Error: unsupported ADrvStrenP %d\n", strength);
		}
	}

	return val;
}

static void prog_atx_impedance(uint16_t *phy,
			       const struct input *input)
{
	int anib, c_addr;
	int atx_impedance;
	int adrv_stren_p;
	int adrv_stren_n;
	uint32_t addr;

	if (input->basic.hard_macro_ver == 4 &&
	    input->adv.atx_impedance == 20) {
		printf("Error:ATxImpedance has to be 40 for HardMacroVer 4\n");
		return;
	}

	adrv_stren_p = map_adrv_stren_p(input->adv.atx_impedance,
					input->basic.hard_macro_ver);
	adrv_stren_n = map_adrv_stren_n(input->adv.atx_impedance,
					input->basic.hard_macro_ver);
	atx_impedance = adrv_stren_n << csr_adrv_stren_n_lsb		|
		       adrv_stren_p << csr_adrv_stren_p_lsb;
	for (anib = 0; anib < input->basic.num_anib; anib++) {
		c_addr = anib << 12;
		addr = t_anib | c_addr | csr_atx_impedance_addr;
		phy_io_write16(phy, addr, atx_impedance);
	}
}

static void prog_dfi_mode(uint16_t *phy,
			  const struct input *input)
{
	int dfi_mode;
	uint32_t addr;

	if (input->basic.dfi1exists == 1) {
		dfi_mode = 0x5;	/* DFI1 exists but disabled */
	} else {
		dfi_mode = 0x1;	/* DFI1 does not physically exists */
	}
	addr = t_master | csr_dfi_mode_addr;
	phy_io_write16(phy, addr, dfi_mode);
}

static void prog_acx4_anib_dis(uint16_t *phy, const struct input *input)
{
	uint32_t addr;

	addr = t_master | csr_acx4_anib_dis_addr;
	phy_io_write16(phy, addr, 0x0);
	debug("%s 0x%x\n", __func__, phy_io_read16(phy, addr));
}

static void prog_dfi_camode(uint16_t *phy,
			    const struct input *input)
{
	int dfi_camode = 2;
	uint32_t addr = t_master | csr_dfi_camode_addr;

	phy_io_write16(phy, addr, dfi_camode);
}

static void prog_cal_drv_str0(uint16_t *phy,
			      const struct input *input)
{
	int cal_drv_str0;
	int cal_drv_str_pd50;
	int cal_drv_str_pu50;
	uint32_t addr;

	cal_drv_str_pu50 = input->adv.ext_cal_res_val;
	cal_drv_str_pd50 = cal_drv_str_pu50;
	cal_drv_str0 = cal_drv_str_pu50 << csr_cal_drv_str_pu50_lsb |
			cal_drv_str_pd50;
	addr = t_master | csr_cal_drv_str0_addr;
	phy_io_write16(phy, addr, cal_drv_str0);
}

static void prog_cal_uclk_info(uint16_t *phy,
			       const struct input *input)
{
	int cal_uclk_ticks_per1u_s;
	uint32_t addr;

	cal_uclk_ticks_per1u_s = input->basic.frequency >> 1;
	if (cal_uclk_ticks_per1u_s < 24) {
		cal_uclk_ticks_per1u_s = 24;
	}

	addr = t_master | csr_cal_uclk_info_addr;
	phy_io_write16(phy, addr, cal_uclk_ticks_per1u_s);
}

static void prog_cal_rate(uint16_t *phy,
			  const struct input *input)
{
	int cal_rate;
	int cal_interval;
	int cal_once;
	uint32_t addr;

	cal_interval = input->adv.cal_interval;
	cal_once = input->adv.cal_once;
	cal_rate = cal_once << csr_cal_once_lsb		|
		  cal_interval << csr_cal_interval_lsb;
	addr = t_master | csr_cal_rate_addr;
	phy_io_write16(phy, addr, cal_rate);
}

static void prog_vref_in_global(uint16_t *phy,
				const struct input *input,
				const struct ddr4u1d *msg)
{
	int vref_in_global;
	int global_vref_in_dac = 0;
	int global_vref_in_sel = 0;
	uint32_t addr;

	/*
	 * phy_vref_prcnt = msg->phy_vref / 128.0
	 *  global_vref_in_dac = (phy_vref_prcnt - 0.345) / 0.005;
	 */
	global_vref_in_dac = (msg->phy_vref * 1000 - 345 * 128 + 320) /
			     (5 * 128);

	vref_in_global = global_vref_in_dac << csr_global_vref_in_dac_lsb |
		       global_vref_in_sel;
	addr = t_master | csr_vref_in_global_addr;
	phy_io_write16(phy, addr, vref_in_global);
}

static void prog_dq_dqs_rcv_cntrl(uint16_t *phy,
				  const struct input *input)
{
	int lane, byte, b_addr, c_addr;
	int dq_dqs_rcv_cntrl;
	int gain_curr_adj_defval = 0xb;
	int major_mode_dbyte = 3;
	int dfe_ctrl_defval = 0;
	int ext_vref_range_defval = 0;
	int sel_analog_vref = 1;
	uint32_t addr;

	dq_dqs_rcv_cntrl = gain_curr_adj_defval << csr_gain_curr_adj_lsb |
			major_mode_dbyte << csr_major_mode_dbyte_lsb	|
			dfe_ctrl_defval << csr_dfe_ctrl_lsb		|
			ext_vref_range_defval << csr_ext_vref_range_lsb	|
			sel_analog_vref << csr_sel_analog_vref_lsb;
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		for (lane = 0; lane <= 1; lane++) {
			b_addr = lane << 8;
			addr = t_dbyte | c_addr | b_addr |
					csr_dq_dqs_rcv_cntrl_addr;
			phy_io_write16(phy, addr, dq_dqs_rcv_cntrl);
		}
	}
}

static void prog_mem_alert_control(uint16_t *phy,
				   const struct input *input)
{
	int mem_alert_control;
	int mem_alert_control2;
	int malertpu_en;
	int malertrx_en;
	int malertvref_level;
	int malertpu_stren;
	int malertsync_bypass;
	int malertdisable_val_defval = 1;
	uint32_t addr;

	if (input->basic.dram_type == DDR4 && input->adv.mem_alert_en == 1) {
		malertpu_en = 1;
		malertrx_en = 1;
		malertpu_stren = input->adv.mem_alert_puimp;
		malertvref_level = input->adv.mem_alert_vref_level;
		malertsync_bypass = input->adv.mem_alert_sync_bypass;
		mem_alert_control = malertdisable_val_defval << 14	|
				  malertrx_en << 13		|
				  malertpu_en << 12		|
				  malertpu_stren << 8		|
				  malertvref_level;
		mem_alert_control2 = malertsync_bypass <<
					csr_malertsync_bypass_lsb;
		addr = t_master | csr_mem_alert_control_addr;
		phy_io_write16(phy, addr, mem_alert_control);
		addr = t_master | csr_mem_alert_control2_addr;
		phy_io_write16(phy, addr, mem_alert_control2);
	}
}

static void prog_dfi_freq_ratio(uint16_t *phy,
				const struct input *input)
{
	int dfi_freq_ratio;
	uint32_t addr = t_master | csr_dfi_freq_ratio_addr;

	dfi_freq_ratio = input->basic.dfi_freq_ratio;
	phy_io_write16(phy, addr, dfi_freq_ratio);
}

static void prog_tristate_mode_ca(uint16_t *phy,
				  const struct input *input)
{
	int tristate_mode_ca;
	int dis_dyn_adr_tri;
	int ddr2tmode;
	int ck_dis_val_def = 1;
	uint32_t addr = t_master | csr_tristate_mode_ca_addr;

	dis_dyn_adr_tri = input->adv.dis_dyn_adr_tri;
	ddr2tmode = input->adv.is2ttiming;
	tristate_mode_ca = ck_dis_val_def << csr_ck_dis_val_lsb	|
			 ddr2tmode << csr_ddr2tmode_lsb		|
			 dis_dyn_adr_tri << csr_dis_dyn_adr_tri_lsb;
	phy_io_write16(phy, addr, tristate_mode_ca);
}

static void prog_dfi_xlat(uint16_t *phy,
			  const struct input *input)
{
	uint16_t loop_vector;
	int dfifreqxlat_dat;
	int pllbypass_dat;
	uint32_t addr;

	/* fIXME: Shall unused P1, P2, P3 be bypassed? */
	pllbypass_dat = input->basic.pll_bypass; /* only [0] is used */
	for (loop_vector = 0; loop_vector < 8; loop_vector++) {
		if (loop_vector == 0) {
			dfifreqxlat_dat = pllbypass_dat + 0x5555;
		} else if (loop_vector == 7) {
			dfifreqxlat_dat = 0xf000;
		} else {
			dfifreqxlat_dat = 0x5555;
		}
		addr = t_master | (csr_dfi_freq_xlat0_addr + loop_vector);
		phy_io_write16(phy, addr, dfifreqxlat_dat);
	}
}

static void prog_dbyte_misc_mode(uint16_t *phy,
				 const struct input *input,
				 const struct ddr4u1d *msg)
{
	int dbyte_misc_mode;
	int dq_dqs_rcv_cntrl1;
	int dq_dqs_rcv_cntrl1_1;
	int byte, c_addr;
	uint32_t addr;

	dbyte_misc_mode = 0x1 << csr_dbyte_disable_lsb;
	dq_dqs_rcv_cntrl1 = 0x1ff << csr_power_down_rcvr_lsb		|
			 0x1 << csr_power_down_rcvr_dqs_lsb	|
			 0x1 << csr_rx_pad_standby_en_lsb;
	dq_dqs_rcv_cntrl1_1 = (0x100 << csr_power_down_rcvr_lsb |
			csr_rx_pad_standby_en_mask);
	for (byte = 0; byte < input->basic.num_dbyte; byte++) {
		c_addr = byte << 12;
		if (byte <= input->basic.num_active_dbyte_dfi0 - 1) {
			/* disable RDBI lane if not used. */
			if ((input->basic.dram_data_width != 4) &&
				(((msg->mr5 >> 12) & 0x1) == 0)) {
				addr = t_dbyte
					| c_addr
					| csr_dq_dqs_rcv_cntrl1_addr;
				phy_io_write16(phy, addr, dq_dqs_rcv_cntrl1_1);
			}
		} else {
			addr = t_dbyte | c_addr | csr_dbyte_misc_mode_addr;
			phy_io_write16(phy, addr, dbyte_misc_mode);
			addr = t_dbyte | c_addr | csr_dq_dqs_rcv_cntrl1_addr;
			phy_io_write16(phy, addr, dq_dqs_rcv_cntrl1);
		}
	}
}

static void prog_master_x4config(uint16_t *phy,
				 const struct input *input)
{
	int master_x4config;
	int x4tg;
	uint32_t addr = t_master | csr_master_x4config_addr;

	x4tg = input->basic.dram_data_width == 4 ? 0xf : 0;
	master_x4config = x4tg << csr_x4tg_lsb;
	phy_io_write16(phy, addr, master_x4config);
}

static void prog_dmipin_present(uint16_t *phy,
				const struct input *input,
				const struct ddr4u1d *msg)
{
	int dmipin_present;
	uint32_t addr = t_master | csr_dmipin_present_addr;

	dmipin_present = (msg->mr5 >> 12) & 0x1;
	phy_io_write16(phy, addr, dmipin_present);
}

static void prog_dfi_phyupd(uint16_t *phy,
			  const struct input *input)
{
	int dfiphyupd_dat;
	uint32_t addr;

	addr = t_master | (csr_dfiphyupd_addr);
	dfiphyupd_dat = phy_io_read16(phy, addr) &
				~csr_dfiphyupd_threshold_mask;

	phy_io_write16(phy, addr, dfiphyupd_dat);
}

static void prog_cal_misc2(uint16_t *phy,
			  const struct input *input)
{
	int cal_misc2_dat, cal_drv_pdth_data, cal_offsets_dat;
	uint32_t addr;

	addr = t_master | (csr_cal_misc2_addr);
	cal_misc2_dat = phy_io_read16(phy, addr) |
			(1 << csr_cal_misc2_err_dis);

	phy_io_write16(phy, addr, cal_misc2_dat);

	addr = t_master | (csr_cal_offsets_addr);

	cal_drv_pdth_data = 0x9 << 6;
	cal_offsets_dat = (phy_io_read16(phy, addr) & ~csr_cal_drv_pdth_mask)
			| cal_drv_pdth_data;

	phy_io_write16(phy, addr, cal_offsets_dat);
}

static int c_init_phy_config(uint16_t **phy_ptr,
			     unsigned int ip_rev,
			     const struct input *input,
			     const void *msg)
{
	int i;
	uint16_t *phy;
	__unused const soc_info_t *soc_info;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}

		debug("Initialize PHY %d config\n", i);
		prog_dfi_phyupd(phy, input);
		prog_cal_misc2(phy, input);
		prog_tx_pre_drv_mode(phy, input);
		prog_atx_pre_drv_mode(phy, input);
		prog_enable_cs_multicast(phy, input);	/* rdimm and lrdimm */
		prog_dfi_rd_data_cs_dest_map(phy, ip_rev, input, msg);
		prog_pll_ctrl2(phy, input);
#ifdef DDR_PLL_FIX
		soc_info = get_soc_info();
		debug("SOC_SI_REV = %x\n", soc_info->maj_ver);
		if (soc_info->maj_ver == 1) {
			prog_pll_pwr_dn(phy, input);

			/*Enable FFE aka TxEqualizationMode for rev1 SI*/
			phy_io_write16(phy, 0x010048, 0x1);
		}
#endif
		prog_ard_ptr_init_val(phy, input);
		prog_dqs_preamble_control(phy, input);
		prog_dll_lck_param(phy, input);
		prog_dll_gain_ctl(phy, input);
		prog_proc_odt_time_ctl(phy, input);
		prog_tx_odt_drv_stren(phy, input);
		prog_tx_impedance_ctrl1(phy, input);
		prog_atx_impedance(phy, input);
		prog_dfi_mode(phy, input);
		prog_dfi_camode(phy, input);
		prog_cal_drv_str0(phy, input);
		prog_cal_uclk_info(phy, input);
		prog_cal_rate(phy, input);
		prog_vref_in_global(phy, input, msg);
		prog_dq_dqs_rcv_cntrl(phy, input);
		prog_mem_alert_control(phy, input);
		prog_dfi_freq_ratio(phy, input);
		prog_tristate_mode_ca(phy, input);
		prog_dfi_xlat(phy, input);
		prog_dbyte_misc_mode(phy, input, msg);
		prog_master_x4config(phy, input);
		prog_dmipin_present(phy, input, msg);
		prog_acx4_anib_dis(phy, input);
	}

	return 0;
}

static uint32_t get_mail(uint16_t *phy, int stream)
{
	int timeout;
	uint32_t mail = 0U;

	timeout = TIMEOUTDEFAULT;
	while (((--timeout) != 0) &&
	       ((phy_io_read16(phy, t_apbonly | csr_uct_shadow_regs)
		& uct_write_prot_shadow_mask) != 0)) {
		mdelay(10);
	}
	if (timeout == 0) {
		ERROR("Timeout getting mail from PHY\n");
		return 0xFFFF;
	}

	mail = phy_io_read16(phy, t_apbonly |
			     csr_uct_write_only_shadow);
	if (stream != 0) {
		mail |= phy_io_read16(phy, t_apbonly |
				      csr_uct_dat_write_only_shadow) << 16;
	}

	/* Ack */
	phy_io_write16(phy, t_apbonly | csr_dct_write_prot, 0);

	timeout = TIMEOUTDEFAULT;
	while (((--timeout) != 0) &&
	       ((phy_io_read16(phy, t_apbonly | csr_uct_shadow_regs)
		 & uct_write_prot_shadow_mask) == 0)) {
		mdelay(1);
	}
	if (timeout == 0) {
		ERROR("Timeout ack PHY mail\n");
	}

	/* completed */
	phy_io_write16(phy, t_apbonly | csr_dct_write_prot, 1U);

	return mail;
}

#ifdef DDR_PHY_DEBUG
static const char *lookup_msg(uint32_t index, int train2d)
{
	int i;
	int size;
	const struct phy_msg *messages;
	const char *ptr = NULL;

	if (train2d != 0) {
		messages = messages_2d;
		size = ARRAY_SIZE(messages_2d);
	} else {
		messages = messages_1d;
		size = ARRAY_SIZE(messages_1d);
	}
	for (i = 0; i < size; i++) {
		if (messages[i].index == index) {
			ptr = messages[i].msg;
			break;
		}
	}

	return ptr;
}
#endif

#define MAX_ARGS 32
static void decode_stream_message(uint16_t *phy, int train2d)
{
	uint32_t index __unused;

	__unused const char *format;
	__unused uint32_t args[MAX_ARGS];
	__unused int i;

#ifdef DDR_PHY_DEBUG
	index = get_mail(phy, 1);
	if ((index & 0xffff) > MAX_ARGS) {	/* up to MAX_ARGS args so far */
		printf("Program error in %s\n", __func__);
	}
	for (i = 0; i < (index & 0xffff) && i < MAX_ARGS; i++) {
		args[i] = get_mail(phy, 1);
	}

	format = lookup_msg(index, train2d);
	if (format != NULL) {
		printf("0x%08x: ", index);
		printf(format, args[0], args[1], args[2], args[3], args[4],
		       args[5], args[6], args[7], args[8], args[9], args[10],
		       args[11], args[12], args[13], args[14], args[15],
		       args[16], args[17], args[18], args[19], args[20],
		       args[21], args[22], args[23], args[24], args[25],
		       args[26], args[27], args[28], args[29], args[30],
		       args[31]);
	}
#endif
}

static int wait_fw_done(uint16_t *phy, int train2d)
{
	uint32_t mail = 0U;

	while (mail == U(0x0)) {
		mail = get_mail(phy, 0);
		switch (mail) {
		case U(0x7):
			debug("%s Training completed\n", train2d ? "2D" : "1D");
			break;
		case U(0xff):
			debug("%s Training failure\n", train2d ? "2D" : "1D");
			break;
		case U(0x0):
			debug("End of initialization\n");
			mail = 0U;
			break;
		case U(0x1):
			debug("End of fine write leveling\n");
			mail = 0U;
			break;
		case U(0x2):
			debug("End of read enable training\n");
			mail = 0U;
			break;
		case U(0x3):
			debug("End of read delay center optimization\n");
			mail = 0U;
			break;
		case U(0x4):
			debug("End of write delay center optimization\n");
			mail = 0U;
			break;
		case U(0x5):
			debug("End of 2D read delay/voltage center optimztn\n");
			mail = 0U;
			break;
		case U(0x6):
			debug("End of 2D write delay/voltage center optmztn\n");
			mail = 0U;
			break;
		case U(0x8):
			decode_stream_message(phy, train2d);
			mail = 0U;
			break;
		case U(0x9):
			debug("End of max read latency training\n");
			mail = 0U;
			break;
		case U(0xa):
			debug("End of read dq deskew training\n");
			mail = 0U;
			break;
		case U(0xc):
			debug("End of LRDIMM Specific training, including:\n");
			debug("/tDWL, MREP, MRD and MWD\n");
			mail = 0U;
			break;
		case U(0xd):
			debug("End of CA training\n");
			mail = 0U;
			break;
		case U(0xfd):
			debug("End of MPR read delay center optimization\n");
			mail = 0U;
			break;
		case U(0xfe):
			debug("End of Write leveling coarse delay\n");
			mail = 0U;
			break;
		case U(0xffff):
			debug("Timed out\n");
			break;
		default:
			mail = 0U;
			break;
		}
	}

	if (mail == U(0x7)) {
		return 0;
	} else if (mail == U(0xff)) {
		return -EIO;
	} else if (mail == U(0xffff)) {
		return -ETIMEDOUT;
	}

	debug("PHY_GEN2 FW: Unxpected mail = 0x%x\n", mail);

	return -EINVAL;
}

static int g_exec_fw(uint16_t **phy_ptr, int train2d, struct input *input)
{
	int ret = -EINVAL;
	int i;
	uint16_t *phy;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}
		debug("Applying PLL optimal settings\n");
		prog_pll_ctrl2(phy, input);
		prog_pll_ctrl(phy, input);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_cont_mux_sel_addr,
			       0x1);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       csr_reset_to_micro_mask |
			       csr_stall_to_micro_mask);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       csr_stall_to_micro_mask);
		phy_io_write16(phy,
			       t_apbonly | csr_micro_reset_addr,
			       0);

		ret = wait_fw_done(phy, train2d);
		if (ret == -ETIMEDOUT) {
			ERROR("Wait timed out: Firmware execution on PHY %d\n",
			      i);
		}
	}
	return ret;
}

static inline int send_fw(uint16_t *phy,
			   uint32_t dst,
			   uint16_t *img,
			   uint32_t size)
{
	uint32_t i;

	if ((size % 2U) != 0U) {
		ERROR("Wrong image size 0x%x\n", size);
		return -EINVAL;
	}

	for (i = 0U; i < size / 2; i++) {
		phy_io_write16(phy, dst + i, *(img + i));
	}

	return 0;
}

static int load_fw(uint16_t **phy_ptr,
		   struct input *input,
		   int train2d,
		   void *msg,
		   size_t len,
		   uintptr_t phy_gen2_fw_img_buf,
		   int (*img_loadr)(unsigned int, uintptr_t *, uint32_t *),
		   uint32_t warm_boot_flag)
{
	uint32_t imem_id, dmem_id;
	uintptr_t image_buf;
	uint32_t size;
	int ret;
	int i;
	uint16_t *phy;

	switch (input->basic.dimm_type) {
	case UDIMM:
	case SODIMM:
	case NODIMM:
		imem_id = train2d ? DDR_IMEM_UDIMM_2D_IMAGE_ID :
			  DDR_IMEM_UDIMM_1D_IMAGE_ID;
		dmem_id = train2d ? DDR_DMEM_UDIMM_2D_IMAGE_ID :
			  DDR_DMEM_UDIMM_1D_IMAGE_ID;
		break;
	case RDIMM:
		imem_id = train2d ? DDR_IMEM_RDIMM_2D_IMAGE_ID :
			  DDR_IMEM_RDIMM_1D_IMAGE_ID;
		dmem_id = train2d ? DDR_DMEM_RDIMM_2D_IMAGE_ID :
			  DDR_DMEM_RDIMM_1D_IMAGE_ID;
		break;
	default:
		ERROR("Unsupported DIMM type\n");
		return -EINVAL;
	}

	size = PHY_GEN2_MAX_IMAGE_SIZE;
	image_buf = (uintptr_t)phy_gen2_fw_img_buf;
	mmap_add_dynamic_region(phy_gen2_fw_img_buf,
			phy_gen2_fw_img_buf,
			PHY_GEN2_MAX_IMAGE_SIZE,
			MT_MEMORY | MT_RW | MT_SECURE);
	ret = img_loadr(imem_id, &image_buf, &size);
	if (ret != 0) {
		ERROR("Failed to load %d firmware.\n", imem_id);
		return ret;
	}
	debug("Loaded Imaged id %d of size %x at address %lx\n",
						imem_id, size, image_buf);

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}

		if (warm_boot_flag != DDR_WARM_BOOT) {
			if (train2d == 0) {
				phy_io_write16(phy, t_master |
						csr_mem_reset_l_addr,
						csr_protect_mem_reset_mask);
			}
		}
		/* Enable access to the internal CSRs */
		phy_io_write16(phy, t_apbonly | csr_micro_cont_mux_sel_addr, 0);

		ret = send_fw(phy, PHY_GEN2_IMEM_ADDR,
			      (uint16_t *)image_buf, size);
		if (ret != 0) {
			return ret;
		}
	}

	size = PHY_GEN2_MAX_IMAGE_SIZE;
	image_buf = (uintptr_t)phy_gen2_fw_img_buf;
	ret = img_loadr(dmem_id, &image_buf, &size);
	if (ret != 0) {
		ERROR("Failed to load %d firmware.\n", dmem_id);
		return ret;
	}
	debug("Loaded Imaged id %d of size %x at address %lx\n",
						dmem_id, size, image_buf);
	image_buf += len;
	size -= len;

	for (i = 0; i < NUM_OF_DDRC; i++) {
		phy = phy_ptr[i];
		if (phy == NULL) {
			continue;
		}

		ret = send_fw(phy, PHY_GEN2_DMEM_ADDR, msg, len);
		if (ret != 0) {
			return ret;
		}

		ret = send_fw(phy, PHY_GEN2_DMEM_ADDR + len / 2,
			      (uint16_t *)image_buf, size);
		if (ret != 0) {
			return ret;
		}
	}

	return ret;
}

static void parse_odt(const unsigned int val,
		       const int read,
		       const int i,
		       const unsigned int cs_d0,
		       const unsigned int cs_d1,
		       unsigned int *odt)
{
	int shift = read ? 4 : 0;
	int j;

	if (i < 0 || i > 3) {
		printf("Error: invalid chip-select value\n");
	}
	switch (val) {
	case DDR_ODT_CS:
		odt[i] |= (1 << i) << shift;
		break;
	case DDR_ODT_ALL_OTHER_CS:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (i == j) {
				continue;
			}
			if (((cs_d0 | cs_d1) & (1 << j)) == 0) {
				continue;
			}
			odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_CS_AND_OTHER_DIMM:
		odt[i] |= (1 << i) << 4;
		/* fallthrough */
	case DDR_ODT_OTHER_DIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if ((((cs_d0 & (1 << i)) != 0) &&
						((cs_d1 & (1 << j)) != 0)) ||
			    (((cs_d1 & (1 << i)) != 0) &&
						((cs_d0 & (1 << j)) != 0))) {
				odt[j] |= (1 << i) << shift;
			}
		}
		break;
	case DDR_ODT_ALL:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (((cs_d0 | cs_d1) & (1 << j)) == 0) {
				continue;
			}
			odt[j] |= (1 << i) << shift;
		}
		break;
	case DDR_ODT_SAME_DIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if ((((cs_d0 & (1 << i)) != 0) &&
						((cs_d0 & (1 << j)) != 0)) ||
			    (((cs_d1 & (1 << i)) != 0) &&
						((cs_d1 & (1 << j)) != 0))) {
				odt[j] |= (1 << i) << shift;
			}
		}
		break;
	case DDR_ODT_OTHER_CS_ONSAMEDIMM:
		for (j = 0; j < DDRC_NUM_CS; j++) {
			if (i == j) {
				continue;
			}
			if ((((cs_d0 & (1 << i)) != 0) &&
						((cs_d0 & (1 << j)) != 0)) ||
			    (((cs_d1 & (1 << i)) != 0) &&
						((cs_d1 & (1 << j)) != 0))) {
				odt[j] |= (1 << i) << shift;
			}
		}
		break;
	case DDR_ODT_NEVER:
		break;
	default:
		break;
	}
}

#ifdef DEBUG_DDR_INPUT_CONFIG
char *dram_types_str[] = {
		"DDR4",
		"DDR3",
		"LDDDR4",
		"LPDDR3",
		"LPDDR2",
		"DDR5"
};

char *dimm_types_str[] = {
		"UDIMM",
		"SODIMM",
		"RDIMM",
		"LRDIMM",
		"NODIMM",
};


static void print_jason_format(struct input *input,
			       struct ddr4u1d *msg_1d,
			       struct ddr4u2d *msg_2d)
{

	printf("\n{");
	printf("\n    \"dram_type\": \"%s\",", dram_types_str[input->basic.dram_type]);
	printf("\n    \"dimm_type\": \"%s\",", dimm_types_str[input->basic.dimm_type]);
	printf("\n    \"hard_macro_ver\": \"%d\",", input->basic.hard_macro_ver);
	printf("\n    \"num_dbyte\": \"0x%04x\",", (unsigned int)input->basic.num_dbyte);
	printf("\n    \"num_active_dbyte_dfi0\": \"0x%04x\",", (unsigned int)input->basic.num_active_dbyte_dfi0);
	printf("\n    \"num_anib\": \"0x%04x\",", (unsigned int)input->basic.num_anib);
	printf("\n    \"num_rank_dfi0\": \"0x%04x\",", (unsigned int)input->basic.num_rank_dfi0);
	printf("\n    \"num_pstates\": \"0x%04x\",", (unsigned int)input->basic.num_pstates);
	printf("\n    \"frequency\": \"%d\",", input->basic.frequency);
	printf("\n    \"pll_bypass\": \"0x%04x\",", (unsigned int)input->basic.dfi_freq_ratio);
	printf("\n    \"dfi_freq_ratio\": \"0x%04x\",", (unsigned int)input->basic.dfi_freq_ratio);
	printf("\n    \"dfi1_exists\":  \"0x%04x\",", (unsigned int)input->basic.dfi1exists);
	printf("\n    \"dram_data_width\": \"0x%04x\",", (unsigned int)input->basic.dram_data_width);
	printf("\n    \"dram_byte_swap\": \"0x%04x\",", (unsigned int)input->adv.dram_byte_swap);
	printf("\n    \"ext_cal_res_val\": \"0x%04x\",", (unsigned int)input->adv.ext_cal_res_val);
	printf("\n    \"tx_slew_rise_dq\": \"0x%04x\",", (unsigned int)input->adv.tx_slew_rise_dq);
	printf("\n    \"tx_slew_fall_dq\": \"0x%04x\",", (unsigned int)input->adv.tx_slew_fall_dq);
	printf("\n    \"tx_slew_rise_ac\": \"0x%04x\",", (unsigned int)input->adv.tx_slew_rise_ac);
	printf("\n    \"tx_slew_fall_ac\": \"0x%04x\",", (unsigned int)input->adv.tx_slew_fall_ac);
	printf("\n    \"odt_impedance\": \"%d\",", input->adv.odtimpedance);
	printf("\n    \"tx_impedance\": \"%d\",", input->adv.tx_impedance);
	printf("\n    \"atx_impedance\": \"%d\",", input->adv.atx_impedance);
	printf("\n    \"mem_alert_en\": \"0x%04x\",", (unsigned int)input->adv.mem_alert_en);
	printf("\n    \"mem_alert_pu_imp\": \"0x%04x\",", (unsigned int)input->adv.mem_alert_puimp);
	printf("\n    \"mem_alert_vref_level\": \"0x%04x\",", (unsigned int)input->adv.mem_alert_vref_level);
	printf("\n    \"mem_alert_sync_bypass\": \"0x%04x\",", (unsigned int)input->adv.mem_alert_sync_bypass);
	printf("\n    \"cal_interval\": \"0x%04x\",", (unsigned int)input->adv.cal_interval);
	printf("\n    \"cal_once\": \"0x%04x\",", (unsigned int)input->adv.cal_once);
	printf("\n    \"dis_dyn_adr_tri\": \"0x%04x\",", (unsigned int)input->adv.dis_dyn_adr_tri);
	printf("\n    \"is2t_timing\": \"0x%04x\",", (unsigned int)input->adv.is2ttiming);
	printf("\n    \"d4rx_preabmle_length\": \"0x%04x\",", (unsigned int)input->adv.d4rx_preamble_length);
	printf("\n    \"d4tx_preamble_length\": \"0x%04x\",", (unsigned int)input->adv.d4tx_preamble_length);
	printf("\n    \"msg_misc\": \"0x%02x\",", (unsigned int)msg_1d->msg_misc);
	printf("\n    \"reserved00\": \"0x%01x\",", (unsigned int)msg_1d->reserved00);
	printf("\n    \"hdt_ctrl\": \"0x%02x\",", (unsigned int)msg_1d->hdt_ctrl);
	printf("\n    \"cs_present\": \"0x%02x\",", (unsigned int)msg_1d->cs_present);
	printf("\n    \"phy_vref\": \"0x%02x\",", (unsigned int)msg_1d->phy_vref);
	printf("\n    \"dfi_mrl_margin\": \"0x%02x\",", (unsigned int)msg_1d->dfimrlmargin);
	printf("\n    \"addr_mirror\": \"0x%02x\",", (unsigned int)msg_1d->addr_mirror);
	printf("\n    \"wr_odt_pat_rank0\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl0 & 0x0f));
	printf("\n    \"wr_odt_pat_rank1\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl1 & 0x0f));
	printf("\n    \"wr_odt_pat_rank2\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl2 & 0x0f));
	printf("\n    \"wr_odt_pat_rank3\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl3 & 0x0f));
	printf("\n    \"rd_odt_pat_rank0\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl0 & 0xf0));
	printf("\n    \"rd_odt_pat_rank1\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl1 & 0xf0));
	printf("\n    \"rd_odt_pat_rank2\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl2 & 0xf0));
	printf("\n    \"rd_odt_pat_rank3\": \"0x%02x\",", (unsigned int)(msg_1d->acsm_odt_ctrl3 & 0xf0));
	printf("\n    \"d4_misc\": \"0x%01x\",", (unsigned int)msg_1d->d4misc);
	printf("\n    \"share_2d_vref_results\": \"0x%01x\",", (unsigned int)msg_1d->share2dvref_result);
	printf("\n    \"sequence_ctrl\": \"0x%04x\",", (unsigned int)msg_1d->sequence_ctrl);
	printf("\n    \"mr0\": \"0x%04x\",", (unsigned int)msg_1d->mr0);
	printf("\n    \"mr1\": \"0x%04x\",", (unsigned int)msg_1d->mr1);
	printf("\n    \"mr2\": \"0x%04x\",", (unsigned int)msg_1d->mr2);
	printf("\n    \"mr3\": \"0x%04x\",", (unsigned int)msg_1d->mr3);
	printf("\n    \"mr4\": \"0x%04x\",", (unsigned int)msg_1d->mr4);
	printf("\n    \"mr5\": \"0x%04x\",", (unsigned int)msg_1d->mr5);
	printf("\n    \"mr6\": \"0x%04x\",", (unsigned int)msg_1d->mr6);
	printf("\n    \"alt_cal_l\": \"0x%04x\",", (unsigned int)msg_1d->alt_cas_l);
	printf("\n    \"alt_wcal_l\": \"0x%04x\",", (unsigned int)msg_1d->alt_wcas_l);
	printf("\n    \"sequence_ctrl_2d\": \"0x%04x\",", (unsigned int)msg_2d->sequence_ctrl);
	printf("\n    \"rtt_nom_wr_park0\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park0);
	printf("\n    \"rtt_nom_wr_park1\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park1);
	printf("\n    \"rtt_nom_wr_park2\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park2);
	printf("\n    \"rtt_nom_wr_park3\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park3);
	printf("\n    \"rtt_nom_wr_park4\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park4);
	printf("\n    \"rtt_nom_wr_park5\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park5);
	printf("\n    \"rtt_nom_wr_park6\": \"0x%01x\",", (unsigned int)msg_1d->rtt_nom_wr_park6);
	printf("\n    \"rtt_nom_wr_park7\": \"0x%01x\"", (unsigned int)msg_1d->rtt_nom_wr_park7);
	printf("\n}");
	printf("\n");
}
#endif

int compute_ddr_phy(struct ddr_info *priv)
{
	const unsigned long clk = priv->clk;
	const struct memctl_opt *popts = &priv->opt;
	const struct ddr_conf *conf = &priv->conf;
	const struct dimm_params *dimm_param = &priv->dimm;
	struct ddr_cfg_regs *regs = &priv->ddr_reg;
	int ret;
	static struct input input;
	static struct ddr4u1d msg_1d;
	static struct ddr4u2d msg_2d;
	unsigned int i;
	unsigned int odt_rd, odt_wr;
	__unused const soc_info_t *soc_info;
#ifdef NXP_APPLY_MAX_CDD
	unsigned int tcfg0, tcfg4, rank;
#endif

	if (dimm_param == NULL) {
		ERROR("Empty DIMM parameters.\n");
		return -EINVAL;
	}

	zeromem(&input, sizeof(input));
	zeromem(&msg_1d, sizeof(msg_1d));
	zeromem(&msg_2d, sizeof(msg_2d));

	input.basic.dram_type = DDR4;
	/* FIXME: Add condition for LRDIMM */
	input.basic.dimm_type = (dimm_param->rdimm != 0) ? RDIMM : UDIMM;
	input.basic.num_dbyte = dimm_param->primary_sdram_width / 8 +
				 dimm_param->ec_sdram_width / 8;
	input.basic.num_active_dbyte_dfi0 = input.basic.num_dbyte;
	input.basic.num_rank_dfi0 = dimm_param->n_ranks;
	input.basic.dram_data_width = dimm_param->device_width;
	input.basic.hard_macro_ver	= 0xa;
	input.basic.num_pstates	= 1;
	input.basic.dfi_freq_ratio	= 1;
	input.basic.num_anib		= 0xc;
	input.basic.train2d		= popts->skip2d ? 0 : 1;
	input.basic.frequency = (int) (clk / 2000000ul);
	debug("frequency = %dMHz\n", input.basic.frequency);
	input.cs_d0 = conf->cs_on_dimm[0];
#if DDRC_NUM_DIMM > 1
	input.cs_d1 = conf->cs_on_dimm[1];
#endif
	input.mirror = dimm_param->mirrored_dimm;
	input.mr[0] = regs->sdram_mode[0] & U(0xffff);
	input.mr[1] = regs->sdram_mode[0] >> 16U;
	input.mr[2] = regs->sdram_mode[1] >> 16U;
	input.mr[3] = regs->sdram_mode[1] & U(0xffff);
	input.mr[4] = regs->sdram_mode[8] >> 16U;
	input.mr[5] = regs->sdram_mode[8] & U(0xffff);
	input.mr[6] = regs->sdram_mode[9] >> 16U;
	input.vref = popts->vref_phy;
	debug("Vref_phy = %d percent\n", (input.vref * 100U) >> 7U);
	for (i = 0U; i < DDRC_NUM_CS; i++) {
		if ((regs->cs[i].config & SDRAM_CS_CONFIG_EN) == 0U) {
			continue;
		}
		odt_rd = (regs->cs[i].config >> 20U) & U(0x7);
		odt_wr = (regs->cs[i].config >> 16U) & U(0x7);
		parse_odt(odt_rd, true, i, input.cs_d0, input.cs_d1,
			   input.odt);
		parse_odt(odt_wr, false, i, input.cs_d0, input.cs_d1,
			   input.odt);
	}

	/* Do not set sdram_cfg[RD_EN] or sdram_cfg2[RCW_EN] for RDIMM */
	if (dimm_param->rdimm != 0U) {
		regs->sdram_cfg[0] &= ~(1 << 28U);
		regs->sdram_cfg[1] &= ~(1 << 2U);
		input.rcw[0] = (regs->sdram_rcw[0] >> 28U) & U(0xf);
		input.rcw[1] = (regs->sdram_rcw[0] >> 24U) & U(0xf);
		input.rcw[2] = (regs->sdram_rcw[0] >> 20U) & U(0xf);
		input.rcw[3] = (regs->sdram_rcw[0] >> 16U) & U(0xf);
		input.rcw[4] = (regs->sdram_rcw[0] >> 12U) & U(0xf);
		input.rcw[5] = (regs->sdram_rcw[0] >> 8U) & U(0xf);
		input.rcw[6] = (regs->sdram_rcw[0] >> 4U) & U(0xf);
		input.rcw[7] = (regs->sdram_rcw[0] >> 0U) & U(0xf);
		input.rcw[8] = (regs->sdram_rcw[1] >> 28U) & U(0xf);
		input.rcw[9] = (regs->sdram_rcw[1] >> 24U) & U(0xf);
		input.rcw[10] = (regs->sdram_rcw[1] >> 20U) & U(0xf);
		input.rcw[11] = (regs->sdram_rcw[1] >> 16U) & U(0xf);
		input.rcw[12] = (regs->sdram_rcw[1] >> 12U) & U(0xf);
		input.rcw[13] = (regs->sdram_rcw[1] >> 8U) & U(0xf);
		input.rcw[14] = (regs->sdram_rcw[1] >> 4U) & U(0xf);
		input.rcw[15] = (regs->sdram_rcw[1] >> 0U) & U(0xf);
		input.rcw3x = (regs->sdram_rcw[2] >> 8U) & U(0xff);
	}

	input.adv.odtimpedance = popts->odt ? popts->odt : 60;
	input.adv.tx_impedance = popts->phy_tx_impedance ?
					popts->phy_tx_impedance : 28;
	input.adv.atx_impedance = popts->phy_atx_impedance ?
					popts->phy_atx_impedance : 30;

	debug("Initializing input adv data structure\n");
	phy_gen2_init_input(&input);

	debug("Initializing message block\n");
	ret = phy_gen2_msg_init(&msg_1d, &msg_2d, &input);
	if (ret != 0) {
		ERROR("Init msg failed (error code %d)\n", ret);
		return ret;
	}

	ret = c_init_phy_config(priv->phy, priv->ip_rev, &input, &msg_1d);
	if (ret != 0) {
		ERROR("Init PHY failed (error code %d)\n", ret);
		return ret;
	}
#ifdef NXP_WARM_BOOT
	debug("Warm boot flag value %0x\n", priv->warm_boot_flag);
	if (priv->warm_boot_flag == DDR_WARM_BOOT) {
		debug("Restoring the Phy training data\n");
		// Restore the training data
		ret = restore_phy_training_values(priv->phy,
						  PHY_TRAINING_REGS_ON_FLASH,
						  priv->num_ctlrs,
						  input.basic.train2d);
		if (ret != 0) {
			ERROR("Restoring of training data failed %d\n", ret);
			return ret;
		}
	} else {
#endif

		debug("Load 1D firmware\n");
		ret = load_fw(priv->phy, &input, 0, &msg_1d,
			      sizeof(struct ddr4u1d), priv->phy_gen2_fw_img_buf,
					priv->img_loadr, priv->warm_boot_flag);
		if (ret != 0) {
			ERROR("Loading firmware failed (error code %d)\n", ret);
			return ret;
		}

		debug("Execute firmware\n");
		ret = g_exec_fw(priv->phy, 0, &input);
		if (ret != 0) {
			ERROR("Execution FW failed (error code %d)\n", ret);
		}

#ifdef NXP_APPLY_MAX_CDD
		soc_info = get_soc_info(NXP_DCFG_ADDR);
		if (soc_info->maj_ver == 2) {
			tcfg0 = regs->timing_cfg[0];
			tcfg4 = regs->timing_cfg[4];
			rank = findrank(conf->cs_in_use);
			get_cdd_val(priv->phy, rank, input.basic.frequency,
					&tcfg0, &tcfg4);
			regs->timing_cfg[0] = tcfg0;
			regs->timing_cfg[4] = tcfg4;
		}
#endif

		if ((ret == 0) && (input.basic.train2d != 0)) {
			/* 2D training starts here */
			debug("Load 2D firmware\n");
			ret = load_fw(priv->phy, &input, 1, &msg_2d,
				      sizeof(struct ddr4u2d),
				      priv->phy_gen2_fw_img_buf,
				      priv->img_loadr,
				      priv->warm_boot_flag);
			if (ret != 0) {
				ERROR("Loading fw failed (err code %d)\n", ret);
			} else {
				debug("Execute 2D firmware\n");
				ret = g_exec_fw(priv->phy, 1, &input);
				if (ret != 0) {
					ERROR("Execution FW failed (err %d)\n",
					       ret);
				}
			}
		}
#ifdef NXP_WARM_BOOT
		if (priv->warm_boot_flag != DDR_WRM_BOOT_NT_SUPPORTED &&
		    ret == 0) {
			debug("save the phy training data\n");
			//Save training data TBD
			ret = save_phy_training_values(priv->phy,
						PHY_TRAINING_REGS_ON_FLASH,
						priv->num_ctlrs,
						input.basic.train2d);
			if (ret != 0) {
				ERROR("Saving training data failed.");
				ERROR("Warm boot will fail. Error=%d.\n", ret);
			}
		}
	} /* else */
#endif

	if (ret == 0) {
		debug("Load PIE\n");
		i_load_pie(priv->phy, &input, &msg_1d);

		NOTICE("DDR4 %s with %d-rank %d-bit bus (x%d)\n",
		       input.basic.dimm_type == RDIMM ? "RDIMM" :
		       input.basic.dimm_type == LRDIMM ? "LRDIMM" :
		       "UDIMM",
		       dimm_param->n_ranks,
		       dimm_param->primary_sdram_width,
		       dimm_param->device_width);
	}
#ifdef DEBUG_DDR_INPUT_CONFIG
	print_jason_format(&input, &msg_1d, &msg_2d);
#endif

	return ret;
}
