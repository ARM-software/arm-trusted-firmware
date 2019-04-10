/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLAT_PM_H__
#define __PLAT_PM_H__

#ifndef __ASSEMBLY__
extern void gic_cpuif_init(void);
extern void gic_rdist_save(void);
extern void gic_rdist_restore(void);
extern void gic_rdist_restore_all(void);
extern void gic_cpuif_deactivate(unsigned int gicc_base);
extern void gic_dist_save(void);
extern void gic_dist_restore(void);
extern void gic_setup(void);
extern void gic_sgi_save_all(void);
extern void gic_sgi_restore_all(void);
extern void gic_sync_dcm_enable(void);
extern void gic_sync_dcm_disable(void);

extern void dfd_resume(void);
#endif

#endif /* __PLAT_PM_H__ */
