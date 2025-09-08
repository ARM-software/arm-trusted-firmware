#
# Copyright (c) 2021-2025, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: RMMD is only supported on aarch64.")
endif

# Include TRP makefile only if RMM is not defined.
ifeq ($(RMM),)
        include services/std_svc/rmmd/trp/trp.mk
endif

RMMD_SOURCES	+=	$(addprefix services/std_svc/rmmd/,	\
			${ARCH}/rmmd_helpers.S			\
			rmmd_main.c				\
			rmmd_attest.c				\
			rmmd_keymgmt.c				\
			rmmd_mem.c)

# Let the top-level Makefile know that we intend to include RMM image
NEED_RMM	:=	yes
