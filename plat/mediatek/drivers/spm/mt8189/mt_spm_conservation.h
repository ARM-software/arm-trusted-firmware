/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_CONSERVATION_H
#define MT_SPM_CONSERVATION_H

#include <mt_spm.h>
#include <mt_spm_internal.h>

int spm_conservation(int state_id, uint32_t ext_opand,
		     struct spm_lp_scen *spm_lp, uint32_t resource_req);

void spm_conservation_finish(int state_id, uint32_t ext_opand,
			     struct spm_lp_scen *spm_lp,
			     struct wake_status **status);

int spm_conservation_get_result(struct wake_status **res);

int spm_conservation_fw_run(uint32_t first, void *pwrctrl);

int spm_conservation_wakeup_obs(int is_set, int cat, uint32_t wake_src_bits);

#endif /* MT_SPM_CONSERVATION_H */
