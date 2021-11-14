#
# Copyright (c) 2021, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

MCDI_TINYSYS_TYPE = sspm
MCDI_TINYSYS_MBOX_TYPE = share_sram

CUR_MCDI_FOLDER = ${MTK_PLAT_SOC}/drivers/mcdi

BL31_MT_LPM_PLAT_CFLAGS += -I${CUR_MCDI_FOLDER}/

BL31_MT_LPM_PLAT_SOURCE += \
	${CUR_MCDI_FOLDER}/mt_cpu_pm.c	\
	${CUR_MCDI_FOLDER}/mt_cpu_pm_cpc.c \
	${CUR_MCDI_FOLDER}/mt_mcdi.c	\
	${CUR_MCDI_FOLDER}/mt_lp_irqremain.c


ifeq ($(MCDI_TINYSYS_TYPE), sspm)
BL31_MT_LPM_PLAT_CFLAGS += -DMCDI_TINYSYS_SSPM
BL31_MT_LPM_PLAT_SOURCE += ${CUR_MCDI_FOLDER}/mt_cpu_pm_mbox_sspm.c
else
BL31_MT_LPM_PLAT_CFLAGS += -DMCDI_TINYSYS_MCUPM
BL31_MT_LPM_PLAT_SOURCE += ${CUR_MCDI_FOLDER}/mt_cpu_pm_mbox.c
endif

ifeq ($(MCDI_TINYSYS_MBOX_TYPE), share_sram)
BL31_MT_LPM_PLAT_CFLAGS += -DMCDI_TINYSYS_MBOX_SHARE_SRAM
endif
