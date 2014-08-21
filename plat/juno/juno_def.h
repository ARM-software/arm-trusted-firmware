/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __JUNO_DEF_H__
#define __JUNO_DEF_H__

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define JUNO_BL31_PLAT_PARAM_VAL	0x0f1e2d3c4b5a6978ULL

/*******************************************************************************
 * Juno memory map related constants
 ******************************************************************************/
#define MHU_SECURE_BASE		0x04000000
#define MHU_SECURE_SIZE		0x00001000

#define MHU_PAYLOAD_CACHED	0

#define TRUSTED_MAILBOXES_BASE	MHU_SECURE_BASE
#define TRUSTED_MAILBOX_SHIFT	4

#define EMMC_BASE		0x0c000000
#define EMMC_SIZE		0x04000000

#define PSRAM_BASE		0x14000000
#define PSRAM_SIZE		0x02000000

#define IOFPGA_BASE		0x1c000000
#define IOFPGA_SIZE		0x03000000

#define NSROM_BASE		0x1f000000
#define NSROM_SIZE		0x00001000

/* Following covers Columbus Peripherals excluding NSROM and NSRAM  */
#define DEVICE0_BASE		0x20000000
#define DEVICE0_SIZE		0x0e000000
#define MHU_BASE		0x2b1f0000

#define NSRAM_BASE		0x2e000000
#define NSRAM_SIZE		0x00008000

/* Following covers Juno Peripherals and PCIe expansion area */
#define DEVICE1_BASE		0x40000000
#define DEVICE1_SIZE		0x40000000
#define PCIE_CONTROL_BASE	0x7ff20000

#define DRAM_BASE		0x80000000
#define DRAM_SIZE		0x80000000

/* Memory mapped Generic timer interfaces  */
#define SYS_CNTCTL_BASE		0x2a430000
#define SYS_CNTREAD_BASE	0x2a800000
#define SYS_TIMCTL_BASE		0x2a810000

/* V2M motherboard system registers & offsets */
#define VE_SYSREGS_BASE		0x1c010000
#define V2M_SYS_LED		0x8

/*
 * V2M sysled bit definitions. The values written to this
 * register are defined in arch.h & runtime_svc.h. Only
 * used by the primary cpu to diagnose any cold boot issues.
 *
 * SYS_LED[0]   - Security state (S=0/NS=1)
 * SYS_LED[2:1] - Exception Level (EL3-EL0)
 * SYS_LED[7:3] - Exception Class (Sync/Async & origin)
 *
 */
#define SYS_LED_SS_SHIFT		0x0
#define SYS_LED_EL_SHIFT		0x1
#define SYS_LED_EC_SHIFT		0x3

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
#define GICD_BASE			0x2c010000
#define GICC_BASE			0x2c02f000
#define GICH_BASE			0x2c04f000
#define GICV_BASE			0x2c06f000

#define IRQ_MHU			69
#define IRQ_GPU_SMMU_0		71
#define IRQ_GPU_SMMU_1		73
#define IRQ_ETR_SMMU		75
#define IRQ_TZC400		80
#define IRQ_TZ_WDOG		86

#define IRQ_SEC_PHY_TIMER		29
#define IRQ_SEC_SGI_0			8
#define IRQ_SEC_SGI_1			9
#define IRQ_SEC_SGI_2			10
#define IRQ_SEC_SGI_3			11
#define IRQ_SEC_SGI_4			12
#define IRQ_SEC_SGI_5			13
#define IRQ_SEC_SGI_6			14
#define IRQ_SEC_SGI_7			15
#define IRQ_SEC_SGI_8			16

/*******************************************************************************
 * PL011 related constants
 ******************************************************************************/
/* FPGA UART0 */
#define PL011_UART0_BASE		0x1c090000
/* FPGA UART1 */
#define PL011_UART1_BASE		0x1c0a0000
/* SoC UART0 */
#define PL011_UART2_BASE		0x7ff80000
/* SoC UART1 */
#define PL011_UART3_BASE		0x7ff70000

#define PL011_BAUDRATE			115200

#define PL011_UART0_CLK_IN_HZ		24000000
#define PL011_UART1_CLK_IN_HZ		24000000
#define PL011_UART2_CLK_IN_HZ		7273800
#define PL011_UART3_CLK_IN_HZ		7273800

/*******************************************************************************
 * NIC-400 related constants
 ******************************************************************************/

/* CSS NIC-400 Global Programmers View (GPV) */
#define CSS_NIC400_BASE		0x2a000000

/* The slave_bootsecure controls access to GPU, DMC and CS. */
#define CSS_NIC400_SLAVE_BOOTSECURE		8

/* SoC NIC-400 Global Programmers View (GPV) */
#define SOC_NIC400_BASE		0x7fd00000

#define SOC_NIC400_USB_EHCI	0
#define SOC_NIC400_TLX_MASTER	1
#define SOC_NIC400_USB_OHCI	2
#define SOC_NIC400_PL354_SMC	3
/*
 * The apb4_bridge controls access to:
 *   - the PCIe configuration registers
 *   - the MMU units for USB, HDLCD and DMA
 */
#define SOC_NIC400_APB4_BRIDGE	4
/*
 * The bootsec_bridge controls access to a bunch of peripherals, e.g. the UARTs.
 */
#define SOC_NIC400_BOOTSEC_BRIDGE		5
#define SOC_NIC400_BOOTSEC_BRIDGE_UART1         (1 << 12)

/*******************************************************************************
 * TZC-400 related constants
 ******************************************************************************/
#define TZC400_BASE		0x2a4a0000

#define TZC400_NSAID_CCI400	0  /* Note: Same as default NSAID!! */
#define TZC400_NSAID_PCIE	1
#define TZC400_NSAID_HDLCD0	2
#define TZC400_NSAID_HDLCD1	3
#define TZC400_NSAID_USB	4
#define TZC400_NSAID_DMA330	5
#define TZC400_NSAID_THINLINKS	6
#define TZC400_NSAID_AP		9
#define TZC400_NSAID_GPU	10
#define TZC400_NSAID_SCP	11
#define TZC400_NSAID_CORESIGHT	12

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define CCI400_BASE			0x2c090000
#define CCI400_SL_IFACE3_CLUSTER_IX	1
#define CCI400_SL_IFACE4_CLUSTER_IX	0

/*******************************************************************************
 * SCP <=> AP boot configuration
 ******************************************************************************/
#define SCP_BOOT_CFG_ADDR	0x04000080
#define PRIMARY_CPU_SHIFT	8
#define PRIMARY_CPU_MASK	0xf

#endif /* __JUNO_DEF_H__ */
