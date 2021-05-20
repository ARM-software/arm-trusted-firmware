#
# Copyright (c) 2021 Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_SOURCES		+=	bl32/trp/trp_entry.S		\
				bl32/trp/trp_main.c

BL32_LINKERFILE		:=	bl32/trp/linker.lds

# Include the platform-specific TRP Makefile
# If no platform-specific TRP Makefile exists, it means TRP is not supported
# on this platform.
TRP_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/trp/trp-${PLAT}.mk)
ifeq (,${TRP_PLAT_MAKEFILE})
  $(error TRP is not supported on platform ${PLAT})
else
  include ${TRP_PLAT_MAKEFILE}
endif
