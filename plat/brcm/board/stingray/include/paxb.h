/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAXB_H
#define PAXB_H

/* total number of PCIe cores */
#define NUM_OF_SR_PCIE_CORES           8
#define NUM_OF_NS3Z_PCIE_CORES         1

/*
 * List of PCIe core and PAXB wrapper memory power registers
 */
#define PCIE_CORE_BASE                   0x40000800
#define PCIE_CORE_SOFT_RST_CFG_BASE      (PCIE_CORE_BASE + 0x40)
#define PCIE_CORE_SOFT_RST               0x1
#define PCIE_CORE_ISO_CFG_BASE           (PCIE_CORE_BASE + 0x54)
#define PCIE_CORE_MEM_ISO                0x2
#define PCIE_CORE_ISO                    0x1

#define PCIE_CORE_MEM_PWR_BASE           (PCIE_CORE_BASE + 0x58)
#define PCIE_PAXB_MEM_PWR_BASE           (PCIE_CORE_BASE + 0x5c)
#define PCIE_CORE_PMI_CFG_BASE           (PCIE_CORE_BASE + 0x64)
#define PCIE_CORE_RESERVED_CFG           (PCIE_CORE_BASE + 0x6c)
#define PCIE_CORE_MEM_PWR_STATUS_BASE    (PCIE_CORE_BASE + 0x74)
#define PCIE_PAXB_MEM_PWR_STATUS_BASE    (PCIE_CORE_BASE + 0x78)
#define PCIE_CORE_PWR_OFFSET           0x100

#define SR_A0_DEVICE_ID                 0xd713
#define SR_B0_DEVICE_ID                 0xd714
/* TODO: Modify device ID once available */
#define NS3Z_DEVICE_ID                  0xd715

/* FIXME: change link speed to GEN3 when it's ready */
#define GEN1_LINK_SPEED               1
#define GEN2_LINK_SPEED               2
#define GEN3_LINK_SPEED               3

typedef struct {
	uint32_t type;
	uint32_t device_id;
	uint32_t pipemux_idx;
	uint32_t num_cores;
	int (*pipemux_init)(void);
	int (*phy_init)(void);
	int (*core_needs_enable)(unsigned int core_idx);
	unsigned int (*get_link_width)(unsigned int core_idx);
	unsigned int (*get_link_speed)(void);
} paxb_cfg;

enum paxb_type {
	PAXB_SR,
	PAXB_NS3Z,
};

extern const paxb_cfg *paxb;

#ifdef USE_PAXB
void paxb_init(void);
void paxb_rc_cfg_write(unsigned int core_idx, unsigned int where,
		       uint32_t val);
unsigned int paxb_rc_cfg_read(unsigned int core_idx, unsigned int where);
int pcie_core_needs_enable(unsigned int core_idx);
const paxb_cfg *paxb_get_sr_config(void);
#else
static inline void paxb_init(void)
{
}
#endif

#endif /* PAXB_H */
