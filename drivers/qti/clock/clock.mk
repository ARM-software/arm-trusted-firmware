#
# Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Clock driver
#

$(eval $(call add_define,QTI_CLOCK_ENABLED))

CLOCK_DRV_PATH := drivers/qti/clock

PLAT_INCLUDES += \
	-Iinclude/drivers/qti/clock

BL31_SOURCES += \
	$(CLOCK_DRV_PATH)/clock_driver.c			\
	$(CLOCK_DRV_PATH)/clock_hal.c				\
	$(CLOCK_DRV_PATH)/$(CHIPSET)/clock_init.c		\
	$(CLOCK_DRV_PATH)/$(CHIPSET)/clock_cfg.c

# Sequences shared by the hoya SoCs. hoya_clock_gpu.c and hoya_clock_lpass.c
# are built against the chipset's clock_hwio.h; hoya_clock.c takes every
# register from its caller.
ifneq ($(filter lemans monaco,$(CHIPSET)),)
PLAT_INCLUDES += \
	-I$(CLOCK_DRV_PATH)/hoya \
	-I$(CLOCK_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(CLOCK_DRV_PATH)/hoya/hoya_clock.c			\
	$(CLOCK_DRV_PATH)/hoya/hoya_clock_gpu.c			\
	$(CLOCK_DRV_PATH)/hoya/hoya_clock_lpass.c
endif

# Rail voltage voting (RPMh/BCM back-end). Platforms that scale rails during
# init set QTI_CLOCK_RAIL_VOTE in their platform.mk; others use the stubs.
ifeq ($(QTI_CLOCK_RAIL_VOTE),1)
$(eval $(call add_define,QTI_CLOCK_RAIL_VOTE))
BL31_SOURCES += $(CLOCK_DRV_PATH)/clock_rail.c
endif
