#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

QCBOR_SOURCES	+=	contrib/qcbor/src/qcbor_encode.c \
			contrib/qcbor/src/qcbor_decode.c \
			contrib/qcbor/src/UsefulBuf.c

QCBOR_INCLUDES	+=	contrib/qcbor/inc

# Floating point numbers are not used, so disable the support.
# This reduces the library size as well.
$(eval $(call add_define,QCBOR_DISABLE_FLOAT_HW_USE))
$(eval $(call add_define,USEFULBUF_DISABLE_ALL_FLOAT))
$(eval $(call add_define,QCBOR_DISABLE_PREFERRED_FLOAT))
