/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PCI_SVC_H
#define PCI_SVC_H

#include <lib/utils_def.h>

/* SMCCC PCI platform functions */
#define SMC_PCI_VERSION			U(0x84000130)
#define SMC_PCI_FEATURES		U(0x84000131)
#define SMC_PCI_READ			U(0x84000132)
#define SMC_PCI_WRITE			U(0x84000133)
#define SMC_PCI_SEG_INFO		U(0x84000134)

#define is_pci_fid(_fid) (((_fid) >= SMC_PCI_VERSION) &&  \
			  ((_fid) <= SMC_PCI_SEG_INFO))

uint64_t pci_smc_handler(uint32_t smc_fid, u_register_t x1, u_register_t x2,
			 u_register_t x3,  u_register_t x4, void *cookie,
			 void *handle, u_register_t flags);

#define PCI_ADDR_FUN(dev) ((dev) & U(0x7))
#define PCI_ADDR_DEV(dev) (((dev) >> U(3))  & U(0x001F))
#define PCI_ADDR_BUS(dev) (((dev) >> U(8))  & U(0x00FF))
#define PCI_ADDR_SEG(dev) (((dev) >> U(16)) & U(0xFFFF))
#define PCI_OFFSET_MASK   U(0xFFF)
typedef union {
	struct {
		uint16_t minor;
		uint16_t major;
	} __packed;
	uint32_t val;
} pcie_version;

/*
 * platforms are responsible for providing implementations of these
 * three functions in a manner which conforms to the Arm PCI Configuration
 * Space Access Firmware Interface (DEN0115) and the PCIe specification's
 * sections on PCI configuration access. See the rpi4_pci_svc.c example.
 */
uint32_t pci_read_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t *val);
uint32_t pci_write_config(uint32_t addr, uint32_t off, uint32_t sz, uint32_t val);
uint32_t pci_get_bus_for_seg(uint32_t seg, uint32_t *bus_range, uint32_t *nseg);

/* Return codes for Arm PCI Config Space Access Firmware SMC calls */
#define SMC_PCI_CALL_SUCCESS	       U(0)
#define SMC_PCI_CALL_NOT_SUPPORTED	-1
#define SMC_PCI_CALL_INVAL_PARAM	-2
#define SMC_PCI_CALL_NOT_IMPL		-3

#define SMC_PCI_SZ_8BIT			U(1)
#define SMC_PCI_SZ_16BIT		U(2)
#define SMC_PCI_SZ_32BIT		U(4)

#endif /* PCI_SVC_H */
