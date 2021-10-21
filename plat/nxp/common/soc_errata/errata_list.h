/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef ERRATA_LIST_H
#define ERRATA_LIST_H

#ifdef ERRATA_SOC_A050426
void erratum_a050426(void);
#endif

#ifdef ERRATA_SOC_A008850
void erratum_a008850_early(void);
void erratum_a008850_post(void);
#endif

#endif /* ERRATA_LIST_H */
