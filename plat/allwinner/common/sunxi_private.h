/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SUNXI_PRIVATE_H__
#define __SUNXI_PRIVATE_H__

void sunxi_configure_mmu_el3(int flags);
void sunxi_cpu_off(unsigned int cluster, unsigned int core);
void sunxi_cpu_on(unsigned int cluster, unsigned int core);
void sunxi_disable_secondary_cpus(unsigned int primary_cpu);

uint16_t sunxi_read_soc_id(void);

void sunxi_pmic_setup(void);
void sunxi_security_setup(void);

void __dead2 sunxi_power_down(void);

#endif /* __SUNXI_PRIVATE_H__ */
