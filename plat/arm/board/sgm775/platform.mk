#
# Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(warning Platform ${PLAT} is deprecated. Some of the features might not work as expected)

include plat/arm/css/sgm/sgm-common.mk

SGM775_BASE= plat/arm/board/sgm775

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=      ${SGM775_BASE}/fdts/${PLAT}_fw_config.dts	\
				${SGM775_BASE}/fdts/${PLAT}_tb_fw_config.dts
FW_CONFIG		:=      ${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=      ${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))

PLAT_INCLUDES +=-I${SGM775_BASE}/include/

BL1_SOURCES		+=	${SGM775_BASE}/sgm775_err.c

BL2_SOURCES		+=	lib/utils/mem_region.c                  \
				${SGM775_BASE}/sgm775_err.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c

BL31_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c		\
				lib/utils/mem_region.c			\
				plat/arm/common/arm_nor_psci_mem_protect.c

ifeq (${TRUSTED_BOARD_BOOT}, 1)
BL1_SOURCES		+=	${SGM775_BASE}/sgm775_trusted_boot.c
BL2_SOURCES		+=	${SGM775_BASE}/sgm775_trusted_boot.c
endif
