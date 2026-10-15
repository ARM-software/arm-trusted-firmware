#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# CPUCP (CPU Control Processor) driver
#

$(eval $(call add_define,QTI_CPUCP_ENABLED))

CPUCP_DRV_PATH := drivers/qti/cpucp

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/cpucp			\
	-Iinclude/drivers/qti/cpucp/$(CHIPSET)		\
	-Idrivers/arm/css/scmi

# Kodiak runs the version 3 host interface, with its own start sequence.
ifeq ($(CHIPSET),kodiak)
CPUCP_START_SOURCES := $(CPUCP_DRV_PATH)/kodiak/cpucp_start.c
else
CPUCP_START_SOURCES := $(CPUCP_DRV_PATH)/$(CHIPSET)/cpucp_config.c	\
		       $(CPUCP_DRV_PATH)/hoya/cpucp_start.c
endif

BL31_SOURCES += \
	$(CPUCP_DRV_PATH)/cpucp.c			\
	$(CPUCP_DRV_PATH)/$(CHIPSET)/cpu_config.c	\
	$(CPUCP_START_SOURCES)				\
	drivers/arm/css/scmi/scmi_common.c
