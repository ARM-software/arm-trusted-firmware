#
# Copyright (c) 2026, Texas Instruments Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

DTB_FILE_NAME ?= k3-am62l-evm.dtb

BL32_BASE ?= 0x80200000
$(eval $(call add_define,BL32_BASE))

PRELOADED_BL33_BASE ?= 0x82000000
$(eval $(call add_define,PRELOADED_BL33_BASE))

K3_HW_CONFIG_BASE ?= 0x88000000
$(eval $(call add_define,K3_HW_CONFIG_BASE))

include ${PLAT_PATH}/../common/scmi/ti_scmi.mk
include drivers/ti/clk/ti_clk.mk
include drivers/ti/pd/ti_pd.mk
include ${PLAT_PATH}/board/am62lx/pm/soc_pm.mk

BL31_SOURCES		+=	\
				plat/ti/common/k3_svc.c		\
				drivers/scmi-msg/base.c		\
				drivers/scmi-msg/entry.c	\
				drivers/scmi-msg/smt.c		\
				drivers/scmi-msg/clock.c	\
				drivers/scmi-msg/power_domain.c \

BL1_SOURCES             +=      \
                                ${PLAT_PATH}/board/${TARGET_BOARD}/board_config.c   \

