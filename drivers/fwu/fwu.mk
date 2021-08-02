#
# Copyright (c) 2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

FWU_SRC_DIR	:= drivers/fwu/

FWU_SRCS	:= ${FWU_SRC_DIR}fwu.c

BL2_SOURCES	+= ${FWU_SRCS}
