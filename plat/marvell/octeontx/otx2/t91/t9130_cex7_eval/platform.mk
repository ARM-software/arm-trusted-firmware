#
# Copyright (C) 2018 Marvell International Ltd.
# Copyright (C) 2021 Semihalf.
#
# SPDX-License-Identifier:	BSD-3-Clause
# https://spdx.org/licenses
#

PCI_EP_SUPPORT		:=	0

CP_NUM			:=	1
$(eval $(call add_define,CP_NUM))

DOIMAGE_SEC     	:=	tools/doimage/secure/sec_img_7K.cfg

MARVELL_MOCHI_DRV	:=	drivers/marvell/mochi/ap807_setup.c

BOARD_DIR		:=	$(shell dirname $(lastword $(MAKEFILE_LIST)))

#
# CN913X CEx7 Evaluation Board shares the DRAM connectivity
# and SerDes settings with the CN913X DB - reuse relevant
# board-specific files.
#
T9130_DIR		:=	$(BOARD_DIR)/../t9130
PLAT_INCLUDES		:=	-I$(T9130_DIR)				\
				-I$(T9130_DIR)/board
BLE_PORTING_SOURCES	:=	$(T9130_DIR)/board/dram_port.c		\
				$(BOARD_DIR)/board/marvell_plat_config.c

include plat/marvell/armada/a8k/common/a8k_common.mk

include plat/marvell/armada/common/marvell_common.mk
