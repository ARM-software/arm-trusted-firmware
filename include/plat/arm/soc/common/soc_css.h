/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __SOC_CSS_H__
#define __SOC_CSS_H__

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


#endif /* __SOC_CSS_H__ */
