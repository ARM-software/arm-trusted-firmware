/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <common/debug.h>
#include <errno.h>
#include <lib/mmio.h>
#include <platform_def.h>

#include "ncore_ccu.h"
#include "socfpga_plat_def.h"
#include "socfpga_system_manager.h"

uint32_t poll_active_bit(uint32_t dir);

#define SMMU_DMI			1


static coh_ss_id_t subsystem_id;
void get_subsystem_id(void)
{
	uint32_t snoop_filter, directory, coh_agent;
	snoop_filter = CSIDR_NUM_SF(mmio_read_32(NCORE_CCU_CSR(NCORE_CSIDR)));
	directory = CSUIDR_NUM_DIR(mmio_read_32(NCORE_CCU_CSR(NCORE_CSUIDR)));
	coh_agent = CSUIDR_NUM_CAI(mmio_read_32(NCORE_CCU_CSR(NCORE_CSUIDR)));
	subsystem_id.num_snoop_filter = snoop_filter + 1;
	subsystem_id.num_directory = directory;
	subsystem_id.num_coh_agent = coh_agent;
}
uint32_t directory_init(void)
{
	uint32_t dir_sf_mtn, dir_sf_en;
	uint32_t dir, sf, ret;
	for (dir = 0; dir < subsystem_id.num_directory; dir++) {
		for (sf = 0; sf < subsystem_id.num_snoop_filter; sf++) {
			dir_sf_mtn = DIRECTORY_UNIT(dir, NCORE_DIRUSFMCR);
			dir_sf_en = DIRECTORY_UNIT(dir, NCORE_DIRUSFER);
			/* Initialize All Entries */
			mmio_write_32(dir_sf_mtn, SNOOP_FILTER_ID(sf));
			/* Poll Active Bit */
			ret = poll_active_bit(dir);
			if (ret != 0) {
				ERROR("Timeout during active bit polling");
				return -ETIMEDOUT;
			}
			/* Disable snoop filter, a bit per snoop filter */
			mmio_clrbits_32(dir_sf_en, BIT(sf));
		}
	}
	return 0;
}
uint32_t coherent_agent_intfc_init(void)
{
	uint32_t dir, ca, ca_id, ca_type, ca_snoop_en;
	for (dir = 0; dir < subsystem_id.num_directory; dir++) {
		for (ca = 0; ca < subsystem_id.num_coh_agent; ca++) {
			ca_snoop_en = DIRECTORY_UNIT(ca, NCORE_DIRUCASER0);
			ca_id = mmio_read_32(COH_AGENT_UNIT(ca, NCORE_CAIUIDR));
			/* Coh Agent Snoop Enable */
			if (CACHING_AGENT_BIT(ca_id))
				mmio_setbits_32(ca_snoop_en, BIT(ca));
			/* Coh Agent Snoop DVM Enable */
			ca_type = CACHING_AGENT_TYPE(ca_id);
			if (ca_type == ACE_W_DVM || ca_type == ACE_L_W_DVM)
				mmio_setbits_32(NCORE_CCU_CSR(NCORE_CSADSER0),
				BIT(ca));
		}
	}
	return 0;
}
uint32_t poll_active_bit(uint32_t dir)
{
	uint32_t timeout = 80000;
	uint32_t poll_dir =  DIRECTORY_UNIT(dir, NCORE_DIRUSFMAR);
	while (timeout > 0) {
		if (mmio_read_32(poll_dir) == 0)
			return 0;
		timeout--;
	}
	return -1;
}
void bypass_ocram_firewall(void)
{
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF1),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF2),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF3),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_clrbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF4),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
}
void ncore_enable_ocram_firewall(void)
{
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF1),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF2),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF3),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
	mmio_setbits_32(COH_CPU0_BYPASS_REG(NCORE_FW_OCRAM_BLK_CGF4),
			OCRAM_PRIVILEGED_MASK | OCRAM_SECURE_MASK);
}
uint32_t init_ncore_ccu(void)
{
	uint32_t status;
	get_subsystem_id();
	status = directory_init();
	status = coherent_agent_intfc_init();
	bypass_ocram_firewall();
	return status;
}

void setup_smmu_stream_id(void)
{
	/* Configure Stream ID for Agilex5 */
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_ID_AX_REG_0_DMA0), DMA0);
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_ID_AX_REG_0_DMA1), DMA1);
	mmio_write_32(SOCFPGA_SYSMGR(SDM_TBU_STREAM_ID_AX_REG_1_SDM), SDM);
	/* Reg map showing USB2 but Linux USB0? */
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_USB2), USB0);
	/* Reg map showing USB3 but Linux USB1? */
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_USB3), USB1);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_SDMMC), SDMMC);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_NAND), NAND);
	/* To confirm ETR - core sight debug*/
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_ETR), CORE_SIGHT_DEBUG);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN0), TSN0);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN1), TSN1);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_ID_AX_REG_2_TSN2), TSN2);

	/* Enabled Stream ctrl register for Agilex5 */
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_CTRL_REG_0_DMA0), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(DMA_TBU_STREAM_CTRL_REG_0_DMA1), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(SDM_TBU_STREAM_CTRL_REG_1_SDM), ENABLE_STREAMID_SECURE_TX);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_USB2), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_USB3), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_SDMMC), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_NAND), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(IO_TBU_STREAM_CTRL_REG_2_ETR), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN0), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN1), ENABLE_STREAMID);
	mmio_write_32(SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN2), ENABLE_STREAMID);
}
