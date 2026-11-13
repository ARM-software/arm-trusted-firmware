#
# Copyright (c) 2016-2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef libfdt-mk
        libfdt-mk := $(lastword $(MAKEFILE_LIST))
        libfdt-root := contrib/dtc/libfdt

        include $(libfdt-root)/Makefile.libfdt

        LIBFDT_SRCS := $(addprefix $(libfdt-root)/,$(LIBFDT_SRCS))
        INCLUDES += -I$(libfdt-root)

        $(eval $(call MAKE_LIB,fdt))
endif
