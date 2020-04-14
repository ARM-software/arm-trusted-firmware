/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <paxb.h>
#include <sr_def.h>
#include <sr_utils.h>

#define PCIE_CORE_PWR_ARR_POWERON        0x8
#define PCIE_CORE_PWR_ARR_POWEROK        0x4
#define PCIE_CORE_PWR_POWERON            0x2
#define PCIE_CORE_PWR_POWEROK            0x1

#define PCIE_CORE_USER_CFG               (PCIE_CORE_BASE + 0x38)
#define PCIE_PAXB_SMMU_SID_CFG           (PCIE_CORE_BASE + 0x60)
#ifdef SID_B8_D1_F1
#define PAXB_SMMU_SID_CFG_BUS_WIDTH      (0x8 << 8)
#define PAXB_SMMU_SID_CFG_DEV_WIDTH      (0x1 << 12)
#define PAXB_SMMU_SID_CFG_FUN_WIDTH      (0x1 << 16)
#else
#define PAXB_SMMU_SID_CFG_BUS_WIDTH      (0x2 << 8)
#define PAXB_SMMU_SID_CFG_DEV_WIDTH      (0x5 << 12)
#define PAXB_SMMU_SID_CFG_FUN_WIDTH      (0x3 << 16)
#endif

#define PAXB_APB_TIMEOUT_COUNT_OFFSET 0x034

/* allow up to 5 ms for each power switch to stabilize */
#define PCIE_CORE_PWR_TIMEOUT_MS      5

/* wait 1 microsecond for PCIe core soft reset */
#define PCIE_CORE_SOFT_RST_DELAY_US 1

/*
 * List of PAXB APB registers
 */
#define PAXB_BASE                        0x48000000
#define PAXB_BASE_OFFSET                 0x4000
#define PAXB_OFFSET(core)                (PAXB_BASE + \
					  (core) * PAXB_BASE_OFFSET)

#define PAXB_CLK_CTRL_OFFSET             0x000
#define PAXB_EP_PERST_SRC_SEL_MASK       (1 << 2)
#define PAXB_EP_MODE_PERST_MASK          (1 << 1)
#define PAXB_RC_PCIE_RST_OUT_MASK        (1 << 0)

#define PAXB_MAX_IMAP_WINDOWS            8
#define PAXB_IMAP_REG_WIDTH              8
#define PAXB_IMAP0_REG_WIDTH             4
#define PAXB_AXUSER_REG_WIDTH            4

#define PAXB_CFG_IND_ADDR_OFFSET         0x120
#define PAXB_CFG_IND_DATA_OFFSET         0x124
#define PAXB_CFG_IND_ADDR_MASK           0x1ffc
#define PAXB_CFG_CFG_TYPE_MASK           0x1

#define PAXB_EP_CFG_ADDR_OFFSET          0x1f8
#define PAXB_EP_CFG_DATA_OFFSET          0x1fc
#define PAXB_EP_CFG_ADDR_MASK            0xffc
#define PAXB_EP_CFG_TYPE_MASK            0x1

#define PAXB_0_DEFAULT_IMAP              0xed0
#define DEFAULT_ADDR_INVALID             BIT(0)
#define PAXB_0_DEFAULT_IMAP_AXUSER       0xed8
#define PAXB_0_DEFAULT_IMAP_AXCACHE      0xedc
#define IMAP_AXCACHE                     0xff
#define OARR_VALID                       BIT(0)
#define IMAP_VALID                       BIT(0)

#define PAXB_IMAP0_BASE_OFFSET           0xc00
#define PAXB_IARR0_BASE_OFFSET           0xd00
#define PAXB_IMAP0_OFFSET(idx)           (PAXB_IMAP0_BASE_OFFSET + \
					  (idx) * PAXB_IMAP0_REG_WIDTH)
#define PAXB_IMAP0_WINDOW_SIZE           0x1000

#define PAXB_IMAP2_OFFSET                0xcc0
#define PAXB_IMAP0_REGS_TYPE_OFFSET      0xcd0
#define PAXB_IARR2_LOWER_OFFSET          0xd10

#define PAXB_IMAP3_BASE_OFFSET           0xe08
#define PAXB_IMAP3_OFFSET(idx)           (PAXB_IMAP3_BASE_OFFSET + \
					  (idx) * PAXB_IMAP_REG_WIDTH)

