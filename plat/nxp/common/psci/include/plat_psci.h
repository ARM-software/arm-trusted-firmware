/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef PLAT_PSCI_H
#define PLAT_PSCI_H

 /* core abort current op */
#define CORE_ABORT_OP     0x1

 /* psci power levels - these are actually affinity levels
  * in the psci_power_state_t array
  */
#define PLAT_CORE_LVL  PSCI_CPU_PWR_LVL
#define PLAT_CLSTR_LVL U(1)
#define PLAT_SYS_LVL   U(2)
#define PLAT_MAX_LVL   PLAT_SYS_LVL

 /* core state */
 /* OFF states 0x0 - 0xF */
#define CORE_IN_RESET     0x0
#define CORE_DISABLED     0x1
#define CORE_OFF          0x2
#define CORE_STANDBY      0x3
#define CORE_PWR_DOWN     0x4
#define CORE_WFE          0x6
#define CORE_WFI          0x7
#define CORE_LAST	  0x8
#define CORE_OFF_PENDING  0x9
#define CORE_WORKING_INIT 0xA
#define SYS_OFF_PENDING   0xB
#define SYS_OFF           0xC

 /* ON states 0x10 - 0x1F */
#define CORE_PENDING      0x10
#define CORE_RELEASED     0x11
#define CORE_WAKEUP       0x12
 /* highest off state */
#define CORE_OFF_MAX	  0xF
 /* lowest on state */
#define CORE_ON_MIN       CORE_PENDING

#define  DAIF_SET_MASK          0x3C0
#define  SCTLR_I_C_M_MASK       0x00001005
#define  SCTLR_C_MASK           0x00000004
#define  SCTLR_I_MASK           0x00001000
#define  CPUACTLR_L1PCTL_MASK   0x0000E000
#define  DCSR_RCPM2_BASE        0x20170000
#define  CPUECTLR_SMPEN_MASK    0x40
#define  CPUECTLR_SMPEN_EN      0x40
#define  CPUECTLR_RET_MASK      0x7
#define  CPUECTLR_RET_SET       0x2
#define  CPUECTLR_TIMER_MASK    0x7
#define  CPUECTLR_TIMER_8TICKS  0x2
#define  SCR_IRQ_MASK           0x2
#define  SCR_FIQ_MASK           0x4

/* pwr mgmt features supported in the soc-specific code:
 *   value == 0x0, the soc code does not support this feature
 *   value != 0x0, the soc code supports this feature
 */
#define SOC_CORE_RELEASE      0x1
#define SOC_CORE_RESTART      0x1
#define SOC_CORE_OFF          0x1
#define SOC_CORE_STANDBY      0x1
#define SOC_CORE_PWR_DWN      0x1
#define SOC_CLUSTER_STANDBY   0x1
#define SOC_CLUSTER_PWR_DWN   0x1
#define SOC_SYSTEM_STANDBY    0x1
#define SOC_SYSTEM_PWR_DWN    0x1
#define SOC_SYSTEM_OFF        0x1
#define SOC_SYSTEM_RESET      0x1
#define SOC_SYSTEM_RESET2     0x1

#ifndef __ASSEMBLER__

void __dead2 _psci_system_reset(void);
void __dead2 _psci_system_off(void);
int _psci_cpu_on(u_register_t core_mask);
void _psci_cpu_prep_off(u_register_t core_mask);
void __dead2 _psci_cpu_off_wfi(u_register_t core_mask,
				u_register_t wakeup_address);
void __dead2 _psci_cpu_pwrdn_wfi(u_register_t core_mask,
				u_register_t wakeup_address);
void __dead2 _psci_sys_pwrdn_wfi(u_register_t core_mask,
				u_register_t wakeup_address);
void _psci_wakeup(u_register_t core_mask);
void _psci_core_entr_stdby(u_register_t core_mask);
void _psci_core_prep_stdby(u_register_t core_mask);
void _psci_core_exit_stdby(u_register_t core_mask);
void _psci_core_prep_pwrdn(u_register_t core_mask);
void _psci_core_exit_pwrdn(u_register_t core_mask);
void _psci_clstr_prep_stdby(u_register_t core_mask);
void _psci_clstr_exit_stdby(u_register_t core_mask);
void _psci_clstr_prep_pwrdn(u_register_t core_mask);
void _psci_clstr_exit_pwrdn(u_register_t core_mask);
void _psci_sys_prep_stdby(u_register_t core_mask);
void _psci_sys_exit_stdby(u_register_t core_mask);
void _psci_sys_prep_pwrdn(u_register_t core_mask);
void _psci_sys_exit_pwrdn(u_register_t core_mask);

#endif

#endif /* __PLAT_PSCI_H__ */
