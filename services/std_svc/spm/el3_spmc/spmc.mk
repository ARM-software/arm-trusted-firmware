#
# Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: SPMC is only supported on aarch64.")
endif

SPMC_SOURCES	:=	$(addprefix services/std_svc/spm/el3_spmc/,	\
			spmc_main.c				\
			spmc_setup.c				\
			logical_sp.c				\
			spmc_pm.c				\
			spmc_shared_mem.c)

# Specify platform specific logical partition implementation.
SPMC_LP_SOURCES  := $(addprefix ${PLAT_DIR}/, \
                    ${PLAT}_el3_spmc_logical_sp.c)


SPMC_SOURCES += $(SPMC_LP_SOURCES)

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes
