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
#define PLATFORM_STACK_SIZE	0x800

/* Size of coherent stacks for debug and release builds */
#if DEBUG
#define PCPU_DV_MEM_STACK_SIZE	0x400
#else
#define PCPU_DV_MEM_STACK_SIZE	0x300
#endif

#define FIRMWARE_WELCOME_STR		"Booting trusted firmware boot loader stage 1\n\r"

/* Trusted Boot Firmware BL2 */
#define BL2_IMAGE_NAME			"bl2.bin"

/* EL3 Runtime Firmware BL31 */
#define BL31_IMAGE_NAME			"bl31.bin"

/* Secure Payload BL32 (Trusted OS) */
#define BL32_IMAGE_NAME			"bl32.bin"

/* Non-Trusted Firmware BL33 and its load address */
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

/* Constants for accessing platform configuration */
#define CONFIG_GICD_ADDR		0
#define CONFIG_GICC_ADDR		1
#define CONFIG_GICH_ADDR		2
#define CONFIG_GICV_ADDR		3
#define CONFIG_MAX_AFF0		4
#define CONFIG_MAX_AFF1		5
/* Indicate whether the CPUECTLR SMP bit should be enabled. */
#define CONFIG_CPU_SETUP		6
#define CONFIG_BASE_MMAP		7
/* Indicates whether CCI should be enabled on the platform. */
#define CONFIG_HAS_CCI			8
#define CONFIG_HAS_TZC			9
#define CONFIG_LIMIT			10

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

/* Base address where parameters to BL31 are stored */
#define PARAMS_BASE		TZDRAM_BASE


#define DRAM_BASE              0x80000000ull
#define DRAM_SIZE              0x80000000ull

#define PCIE_EXP_BASE		0x40000000
#define TZRNG_BASE		0x7fe60000
#define TZNVCTR_BASE		0x7fe70000
#define TZROOTKEY_BASE		0x7fe80000

/* Memory mapped Generic timer interfaces  */
#define SYS_CNTCTL_BASE		0x2a430000
#define SYS_CNTREAD_BASE	0x2a800000
#define SYS_TIMCTL_BASE		0x2a810000

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
#define V2M_SYS_ID			0x0
#define V2M_SYS_LED			0x8
#define V2M_SYS_CFGDATA		0xa0
#define V2M_SYS_CFGCTRL		0xa4

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

#define SYS_LED_SS_MASK		0x1
#define SYS_LED_EL_MASK		0x3
#define SYS_LED_EC_MASK		0x1f

/* V2M sysid register bits */
#define SYS_ID_REV_SHIFT	27
#define SYS_ID_HBI_SHIFT	16
#define SYS_ID_BLD_SHIFT	12
#define SYS_ID_ARCH_SHIFT	8
#define SYS_ID_FPGA_SHIFT	0

#define SYS_ID_REV_MASK	0xf
#define SYS_ID_HBI_MASK	0xfff
#define SYS_ID_BLD_MASK	0xf
#define SYS_ID_ARCH_MASK	0xf
#define SYS_ID_FPGA_MASK	0xff

#define SYS_ID_BLD_LENGTH	4

#define REV_FVP		0x0
#define HBI_FVP_BASE		0x020
#define HBI_FOUNDATION		0x010

#define BLD_GIC_VE_MMAP	0x0
#define BLD_GIC_A53A57_MMAP	0x1

#define ARCH_MODEL		0x1

/* FVP Power controller base address*/
#define PWRC_BASE		0x1c100000

/*******************************************************************************
 * Platform specific per affinity states. Distinction between off and suspend
 * is made to allow reporting of a suspended cpu as still being on e.g. in the
 * affinity_info psci call.
 ******************************************************************************/
#define PLATFORM_MAX_AFF0	4
#define PLATFORM_MAX_AFF1	2
#define PLAT_AFF_UNK		0xff

