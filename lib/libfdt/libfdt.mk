#
# Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifndef libfdt-mk
        libfdt-mk := 1

        LIBFDT_SRCS := $(addprefix lib/libfdt/, \
                fdt.c \
                fdt_addresses.c \
                fdt_empty_tree.c \
                fdt_ro.c \
                fdt_rw.c \
                fdt_strerror.c \
                fdt_sw.c \
                fdt_wip.c)

        INCLUDES += -Iinclude/lib/libfdt

        $(eval $(call MAKE_LIB,fdt))
endif
