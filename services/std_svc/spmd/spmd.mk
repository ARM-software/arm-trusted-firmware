#
# Copyright (c) 2021-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

SPMD_SOURCES	+=	$(addprefix services/std_svc/spmd/,	\
			${ARCH}/spmd_helpers.S			\
			spmd_pm.c				\
			spmd_main.c				\
			spmd_logical_sp.c)

# Specify platform specific SPMD logical partition implementation.
SPMD_LP_SOURCES  := $(wildcard $(addprefix ${PLAT_DIR}/, \
					${PLAT}_spmd_logical_sp*.c))

ifeq (${ENABLE_SPMD_LP}, 1)
ifneq ($(wildcard $(SPMD_LP_SOURCES)),)
SPMD_SOURCES += $(SPMD_LP_SOURCES)
endif
endif

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes

# Enable dynamic memory mapping
# The SPMD component maps the SPMC DTB within BL31 virtual space.
PLAT_XLAT_TABLES_DYNAMIC :=	1
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
