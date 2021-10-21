/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <common/debug.h>

#include "errata_list.h"

void soc_errata(void)
{
#ifdef ERRATA_SOC_A050426
	INFO("SoC workaround for Errata A050426 was applied\n");
	erratum_a050426();
#endif
#ifdef ERRATA_SOC_A008850
	INFO("SoC workaround for Errata A008850 Early-Phase was applied\n");
	erratum_a008850_early();
#endif
	/*
	 * The following DDR Erratas workaround are implemented in DDR driver,
	 * but print information here.
	 */
#if ERRATA_DDR_A011396
	INFO("SoC workaround for DDR Errata A011396 was applied\n");
#endif
#if ERRATA_DDR_A050450
	INFO("SoC workaround for DDR Errata A050450 was applied\n");
#endif
}
