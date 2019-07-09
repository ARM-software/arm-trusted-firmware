/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HISI_PWRC_H
#define HISI_PWRC_H

#ifndef __ASSEMBLER__

void hisi_pwrc_set_cluster_wfi(unsigned int id);
void hisi_pwrc_set_core_bx_addr(unsigned int core,
				unsigned int cluster,
				uintptr_t entry_point);
void hisi_pwrc_enable_debug(unsigned int core,
			    unsigned int cluster);
int hisi_pwrc_setup(void);

#endif /*__ASSEMBLER__*/

#endif /* HISI_PWRC_H */
