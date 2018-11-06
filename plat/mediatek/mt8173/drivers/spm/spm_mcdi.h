/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SPM_MCDI_H
#define SPM_MCDI_H

void spm_mcdi_wakeup_all_cores(void);
void spm_mcdi_prepare_for_mtcmos(void);
void spm_mcdi_prepare_for_off_state(unsigned long mpidr, unsigned int afflvl);
void spm_mcdi_finish_for_on_state(unsigned long mpidr, unsigned int afflvl);

#endif /* SPM_MCDI_H */
