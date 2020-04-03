/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <iommu.h>
#include <platform_def.h>
#include <sr_utils.h>

#define PAXC_BASE                  0x60400000
#define PAXC_AXI_CFG_PF            0x10
#define PAXC_AXI_CFG_PF_OFFSET(pf) (PAXC_AXI_CFG_PF + (pf) * 4)
#define PAXC_ARPROT_PF_CFG         0x40
#define PAXC_AWPROT_PF_CFG         0x44

#define PAXC_ARQOS_PF_CFG          0x48
#define PAXC_ARQOS_VAL             0xaaaaaaaa

#define PAXC_AWQOS_PF_CFG          0x4c
#define PAXC_AWQOS_VAL             0xeeeeeeee

#define PAXC_CFG_IND_ADDR_OFFSET   0x1f0
#define PAXC_CFG_IND_ADDR_MASK     0xffc
#define PAXC_CFG_IND_DATA_OFFSET   0x1f4

/* offsets for PAXC root complex configuration space registers */

#define PAXC_CFG_ID_OFFSET           0x434
#define PAXC_RC_VENDOR_ID            0x14e4
#define PAXC_RC_VENDOR_ID_SHIFT      16

#define PAXC_RC_DEVICE_ID            0xd750

#define PAXC_CFG_LINK_CAP_OFFSET     0x4dc
#define PAXC_RC_LINK_CAP_SPD_SHIFT   0
#define PAXC_RC_LINK_CAP_SPD_MASK    (0xf << PAXC_RC_LINK_CAP_SPD_SHIFT)
#define PAXC_RC_LINK_CAP_SPD         3
#define PAXC_RC_LINK_CAP_WIDTH_SHIFT 4
#define PAXC_RC_LINK_CAP_WIDTH_MASK  (0x1f << PAXC_RC_LINK_CAP_WIDTH_SHIFT)
#define PAXC_RC_LINK_CAP_WIDTH       16

/* offsets for MHB registers */

#define MHB_BASE                   0x60401000
#define MHB_MEM_PWR_STATUS_PAXC    (MHB_BASE + 0x1c0)
#define MHB_PWR_ARR_POWERON        0x8
#define MHB_PWR_ARR_POWEROK        0x4
#define MHB_PWR_POWERON            0x2
#define MHB_PWR_POWEROK            0x1
#define MHB_PWR_STATUS_MASK        (MHB_PWR_ARR_POWERON | \
				    MHB_PWR_ARR_POWEROK | \
				    MHB_PWR_POWERON | \
				    MHB_PWR_POWEROK)

/* max number of PFs from Nitro that PAXC sees */
#define MAX_NR_NITRO_PF            8

#ifdef EMULATION_SETUP
static void paxc_reg_dump(void)
{
}
#else
/* total number of PAXC registers */
#define NR_PAXC_REGS               53
static void paxc_reg_dump(void)
{
	uint32_t idx, offset = 0;

	VERBOSE("PAXC register dump start\n");
	for (idx = 0; idx < NR_PAXC_REGS; idx++, offset += 4)
		VERBOSE("offset: 0x%x val: 0x%x\n", offset,
			mmio_read_32(PAXC_BASE + offset));
	VERBOSE("PAXC register dump end\n");
}
#endif /* EMULATION_SETUP */

#ifdef EMULATION_SETUP
static void mhb_reg_dump(void)
{
}
#else
#define NR_MHB_REGS                227
static void mhb_reg_dump(void)
{
	uint32_t idx, offset = 0;

	VERBOSE("MHB register dump start\n");
	for (idx = 0; idx < NR_MHB_REGS; idx++, offset += 4)
		VERBOSE("offset: 0x%x val: 0x%x\n", offset,
			mmio_read_32(MHB_BASE + offset));
	VERBOSE("MHB register dump end\n");
}
#endif /* EMULATION_SETUP */