#define PLAT_AFF0_OFF		0x0
#define PLAT_AFF0_ONPENDING	0x1
#define PLAT_AFF0_SUSPEND	0x2
#define PLAT_AFF0_ON		0x3

#define PLAT_AFF1_OFF		0x0
#define PLAT_AFF1_ONPENDING	0x1
#define PLAT_AFF1_SUSPEND	0x2
#define PLAT_AFF1_ON		0x3

/*******************************************************************************
 * BL2 specific defines.
 ******************************************************************************/
#define BL2_BASE			0x0402D000

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
#define BL31_BASE			0x0400C000

/*******************************************************************************
 * BL32 specific defines.
 ******************************************************************************/
#define BL32_BASE			(TZDRAM_BASE + 0x2000)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_XLAT_TABLES			3
#define MAX_MMAP_REGIONS		16


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


/*******************************************************************************
 * TrustZone address space controller related constants
 ******************************************************************************/
#define TZC400_BASE			0x2a4a0000

/*
 * The NSAIDs for this platform as used to program the TZC400.
 */

/* The FVP has 4 bits of NSAIDs. Used with TZC FAIL_ID (ACE Lite ID width) */
#define FVP_AID_WIDTH			4

/* NSAIDs used by devices in TZC filter 0 on FVP */
#define FVP_NSAID_DEFAULT		0
#define FVP_NSAID_PCI			1
#define FVP_NSAID_VIRTIO		8  /* from FVP v5.6 onwards */
#define FVP_NSAID_AP			9  /* Application Processors */
#define FVP_NSAID_VIRTIO_OLD		15 /* until FVP v5.5 */

/* NSAIDs used by devices in TZC filter 2 on FVP */
#define FVP_NSAID_HDLCD0		2
#define FVP_NSAID_CLCD			7


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
#include <bl_common.h>


typedef volatile struct mailbox {
	unsigned long value
	__attribute__((__aligned__(CACHE_WRITEBACK_GRANULE)));
} mailbox_t;

/*******************************************************************************
 * This structure represents the superset of information that is passed to
 * BL31 e.g. while passing control to it from BL2 which is bl31_tf_params
 * and bl31_plat_params and its elements
 ******************************************************************************/
typedef struct bl2_to_bl31_params_mem {
	bl31_tf_params_t bl31_tf_params;
	bl31_plat_params_t bl31_plat_params;
	image_info_t bl31_image;
	image_info_t bl32_image;
	image_info_t bl33_image;
	el_change_info_t bl33_ep;
	el_change_info_t bl32_ep;
	el_change_info_t bl31_ep;
} bl2_to_bl31_params_mem_t;


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
extern void enable_mmu_el1(void);
extern void enable_mmu_el3(void);
extern void configure_mmu_el1(struct meminfo *mem_layout,
			      unsigned long ro_start,
			      unsigned long ro_limit,
			      unsigned long coh_start,
			      unsigned long coh_limit);
extern void configure_mmu_el3(struct meminfo *mem_layout,
			      unsigned long ro_start,
			      unsigned long ro_limit,
			      unsigned long coh_start,
			      unsigned long coh_limit);
extern unsigned long platform_get_cfgvar(unsigned int);
extern int platform_config_setup(void);
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

/* Declarations for plat_security.c */
extern void plat_security_setup(void);

/* Do adjustments to the entrypoint, mode etc for BL2 */
extern void bl1_plat_bl2_loaded(image_info_t *image, el_change_info_t *ep);

/* Do adjustments to the entrypoint, mode etc for BL31 */
extern void bl2_plat_bl31_loaded(image_info_t *image, el_change_info_t *ep);

/* Do adjustments to the entrypoint, mode etc  for BL32 */
extern void bl2_plat_bl32_loaded(image_info_t *image, el_change_info_t *ep);

/* Do adjustments to the entrypoint, mode etc for BL33 */
extern void bl2_plat_bl33_loaded(image_info_t *image, el_change_info_t *ep);


#endif /*__ASSEMBLY__*/

#endif /* __PLATFORM_H__ */