#define PAXB_IMAP3_0_AXUSER_B_OFFSET     0xe48
#define PAXB_IMAP3_0_AXUSER_OFFSET(idx)  (PAXB_IMAP3_0_AXUSER_B_OFFSET + \
					  (idx) * PAXB_AXUSER_REG_WIDTH)

#define PAXB_IMAP4_BASE_OFFSET           0xe70
#define PAXB_IMAP4_OFFSET(idx)           (PAXB_IMAP4_BASE_OFFSET + \
					  (idx) * PAXB_IMAP_REG_WIDTH)

#define PAXB_IMAP4_0_AXUSER_B_OFFSET     0xeb0
#define PAXB_IMAP4_0_AXUSER_OFFSET(idx)  (PAXB_IMAP4_0_AXUSER_B_OFFSET + \
					  (idx) * PAXB_AXUSER_REG_WIDTH)

#define PAXB_CFG_LINK_STATUS_OFFSET      0xf0c
#define PAXB_CFG_PHYLINKUP_MASK          (1 << 3)
#define PAXB_CFG_DL_ACTIVE_MASK          (1 << 2)

#define PAXB_IMAP0_0_AXUSER_OFFSET       0xf60
#define PAXB_IMAP2_AXUSER_OFFSET         0xfe0

/* cacheable write-back, allocate on both reads and writes */
#define IMAP_ARCACHE                     0x0f0
#define IMAP_AWCACHE                     0xf00
/* normal access, nonsecure access, and data access */
/* AWQOS:0xe and ARQOS:0xa */
/* AWPROT:0x2 and ARPROT:0x1 */
#define IMAP_AXUSER                      0x002e002a

/*
 * List of NIC security and PIPEMUX related registers
 */
#define SR_PCIE_NIC_SECURITY_BASE      0x58100000
#define NS3Z_PCIE_NIC_SECURITY_BASE    0x48100000

#define GITS_TRANSLATER                0x63c30000

#define VENDOR_ID                 0x14e4
#define CFG_RC_DEV_ID             0x434
#define CFG_RC_DEV_SUBID          0x438
#define PCI_BRIDGE_CTRL_REG_OFFSET     0x43c
#define PCI_CLASS_BRIDGE_MASK          0xffff00
#define PCI_CLASS_BRIDGE_SHIFT         8
#define PCI_CLASS_BRIDGE_PCI           0x0604

/*
 * List of PAXB RC configuration space registers
 */

/* first capability list entry */
#define PCI_CAPABILITY_LIST_OFFSET    0x34
#define PCI_CAPABILITY_SPEED_OFFSET   0xc
#define PCI_EP_CAPABILITY_OFFSET      0x10

#define CFG_RC_LINK_STATUS_CTRL_2     0x0dc
#define CFG_RC_LINK_SPEED_SHIFT       0
#define CFG_RC_LINK_SPEED_MASK        (0xf << CFG_RC_LINK_SPEED_SHIFT)

#define CFG_RC_DEVICE_CAP             0x4d4
#define CFG_RC_DEVICE_CAP_MPS_SHIFT   0
#define CFG_RC_DEVICE_CAP_MPS_MASK    (0x7 << CFG_RC_DEVICE_CAP_MPS_SHIFT)
/* MPS 256 bytes */
#define CFG_RC_DEVICE_CAP_MPS_256B    (0x1 << CFG_RC_DEVICE_CAP_MPS_SHIFT)
/* MPS 512 bytes */
#define CFG_RC_DEVICE_CAP_MPS_512B    (0x2 << CFG_RC_DEVICE_CAP_MPS_SHIFT)

#define CFG_RC_TL_FCIMM_NP_LIMIT       0xa10
#define CFG_RC_TL_FCIMM_NP_VAL         0x01500000
#define CFG_RC_TL_FCIMM_P_LIMIT        0xa14
#define CFG_RC_TL_FCIMM_P_VAL          0x03408080

#define CFG_RC_LINK_CAP               0x4dc
#define CFG_RC_LINK_CAP_SPEED_SHIFT   0
#define CFG_RC_LINK_CAP_SPEED_MASK    (0xf << CFG_RC_LINK_CAP_SPEED_SHIFT)
#define CFG_RC_LINK_CAP_WIDTH_SHIFT   4
#define CFG_RC_LINK_CAP_WIDTH_MASK    (0x1f << CFG_RC_LINK_CAP_WIDTH_SHIFT)

