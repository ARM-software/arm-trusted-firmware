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


ifneq ($(wildcard $(SPMC_LP_SOURCES)),)
SPMC_SOURCES += $(SPMC_LP_SOURCES)
endif

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes

ifndef BL32
# The SPMC is paired with a Test Secure Payload source and we intend to
# build the Test Secure Payload if no other image has been provided
# for BL32.
#
# In cases where an associated Secure Payload lies outside this build
# system/source tree, the dispatcher Makefile can either invoke an external
# build command or assume it is pre-built.

BL32_ROOT		:=	bl32/tsp

# Conditionally include SP's Makefile. The assumption is that the TSP's build
# system is compatible with that of Trusted Firmware, and it'll add and populate
# necessary build targets and variables.

include ${BL32_ROOT}/tsp.mk
endif
