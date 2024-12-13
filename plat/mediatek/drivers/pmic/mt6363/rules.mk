#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#Prologue, init variable
LOCAL_DIR := $(call GET_LOCAL_DIR)

#Define your module name
MODULE := mt6363

#Add your source code here
LOCAL_SRCS-y := $(LOCAL_DIR)/${PMIC_CHIP}_psc.c

#Epilogue, build as module
$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
