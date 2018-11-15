/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

/* Marvell CP110 ana A3700 common */

#ifndef PHY_COMPHY_COMMON_H
#define PHY_COMPHY_COMMON_H

/* #define DEBUG_COMPHY */
#ifdef DEBUG_COMPHY
#define debug(format...) printf(format)
#else
#define debug(format, arg...)
#endif

/* A lane is described by 4 fields:
 *  - bit 1~0 represent comphy polarity invert
 *  - bit 7~2 represent comphy speed
 *  - bit 11~8 represent unit index
 *  - bit 16~12 represent mode
 *  - bit 17 represent comphy indication of clock source
 *  - bit 20~18 represents pcie width (in case of pcie comphy config.)
 *  - bit 21 represents the source of the request (Linux/Bootloader),
 *           (reguired only for PCIe!)
 *  - bit 31~22 reserved
 */

#define COMPHY_INVERT_OFFSET	0
#define COMPHY_INVERT_LEN	2
#define COMPHY_INVERT_MASK	COMPHY_MASK(COMPHY_INVERT_OFFSET, \
						COMPHY_INVERT_LEN)
#define COMPHY_SPEED_OFFSET	(COMPHY_INVERT_OFFSET + COMPHY_INVERT_LEN)
#define COMPHY_SPEED_LEN	6
#define COMPHY_SPEED_MASK	COMPHY_MASK(COMPHY_SPEED_OFFSET, \
						COMPHY_SPEED_LEN)
#define COMPHY_UNIT_ID_OFFSET	(COMPHY_SPEED_OFFSET + COMPHY_SPEED_LEN)
#define COMPHY_UNIT_ID_LEN	4
#define COMPHY_UNIT_ID_MASK	COMPHY_MASK(COMPHY_UNIT_ID_OFFSET, \
						COMPHY_UNIT_ID_LEN)
#define COMPHY_MODE_OFFSET	(COMPHY_UNIT_ID_OFFSET + COMPHY_UNIT_ID_LEN)
#define COMPHY_MODE_LEN		5
#define COMPHY_MODE_MASK	COMPHY_MASK(COMPHY_MODE_OFFSET, COMPHY_MODE_LEN)
#define COMPHY_CLK_SRC_OFFSET	(COMPHY_MODE_OFFSET + COMPHY_MODE_LEN)
#define COMPHY_CLK_SRC_LEN	1
#define COMPHY_CLK_SRC_MASK	COMPHY_MASK(COMPHY_CLK_SRC_OFFSET, \
						COMPHY_CLK_SRC_LEN)
#define COMPHY_PCI_WIDTH_OFFSET	(COMPHY_CLK_SRC_OFFSET + COMPHY_CLK_SRC_LEN)
#define COMPHY_PCI_WIDTH_LEN	3
#define COMPHY_PCI_WIDTH_MASK	COMPHY_MASK(COMPHY_PCI_WIDTH_OFFSET, \
						COMPHY_PCI_WIDTH_LEN)
#define COMPHY_PCI_CALLER_OFFSET \
			(COMPHY_PCI_WIDTH_OFFSET + COMPHY_PCI_WIDTH_LEN)
#define COMPHY_PCI_CALLER_LEN	1
#define COMPHY_PCI_CALLER_MASK	COMPHY_MASK(COMPHY_PCI_CALLER_OFFSET, \
						COMPHY_PCI_CALLER_LEN)

#define COMPHY_MASK(offset, len)	(((1 << (len)) - 1) << (offset))

/* Macro which extracts mode from lane description */
#define COMPHY_GET_MODE(x)		(((x) & COMPHY_MODE_MASK) >> \
						COMPHY_MODE_OFFSET)
/* Macro which extracts unit index from lane description */
#define COMPHY_GET_ID(x)		(((x) & COMPHY_UNIT_ID_MASK) >> \
						COMPHY_UNIT_ID_OFFSET)
