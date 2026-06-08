#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# RPMh (Resource Power Manager hardened) command service driver
#

$(eval $(call add_define,QTI_RPMH_ENABLED))

RPMH_DRV_PATH := drivers/qti/rpmh

PLAT_INCLUDES += \
	-I$(RPMH_DRV_PATH) \
	-I$(RPMH_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(RPMH_DRV_PATH)/rpmh_client.c
