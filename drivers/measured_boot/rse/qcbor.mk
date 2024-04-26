#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# TF-A was tested with v1.2 version of QCBOR

ifeq (${QCBOR_DIR},)
        $(error Error: QCBOR_DIR not set)
endif

QCBOR_SOURCES	+=	${QCBOR_DIR}/src/qcbor_encode.c \
			${QCBOR_DIR}/src/qcbor_decode.c \
			${QCBOR_DIR}/src/UsefulBuf.c

QCBOR_INCLUDES	+=	${QCBOR_DIR}/inc

# Floating point numbers are not used, so disable the support.
# This reduces the library size as well.
$(eval $(call add_define,QCBOR_DISABLE_FLOAT_HW_USE))
$(eval $(call add_define,USEFULBUF_DISABLE_ALL_FLOAT))
$(eval $(call add_define,QCBOR_DISABLE_PREFERRED_FLOAT))
