/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <common/debug.h>
#include <ddr.h>
#include <dimm.h>
#include <i2c.h>
#include <lib/utils.h>

int read_spd(unsigned char chip, void *buf, int len)
{
	unsigned char dummy = 0U;
	int ret;

	if (len < 256) {
		ERROR("Invalid SPD length\n");
		return -EINVAL;
	}

	i2c_write(SPD_SPA0_ADDRESS, 0, 1, &dummy, 1);
	ret = i2c_read(chip, 0, 1, buf, 256);
	if (ret == 0) {
		i2c_write(SPD_SPA1_ADDRESS, 0, 1, &dummy, 1);
		ret = i2c_read(chip, 0, 1, buf + 256, min(256, len - 256));
	}
	if (ret != 0) {
		zeromem(buf, len);
	}

	return ret;
}

int crc16(unsigned char *ptr, int count)
{
	int i;
	int crc = 0;

	while (--count >= 0) {
		crc = crc ^ (int)*ptr++ << 8;
		for (i = 0; i < 8; ++i) {
			if ((crc & 0x8000) != 0) {
				crc = crc << 1 ^ 0x1021;
			} else {
				crc = crc << 1;
			}
		}
	}
	return crc & 0xffff;
}

static int ddr4_spd_check(const struct ddr4_spd *spd)
{
	void *p = (void *)spd;
	int csum16;
	int len;
	char crc_lsb;	/* byte 126 */
	char crc_msb;	/* byte 127 */

	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	if (spd->crc[0] != crc_lsb || spd->crc[1] != crc_msb) {
		ERROR("SPD CRC = 0x%x%x, computed CRC = 0x%x%x\n",
		      spd->crc[1], spd->crc[0], crc_msb, crc_lsb);
		return -EINVAL;
	}

	p = (void *)spd + 128;
	len = 126;
	csum16 = crc16(p, len);

	crc_lsb = (char) (csum16 & 0xff);
	crc_msb = (char) (csum16 >> 8);

	if (spd->mod_section.uc[126] != crc_lsb ||
	    spd->mod_section.uc[127] != crc_msb) {
		ERROR("SPD CRC = 0x%x%x, computed CRC = 0x%x%x\n",
		      spd->mod_section.uc[127], spd->mod_section.uc[126],
		      crc_msb, crc_lsb);
		return -EINVAL;
	}

	return 0;
}

static unsigned long long
compute_ranksize(const struct ddr4_spd *spd)
{
	unsigned long long bsize;

	int nbit_sdram_cap_bsize = 0;
	int nbit_primary_bus_width = 0;
	int nbit_sdram_width = 0;
	int die_count = 0;
	bool package_3ds;

	if ((spd->density_banks & 0xf) <= 7) {
		nbit_sdram_cap_bsize = (spd->density_banks & 0xf) + 28;
	}
	if ((spd->bus_width & 0x7) < 4) {
		nbit_primary_bus_width = (spd->bus_width & 0x7) + 3;
	}
	if ((spd->organization & 0x7) < 4) {
		nbit_sdram_width = (spd->organization & 0x7) + 2;
	}
	package_3ds = (spd->package_type & 0x3) == 0x2;
	if (package_3ds) {
		die_count = (spd->package_type >> 4) & 0x7;
	}

	bsize = 1ULL << (nbit_sdram_cap_bsize - 3 +
			 nbit_primary_bus_width - nbit_sdram_width +
			 die_count);

	return bsize;
}

