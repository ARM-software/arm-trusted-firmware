#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${SPD},none)
        $(error "Error: SPD and SPM are incompatible build options.")
endif
ifneq (${ARCH},aarch64)
        $(error "Error: SPM is only supported on aarch64.")
endif

include lib/sprt/sprt_host.mk

SPM_SOURCES	:=	$(addprefix services/std_svc/spm/,	\
			${ARCH}/spm_helpers.S			\
			${ARCH}/spm_shim_exceptions.S		\
			spci.c					\
			spm_buffers.c				\
			spm_main.c				\
			spm_setup.c				\
			spm_xlat.c				\
			sprt.c)					\
			${SPRT_LIB_SOURCES}

INCLUDES	+=	${SPRT_LIB_INCLUDES}

# Let the top-level Makefile know that we intend to include a BL32 image
NEED_BL32		:=	yes
