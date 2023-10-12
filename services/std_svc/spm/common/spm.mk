#
# Copyright (c) 2022-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH},aarch64)
        $(error "Error: SPM is only supported on aarch64.")
endif

INCLUDES	+=	-Iservices/std_svc/spm/common/include

SPM_SOURCES	:=	$(addprefix services/std_svc/spm/common/,	\
			${ARCH}/spm_helpers.S				\
			${ARCH}/spm_shim_exceptions.S)

ifeq (1, $(filter 1, ${SPM_MM} ${SPMC_AT_EL3_SEL0_SP}))
SPM_SOURCES	+=	$(addprefix services/std_svc/spm/common/,       \
			spm_xlat_common.c)
endif

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes
