#
# Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP MIN source files common to CSS platforms
BL32_SOURCES		+=	plat/arm/css/common/css_pm.c			\
				plat/arm/css/common/css_topology.c

ifeq (${CSS_USE_SCMI_SDS_DRIVER},0)
BL32_SOURCES		+=	drivers/arm/css/mhu/css_mhu.c			\
				drivers/arm/css/scp/css_pm_scpi.c		\
				drivers/arm/css/scpi/css_scpi.c
else
BL32_SOURCES		+=	drivers/arm/css/mhu/css_mhu_doorbell.c		\
				drivers/arm/css/scp/css_pm_scmi.c		\
				drivers/arm/css/scmi/scmi_common.c		\
				drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
				drivers/arm/css/scmi/scmi_sys_pwr_proto.c
endif
