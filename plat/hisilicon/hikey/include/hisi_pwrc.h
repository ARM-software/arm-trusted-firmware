/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HISI_PWRC_H__
#define __HISI_PWRC_H__

#ifndef __ASSEMBLY__

void hisi_pwrc_set_cluster_wfi(unsigned int id);
void hisi_pwrc_set_core_bx_addr(unsigned int core,
				unsigned int cluster,
				uintptr_t entry_point);
int hisi_pwrc_setup(void);

#endif /*__ASSEMBLY__*/

#endif /* __HISI_PWRC_H__ */
