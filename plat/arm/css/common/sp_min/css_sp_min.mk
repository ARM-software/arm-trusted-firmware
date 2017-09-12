#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP MIN source files common to CSS platforms
BL32_SOURCES		+=	plat/arm/css/common/css_pm.c			\
				plat/arm/css/common/css_topology.c

ifeq (${CSS_USE_SCMI_SDS_DRIVER},0)
BL32_SOURCES		+=	plat/arm/css/drivers/scp/css_pm_scpi.c		\
				plat/arm/css/drivers/scpi/css_mhu.c		\
				plat/arm/css/drivers/scpi/css_scpi.c
else
BL32_SOURCES		+=	plat/arm/css/drivers/scp/css_pm_scmi.c		\
				plat/arm/css/drivers/scmi/scmi_common.c		\
				plat/arm/css/drivers/scmi/scmi_pwr_dmn_proto.c	\
				plat/arm/css/drivers/scmi/scmi_sys_pwr_proto.c
endif
