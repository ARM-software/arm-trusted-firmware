#
# Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${SPD},none)
        $(error "Error: SPD and SPM_MM are incompatible build options.")
endif
ifneq (${ARCH},aarch64)
        $(error "Error: SPM_MM is only supported on aarch64.")
endif
ifneq (${ENABLE_SVE_FOR_NS},0)
        $(error "Error: SPM_MM is not compatible with ENABLE_SVE_FOR_NS")
endif
ifneq (${ENABLE_SME_FOR_NS},0)
        $(error "Error: SPM_MM is not compatible with ENABLE_SME_FOR_NS")
endif
ifeq (${CTX_INCLUDE_FPREGS},0)
        $(warning "Warning: SPM_MM: CTX_INCLUDE_FPREGS is set to 0")
endif

SPM_MM_SOURCES	:=	$(addprefix services/std_svc/spm/spm_mm/,	\
			spm_mm_main.c					\
			spm_mm_setup.c					\
			spm_mm_xlat.c)


# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes
