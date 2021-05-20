#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: RMMD is only supported on aarch64.")
endif

# In cases where an associated Realm Payload lies outside this build
# system/source tree, then the dispatcher Makefile can either invoke an external
# build command or assume it pre-built

BL32_ROOT		:=	bl32/trp

# Include SP's Makefile. The assumption is that the TRP's build system is
# compatible with that of Trusted Firmware, and it'll add and populate necessary
# build targets and variables
include ${BL32_ROOT}/trp.mk

RMMD_SOURCES	+=	$(addprefix services/std_svc/rmmd/,	\
			${ARCH}/rmmd_helpers.S			\
			rmmd_main.c)

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32	:=	yes
