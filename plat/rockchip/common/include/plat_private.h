/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__

#ifndef __ASSEMBLY__
#include <mmio.h>
#include <psci.h>
#include <stdint.h>
#include <xlat_tables.h>

#define __sramdata __attribute__((section(".sram.data")))
#define __sramconst __attribute__((section(".sram.rodata")))
#define __sramfunc __attribute__((section(".sram.text")))

#define __pmusramdata __attribute__((section(".pmusram.data")))
#define __pmusramconst __attribute__((section(".pmusram.rodata")))
#define __pmusramfunc __attribute__((section(".pmusram.text")))

extern uint32_t __bl31_sram_text_start, __bl31_sram_text_end;
extern uint32_t __bl31_sram_data_start, __bl31_sram_data_end;
extern uint32_t __bl31_sram_stack_start, __bl31_sram_stack_end;
extern uint32_t __bl31_sram_text_real_end, __bl31_sram_data_real_end;
extern uint32_t __sram_incbin_start, __sram_incbin_end;
extern uint32_t __sram_incbin_real_end;


/******************************************************************************
 * The register have write-mask bits, it is mean, if you want to set the bits,
 * you needs set the write-mask bits at the same time,
 * The write-mask bits is in high 16-bits.
 * The fllowing macro definition helps access write-mask bits reg efficient!
 ******************************************************************************/
#define REG_MSK_SHIFT	16

#ifndef WMSK_BIT
#define WMSK_BIT(nr)		BIT((nr) + REG_MSK_SHIFT)
#endif

/* set one bit with write mask */
#ifndef BIT_WITH_WMSK
#define BIT_WITH_WMSK(nr)	(BIT(nr) | WMSK_BIT(nr))
#endif

#ifndef BITS_SHIFT
#define BITS_SHIFT(bits, shift)	(bits << (shift))
#endif

#ifndef BITS_WITH_WMASK
#define BITS_WITH_WMASK(bits, msk, shift)\
	(BITS_SHIFT(bits, shift) | BITS_SHIFT(msk, (shift + REG_MSK_SHIFT)))
#endif

/******************************************************************************
 * Function and variable prototypes
 *****************************************************************************/
void plat_configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long,
			    unsigned long,
			    unsigned long,
			    unsigned long);

void plat_cci_init(void);
void plat_cci_enable(void);
void plat_cci_disable(void);

void plat_delay_timer_init(void);

void params_early_setup(void *plat_params_from_bl2);

void plat_rockchip_gic_driver_init(void);
void plat_rockchip_gic_init(void);
void plat_rockchip_gic_cpuif_enable(void);
void plat_rockchip_gic_cpuif_disable(void);
void plat_rockchip_gic_pcpu_init(void);

void plat_rockchip_pmu_init(void);
void plat_rockchip_soc_init(void);
uintptr_t plat_get_sec_entrypoint(void);

void platform_cpu_warmboot(void);

struct gpio_info *plat_get_rockchip_gpio_reset(void);
struct gpio_info *plat_get_rockchip_gpio_poweroff(void);
struct gpio_info *plat_get_rockchip_suspend_gpio(uint32_t *count);
struct apio_info *plat_get_rockchip_suspend_apio(void);
void plat_rockchip_gpio_init(void);
void plat_rockchip_save_gpio(void);
void plat_rockchip_restore_gpio(void);

int rockchip_soc_cores_pwr_dm_on(unsigned long mpidr, uint64_t entrypoint);
int rockchip_soc_hlvl_pwr_dm_off(uint32_t lvl,
				 plat_local_state_t lvl_state);
int rockchip_soc_cores_pwr_dm_off(void);
int rockchip_soc_sys_pwr_dm_suspend(void);
int rockchip_soc_cores_pwr_dm_suspend(void);
int rockchip_soc_hlvl_pwr_dm_suspend(uint32_t lvl,
				     plat_local_state_t lvl_state);
int rockchip_soc_hlvl_pwr_dm_on_finish(uint32_t lvl,
				       plat_local_state_t lvl_state);
int rockchip_soc_cores_pwr_dm_on_finish(void);
int rockchip_soc_sys_pwr_dm_resume(void);

int rockchip_soc_hlvl_pwr_dm_resume(uint32_t lvl,
				    plat_local_state_t lvl_state);
int rockchip_soc_cores_pwr_dm_resume(void);
void __dead2 rockchip_soc_soft_reset(void);
void __dead2 rockchip_soc_system_off(void);
void __dead2 rockchip_soc_cores_pd_pwr_dn_wfi(
				const psci_power_state_t *target_state);
void __dead2 rockchip_soc_sys_pd_pwr_dn_wfi(void);

extern const unsigned char rockchip_power_domain_tree_desc[];

extern void *pmu_cpuson_entrypoint;
extern uint64_t cpuson_entry_point[PLATFORM_CORE_COUNT];
extern uint32_t cpuson_flags[PLATFORM_CORE_COUNT];

extern const mmap_region_t plat_rk_mmap[];

void rockchip_plat_mmu_el3(void);

#endif /* __ASSEMBLY__ */

/******************************************************************************
 * cpu up status
 * The bits of macro value is not more than 12 bits for cmp instruction!
 ******************************************************************************/
#define PMU_CPU_HOTPLUG		0xf00
#define PMU_CPU_AUTO_PWRDN	0xf0
#define PMU_CLST_RET	0xa5

#endif /* __PLAT_PRIVATE_H__ */
