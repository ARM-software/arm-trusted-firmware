/*
 * Copyright 2021-2022 NXP
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
#if ERRATA_SOC_A009660
	INFO("SoC workaround for Errata A009660 was applied\n");
	erratum_a009660();
#endif
#if ERRATA_SOC_A010539
	INFO("SoC workaround for Errata A010539 was applied\n");
	erratum_a010539();
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
#if ERRATA_DDR_A050958
	INFO("SoC workaround for DDR Errata A050958 was applied\n");
#endif
#if ERRATA_DDR_A008511
	INFO("SoC workaround for DDR Errata A008511 was applied\n");
#endif
#if ERRATA_DDR_A009803
	INFO("SoC workaround for DDR Errata A009803 was applied\n");
#endif
#if ERRATA_DDR_A009942
	INFO("SoC workaround for DDR Errata A009942 was applied\n");
#endif
#if ERRATA_DDR_A010165
	INFO("SoC workaround for DDR Errata A010165 was applied\n");
#endif
#if ERRATA_DDR_A009663
	INFO("SoC workaround for DDR Errata A009663 was applied\n");
#endif
}