/* Macro which extracts speed from lane description */
#define COMPHY_GET_SPEED(x)		(((x) & COMPHY_SPEED_MASK) >> \
						COMPHY_SPEED_OFFSET)
/* Macro which extracts clock source indication from lane description */
#define COMPHY_GET_CLK_SRC(x)		(((x) & COMPHY_CLK_SRC_MASK) >> \
						COMPHY_CLK_SRC_OFFSET)
/* Macro which extracts pcie width indication from lane description */
#define COMPHY_GET_PCIE_WIDTH(x)	(((x) & COMPHY_PCI_WIDTH_MASK) >> \
						COMPHY_PCI_WIDTH_OFFSET)

/* Macro which extracts the caller for pcie power on from lane description */
#define COMPHY_GET_CALLER(x)		(((x) & COMPHY_PCI_CALLER_MASK) >> \
						COMPHY_PCI_CALLER_OFFSET)

/* Macro which extracts the polarity invert from lane description */
#define COMPHY_GET_POLARITY_INVERT(x)	(((x) & COMPHY_INVERT_MASK) >> \
						COMPHY_INVERT_OFFSET)


#define COMPHY_SATA_MODE	0x1
#define COMPHY_SGMII_MODE	0x2	/* SGMII 1G */
#define COMPHY_HS_SGMII_MODE	0x3	/* SGMII 2.5G */
#define COMPHY_USB3H_MODE	0x4
#define COMPHY_USB3D_MODE	0x5
#define COMPHY_PCIE_MODE	0x6
#define COMPHY_RXAUI_MODE	0x7
#define COMPHY_XFI_MODE		0x8
#define COMPHY_SFI_MODE		0x9
#define COMPHY_USB3_MODE	0xa
#define COMPHY_AP_MODE		0xb

#define	COMPHY_UNUSED		0xFFFFFFFF

/* Polarity invert macro */
#define COMPHY_POLARITY_NO_INVERT	0
#define COMPHY_POLARITY_TXD_INVERT	1
#define COMPHY_POLARITY_RXD_INVERT	2
#define COMPHY_POLARITY_ALL_INVERT	(COMPHY_POLARITY_TXD_INVERT | \
					 COMPHY_POLARITY_RXD_INVERT)

enum reg_width_type {
	REG_16BIT = 0,
	REG_32BIT,
};

enum {
	COMPHY_LANE0 = 0,
	COMPHY_LANE1,
	COMPHY_LANE2,
	COMPHY_LANE3,
	COMPHY_LANE4,
	COMPHY_LANE5,
	COMPHY_LANE_MAX,
};

static inline uint32_t polling_with_timeout(uintptr_t addr, uint32_t val,
					    uint32_t mask,
					    uint32_t usec_timeout,
					    enum reg_width_type type)
{
	uint32_t data;

	do {
		udelay(1);
		if (type == REG_16BIT)
			data = mmio_read_16(addr) & mask;
		else
			data = mmio_read_32(addr) & mask;
	} while (data != val  && --usec_timeout > 0);

	if (usec_timeout == 0)
		return data;

	return 0;
}

static inline void reg_set(uintptr_t addr, uint32_t data, uint32_t mask)
{
	debug("<atf>: WR to addr = 0x%lx, data = 0x%x (mask = 0x%x) - ",
	      addr, data, mask);
	debug("old value = 0x%x ==> ", mmio_read_32(addr));
	mmio_clrsetbits_32(addr, mask, data);

	debug("new val 0x%x\n", mmio_read_32(addr));
}

static inline void __unused reg_set16(uintptr_t addr, uint16_t data,
				      uint16_t mask)
{

	debug("<atf>: WR to addr = 0x%lx, data = 0x%x (mask = 0x%x) - ",
	      addr, data, mask);
	debug("old value = 0x%x ==> ", mmio_read_16(addr));
	mmio_clrsetbits_16(addr, mask, data);

	debug("new val 0x%x\n", mmio_read_16(addr));
}

#endif /* PHY_COMPHY_COMMON_H */