int cal_dimm_params(const struct ddr4_spd *spd, struct dimm_params *pdimm)
{
	int ret;
	int i;
	static const unsigned char udimm_rc_e_dq[18] = {
		0x0c, 0x2c, 0x15, 0x35, 0x15, 0x35, 0x0b, 0x2c, 0x15,
		0x35, 0x0b, 0x35, 0x0b, 0x2c, 0x0b, 0x35, 0x15, 0x36
	};
	int spd_error = 0;
	unsigned char *ptr;
	unsigned char val;

	if (spd->mem_type != SPD_MEMTYPE_DDR4) {
		ERROR("Not a DDR4 DIMM.\n");
		return -EINVAL;
	}

	ret = ddr4_spd_check(spd);
	if (ret != 0) {
		ERROR("DIMM SPD checksum mismatch\n");
		return -EINVAL;
	}

	/*
	 * The part name in ASCII in the SPD EEPROM is not null terminated.
	 * Guarantee null termination here by presetting all bytes to 0
	 * and copying the part name in ASCII from the SPD onto it
	 */
	if ((spd->info_size_crc & 0xF) > 2) {
		memcpy(pdimm->mpart, spd->mpart, sizeof(pdimm->mpart) - 1);
	}

	/* DIMM organization parameters */
	pdimm->n_ranks = ((spd->organization >> 3) & 0x7) + 1;
	debug("n_ranks %d\n", pdimm->n_ranks);
	pdimm->rank_density = compute_ranksize(spd);
	if (pdimm->rank_density == 0) {
		return -EINVAL;
	}

	debug("rank_density 0x%llx\n", pdimm->rank_density);
	pdimm->capacity = pdimm->n_ranks * pdimm->rank_density;
	debug("capacity 0x%llx\n", pdimm->capacity);
	pdimm->die_density = spd->density_banks & 0xf;
	debug("die density 0x%x\n", pdimm->die_density);
	pdimm->primary_sdram_width = 1 << (3 + (spd->bus_width & 0x7));
	debug("primary_sdram_width %d\n", pdimm->primary_sdram_width);
	if (((spd->bus_width >> 3) & 0x3) != 0) {
		pdimm->ec_sdram_width = 8;
	} else {
		pdimm->ec_sdram_width = 0;
	}
	debug("ec_sdram_width %d\n", pdimm->ec_sdram_width);
	pdimm->device_width = 1 << ((spd->organization & 0x7) + 2);
	debug("device_width %d\n", pdimm->device_width);
	pdimm->package_3ds = (spd->package_type & 0x3) == 0x2 ?
			     (spd->package_type >> 4) & 0x7 : 0;
	debug("package_3ds %d\n", pdimm->package_3ds);

	switch (spd->module_type & DDR4_SPD_MODULETYPE_MASK) {
	case DDR4_SPD_RDIMM:
	case DDR4_SPD_MINI_RDIMM:
	case DDR4_SPD_72B_SO_RDIMM:
		pdimm->rdimm = 1;
		pdimm->rc = spd->mod_section.registered.ref_raw_card & 0x8f;
		if ((spd->mod_section.registered.reg_map & 0x1) != 0) {
			pdimm->mirrored_dimm = 1;
		}
		val = spd->mod_section.registered.ca_stren;
		pdimm->rcw[3] = val >> 4;
		pdimm->rcw[4] = ((val & 0x3) << 2) | ((val & 0xc) >> 2);
		val = spd->mod_section.registered.clk_stren;
		pdimm->rcw[5] = ((val & 0x3) << 2) | ((val & 0xc) >> 2);
		pdimm->rcw[6] = 0xf;
		/* A17 used for 16Gb+, C[2:0] used for 3DS */
		pdimm->rcw[8] = pdimm->die_density >= 0x6 ? 0x0 : 0x8 |
				(pdimm->package_3ds > 0x3 ? 0x0 :
				 (pdimm->package_3ds > 0x1 ? 0x1 :
				  (pdimm->package_3ds > 0 ? 0x2 : 0x3)));
		if (pdimm->package_3ds != 0 || pdimm->n_ranks != 4) {
			pdimm->rcw[13] = 0x4;
		} else {
			pdimm->rcw[13] = 0x5;
		}
		pdimm->rcw[13] |= pdimm->mirrored_dimm ? 0x8 : 0;
		break;

	case DDR4_SPD_UDIMM:
	case DDR4_SPD_SO_DIMM:
	case DDR4_SPD_MINI_UDIMM:
	case DDR4_SPD_72B_SO_UDIMM:
	case DDR4_SPD_16B_SO_DIMM:
	case DDR4_SPD_32B_SO_DIMM:
		pdimm->rc = spd->mod_section.unbuffered.ref_raw_card & 0x8f;
		if ((spd->mod_section.unbuffered.addr_mapping & 0x1) != 0) {
			pdimm->mirrored_dimm = 1;
		}
		if ((spd->mod_section.unbuffered.mod_height & 0xe0) == 0 &&
		    (spd->mod_section.unbuffered.ref_raw_card == 0x04)) {
			/* Fix SPD error found on DIMMs with raw card E0 */
			for (i = 0; i < 18; i++) {
				if (spd->mapping[i] == udimm_rc_e_dq[i]) {
					continue;
				}
				spd_error = 1;
				ptr = (unsigned char *)&spd->mapping[i];
				*ptr = udimm_rc_e_dq[i];
			}
			if (spd_error != 0) {
				INFO("SPD DQ mapping error fixed\n");
			}
		}
		break;

	default:
		ERROR("Unknown module_type 0x%x\n", spd->module_type);
		return -EINVAL;
	}
	debug("rdimm %d\n", pdimm->rdimm);
	debug("mirrored_dimm %d\n", pdimm->mirrored_dimm);
	debug("rc 0x%x\n", pdimm->rc);

	/* SDRAM device parameters */
	pdimm->n_row_addr = ((spd->addressing >> 3) & 0x7) + 12;
	debug("n_row_addr %d\n", pdimm->n_row_addr);
	pdimm->n_col_addr = (spd->addressing & 0x7) + 9;
	debug("n_col_addr %d\n", pdimm->n_col_addr);
	pdimm->bank_addr_bits = (spd->density_banks >> 4) & 0x3;
	debug("bank_addr_bits %d\n", pdimm->bank_addr_bits);
	pdimm->bank_group_bits = (spd->density_banks >> 6) & 0x3;
	debug("bank_group_bits %d\n", pdimm->bank_group_bits);

	if (pdimm->ec_sdram_width != 0) {
		pdimm->edc_config = 0x02;
	} else {
		pdimm->edc_config = 0x00;
	}
	debug("edc_config %d\n", pdimm->edc_config);

	/* DDR4 spec has BL8 -bit3, BC4 -bit2 */
	pdimm->burst_lengths_bitmask = 0x0c;
	debug("burst_lengths_bitmask 0x%x\n", pdimm->burst_lengths_bitmask);

	/* MTB - medium timebase
	 * The MTB in the SPD spec is 125ps,
	 *
	 * FTB - fine timebase
	 * use 1/10th of ps as our unit to avoid floating point
	 * eg, 10 for 1ps, 25 for 2.5ps, 50 for 5ps
	 */
	if ((spd->timebases & 0xf) == 0x0) {
		pdimm->mtb_ps = 125;
		pdimm->ftb_10th_ps = 10;

	} else {
		ERROR("Unknown Timebases\n");
		return -EINVAL;
	}

	/* sdram minimum cycle time */
	pdimm->tckmin_x_ps = spd_to_ps(spd->tck_min, spd->fine_tck_min);
	debug("tckmin_x_ps %d\n", pdimm->tckmin_x_ps);

	/* sdram max cycle time */
	pdimm->tckmax_ps = spd_to_ps(spd->tck_max, spd->fine_tck_max);
	debug("tckmax_ps %d\n", pdimm->tckmax_ps);

	/*
	 * CAS latency supported
	 * bit0 - CL7
	 * bit4 - CL11
	 * bit8 - CL15
	 * bit12- CL19
	 * bit16- CL23
	 */
	pdimm->caslat_x  = (spd->caslat_b1 << 7)	|
			   (spd->caslat_b2 << 15)	|
			   (spd->caslat_b3 << 23);
	debug("caslat_x 0x%x\n", pdimm->caslat_x);

	if (spd->caslat_b4 != 0) {
		WARN("Unhandled caslat_b4 value\n");
	}

	/*
	 * min CAS latency time
	 */
	pdimm->taa_ps = spd_to_ps(spd->taa_min, spd->fine_taa_min);
	debug("taa_ps %d\n", pdimm->taa_ps);

	/*
	 * min RAS to CAS delay time
	 */
	pdimm->trcd_ps = spd_to_ps(spd->trcd_min, spd->fine_trcd_min);
	debug("trcd_ps %d\n", pdimm->trcd_ps);

	/*
	 * Min Row Precharge Delay Time
	 */
	pdimm->trp_ps = spd_to_ps(spd->trp_min, spd->fine_trp_min);
	debug("trp_ps %d\n", pdimm->trp_ps);

	/* min active to precharge delay time */
	pdimm->tras_ps = (((spd->tras_trc_ext & 0xf) << 8) +
			  spd->tras_min_lsb) * pdimm->mtb_ps;
	debug("tras_ps %d\n", pdimm->tras_ps);

	/* min active to actice/refresh delay time */
	pdimm->trc_ps = spd_to_ps((((spd->tras_trc_ext & 0xf0) << 4) +
				   spd->trc_min_lsb), spd->fine_trc_min);
	debug("trc_ps %d\n", pdimm->trc_ps);
	/* Min Refresh Recovery Delay Time */
	pdimm->trfc1_ps = ((spd->trfc1_min_msb << 8) | (spd->trfc1_min_lsb)) *
		       pdimm->mtb_ps;
	debug("trfc1_ps %d\n", pdimm->trfc1_ps);
	pdimm->trfc2_ps = ((spd->trfc2_min_msb << 8) | (spd->trfc2_min_lsb)) *
		       pdimm->mtb_ps;
	debug("trfc2_ps %d\n", pdimm->trfc2_ps);
	pdimm->trfc4_ps = ((spd->trfc4_min_msb << 8) | (spd->trfc4_min_lsb)) *
			pdimm->mtb_ps;
	debug("trfc4_ps %d\n", pdimm->trfc4_ps);
	/* min four active window delay time */
	pdimm->tfaw_ps = (((spd->tfaw_msb & 0xf) << 8) | spd->tfaw_min) *
			pdimm->mtb_ps;
	debug("tfaw_ps %d\n", pdimm->tfaw_ps);

	/* min row active to row active delay time, different bank group */
	pdimm->trrds_ps = spd_to_ps(spd->trrds_min, spd->fine_trrds_min);
	debug("trrds_ps %d\n", pdimm->trrds_ps);
	/* min row active to row active delay time, same bank group */
	pdimm->trrdl_ps = spd_to_ps(spd->trrdl_min, spd->fine_trrdl_min);
	debug("trrdl_ps %d\n", pdimm->trrdl_ps);
	/* min CAS to CAS Delay Time (tCCD_Lmin), same bank group */
	pdimm->tccdl_ps = spd_to_ps(spd->tccdl_min, spd->fine_tccdl_min);
	debug("tccdl_ps %d\n", pdimm->tccdl_ps);
	if (pdimm->package_3ds != 0) {
		if (pdimm->die_density > 5) {
			debug("Unsupported logical rank density 0x%x\n",
				  pdimm->die_density);
			return -EINVAL;
		}
		pdimm->trfc_slr_ps = (pdimm->die_density <= 4) ?
				     260000 : 350000;
	}
	debug("trfc_slr_ps %d\n", pdimm->trfc_slr_ps);

	/* 15ns for all speed bins */
	pdimm->twr_ps = 15000;
	debug("twr_ps %d\n", pdimm->twr_ps);

	/*
	 * Average periodic refresh interval
	 * tREFI = 7.8 us at normal temperature range
	 */
	pdimm->refresh_rate_ps = 7800000;
	debug("refresh_rate_ps %d\n", pdimm->refresh_rate_ps);

	for (i = 0; i < 18; i++) {
		pdimm->dq_mapping[i] = spd->mapping[i];
		debug("dq_mapping 0x%x\n", pdimm->dq_mapping[i]);
	}

	pdimm->dq_mapping_ors = ((spd->mapping[0] >> 6) & 0x3) == 0 ? 1 : 0;
	debug("dq_mapping_ors %d\n", pdimm->dq_mapping_ors);

	return 0;
}
