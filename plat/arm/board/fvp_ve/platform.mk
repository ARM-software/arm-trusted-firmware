#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifdef ARM_CORTEX_A5
# Use the SP804 timer instead of the generic one
FVP_VE_USE_SP804_TIMER	:= 1
$(eval $(call add_define,FVP_VE_USE_SP804_TIMER))
BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
endif

FVP_VE_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

FVP_VE_SECURITY_SOURCES	:=	plat/arm/board/fvp_ve/fvp_ve_security.c

PLAT_INCLUDES		:=	-Iplat/arm/board/fvp_ve/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/fvp_ve/fvp_ve_common.c		\
				plat/arm/common/${ARCH}/arm_helpers.S		\
				plat/arm/common/arm_common.c			\
				plat/arm/common/arm_console.c			\
				drivers/arm/pl011/${ARCH}/pl011_console.S	\
				plat/arm/board/common/${ARCH}/board_arm_helpers.S

ifdef ARM_CORTEX_A5
FVP_VE_CPU_LIBS		:=	lib/cpus/aarch32/cortex_a5.S
else
FVP_VE_CPU_LIBS		:=	lib/cpus/aarch32/cortex_a7.S
endif

BL1_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_err.c			\
				plat/arm/board/fvp_ve/fvp_ve_err.c		\
				plat/arm/common/arm_io_storage.c		\
				drivers/cfi/v2m/v2m_flash.c			\
				plat/arm/board/fvp_ve/${ARCH}/fvp_ve_helpers.S	\
				plat/arm/board/fvp_ve/fvp_ve_bl1_setup.c	\
				lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				${FVP_VE_CPU_LIBS}					\
				${DYN_CFG_SOURCES}

BL2_SOURCES		+=	plat/arm/board/fvp_ve/fvp_ve_bl2_setup.c		\
				lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				drivers/delay_timer/delay_timer.c		\
				drivers/delay_timer/generic_delay_timer.c	\
				drivers/cfi/v2m/v2m_flash.c			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/arm/common/arm_bl2_setup.c			\
				plat/arm/common/arm_err.c			\
				plat/arm/board/fvp_ve/fvp_ve_err.c		\
				plat/arm/common/arm_io_storage.c		\
				plat/arm/common/${ARCH}/arm_bl2_mem_params_desc.c	\
				plat/arm/common/arm_image_load.c		\
				common/desc_image_load.c			\
				${DYN_CFG_SOURCES}				\
				${FVP_VE_SECURITY_SOURCES}

# Add the FDT_SOURCES and options for Dynamic Config (only for Unix env)
ifdef UNIX_MK

FDT_SOURCES		+=	plat/arm/board/fvp_ve/fdts/fvp_ve_fw_config.dts

FVP_TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/fvp_ve_fw_config.dtb

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TB_FW_CONFIG},--tb-fw-config))

FDT_SOURCES		+=	${FVP_HW_CONFIG_DTS}
$(eval FVP_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb, \
	fdts/$(notdir ${FVP_HW_CONFIG_DTS})))
# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_HW_CONFIG},--hw-config))
endif

NEED_BL32 := yes

# Modification of arm_common.mk

# Process ARM_DISABLE_TRUSTED_WDOG flag
# By default, Trusted Watchdog is always enabled unless SPIN_ON_BL1_EXIT is set
ARM_DISABLE_TRUSTED_WDOG	:=	0
ifeq (${SPIN_ON_BL1_EXIT}, 1)
ARM_DISABLE_TRUSTED_WDOG	:=	1
endif
$(eval $(call assert_boolean,ARM_DISABLE_TRUSTED_WDOG))
$(eval $(call add_define,ARM_DISABLE_TRUSTED_WDOG))

# Use translation tables library v1 if using Cortex-A5
ifdef ARM_CORTEX_A5
ARM_XLAT_TABLES_LIB_V1		:=	1
else
ARM_XLAT_TABLES_LIB_V1		:=	0
endif
$(eval $(call assert_boolean,ARM_XLAT_TABLES_LIB_V1))
$(eval $(call add_define,ARM_XLAT_TABLES_LIB_V1))

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
	# Only use nonlpae version of xlatv1 otherwise use xlat v2
	PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/${ARCH}/nonlpae_tables.c
else
	include lib/xlat_tables_v2/xlat_tables.mk
	PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}
endif

# Firmware Configuration Framework sources
include lib/fconf/fconf.mk

# Add `libfdt` and Arm common helpers required for Dynamic Config
include lib/libfdt/libfdt.mk

DYN_CFG_SOURCES		+=	plat/arm/common/arm_dyn_cfg.c		\
				plat/arm/common/arm_dyn_cfg_helpers.c	\
				common/fdt_wrappers.c

