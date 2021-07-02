/*
 * Copyright (c) 2021, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_CONSERVATION_H
#define MT_SPM_CONSERVATION_H

#include <mt_spm_internal.h>

extern int spm_conservation(int state_id, unsigned int ext_opand,
			    struct spm_lp_scen *spm_lp,
			    unsigned int resource_req);
extern void spm_conservation_finish(int state_id, unsigned int ext_opand,
				    struct spm_lp_scen *spm_lp,
				    struct wake_status **status);
extern int spm_conservation_get_result(struct wake_status **res);
extern void spm_conservation_pwrctrl_init(struct pwr_ctrl *pwrctrl);
#endif /* MT_SPM_CONSERVATION_H */
