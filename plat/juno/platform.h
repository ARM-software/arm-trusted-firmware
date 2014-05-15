/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <arch.h>

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/

/* Size of cacheable stacks */
#define PLATFORM_STACK_SIZE    0x800

/* Size of coherent stacks for debug and release builds */
#if DEBUG
#define PCPU_DV_MEM_STACK_SIZE 0x400
#else
#define PCPU_DV_MEM_STACK_SIZE 0x300
#endif

#define FIRMWARE_WELCOME_STR		"Booting trusted firmware boot loader stage 1\n\r"

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_NAME			"bl2.bin"

/* SCP Firmware BL3-0 */
#define BL30_IMAGE_NAME			"bl30.bin"

/* EL3 Runtime Firmware BL3-1 */
#define BL31_IMAGE_NAME			"bl31.bin"

/* Secure Payload BL3-2 (Trusted OS) */
#define BL32_IMAGE_NAME			"bl32.bin"

/* Non-Trusted Firmware BL3-3 and its load address */
#define BL33_IMAGE_NAME			"bl33.bin" /* e.g. UEFI */
#define NS_IMAGE_OFFSET			0xE0000000

/* Firmware Image Package */
#define FIP_IMAGE_NAME			"fip.bin"

#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CORE_COUNT             6
#define PRIMARY_CPU			0x100
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4


/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/

#define MHU_SECURE_BASE		0x04000000
#define MHU_SECURE_SIZE		0x00001000

#define MHU_PAYLOAD_CACHED	0

#define TRUSTED_MAILBOXES_BASE	MHU_SECURE_BASE
#define TRUSTED_MAILBOX_SHIFT	4

#define TZRAM_BASE		0x04001000
#define TZRAM_SIZE		0x0003F000

#define FLASH_BASE		0x08000000
#define FLASH_SIZE		0x04000000

#define BL1_ROM_BYPASS_OFFSET	0x03EC0000	// agreed BYPASS offset from start of NOR flash

#ifndef TZROM_BASE
#define TZROM_BASE		FLASH_BASE + BL1_ROM_BYPASS_OFFSET	// Use the agreed BYPASS address
#endif
#define TZROM_SIZE		0x00010000

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

/* Counter timer module offsets */
#define CNTNSAR			0x4
#define CNTNSAR_NS_SHIFT(x)	x

#define CNTACR_BASE(x)		(0x40 + (x << 2))
#define CNTACR_RPCT_SHIFT	0x0
#define CNTACR_RVCT_SHIFT	0x1
#define CNTACR_RFRQ_SHIFT	0x2
#define CNTACR_RVOFF_SHIFT	0x3
#define CNTACR_RWVT_SHIFT	0x4
#define CNTACR_RWPT_SHIFT	0x5

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
 * BL2 specific defines.
 ******************************************************************************/
#define BL2_BASE			0x0402a000

/*******************************************************************************
 * BL3-1 specific defines.
 ******************************************************************************/
#define BL31_BASE			0x0400C000

/*******************************************************************************
 * BL3-0 specific defines.
 * BL3-0 is loaded to the same place as BL3-1.  Once BL3-0 is transferred to the
 * SCP, it is discarded and BL3-1 is loaded over the top.
 ******************************************************************************/
#define BL30_BASE			BL31_BASE

/*******************************************************************************
 * Workaround for issue #68
 * We don't support BL3-2 on Juno yet
 ******************************************************************************/
#define BL32_BASE			0

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_XLAT_TABLES			5
#define MAX_MMAP_REGIONS		16


/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define CCI400_BASE			0x2c090000
#define CCI400_SL_IFACE_CLUSTER0	4
#define CCI400_SL_IFACE_CLUSTER1	3
#define CCI400_SL_IFACE_INDEX(mpidr)	(mpidr & MPIDR_CLUSTER_MASK ? \
					 CCI400_SL_IFACE_CLUSTER1 :   \
					 CCI400_SL_IFACE_CLUSTER0)

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

#define TZC400_NSAID_CCI400	0  /* Note: Same as default NSAID!! HW fix in next revision */
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
 * Declarations and constants to access the mailboxes safely. Each mailbox is
 * aligned on the biggest cache line size in the platform. This is known only
 * to the platform as it might have a combination of integrated and external
 * caches. Such alignment ensures that two maiboxes do not sit on the same cache
 * line at any cache level. They could belong to different cpus/clusters &
 * get written while being protected by different locks causing corruption of
 * a valid mailbox address.
 ******************************************************************************/
#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

#ifndef __ASSEMBLY__

#include <stdint.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct plat_pm_ops;
struct meminfo;

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
extern unsigned long *bl1_normal_ram_base;
extern unsigned long *bl1_normal_ram_len;
extern unsigned long *bl1_normal_ram_limit;
extern unsigned long *bl1_normal_ram_zi_base;
extern unsigned long *bl1_normal_ram_zi_len;

extern unsigned long *bl1_coherent_ram_base;
extern unsigned long *bl1_coherent_ram_len;
extern unsigned long *bl1_coherent_ram_limit;
extern unsigned long *bl1_coherent_ram_zi_base;
extern unsigned long *bl1_coherent_ram_zi_len;
extern unsigned long warm_boot_entrypoint;

extern void bl1_plat_arch_setup(void);
extern void bl2_plat_arch_setup(void);
extern void bl31_plat_arch_setup(void);
extern int platform_setup_pm(const struct plat_pm_ops **);
extern unsigned int platform_get_core_pos(unsigned long mpidr);
extern void enable_mmu(void);
extern void configure_mmu(struct meminfo *,
			  unsigned long,
			  unsigned long,
			  unsigned long,
			  unsigned long);
extern void plat_report_exception(unsigned long);
extern unsigned long plat_get_ns_image_entrypoint(void);
extern unsigned long platform_get_stack(unsigned long mpidr);
extern uint64_t plat_get_syscnt_freq(void);

/* Declarations for fvp_gic.c */
extern void gic_cpuif_deactivate(unsigned int);
extern void gic_cpuif_setup(unsigned int);
extern void gic_pcpu_distif_setup(unsigned int);
extern void gic_setup(void);

/* Declarations for fvp_topology.c */
extern int plat_setup_topology(void);
extern int plat_get_max_afflvl(void);
extern unsigned int plat_get_aff_count(unsigned int, unsigned long);
extern unsigned int plat_get_aff_state(unsigned int, unsigned long);

/* Declarations for plat_io_storage.c */
extern void io_setup(void);
extern int plat_get_image_source(const char *image_name,
		uintptr_t *dev_handle, uintptr_t *image_spec);

#endif /*__ASSEMBLY__*/

#endif /* __PLATFORM_H__ */
