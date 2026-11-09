#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

PDC_DRV_PATH := drivers/qti/pdc

# Every supported apps PDC has the hoya register map and the hoya interrupt,
# GPIO and TCS tables; only the sequences differ per chipset.
PDC_COMMON := $(PDC_DRV_PATH)/hoya
ifneq ($(filter lemans monaco,$(CHIPSET)),)
PDC_SEQ_CFG := $(PDC_COMMON)/pdc_seq_cfg.c
else
PDC_SEQ_CFG := $(PDC_DRV_PATH)/$(CHIPSET)/pdc_seq_cfg.c
endif

PLAT_INCLUDES += \
	-I$(PDC_COMMON)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(PDC_SEQ_CFG)						\
	$(PDC_COMMON)/interrupt_table.c				\
	$(PDC_COMMON)/gpio_table.c				\
	$(PDC_COMMON)/tcs_resource.c
