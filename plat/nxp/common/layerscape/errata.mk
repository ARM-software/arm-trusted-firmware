#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Platform Errata Build flags.
# These should be enabled by the platform if the erratum workaround needs to be
# applied.

# Flag to apply erratum 8850 workaround during reset.
ERRATA_LS_A008850	?=0

# Process ERRATA_LS_A008850 flag
$(eval $(call assert_boolean,ERRATA_LS_A008850))
$(eval $(call add_define,ERRATA_LS_A008850))

# Flag to apply erratum 9660 workaround during reset.
ERRATA_LS_A009660	?=0

# Process ERRATA_LS_A009660 flag
$(eval $(call assert_boolean,ERRATA_LS_A009660))
$(eval $(call add_define,ERRATA_LS_A009660))

ERRATA_LS_A010539	?=0

# Process ERRATA_LS_A010539 flag
$(eval $(call assert_boolean,ERRATA_LS_A010539))
$(eval $(call add_define,ERRATA_LS_A010539))

BL2_SOURCES	+= 	${PLAT_COMMON_PATH}/layerscape/ls_errata.c
