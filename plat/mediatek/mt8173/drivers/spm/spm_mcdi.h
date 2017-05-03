/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __SPM_MCDI_H__
#define __SPM_MCDI_H__

void spm_mcdi_wakeup_all_cores(void);
void spm_mcdi_prepare_for_mtcmos(void);
void spm_mcdi_prepare_for_off_state(unsigned long mpidr, unsigned int afflvl);
void spm_mcdi_finish_for_on_state(unsigned long mpidr, unsigned int afflvl);

#endif /* __SPM_MCDI_H__ */
