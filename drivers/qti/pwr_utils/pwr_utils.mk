#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Power utils (vlvl <-> hlvl) driver
#

$(eval $(call add_define,QTI_PWR_UTILS_ENABLED))

PWR_UTILS_DRV_PATH := drivers/qti/pwr_utils
CMD_DB_DRV_PATH := drivers/qti/cmd_db

BL31_SOURCES += \
	$(PWR_UTILS_DRV_PATH)/pwr_utils.c			\
	$(CMD_DB_DRV_PATH)/cmd_db.c
