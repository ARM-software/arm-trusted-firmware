/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef LS_INTERCONNECT_H
#define LS_INTERCONNECT_H

#if (INTERCONNECT == CCI400)
#define CCI_TERMINATE_BARRIER_TX	0x8
#endif

/* Interconnect CCI/CCN functions */
void plat_ls_interconnect_enter_coherency(unsigned int num_clusters);
void plat_ls_interconnect_exit_coherency(void);

#endif