static void paxc_rc_cfg_write(uint32_t where, uint32_t val)
{
	mmio_write_32(PAXC_BASE + PAXC_CFG_IND_ADDR_OFFSET,
		      where & PAXC_CFG_IND_ADDR_MASK);
	mmio_write_32(PAXC_BASE + PAXC_CFG_IND_DATA_OFFSET, val);
}

static uint32_t paxc_rc_cfg_read(uint32_t where)
{
	mmio_write_32(PAXC_BASE + PAXC_CFG_IND_ADDR_OFFSET,
		      where & PAXC_CFG_IND_ADDR_MASK);
	return mmio_read_32(PAXC_BASE + PAXC_CFG_IND_DATA_OFFSET);
}

/*
 * Function to program PAXC root complex link capability register
 */
static void paxc_cfg_link_cap(void)
{
	uint32_t val;

	val = paxc_rc_cfg_read(PAXC_CFG_LINK_CAP_OFFSET);
	val &= ~(PAXC_RC_LINK_CAP_SPD_MASK | PAXC_RC_LINK_CAP_WIDTH_MASK);
	val |= (PAXC_RC_LINK_CAP_SPD << PAXC_RC_LINK_CAP_SPD_SHIFT) |
		(PAXC_RC_LINK_CAP_WIDTH << PAXC_RC_LINK_CAP_WIDTH_SHIFT);
	paxc_rc_cfg_write(PAXC_CFG_LINK_CAP_OFFSET, val);
}

/*
 * Function to program PAXC root complex vendor ID and device ID
 */
static void paxc_cfg_id(void)
{
	uint32_t val;

	val = (PAXC_RC_VENDOR_ID << PAXC_RC_VENDOR_ID_SHIFT) |
		PAXC_RC_DEVICE_ID;
	paxc_rc_cfg_write(PAXC_CFG_ID_OFFSET, val);
}

void paxc_init(void)
{
	unsigned int pf_index;
	unsigned int val;

	val = mmio_read_32(MHB_MEM_PWR_STATUS_PAXC);
	if ((val & MHB_PWR_STATUS_MASK) != MHB_PWR_STATUS_MASK) {
		INFO("PAXC not powered\n");
		return;
	}

	paxc_cfg_id();
	paxc_cfg_link_cap();

	paxc_reg_dump();
	mhb_reg_dump();

#ifdef USE_DDR
	/*
	 * Set AWCACHE and ARCACHE to 0xff (Cacheable write-back,
	 * allocate on both reads and writes) per
	 * recommendation from the ASIC team
	 */
	val = 0xff;
#else
	/* disable IO cache if non-DDR memory is used, e.g., external SRAM */
	val = 0x0;
#endif
	for (pf_index = 0; pf_index < MAX_NR_NITRO_PF; pf_index++)
		mmio_write_32(PAXC_BASE + PAXC_AXI_CFG_PF_OFFSET(pf_index),
			      val);

	/*
	 * Set ARPROT and AWPROT to enable non-secure access from
	 * PAXC to all PFs, PF0 to PF7
	 */
	mmio_write_32(PAXC_BASE + PAXC_ARPROT_PF_CFG, 0x22222222);
	mmio_write_32(PAXC_BASE + PAXC_AWPROT_PF_CFG, 0x22222222);

	mmio_write_32(PAXC_BASE + PAXC_ARQOS_PF_CFG, PAXC_ARQOS_VAL);
	mmio_write_32(PAXC_BASE + PAXC_AWQOS_PF_CFG, PAXC_AWQOS_VAL);

	INFO("PAXC init done\n");
}

/*
 * These defines do not match the regfile but they are renamed in a way such
 * that they are much more readible
 */

