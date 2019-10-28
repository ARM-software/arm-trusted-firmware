#
# Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: SPMD is only supported on aarch64.")
endif

SPMD_SOURCES	+=	$(addprefix services/std_svc/spmd/,	\
			${ARCH}/spmd_helpers.S			\
			spmd_pm.c				\
			spmd_main.c)

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes

# Enable dynamic memory mapping
# The SPMD component maps the SPMC DTB within BL31 virtual space.
PLAT_XLAT_TABLES_DYNAMIC :=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
