/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_CSS_H
#define SOC_CSS_H

/*
 * Utility functions for ARM CSS SoCs
 */
void soc_css_init_nic400(void);
void soc_css_init_pcie(void);

static inline void soc_css_security_setup(void)
{
	soc_css_init_nic400();
	soc_css_init_pcie();
}

#endif /* SOC_CSS_H */
