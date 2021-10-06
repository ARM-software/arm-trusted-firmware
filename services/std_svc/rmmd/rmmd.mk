#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: RMMD is only supported on aarch64.")
endif

include services/std_svc/rmmd/trp/trp.mk

RMMD_SOURCES	+=	$(addprefix services/std_svc/rmmd/,	\
			${ARCH}/rmmd_helpers.S			\
			rmmd_main.c)

# Let the top-level Makefile know that we intend to include RMM image
NEED_RMM	:=	yes
