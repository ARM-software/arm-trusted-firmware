#
# Copyright (c) 2022, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# call add_defined_option to evaluate MTK defined value
$(eval $(call add_defined_option,MTK_SIP_KERNEL_BOOT_ENABLE))
$(eval $(call add_defined_option,PLAT_EXTRA_RODATA_INCLUDES))
$(eval $(call add_defined_option,MTK_EXTRA_LINKERFILE))
$(eval $(call add_defined_option,MTK_BL31_AS_BL2))
$(eval $(call add_defined_option,MTK_BL33_IS_64BIT))
$(eval $(call add_defined_option,PLAT_XLAT_TABLES_DYNAMIC))
$(eval $(call add_defined_option,MTK_ADAPTED))
$(eval $(call add_defined_option,MTK_PUBEVENT_ENABLE))
$(eval $(call add_defined_option,MTK_SOC))
$(eval $(call add_defined_option,UART_CLOCK))
$(eval $(call add_defined_option,UART_BAUDRATE))
$(eval $(call add_defined_option,CONFIG_MTK_MCUSYS))
$(eval $(call add_defined_option,CONFIG_MTK_PM_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_CPU_PM_SUPPORT))
$(eval $(call add_defined_option,CONFIG_MTK_SMP_EN))
$(eval $(call add_defined_option,CONFIG_MTK_CPU_SUSPEND_EN))
$(eval $(call add_defined_option,CONFIG_MTK_PM_ARCH))
$(eval $(call add_defined_option,CONFIG_MTK_CPU_PM_ARCH))