#define CFG_LINK_CAP_RC               0x4f0
#define CFG_RC_DL_ACTIVE_SHIFT        0
#define CFG_RC_DL_ACTIVE_MASK         (0x1 << CFG_RC_DL_ACTIVE_SHIFT)
#define CFG_RC_SLOT_CLK_SHIFT         1
#define CFG_RC_SLOT_CLK_MASK          (0x1 << CFG_RC_SLOT_CLK_SHIFT)

#define CFG_ROOT_CAP_RC               0x4f8
#define CFG_ROOT_CAP_LTR_SHIFT        1
#define CFG_ROOT_CAP_LTR_MASK         (0x1 << CFG_ROOT_CAP_LTR_SHIFT)

#define CFG_RC_CLKREQ_ENABLED         0x4fc
#define CFG_RC_CLKREQ_ENABLED_SHIFT   0
#define CFG_RC_CLKREQ_ENABLED_MASK    (0x1 << CFG_RC_CLKREQ_ENABLED_SHIFT)

#define CFG_RC_COEFF_ADDR             0x638

#define CFG_RC_TL_CTRL_0              0x800
#define RC_MEM_DW_CHK_MASK            0x03fe

#define CFG_RC_PDL_CTRL_4             0x1010
#define NPH_FC_INIT_SHIFT             24
#define NPH_FC_INIT_MASK              (U(0xff) << NPH_FC_INIT_SHIFT)
#define PD_FC_INIT_SHIFT              12
#define PD_FC_INIT_MASK               (0xffff << PD_FC_INIT_SHIFT)

#define CFG_RC_PDL_CTRL_5             0x1014
#define PH_INIT_SHIFT                 0
#define PH_INIT_MASK                  (0xff << PH_INIT_SHIFT)

#define DL_STATUS_OFFSET              0x1048
#define PHYLINKUP                     BIT(13)

#define PH_INIT                       0x10
#define PD_FC_INIT                    0x100
#define NPH_FC_INIT                   0x8

#define SRP_PH_INIT                   0x7F
#define SRP_PD_FC_INIT                0x200
#define SRP_NPH_FC_INIT               0x7F

#define CFG_ADDR_BUS_NUM_SHIFT        20
#define CFG_ADDR_DEV_NUM_SHIFT        15
#define CFG_ADDR_FUNC_NUM_SHIFT       12
#define CFG_ADDR_REG_NUM_SHIFT        2
#define CFG_ADDR_REG_NUM_MASK         0x00000ffc
#define CFG_ADDR_CFG_TYPE_MASK        0x00000003

#define DL_LINK_UP_TIMEOUT_MS         1000

#define CFG_RETRY_STATUS              0xffff0001
#define CRS_TIMEOUT_MS                5000

/* create EP config data to write */
#define DEF_BUS_NO                    1 /* default bus 1 */
#define DEF_SLOT_NO                   0 /* default slot 0 */
#define DEF_FN_NO                     0 /* default fn 0 */

#define EP_CONFIG_VAL(bus_no, slot, fn, where) \
	(((bus_no) << CFG_ADDR_BUS_NUM_SHIFT) | \
	((slot) << CFG_ADDR_DEV_NUM_SHIFT) | \
	((fn) << CFG_ADDR_FUNC_NUM_SHIFT) | \
	((where) & CFG_ADDR_REG_NUM_MASK) | \
	(1 & CFG_ADDR_CFG_TYPE_MASK))

/* PAXB security offset */
#define PAXB_SECURITY_IDM_OFFSET 0x1c
#define PAXB_SECURITY_APB_OFFSET 0x24
#define PAXB_SECURITY_ECAM_OFFSET 0x3c

#define paxb_get_config(type) paxb_get_##type##_config()

static unsigned int paxb_sec_reg_offset[] = {
	0x0c, /* PAXB0 AXI */
	0x10, /* PAXB1 AXI */
	0x14, /* PAXB2 AXI */
	0x18, /* PAXB3 AXI */
	0x20, /* PAXB4 AXI */
	0x28, /* PAXB5 AXI */
	0x2c, /* PAXB6 AXI */
	0x30, /* PAXB7 AXI */
	0x24, /* PAXB APB */
};

const paxb_cfg *paxb;

/*
 * Given a PIPEMUX strap and PCIe core index, this function returns 1 if a
 * PCIe core needs to be enabled
 */
int pcie_core_needs_enable(unsigned int core_idx)
{
	if (paxb->core_needs_enable)
		return paxb->core_needs_enable(core_idx);

	return 0;
}

