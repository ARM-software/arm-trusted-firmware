#
# Copyright (c) 2021-2022 Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

RMM_SOURCES		+=	services/std_svc/rmmd/trp/trp_entry.S \
				services/std_svc/rmmd/trp/trp_main.c  \
				services/std_svc/rmmd/trp/trp_helpers.c

RMM_LINKERFILE		:=	services/std_svc/rmmd/trp/linker.lds

# Include the platform-specific TRP Makefile
# If no platform-specific TRP Makefile exists, it means TRP is not supported
# on this platform.
TRP_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/trp/trp-${PLAT}.mk)
ifeq (,${TRP_PLAT_MAKEFILE})
  $(error TRP is not supported on platform ${PLAT})
else
  include ${TRP_PLAT_MAKEFILE}
endif
