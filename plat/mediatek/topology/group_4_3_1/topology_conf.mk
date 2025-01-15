#
# Copyright (c) 2025, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_MAX_PWR_LVL := 2
$(eval $(call add_defined_option,PLAT_MAX_PWR_LVL))

PLAT_MAX_OFF_STATE := 2
$(eval $(call add_defined_option,PLAT_MAX_OFF_STATE))

PLAT_AFFLV_SYSTEM := 2
$(eval $(call add_defined_option,PLAT_AFFLV_SYSTEM))

PLAT_AFFLV_CLUSTER := 1
$(eval $(call add_defined_option,PLAT_AFFLV_CLUSTER))

PLAT_AFFLV_MCUSYS := 2
$(eval $(call add_defined_option,PLAT_AFFLV_MCUSYS))
