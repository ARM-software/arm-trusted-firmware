#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Only aarch64 ARCH supported for FVP_R
ARCH	:= aarch64

# Override to exclude BL2, BL2U, BL31, and BL33 for FVP_R
override NEED_BL2	:= no
override NEED_BL2U	:= no
override NEED_BL31	:= no
NEED_BL32		:= no

override CTX_INCLUDE_AARCH32_REGS	:=	0

# Use MPU-based memory management:
XLAT_MPU_LIB_V1		:=	1

# FVP R will not have more than 2 clusters so just use CCI interconnect
FVP_R_INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c


include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk

PLAT_INCLUDES		:=	-Iplat/arm/board/fvp_r/include

FVP_R_BL_COMMON_SOURCES	:=	plat/arm/board/fvp_r/fvp_r_common.c		\
				plat/arm/board/fvp_r/fvp_r_context_mgmt.c	\
				plat/arm/board/fvp_r/fvp_r_debug.S		\
				plat/arm/board/fvp_r/fvp_r_err.c		\
				plat/arm/board/fvp_r/fvp_r_helpers.S		\
				plat/arm/board/fvp_r/fvp_r_misc_helpers.S

FVP_R_BL1_SOURCES	:=	plat/arm/board/fvp_r/fvp_r_bl1_arch_setup.c	\
				plat/arm/board/fvp_r/fvp_r_bl1_setup.c		\
				plat/arm/board/fvp_r/fvp_r_io_storage.c		\
				plat/arm/board/fvp_r/fvp_r_bl1_entrypoint.S	\
				plat/arm/board/fvp_r/fvp_r_bl1_exceptions.S	\
				plat/arm/board/fvp_r/fvp_r_bl1_main.c

FVP_R_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

FVP_R_DYNC_CFG_SOURCES	:=	common/fdt_wrappers.c				\
				plat/arm/common/arm_dyn_cfg.c

ifeq (${TRUSTED_BOARD_BOOT},1)
FVP_R_AUTH_SOURCES	:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				lib/fconf/fconf_tbbr_getter.c			\
				plat/common/tbbr/plat_tbbr.c			\
				drivers/auth/tbbr/tbbr_cot_bl1_r64.c		\
				drivers/auth/tbbr/tbbr_cot_common.c		\
				plat/arm/board/common/board_arm_trusted_boot.c	\
				plat/arm/board/common/rotpk/arm_dev_rotpk.S	\
				plat/arm/board/fvp_r/fvp_r_trusted_boot.c

FVP_R_BL1_SOURCES	+=	${MBEDTLS_SOURCES}				\
				${FVP_R_AUTH_SOURCES}
endif

ifeq (${USE_SP804_TIMER},1)
FVP_R_BL_COMMON_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
FVP_R_BL_COMMON_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif

# Enable Activity Monitor Unit extensions by default
ENABLE_AMU			:=	1

ifneq (${ENABLE_STACK_PROTECTOR},0)
FVP_R_BL_COMMON_SOURCES	+=	plat/arm/board/fvp_r/fvp_r_stack_protector.c
endif

override BL1_SOURCES	:=	drivers/arm/sp805/sp805.c			\
				drivers/cfi/v2m/v2m_flash.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				drivers/io/io_semihosting.c			\
				lib/cpus/aarch64/cpu_helpers.S			\
				lib/fconf/fconf_dyn_cfg_getter.c		\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/common/arm_bl1_setup.c			\
				plat/arm/common/arm_err.c			\
				plat/arm/common/arm_io_storage.c		\
				plat/arm/common/fconf/arm_fconf_io.c		\
				plat/common/plat_bl1_common.c			\
				plat/common/aarch64/platform_up_stack.S		\
				${FVP_R_BL1_SOURCES}				\
				${FVP_R_BL_COMMON_SOURCES}			\
				${FVP_R_CPU_LIBS}				\
				${FVP_R_DYNC_CFG_SOURCES}			\
				${FVP_R_INTERCONNECT_SOURCES}