static void pcie_set_default_tx_coeff(uint32_t core_idx, uint32_t link_width)
{
	unsigned int lanes = 0;
	uint32_t data, addr;

	addr = CFG_RC_COEFF_ADDR;
	for (lanes = 0; lanes < link_width; lanes = lanes + 2) {
		data = paxb_rc_cfg_read(core_idx, addr);
		data &= 0xf0f0f0f0;
		data |= (7 & 0xf);
		data |= (7 & 0xf) << 8;
		data |= (7 & 0xf) << 16;
		data |= (7 & 0xf) << 24;

		paxb_rc_cfg_write(core_idx, addr, data);
		addr += 4;
	}
}

static int paxb_rc_link_init(void)
{
	uint32_t val, link_speed;
	unsigned int link_width;
	uint32_t core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		link_width = paxb->get_link_width(core_idx);
		if (!link_width) {
			ERROR("Unsupported PIPEMUX\n");
			return -EOPNOTSUPP;
		}

		link_speed = paxb->get_link_speed();
		/* program RC's link cap reg to advertise proper link width */
		val = paxb_rc_cfg_read(core_idx, CFG_RC_LINK_CAP);
		val &= ~CFG_RC_LINK_CAP_WIDTH_MASK;
		val |= (link_width << CFG_RC_LINK_CAP_WIDTH_SHIFT);
		paxb_rc_cfg_write(core_idx, CFG_RC_LINK_CAP, val);

		/* program RC's link cap reg to advertise proper link speed */
		val = paxb_rc_cfg_read(core_idx, CFG_RC_LINK_CAP);
		val &= ~CFG_RC_LINK_CAP_SPEED_MASK;
		val |= link_speed << CFG_RC_LINK_CAP_SPEED_SHIFT;
		paxb_rc_cfg_write(core_idx, CFG_RC_LINK_CAP, val);

		/* also need to program RC's link status control register */
		val = paxb_rc_cfg_read(core_idx, CFG_RC_LINK_STATUS_CTRL_2);
		val &= ~(CFG_RC_LINK_SPEED_MASK);
		val |= link_speed << CFG_RC_LINK_SPEED_SHIFT;
		paxb_rc_cfg_write(core_idx, CFG_RC_LINK_STATUS_CTRL_2, val);

#ifdef WAR_PLX_PRESET_PARITY_FAIL
		/* WAR to avoid crash with PLX switch in GEN3*/
		/* While PRESET, PLX switch is not fixing parity so disabled */
		val = paxb_rc_cfg_read(core_idx, CFG_RC_REG_PHY_CTL_10);
		val &= ~(PHY_CTL_10_GEN3_MATCH_PARITY);
		paxb_rc_cfg_write(core_idx, CFG_RC_REG_PHY_CTL_10, val);
#endif
		pcie_set_default_tx_coeff(core_idx, link_width);
	}
	return 0;
}

#ifdef PAXB_LINKUP
static void paxb_perst_ctrl(unsigned int core_idx, bool assert)
{
	uint32_t clk_ctrl = PAXB_OFFSET(core_idx) + PAXB_CLK_CTRL_OFFSET;

	if (assert) {
		mmio_clrbits_32(clk_ctrl, PAXB_EP_PERST_SRC_SEL_MASK |
				PAXB_EP_MODE_PERST_MASK |
				PAXB_RC_PCIE_RST_OUT_MASK);
		udelay(250);
	} else {
		mmio_setbits_32(clk_ctrl, PAXB_RC_PCIE_RST_OUT_MASK);
		mdelay(100);
	}
}

static void paxb_start_link_up(void)
{
	unsigned int core_idx;
	uint32_t val, timeout;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		/* toggle PERST */
		paxb_perst_ctrl(core_idx, true);
		paxb_perst_ctrl(core_idx, false);

		timeout = DL_LINK_UP_TIMEOUT_MS;
		/* wait for Link up */
		do {
			val = mmio_read_32(PAXB_OFFSET(core_idx) +
					   PAXB_CFG_LINK_STATUS_OFFSET);
			if (val & PAXB_CFG_DL_ACTIVE_MASK)
				break;

			mdelay(1);
		} while (--timeout);

		if (!timeout)
			ERROR("PAXB core %u link is down\n", core_idx);
	}
}
#endif