#define MHB_NIC_SECURITY_BASE  0x60500000
#define MHB_NIC_PAXC_AXI_NS    0x0008
#define MHB_NIC_IDM_NS         0x000c
#define MHB_NIC_MHB_APB_NS     0x0010
#define MHB_NIC_NITRO_AXI_NS   0x0014
#define MHB_NIC_PCIE_AXI_NS    0x0018
#define MHB_NIC_PAXC_APB_NS    0x001c
#define MHB_NIC_EP_APB_NS      0x0020

#define MHB_NIC_PAXC_APB_S_IDM_SHIFT     5
#define MHB_NIC_EP_APB_S_IDM_SHIFT       4
#define MHB_NIC_MHB_APB_S_IDM_SHIFT      3
#define MHB_NIC_PAXC_AXI_S_IDM_SHIFT     2
#define MHB_NIC_PCIE_AXI_S_IDM_SHIFT     1
#define MHB_NIC_NITRO_AXI_S_IDM_SHIFT    0

#define NIC400_NITRO_TOP_NIC_SECURITY_BASE 0x60d00000

#define NITRO_NIC_SECURITY_3_SHIFT       0x14
#define NITRO_NIC_SECURITY_4_SHIFT       0x18
#define NITRO_NIC_SECURITY_5_SHIFT       0x1c
#define NITRO_NIC_SECURITY_6_SHIFT       0x20

void paxc_mhb_ns_init(void)
{
	unsigned int val;
	uintptr_t mhb_nic_gpv = MHB_NIC_SECURITY_BASE;
#ifndef NITRO_SECURE_ACCESS
	uintptr_t nic400_nitro_gpv = NIC400_NITRO_TOP_NIC_SECURITY_BASE;
#endif /* NITRO_SECURE_ACCESS */

	/* set PAXC AXI to allow non-secure access */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_PAXC_AXI_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_PAXC_AXI_NS, val);

	/* set various MHB IDM interfaces to allow non-secure access */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_IDM_NS);
	val |= (0x1 << MHB_NIC_PAXC_APB_S_IDM_SHIFT);
	val |= (0x1 << MHB_NIC_EP_APB_S_IDM_SHIFT);
	val |= (0x1 << MHB_NIC_MHB_APB_S_IDM_SHIFT);
	val |= (0x1 << MHB_NIC_PAXC_AXI_S_IDM_SHIFT);
	val |= (0x1 << MHB_NIC_PCIE_AXI_S_IDM_SHIFT);
	val |= (0x1 << MHB_NIC_NITRO_AXI_S_IDM_SHIFT);
	mmio_write_32(mhb_nic_gpv + MHB_NIC_IDM_NS, val);

	/* set MHB APB to allow non-secure access  */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_MHB_APB_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_MHB_APB_NS, val);

	/* set Nitro AXI to allow non-secure access  */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_NITRO_AXI_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_NITRO_AXI_NS, val);

	/* set PCIe AXI to allow non-secure access  */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_PCIE_AXI_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_PCIE_AXI_NS, val);

	/* set PAXC APB to allow non-secure access  */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_PAXC_APB_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_PAXC_APB_NS, val);

	/* set EP APB to allow non-secure access  */
	val = mmio_read_32(mhb_nic_gpv + MHB_NIC_EP_APB_NS);
	val |= 0x1;
	mmio_write_32(mhb_nic_gpv + MHB_NIC_EP_APB_NS, val);

#ifndef NITRO_SECURE_ACCESS
	/* Set NIC400 to allow non-secure access */
	mmio_setbits_32(nic400_nitro_gpv + NITRO_NIC_SECURITY_3_SHIFT, 0x1);
	mmio_setbits_32(nic400_nitro_gpv + NITRO_NIC_SECURITY_4_SHIFT, 0x1);
	mmio_setbits_32(nic400_nitro_gpv + NITRO_NIC_SECURITY_5_SHIFT, 0x1);
	mmio_setbits_32(nic400_nitro_gpv + NITRO_NIC_SECURITY_6_SHIFT, 0x1);
#endif /* NITRO_SECURE_ACCESS */
}
