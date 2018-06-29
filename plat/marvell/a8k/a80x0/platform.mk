#
# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses
#

PCI_EP_SUPPORT		:= 0

CP_NUM			:= 2
$(eval $(call add_define,CP_NUM))

DOIMAGE_SEC     	:=	tools/doimage/secure/sec_img_8K.cfg

MARVELL_MOCHI_DRV	:=	drivers/marvell/mochi/apn806_setup.c

include plat/marvell/a8k/common/a8k_common.mk

include plat/marvell/common/marvell_common.mk
PLAT_INCLUDES		+=	-Iplat/marvell/a8k/a80x0/board