static void pcie_core_soft_reset(unsigned int core_idx)
{
	uint32_t offset = core_idx * PCIE_CORE_PWR_OFFSET;
	uintptr_t ctrl = (uintptr_t)(PCIE_CORE_SOFT_RST_CFG_BASE + offset);

	/* Put PCIe core in soft reset */
	mmio_clrbits_32(ctrl, PCIE_CORE_SOFT_RST);

	/* Wait for 1 us before pulling PCIe core out of soft reset */
	udelay(PCIE_CORE_SOFT_RST_DELAY_US);

	mmio_setbits_32(ctrl, PCIE_CORE_SOFT_RST);
}

static int pcie_core_pwron_switch(uintptr_t ctrl, uintptr_t status,
				  uint32_t mask)
{
	uint32_t val;
	unsigned int timeout = PCIE_CORE_PWR_TIMEOUT_MS;

	/* enable switch */
	mmio_setbits_32(ctrl, mask);

	/* now wait for it to stabilize */
	do {
		val = mmio_read_32(status);
		if ((val & mask) == mask)
			return 0;
		mdelay(1);
	} while (--timeout);

	return -EIO;
}

static int pcie_core_pwr_seq(uintptr_t ctrl, uintptr_t status)
{
	int ret;

	/*
	 * Enable the switch with the following sequence:
	 * 1. Array weak switch output switch
	 * 2. Array strong switch
	 * 3. Weak switch output acknowledge
	 * 4. Strong switch output acknowledge
	 */
	ret = pcie_core_pwron_switch(ctrl, status, PCIE_CORE_PWR_ARR_POWERON);
	if (ret)
		return ret;

	ret = pcie_core_pwron_switch(ctrl, status, PCIE_CORE_PWR_ARR_POWEROK);
	if (ret)
		return ret;

	ret = pcie_core_pwron_switch(ctrl, status, PCIE_CORE_PWR_POWERON);
	if (ret)
		return ret;

	ret = pcie_core_pwron_switch(ctrl, status, PCIE_CORE_PWR_POWEROK);
	if (ret)
		return ret;

	return 0;
}

/*
 * This function enables PCIe core and PAXB memory buffer power, and then
 * remove the PCIe core from isolation
 */
static int pcie_core_pwr_init(unsigned int core_idx)
{
	int ret;
	uint32_t offset = core_idx * PCIE_CORE_PWR_OFFSET;
	uintptr_t ctrl, status;

	/* enable mem power to PCIe core */
	ctrl = (uintptr_t)(PCIE_CORE_MEM_PWR_BASE + offset);
	status = (uintptr_t)(PCIE_CORE_MEM_PWR_STATUS_BASE + offset);
	ret = pcie_core_pwr_seq(ctrl, status);
	if (ret) {
		ERROR("PCIe core mem power failed\n");
		return ret;
	}

	/* now enable mem power to PAXB wrapper */
	ctrl = (uintptr_t)(PCIE_PAXB_MEM_PWR_BASE + offset);
	status = (uintptr_t)(PCIE_PAXB_MEM_PWR_STATUS_BASE + offset);
	ret = pcie_core_pwr_seq(ctrl, status);
	if (ret) {
		ERROR("PAXB mem power failed\n");
		return ret;
	}

	/* now remove power isolation */
	ctrl = (uintptr_t)(PCIE_CORE_ISO_CFG_BASE + offset);
	mmio_clrbits_32(ctrl, PCIE_CORE_ISO | PCIE_CORE_MEM_ISO);

	return 0;
}

static void pcie_ss_reset(void)
{
	mmio_setbits_32(CDRU_MISC_RESET_CONTROL,
			1 << CDRU_MISC_RESET_CONTROL__CDRU_PCIE_RESET_N_R);
}

/*
 * This function reads the PIPEMUX strap, figures out all the PCIe cores that
 * need to be enabled and enable the mem power for those cores
 */
static int pcie_cores_init(void)
{
	int ret = 0;
	uint32_t core_idx;

	if (paxb->pipemux_init) {
		ret = paxb->pipemux_init();
		if (ret)
			return ret;
	}

	/* bring PCIe subsystem out of reset */
	pcie_ss_reset();

	/* power up all PCIe cores that will be used as RC */
	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		ret = pcie_core_pwr_init(core_idx);
		if (ret) {
			ERROR("PCIe core %u power up failed\n", core_idx);
			return ret;
		}

		pcie_core_soft_reset(core_idx);

		VERBOSE("PCIe core %u is powered up\n", core_idx);
	}

	return ret;
}

