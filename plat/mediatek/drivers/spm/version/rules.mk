#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LOCAL_DIR := $(call GET_LOCAL_DIR)

MODULE := spm_version

LOCAL_SRCS-y :=
ifneq (${NOTIFIER_VER},)
LOCAL_SRCS-y += ${LOCAL_DIR}/notifier/${NOTIFIER_VER}/mt_spm_sspm_notifier.c
endif
ifneq (${PMIC_GS_DUMP_VER},)
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic_gs/${PMIC_GS_DUMP_VER}/mt_spm_pmic_gs_dump.c
endif
ifneq (${PMIC_WRAP_VER},)
LOCAL_SRCS-y += ${LOCAL_DIR}/pmic_wrap/${PMIC_WRAP_VER}/mt_spm_pmic_wrap.c
endif

$(eval $(call MAKE_MODULE,$(MODULE),$(LOCAL_SRCS-y),$(MTK_BL)))
$(eval $(call add_defined_option,CONFIG_MTK_VCOREDVFS_SUPPORT))
