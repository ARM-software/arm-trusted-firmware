/*
 * Copyright 2021-2022 NXP
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

#ifdef ERRATA_SOC_A009660
void erratum_a009660(void);
#endif

#ifdef ERRATA_SOC_A010539
void erratum_a010539(void);
#endif

#endif /* ERRATA_LIST_H */