void paxb_rc_cfg_write(unsigned int core_idx, unsigned int where,
			      uint32_t val)
{
	mmio_write_32(PAXB_OFFSET(core_idx) + PAXB_CFG_IND_ADDR_OFFSET,
		      (where & PAXB_CFG_IND_ADDR_MASK) |
		      PAXB_CFG_CFG_TYPE_MASK);
	mmio_write_32(PAXB_OFFSET(core_idx) + PAXB_CFG_IND_DATA_OFFSET, val);
}

unsigned int paxb_rc_cfg_read(unsigned int core_idx, unsigned int where)
{
	unsigned int val;

	mmio_write_32(PAXB_OFFSET(core_idx) + PAXB_CFG_IND_ADDR_OFFSET,
		      (where & PAXB_CFG_IND_ADDR_MASK) |
		      PAXB_CFG_CFG_TYPE_MASK);
	val = mmio_read_32(PAXB_OFFSET(core_idx) + PAXB_CFG_IND_DATA_OFFSET);

	return val;
}

static void paxb_cfg_mps(void)
{
	uint32_t val, core_idx, mps;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		val = paxb_rc_cfg_read(core_idx, CFG_RC_DEVICE_CAP);
		val &= ~CFG_RC_DEVICE_CAP_MPS_MASK;
		mps = CFG_RC_DEVICE_CAP_MPS_256B;
		if (core_idx == 0 || core_idx == 1 ||
		    core_idx == 6 || core_idx == 7) {
			mps = CFG_RC_DEVICE_CAP_MPS_512B;
		}
		val |= mps;
		paxb_rc_cfg_write(core_idx, CFG_RC_DEVICE_CAP, val);
	}
}

static void paxb_cfg_dev_id(void)
{
	uint32_t val, core_idx;
	uint32_t device_id;

	device_id = paxb->device_id;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		/* Set Core in RC mode */
		mmio_setbits_32(PCIE_CORE_USER_CFG +
				(core_idx * PCIE_CORE_PWR_OFFSET), 1);

		/* force class to PCI_CLASS_BRIDGE_PCI (0x0604) */
		val = paxb_rc_cfg_read(core_idx, PCI_BRIDGE_CTRL_REG_OFFSET);
		val &= ~PCI_CLASS_BRIDGE_MASK;
		val |= (PCI_CLASS_BRIDGE_PCI << PCI_CLASS_BRIDGE_SHIFT);
		paxb_rc_cfg_write(core_idx, PCI_BRIDGE_CTRL_REG_OFFSET, val);

		val = (VENDOR_ID << 16) | device_id;
		paxb_rc_cfg_write(core_idx, CFG_RC_DEV_ID, val);

		val = (device_id << 16) | VENDOR_ID;
		paxb_rc_cfg_write(core_idx, CFG_RC_DEV_SUBID, val);
	}
}

static void paxb_cfg_tgt_trn(void)
{
	uint32_t val, core_idx;

	/*
	 * Disable all mem Rd/Wr size check so it allows target read/write
	 * transactions to be more than stipulated DW. As a result, PAXB root
	 * complex will not abort these read/write transcations beyond
	 * stipulated limit
	 */
	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		val = paxb_rc_cfg_read(core_idx, CFG_RC_TL_CTRL_0);
		val &= ~(RC_MEM_DW_CHK_MASK);
		paxb_rc_cfg_write(core_idx, CFG_RC_TL_CTRL_0, val);
	}
}

static void paxb_cfg_pdl_ctrl(void)
{
	uint32_t val, core_idx;
	uint32_t nph, ph, pd;

	/* increase the credit counter to 4 for non-posted header */
	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		nph = NPH_FC_INIT;
		ph = PH_INIT;
		pd = PD_FC_INIT;

		if (core_idx == 0 || core_idx == 1 ||
		    core_idx == 6 || core_idx == 7) {
			nph = SRP_NPH_FC_INIT;
			ph = SRP_PH_INIT;
			pd = SRP_PD_FC_INIT;
		}
		val = paxb_rc_cfg_read(core_idx, CFG_RC_PDL_CTRL_4);
		val &= ~NPH_FC_INIT_MASK;
		val &= ~PD_FC_INIT_MASK;
		val = val | (nph << NPH_FC_INIT_SHIFT);
		val = val | (pd << PD_FC_INIT_SHIFT);
		paxb_rc_cfg_write(core_idx, CFG_RC_PDL_CTRL_4, val);

		val = paxb_rc_cfg_read(core_idx, CFG_RC_PDL_CTRL_5);
		val &= ~PH_INIT_MASK;
		val = val | (ph << PH_INIT_SHIFT);
		paxb_rc_cfg_write(core_idx, CFG_RC_PDL_CTRL_5, val);

		/*
		 * ASIC to give more optmized value after further investigation.
		 * till then this is important to have to get similar
		 * performance on all the slots.
		 */
		paxb_rc_cfg_write(core_idx, CFG_RC_TL_FCIMM_NP_LIMIT,
				CFG_RC_TL_FCIMM_NP_VAL);

		paxb_rc_cfg_write(core_idx, CFG_RC_TL_FCIMM_P_LIMIT,
				CFG_RC_TL_FCIMM_P_VAL);
	}
}

