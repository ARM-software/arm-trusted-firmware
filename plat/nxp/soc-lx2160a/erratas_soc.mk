#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 50426 workaround during reset.
ERRATA_SOC_A050426	?= 0

# Process ERRATA_SOC_A050426 flag
ifeq (${ERRATA_SOC_A050426}, 1)
INCL_SOC_ERRATA_SOURCES	:= yes
$(eval $(call add_define,ERRATA_SOC_A050426))
endif

ifeq (${INCL_SOC_ERRATA_SOURCES},yes)
BL2_SOURCES	+= 	${PLAT_SOC_PATH}/erratas_soc.c
endif
