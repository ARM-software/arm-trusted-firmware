/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/st/stm32mp_tamp_nvram.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define BKPREG_ZONE_1_RIF1 U(0)
#define BKPREG_ZONE_1_RIF2 U(1)
#define BKPREG_ZONE_2_RIF1 U(2)
#define BKPREG_ZONE_2_RIF2 U(3)
#define BKPREG_ZONE_3_RIF1 U(4)
#define BKPREG_ZONE_3_RIF0 U(5)
#define BKPREG_ZONE_3_RIF2 U(6)
#define NB_CMPT_MP2 U(3)

static const struct stm32_tamp_nvram_reg_field
	mp25_zone_cfg_fields[MAX_TAMP_BACKUP_REGS_ZONES - 1] = {
		[BKPREG_ZONE_1_RIF1] = REG_FIELD(TAMP_BKPRIFR(1), U(0), U(7)),
		[BKPREG_ZONE_1_RIF2] = REG_FIELD(TAMP_SECCFGR, U(0), U(7)),
		[BKPREG_ZONE_2_RIF1] = REG_FIELD(TAMP_BKPRIFR(2), U(0), U(7)),
		[BKPREG_ZONE_2_RIF2] = REG_FIELD(TAMP_SECCFGR, U(16), U(23)),
		[BKPREG_ZONE_3_RIF1] = REG_FIELD(TAMP_BKPRIFR(3), U(0), U(7)),
		[BKPREG_ZONE_3_RIF0] = REG_FIELD(TAMP_BKPRIFR(3), U(16), U(23)),
	};

static const struct stm32_tamp_nvram_reg_field
	mp25_rxcidcfg_cfen_fields[NB_CMPT_MP2] = {
		REG_FIELD(TAMP_RXCIDCFGR(0), U(0), U(0)),
		REG_FIELD(TAMP_RXCIDCFGR(1), U(0), U(0)),
		REG_FIELD(TAMP_RXCIDCFGR(2), U(0), U(0)),
	};

static const struct stm32_tamp_nvram_reg_field
	mp25_rxcidcfg_fields[NB_CMPT_MP2] = {
		REG_FIELD(TAMP_RXCIDCFGR(0), U(4), U(6)),
		REG_FIELD(TAMP_RXCIDCFGR(1), U(4), U(6)),
		REG_FIELD(TAMP_RXCIDCFGR(2), U(4), U(6)),
	};

static const struct stm32_tamp_nvram_cdata stm32mp25_tamp_nvram = {
	.nb_zones = MAX_TAMP_BACKUP_REGS_ZONES,
	.zone_cfg = mp25_zone_cfg_fields,
};

const struct stm32_tamp_nvram_cdata *stm32_tamp_nvram_get_cdata(void)
{
	return &stm32mp25_tamp_nvram;
}

static int stm32_tamp_nvram_is_isolation_en_mp2(const struct stm32_tamp_nvram_drv_data *drv_data)
{
	unsigned int nb_compartment_enabled = 0;
	uint32_t cfen = 0;
	struct stm32_tamp_nvram_reg_field cfen_field = REG_FIELD_NULL;
	int ret = 1;

	for (unsigned int i = 0; i < NB_CMPT_MP2; i++) {
		cfen_field = mp25_rxcidcfg_cfen_fields[i];
		if (stm32_tamp_nvram_reg_field_read(drv_data->parent_base,
						    cfen_field, &cfen) != 0) {
			NOTICE("TFA: Can't read field for registers zones\n");
			nb_compartment_enabled = UINT_MAX;
			break;
		}

		nb_compartment_enabled += cfen;
	}

	if (nb_compartment_enabled == U(0)) {
		ret = 0;
	} else if (nb_compartment_enabled == NB_CMPT_MP2) {
		ret = 1;
	} else {
		ret = -1;
	}

	return ret;
}

static bool *
stm32_tamp_get_cmpt_owner_mp2(struct stm32_tamp_nvram_drv_data *drv_data)
{
	struct stm32_tamp_nvram_reg_field cid_field = REG_FIELD_NULL;
	uint32_t cid_per_zone = U(0);
	int isolation_enabled = 0;
	static bool compartment_owner[3];
	unsigned int i = 0;
	bool *ret = NULL;

	isolation_enabled = stm32_tamp_nvram_is_isolation_en_mp2(drv_data);
	if (isolation_enabled >= 0) {
		ret = compartment_owner;
		for (i = U(0); i < NB_CMPT_MP2; i++) {
			if (isolation_enabled != 0) {
				cid_field = mp25_rxcidcfg_fields[i];

				if (stm32_tamp_nvram_reg_field_read(drv_data->parent_base,
								    cid_field,
								    &cid_per_zone) != 0) {
					NOTICE("TFA: Can't read field for registers zones\n");
					ret = NULL;
					break;
				}

				if (cid_per_zone == CURRENT_CID) {
					compartment_owner[i] = true;
				} else {
					compartment_owner[i] = false;
				}
			} else {
				compartment_owner[i] = true;
			}
		}
	}

	return ret;
}

enum stm32_tamp_bkpreg_access *
stm32_tamp_nvram_get_access_rights(struct stm32_tamp_nvram_drv_data *drv_data)
{
	static enum stm32_tamp_bkpreg_access static_bkpreg_access[MAX_TAMP_BACKUP_REGS_ZONES];
	unsigned int nb_zones = drv_data->cdata->nb_zones;
	enum stm32_tamp_bkpreg_access *bkpreg_access = static_bkpreg_access;

	drv_data->compartment_owner = stm32_tamp_get_cmpt_owner_mp2(drv_data);
	if (drv_data->compartment_owner != NULL) {
		bkpreg_access = static_bkpreg_access;
		for (unsigned int protection_zone_idx = 0;
		     (protection_zone_idx < nb_zones); protection_zone_idx++) {
			switch (protection_zone_idx) {
			case BKPREG_ZONE_1_RIF1:
				if (drv_data->compartment_owner[U(1)]) {
					bkpreg_access[protection_zone_idx] =
						RSWS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_1_RIF2:
				if (drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RSWS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_2_RIF1:
				if (drv_data->compartment_owner[U(1)]) {
					bkpreg_access[protection_zone_idx] =
						RNSWS;
				} else if (drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RONS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_2_RIF2:
				if (drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RNSWS;
				} else if (drv_data->compartment_owner[U(1)]) {
					bkpreg_access[protection_zone_idx] =
						RONS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_3_RIF1:
				if (drv_data->compartment_owner[U(1)]) {
					bkpreg_access[protection_zone_idx] =
						RNSWNS;
				} else if (drv_data->compartment_owner[U(0)] ||
					   drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RONS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_3_RIF0:
				if (drv_data->compartment_owner[U(0)]) {
					bkpreg_access[protection_zone_idx] =
						RNSWNS;
				} else if (drv_data->compartment_owner[U(1)] ||
					   drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RONS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			case BKPREG_ZONE_3_RIF2:
				if (drv_data->compartment_owner[U(2)]) {
					bkpreg_access[protection_zone_idx] =
						RNSWNS;
				} else if (drv_data->compartment_owner[U(0)] ||
					   drv_data->compartment_owner[U(1)]) {
					bkpreg_access[protection_zone_idx] =
						RONS;
				} else {
					bkpreg_access[protection_zone_idx] =
						NORNOW;
				}
				break;
			default:
				bkpreg_access = NULL;
				break;
			}

			if (bkpreg_access == NULL) {
				break;
			}
		}
	}

	return bkpreg_access;
}