static void paxb_cfg_clkreq(void)
{
	uint32_t val, core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		val = paxb_rc_cfg_read(core_idx, CFG_RC_CLKREQ_ENABLED);
		val &= ~CFG_RC_CLKREQ_ENABLED_MASK;
		paxb_rc_cfg_write(core_idx, CFG_RC_CLKREQ_ENABLED, val);
	}
}

static void paxb_cfg_dl_active(bool enable)
{
	uint32_t val, core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		val = paxb_rc_cfg_read(core_idx, CFG_LINK_CAP_RC);
		if (enable)
			val |= CFG_RC_DL_ACTIVE_MASK;
		else
			val &= ~CFG_RC_DL_ACTIVE_MASK;
		paxb_rc_cfg_write(core_idx, CFG_LINK_CAP_RC, val);
	}
}

static void paxb_cfg_LTR(int enable)
{
	uint32_t val, core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		val = paxb_rc_cfg_read(core_idx, CFG_ROOT_CAP_RC);
		if (enable)
			val |= CFG_ROOT_CAP_LTR_MASK;
		else
			val &= ~CFG_ROOT_CAP_LTR_MASK;
		paxb_rc_cfg_write(core_idx, CFG_ROOT_CAP_RC, val);
	}
}

static void paxb_ib_regs_bypass(void)
{
	unsigned int i, j;

	for (i = 0; i < paxb->num_cores; i++) {
		if (!pcie_core_needs_enable(i))
			continue;

		/* Configure Default IMAP window */
		mmio_write_32(PAXB_OFFSET(i) + PAXB_0_DEFAULT_IMAP,
			      DEFAULT_ADDR_INVALID);
		mmio_write_32(PAXB_OFFSET(i) + PAXB_0_DEFAULT_IMAP_AXUSER,
			      IMAP_AXUSER);
		mmio_write_32(PAXB_OFFSET(i) + PAXB_0_DEFAULT_IMAP_AXCACHE,
			      IMAP_AXCACHE);

		/* Configure MSI IMAP window */
		mmio_setbits_32(PAXB_OFFSET(i) +
				PAXB_IMAP0_REGS_TYPE_OFFSET,
				0x1);
		mmio_write_32(PAXB_OFFSET(i) + PAXB_IARR0_BASE_OFFSET,
			      GITS_TRANSLATER | OARR_VALID);
		for (j = 0; j < PAXB_MAX_IMAP_WINDOWS; j++) {
			mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP0_OFFSET(j),
				      (GITS_TRANSLATER +
				       (j * PAXB_IMAP0_WINDOW_SIZE)) |
				      IMAP_VALID);
		}
	}
}

static void paxb_ib_regs_init(void)
{
	unsigned int core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		/* initialize IARR2 to zero */
		mmio_write_32(PAXB_OFFSET(core_idx) + PAXB_IARR2_LOWER_OFFSET,
			      0x0);
		mmio_setbits_32(PAXB_OFFSET(core_idx) +
				PAXB_IMAP0_REGS_TYPE_OFFSET,
				0x1);
	}
}

static void paxb_cfg_apb_timeout(void)
{
	unsigned int core_idx;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		/* allow unlimited timeout */
		mmio_write_32(PAXB_OFFSET(core_idx) +
			PAXB_APB_TIMEOUT_COUNT_OFFSET,
			0xFFFFFFFF);
	}
}

static void paxb_smmu_cfg(void)
{
	unsigned int core_idx;
	uint32_t offset;
	uint32_t val;

	for (core_idx = 0; core_idx < paxb->num_cores; core_idx++) {
		if (!pcie_core_needs_enable(core_idx))
			continue;

		offset = core_idx * PCIE_CORE_PWR_OFFSET;
		val = mmio_read_32(PCIE_PAXB_SMMU_SID_CFG + offset);
		val &= ~(0xFFF00);
		val |= (PAXB_SMMU_SID_CFG_FUN_WIDTH |
			PAXB_SMMU_SID_CFG_DEV_WIDTH |
			PAXB_SMMU_SID_CFG_BUS_WIDTH);
		mmio_write_32(PCIE_PAXB_SMMU_SID_CFG + offset, val);
		val = mmio_read_32(PCIE_PAXB_SMMU_SID_CFG + offset);
		VERBOSE("smmu cfg reg 0x%x\n", val);
	}
}

