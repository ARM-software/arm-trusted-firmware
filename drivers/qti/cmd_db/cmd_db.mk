#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RPMh command database driver
#

$(eval $(call add_define,QTI_CMD_DB_ENABLED))

CMD_DB_DRV_PATH := drivers/qti/cmd_db

BL31_SOURCES += \
	$(CMD_DB_DRV_PATH)/cmd_db.c
