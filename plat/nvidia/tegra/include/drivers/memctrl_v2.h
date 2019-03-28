/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMCTRL_V2_H
#define MEMCTRL_V2_H

#include <tegra_def.h>

#ifndef __ASSEMBLY__

#include <lib/mmio.h>
#include <stdint.h>

/*******************************************************************************
 * Structure to hold the transaction override settings to use to override
 * client inputs
 ******************************************************************************/
typedef struct mc_txn_override_cfg {
	uint32_t offset;
	uint8_t cgid_tag;
} mc_txn_override_cfg_t;

#define mc_make_txn_override_cfg(off, val) \
	{ \
		.offset = MC_TXN_OVERRIDE_CONFIG_ ## off, \
		.cgid_tag = MC_TXN_OVERRIDE_ ## val \
	}

/*******************************************************************************
 * Structure to hold the Stream ID to use to override client inputs
 ******************************************************************************/
typedef struct mc_streamid_override_cfg {
	uint32_t offset;
	uint8_t stream_id;
} mc_streamid_override_cfg_t;

/*******************************************************************************
 * Structure to hold the Stream ID Security Configuration settings
 ******************************************************************************/
typedef struct mc_streamid_security_cfg {
	char *name;
	uint32_t offset;
	int override_enable;
	int override_client_inputs;
	int override_client_ns_flag;
} mc_streamid_security_cfg_t;

#define OVERRIDE_DISABLE				1U
#define OVERRIDE_ENABLE					0U
#define CLIENT_FLAG_SECURE				0U
#define CLIENT_FLAG_NON_SECURE				1U
#define CLIENT_INPUTS_OVERRIDE				1U
#define CLIENT_INPUTS_NO_OVERRIDE			0U
/*******************************************************************************
 * StreamID to indicate no SMMU translations (requests to be steered on the
 * SMMU bypass path)
 ******************************************************************************/
#define MC_STREAM_ID_MAX			0x7FU

/*******************************************************************************
 * Memory Controller SMMU Bypass config register
 ******************************************************************************/
#define MC_SMMU_BYPASS_CONFIG			0x1820U
#define MC_SMMU_BYPASS_CTRL_MASK		0x3U
#define MC_SMMU_BYPASS_CTRL_SHIFT		0U
#define MC_SMMU_CTRL_TBU_BYPASS_ALL		(0U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_RSVD			(1U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID	(2U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_CTRL_TBU_BYPASS_NONE		(3U << MC_SMMU_BYPASS_CTRL_SHIFT)
#define MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT	(1U << 31)
#define MC_SMMU_BYPASS_CONFIG_SETTINGS		(MC_SMMU_BYPASS_CONFIG_WRITE_ACCESS_BIT | \
						 MC_SMMU_CTRL_TBU_BYPASS_SPL_STREAMID)

#define mc_make_sec_cfg(off, ns, ovrrd, access) \
	{ \
		.name = # off, \
		.offset = MC_STREAMID_OVERRIDE_TO_SECURITY_CFG( \
				MC_STREAMID_OVERRIDE_CFG_ ## off), \
		.override_client_ns_flag = CLIENT_FLAG_ ## ns, \
		.override_client_inputs = CLIENT_INPUTS_ ## ovrrd, \
		.override_enable = OVERRIDE_ ## access \
	}

/*******************************************************************************
 * Structure to hold Memory Controller's Configuration settings
 ******************************************************************************/
typedef struct tegra_mc_settings {
	const uint32_t *streamid_override_cfg;
	uint32_t num_streamid_override_cfgs;
	const mc_streamid_security_cfg_t *streamid_security_cfg;
	uint32_t num_streamid_security_cfgs;
	const mc_txn_override_cfg_t *txn_override_cfg;
	uint32_t num_txn_override_cfgs;
	void (*reconfig_mss_clients)(void);
	void (*set_txn_overrides)(void);
} tegra_mc_settings_t;

static inline uint32_t tegra_mc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_BASE + off);
}

static inline void tegra_mc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_BASE + off, val);
}

static inline uint32_t tegra_mc_streamid_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_MC_STREAMID_BASE + off);
}

static inline void tegra_mc_streamid_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_MC_STREAMID_BASE + off, val);
}

#define mc_set_pcfifo_unordered_boot_so_mss(id, client) \
	((uint32_t)~MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_MASK | \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_UNORDERED)

#define mc_set_pcfifo_ordered_boot_so_mss(id, client) \
	 MC_PCFIFO_CLIENT_CONFIG##id##_PCFIFO_##client##_ORDERED

#define mc_set_tsa_passthrough(client) \
	{ \
		mmio_write_32(TEGRA_TSA_BASE + TSA_CONFIG_STATIC0_CSW_##client, \
			(TSA_CONFIG_STATIC0_CSW_##client##_RESET & \
			 (uint32_t)~TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK) | \
			(uint32_t)TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU); \
	}

#define mc_set_tsa_w_passthrough(client) \
	{ \
		mmio_write_32(TEGRA_TSA_BASE + TSA_CONFIG_STATIC0_CSW_##client, \
			(TSA_CONFIG_STATIC0_CSW_RESET_W & \
			 (uint32_t)~TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK) | \
			(uint32_t)TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU); \
	}

#define mc_set_tsa_r_passthrough(client) \
	{ \
		mmio_write_32(TEGRA_TSA_BASE + TSA_CONFIG_STATIC0_CSR_##client, \
			(TSA_CONFIG_STATIC0_CSR_RESET_R & \
			 (uint32_t)~TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_MASK) | \
			(uint32_t)TSA_CONFIG_CSW_MEMTYPE_OVERRIDE_PASTHRU); \
	}

#define mc_set_txn_override(client, normal_axi_id, so_dev_axi_id, normal_override, so_dev_override) \
	{ \
		tegra_mc_write_32(MC_TXN_OVERRIDE_CONFIG_##client, \
				  MC_TXN_OVERRIDE_##normal_axi_id | \
				  MC_TXN_OVERRIDE_CONFIG_COH_PATH_##so_dev_override##_SO_DEV | \
				  MC_TXN_OVERRIDE_CONFIG_COH_PATH_##normal_override##_NORMAL | \
				  MC_TXN_OVERRIDE_CONFIG_CGID_##so_dev_axi_id); \
	}

/*******************************************************************************
 * Handler to read memory configuration settings
 *
 * Implemented by SoCs under tegra/soc/txxx
 ******************************************************************************/
tegra_mc_settings_t *tegra_get_mc_settings(void);

/*******************************************************************************
 * Handler to program the scratch registers with TZDRAM settings for the
 * resume firmware.
 *
 * Implemented by SoCs under tegra/soc/txxx
 ******************************************************************************/
void plat_memctrl_tzdram_setup(uint64_t phys_base, uint64_t size_in_bytes);

#endif /* __ASSEMBLY__ */

#endif /* MEMCTRL_V2_H */