static void paxb_cfg_coherency(void)
{
	unsigned int i, j;

	for (i = 0; i < paxb->num_cores; i++) {
		if (!pcie_core_needs_enable(i))
			continue;

#ifdef USE_DDR
		mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP2_OFFSET,
			      IMAP_ARCACHE | IMAP_AWCACHE);
#endif

		mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP0_0_AXUSER_OFFSET,
			      IMAP_AXUSER);

		mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP2_AXUSER_OFFSET,
			      IMAP_AXUSER);

		for (j = 0; j < PAXB_MAX_IMAP_WINDOWS; j++) {
#ifdef USE_DDR
			mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP3_OFFSET(j),
				      IMAP_ARCACHE | IMAP_AWCACHE);
			mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP4_OFFSET(j),
				      IMAP_ARCACHE | IMAP_AWCACHE);
#endif
			/* zero out IMAP0 mapping windows for MSI/MSI-X */
			mmio_write_32(PAXB_OFFSET(i) + PAXB_IMAP0_OFFSET(j),
				      0x0);

			mmio_write_32(PAXB_OFFSET(i) +
				      PAXB_IMAP3_0_AXUSER_OFFSET(j),
				      IMAP_AXUSER);
			mmio_write_32(PAXB_OFFSET(i) +
				      PAXB_IMAP4_0_AXUSER_OFFSET(j),
				      IMAP_AXUSER);
		}
	}
}

/*
 * This function configures all PAXB related blocks to allow non-secure access
 */
void paxb_ns_init(enum paxb_type type)
{
	unsigned int reg;

	switch (type) {
	case PAXB_SR:
		for (reg = 0; reg < ARRAY_SIZE(paxb_sec_reg_offset); reg++) {

			mmio_setbits_32(SR_PCIE_NIC_SECURITY_BASE +
					paxb_sec_reg_offset[reg], 0x1);
		}
	/* Enabled all PAXB's relevant IDM blocks access in non-secure mode */
	mmio_setbits_32(SR_PCIE_NIC_SECURITY_BASE + PAXB_SECURITY_IDM_OFFSET,
			0xffff);
		break;
	case PAXB_NS3Z:
		mmio_setbits_32(NS3Z_PCIE_NIC_SECURITY_BASE +
				paxb_sec_reg_offset[0], 0x1);
		mmio_setbits_32(NS3Z_PCIE_NIC_SECURITY_BASE +
				PAXB_SECURITY_IDM_OFFSET, 0xffff);
		mmio_setbits_32(NS3Z_PCIE_NIC_SECURITY_BASE +
				PAXB_SECURITY_APB_OFFSET, 0x7);
		mmio_setbits_32(NS3Z_PCIE_NIC_SECURITY_BASE +
				PAXB_SECURITY_ECAM_OFFSET, 0x1);
		break;
	}
}

static int paxb_set_config(void)
{
	paxb = paxb_get_config(sr);
	if (paxb)
		return 0;

	return -ENODEV;
}

void paxb_init(void)
{
	int ret;

	ret = paxb_set_config();
	if (ret)
		return;

	paxb_ns_init(paxb->type);

	ret = pcie_cores_init();
	if (ret)
		return;

	if (paxb->phy_init) {
		ret = paxb->phy_init();
		if (ret)
			return;
	}

	paxb_cfg_dev_id();
	paxb_cfg_tgt_trn();
	paxb_cfg_pdl_ctrl();
	if (paxb->type == PAXB_SR) {
		paxb_ib_regs_init();
		paxb_cfg_coherency();
	} else
		paxb_ib_regs_bypass();

	paxb_cfg_apb_timeout();
	paxb_smmu_cfg();
	paxb_cfg_clkreq();
	paxb_rc_link_init();

	/* Stingray Doesn't support LTR */
	paxb_cfg_LTR(false);
	paxb_cfg_dl_active(true);

	paxb_cfg_mps();

#ifdef PAXB_LINKUP
	paxb_start_link_up();
#endif
	INFO("PAXB init done\n");
}
