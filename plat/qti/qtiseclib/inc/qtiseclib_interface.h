#ifndef __QTISECLIB_INTERFACE_H__
#define __QTISECLIB_INTERFACE_H__

/********************************************************************
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#include <stdint.h>
#include <qtiseclib_defs.h>

/* qtiseclib published api's. */
u_register_t qtiseclib_get_stack_protector_canary(void);
void qtiseclib_cpuss_boot_setup(void);
void qtiseclib_bl31_platform_setup(void);
void qtiseclib_bl31_early_platform_setup(void);
void qtiseclib_kryo3_gold_reset_func(void);
void qtiseclib_kryo3_silver_reset_func(void);
void qtiseclib_invoke_isr(uint32_t irq, void *handle);
void qtiseclib_sip_syscall(qtiseclib_smc_param_t * p_smcparams,
			   qtiseclib_smc_rsp_t * p_smcrsps);
void qtiseclib_panic(void);

int qtiseclib_psci_init(void);
int qtiseclib_psci_node_power_on(u_register_t mpidr);
void qtiseclib_psci_node_on_finish(const uint8_t * states);
void qtiseclib_psci_cpu_standby(uint8_t pwr_state);
void qtiseclib_psci_node_power_off(const uint8_t * states);
void qtiseclib_psci_node_suspend(const uint8_t * states);
void qtiseclib_psci_node_suspend_finish(const uint8_t * states);
int qtiseclib_psci_validate_power_state(unsigned int pwr_state, uint8_t * req_state);

#endif /* __QTISECLIB_INTERFACE_H__ */
