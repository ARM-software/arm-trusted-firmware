#
# Copyright (c) 2019-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Firmware Configuration Framework sources
include common/fdt_wrappers.mk
include lib/fconf/fconf.mk

BL1_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}
BL2_SOURCES		+=	${FCONF_SOURCES} ${FCONF_DYN_SOURCES}

# Add `libfdt` and Arm common helpers required for Dynamic Config
include lib/libfdt/libfdt.mk

DYN_CFG_SOURCES		+=	plat/arm/common/arm_dyn_cfg.c		\
				plat/arm/common/arm_dyn_cfg_helpers.c

DYN_CFG_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

A5DS_GIC_SOURCES	:=	${GICV2_SOURCES}			\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

A5DS_SECURITY_SOURCES	:=	plat/arm/board/a5ds/a5ds_security.c

PLAT_INCLUDES		:=	-Iplat/arm/board/a5ds/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/arm/board/a5ds/a5ds_common.c		\
				plat/arm/common/${ARCH}/arm_helpers.S		\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_console.c			\
				plat/arm/board/common/${ARCH}/board_arm_helpers.S

A5DS_CPU_LIBS		:=	lib/cpus/aarch32/cortex_a5.S

BL1_SOURCES		+=	drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				drivers/cfi/v2m/v2m_flash.c			\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_err.c			\
				plat/arm/board/a5ds/a5ds_err.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/arm/common/fconf/arm_fconf_io.c		\
				plat/arm/board/a5ds/${ARCH}/a5ds_helpers.S	\
				plat/arm/board/a5ds/a5ds_bl1_setup.c		\
				lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				${A5DS_CPU_LIBS}				\
				${DYN_CFG_SOURCES}

BL2_SOURCES		+=	lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/cfi/v2m/v2m_flash.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/board/a5ds/a5ds_bl2_setup.c		\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_err.c			\
				plat/arm/board/a5ds/a5ds_err.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/arm/common/fconf/arm_fconf_io.c		\
				plat/arm/common/${ARCH}/arm_bl2_mem_params_desc.c	\
				plat/arm/common/arm_image_load.c		\
				common/desc_image_load.c			\
				${DYN_CFG_SOURCES}				\
				${A5DS_SECURITY_SOURCES}

# Add the FDT_SOURCES and options for Dynamic Config (only for Unix env)
ifdef UNIX_MK

FW_CONFIG	:=      ${BUILD_PLAT}/fdts/a5ds_fw_config.dtb
TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/a5ds_tb_fw_config.dtb

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))

$(eval FVP_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb, \
	fdts/$(notdir ${FVP_HW_CONFIG_DTS})))
# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_HW_CONFIG},--hw-config,${FVP_HW_CONFIG}))

FDT_SOURCES		+=	plat/arm/board/a5ds/fdts/a5ds_fw_config.dts \
				plat/arm/board/a5ds/fdts/a5ds_tb_fw_config.dts \
					${FVP_HW_CONFIG_DTS}
endif

NEED_BL32 := yes

MULTI_CONSOLE_API		:=	1

PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/aarch32/nonlpae_tables.c

# Use translation tables library v1 when using Cortex-A5
ARM_XLAT_TABLES_LIB_V1		:=	1
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))

$(eval $(call assert_boolean,ARM_DISABLE_TRUSTED_WDOG))
$(eval $(call add_define,ARM_DISABLE_TRUSTED_WDOG))
