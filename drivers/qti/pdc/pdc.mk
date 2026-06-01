#
# Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# PDC (Power Domain Controller) driver
#

$(eval $(call add_define,QTI_PDC_ENABLED))

PDC_DRV_PATH := drivers/qti/pdc
CMD_DB_DRV_PATH := drivers/qti/cmd_db

PLAT_INCLUDES += \
	-I$(PDC_DRV_PATH)/$(CHIPSET)

BL31_SOURCES += \
	$(PDC_DRV_PATH)/pdc.c					\
	$(PDC_DRV_PATH)/pdc_seq.c				\
	$(PDC_DRV_PATH)/pdc_tcs.c				\
	$(CMD_DB_DRV_PATH)/cmd_db.c				\
	$(PDC_DRV_PATH)/$(CHIPSET)/pdc_seq_cfg.c		\
	$(PDC_DRV_PATH)/$(CHIPSET)/interrupt_table.c		\
	$(PDC_DRV_PATH)/$(CHIPSET)/gpio_table.c			\
	$(PDC_DRV_PATH)/$(CHIPSET)/mux_table.c			\
	$(PDC_DRV_PATH)/$(CHIPSET)/tcs_resource.c
