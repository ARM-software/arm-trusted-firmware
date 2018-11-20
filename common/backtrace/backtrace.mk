#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Enable backtrace by default in DEBUG AArch64 builds
ifeq (${ARCH},aarch32)
        ENABLE_BACKTRACE 	:=	0
else
        ENABLE_BACKTRACE 	:=	${DEBUG}
endif

ifeq (${ENABLE_BACKTRACE},1)
        # Force the compiler to include the frame pointer
        TF_CFLAGS		+=	-fno-omit-frame-pointer

        BL_COMMON_SOURCES	+=	common/backtrace/backtrace.c
endif

ifeq (${ARCH},aarch32)
        ifeq (${ENABLE_BACKTRACE},1)
                ifneq (${AARCH32_INSTRUCTION_SET},A32)
                        $(error Error: AARCH32_INSTRUCTION_SET=A32 is needed \
                        for ENABLE_BACKTRACE when compiling for AArch32.)
                endif
        endif
endif

$(eval $(call assert_boolean,ENABLE_BACKTRACE))
$(eval $(call add_define,ENABLE_BACKTRACE))
