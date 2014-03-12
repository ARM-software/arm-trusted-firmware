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
#include <mmio.h>
#include <psci.h>
#include <bl_common.h>
#include "io_storage.h"


/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE		0x800

#define FIRMWARE_WELCOME_STR		"Booting trusted firmware boot loader stage 1\n\r"

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_NAME			"bl2.bin"

/* EL3 Runtime Firmware BL3-1 */
#define BL31_IMAGE_NAME			"bl31.bin"

/* Secure Payload BL3-2 (Trusted OS) */
#define BL32_IMAGE_NAME			"bl32.bin"

/* Non-Trusted Firmware BL3-3 and its load address */
#define BL33_IMAGE_NAME			"bl33.bin" /* e.g. UEFI */
#define NS_IMAGE_OFFSET			(DRAM_BASE + 0x8000000) /* DRAM + 128MB */

/* Firmware Image Package */
#define FIP_IMAGE_NAME			"fip.bin"

#define PLATFORM_CACHE_LINE_SIZE	64
#define PLATFORM_CLUSTER_COUNT		2ull
#define PLATFORM_CLUSTER0_CORE_COUNT	4
#define PLATFORM_CLUSTER1_CORE_COUNT	4
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER1_CORE_COUNT + \
						PLATFORM_CLUSTER0_CORE_COUNT)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PRIMARY_CPU			0x0
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4


/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZROM_BASE		0x00000000
#define TZROM_SIZE		0x04000000

#define TZRAM_BASE		0x04000000
#define TZRAM_SIZE		0x40000

#define FLASH0_BASE		0x08000000
#define FLASH0_SIZE		TZROM_SIZE

#define FLASH1_BASE		0x0c000000
#define FLASH1_SIZE		0x04000000

#define PSRAM_BASE		0x14000000
#define PSRAM_SIZE		0x04000000

#define VRAM_BASE		0x18000000
#define VRAM_SIZE		0x02000000

/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE		0x1a000000
#define DEVICE0_SIZE		0x12200000

#define DEVICE1_BASE		0x2f000000
#define DEVICE1_SIZE		0x200000

#define NSRAM_BASE		0x2e000000
#define NSRAM_SIZE		0x10000

/* Location of trusted dram on the base fvp */
#define TZDRAM_BASE		0x06000000
#define TZDRAM_SIZE		0x02000000
#define MBOX_OFF		0x1000
#define AFFMAP_OFF		0x1200

#define DRAM_BASE              0x80000000ull
#define DRAM_SIZE              0x80000000ull

#define PCIE_EXP_BASE		0x40000000
#define TZRNG_BASE		0x7fe60000
#define TZNVCTR_BASE		0x7fe70000
#define TZROOTKEY_BASE		0x7fe80000

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

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#define BL2_BASE			0x0402D000

/*******************************************************************************
 * BL3-1 specific defines.
 ******************************************************************************/
#define BL31_BASE			0x0400C000

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define EL3_ADDR_SPACE_SIZE		(1ull << 32)
#define EL3_NUM_PAGETABLES		2
#define EL3_TROM_PAGETABLE		0
#define EL3_TRAM_PAGETABLE		1

#define ADDR_SPACE_SIZE			(1ull << 32)

#define NUM_L2_PAGETABLES		2
#define GB1_L2_PAGETABLE		0
#define GB2_L2_PAGETABLE		1

#define NUM_L3_PAGETABLES		2
#define TZRAM_PAGETABLE			0
#define NSRAM_PAGETABLE			1

/*******************************************************************************
 * CCI-400 related constants
 ******************************************************************************/
#define CCI400_BASE			0x2c090000
#define CCI400_SL_IFACE_CLUSTER0	3
#define CCI400_SL_IFACE_CLUSTER1	4
#define CCI400_SL_IFACE_INDEX(mpidr)	(mpidr & MPIDR_CLUSTER_MASK ? \
					 CCI400_SL_IFACE_CLUSTER1 :   \
					 CCI400_SL_IFACE_CLUSTER0)

/*******************************************************************************
 * GIC-400 & interrupt handling related constants
 ******************************************************************************/
/* VE compatible GIC memory map */
#define VE_GICD_BASE			0x2c001000
#define VE_GICC_BASE			0x2c002000
#define VE_GICH_BASE			0x2c004000
#define VE_GICV_BASE			0x2c006000

/* Base FVP compatible GIC memory map */
#define BASE_GICD_BASE			0x2f000000
#define BASE_GICR_BASE			0x2f100000
#define BASE_GICC_BASE			0x2c000000
#define BASE_GICH_BASE			0x2c010000
#define BASE_GICV_BASE			0x2c02f000

#define IRQ_TZ_WDOG			56
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
#define PL011_UART0_BASE		0x1c090000
#define PL011_UART1_BASE		0x1c0a0000
#define PL011_UART2_BASE		0x1c0b0000
#define PL011_UART3_BASE		0x1c0c0000
#define PL011_BASE			PL011_UART0_BASE

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

typedef volatile struct {
	unsigned long value
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
} mailbox;

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
extern int platform_setup_pm(plat_pm_ops **);
extern unsigned int platform_get_core_pos(unsigned long mpidr);
extern void disable_mmu(void);
extern void enable_mmu(void);
extern void configure_mmu(meminfo *,
			  unsigned long,
			  unsigned long,
			  unsigned long,
			  unsigned long);
extern void plat_report_exception(unsigned long);
extern unsigned long plat_get_ns_image_entrypoint(void);
extern unsigned long platform_get_stack(unsigned long mpidr);

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
		io_dev_handle *dev_handle, void **image_spec);

#endif /*__ASSEMBLY__*/

#endif /* __PLATFORM_H__ */
