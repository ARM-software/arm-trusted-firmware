#
# Copyright (C) 2021 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#

PCI_EP_SUPPORT		:= 0

CP_NUM			:= 1
$(eval $(call add_define,CP_NUM))

DOIMAGE_SEC     	:=	tools/doimage/secure/sec_img_7K.cfg

MARVELL_MOCHI_DRV	:=	drivers/marvell/mochi/apn806_setup.c

BOARD_DIR		:= $(shell dirname $(lastword $(MAKEFILE_LIST)))
include plat/marvell/armada/a8k/common/a8k_common.mk

include plat/marvell/armada/common/marvell_common.mk
