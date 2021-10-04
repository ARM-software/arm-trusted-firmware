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

# Default cluster count for FVP_R
FVP_R_CLUSTER_COUNT	:= 2

# Default number of CPUs per cluster on FVP_R
FVP_R_MAX_CPUS_PER_CLUSTER	:= 4

# Default number of threads per CPU on FVP_R
FVP_R_MAX_PE_PER_CPU	:= 1

# Use MPU-based memory management:
XLAT_MPU_LIB_V1		:=	1

# Pass FVP_R_CLUSTER_COUNT to the build system.
$(eval $(call add_define,FVP_R_CLUSTER_COUNT))

# Pass FVP_R_MAX_CPUS_PER_CLUSTER to the build system.
$(eval $(call add_define,FVP_R_MAX_CPUS_PER_CLUSTER))

# Pass FVP_R_MAX_PE_PER_CPU to the build system.
$(eval $(call add_define,FVP_R_MAX_PE_PER_CPU))

# Sanity check the cluster count and if FVP_R_CLUSTER_COUNT <= 2,
# choose the CCI driver , else the CCN driver
ifeq ($(FVP_R_CLUSTER_COUNT), 0)
$(error "Incorrect cluster count specified for FVP_R port")
else ifeq ($(FVP_R_CLUSTER_COUNT),$(filter $(FVP_R_CLUSTER_COUNT),1 2))
FVP_R_INTERCONNECT_DRIVER := FVP_R_CCI
else
FVP_R_INTERCONNECT_DRIVER := FVP_R_CCN
endif

$(eval $(call add_define,FVP_R_INTERCONNECT_DRIVER))

ifeq (${FVP_R_INTERCONNECT_DRIVER}, FVP_R_CCI)
FVP_R_INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c
else ifeq (${FVP_R_INTERCONNECT_DRIVER}, FVP_R_CCN)
FVP_R_INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c		\
					plat/arm/common/arm_ccn.c
else
$(error "Incorrect CCN driver chosen on FVP_R port")
endif

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk

PLAT_INCLUDES		:=	-Iplat/arm/board/fvp_r/include

FVP_R_BL_COMMON_SOURCES	:=	plat/arm/board/fvp_r/fvp_r_common.c		\
				plat/arm/board/fvp_r/fvp_r_context_mgmt.c	\
				plat/arm/board/fvp_r/fvp_r_context.S		\
				plat/arm/board/fvp_r/fvp_r_debug.S		\
				plat/arm/board/fvp_r/fvp_r_err.c		\
				plat/arm/board/fvp_r/fvp_r_helpers.S		\
				plat/arm/board/fvp_r/fvp_r_misc_helpers.S	\
				plat/arm/board/fvp_r/fvp_r_pauth_helpers.S

FVP_R_BL1_SOURCES	:=	plat/arm/board/fvp_r/fvp_r_bl1_arch_setup.c	\
				plat/arm/board/fvp_r/fvp_r_bl1_setup.c		\
				plat/arm/board/fvp_r/fvp_r_io_storage.c		\
				plat/arm/board/fvp_r/fvp_r_bl1_context_mgmt.c	\
				plat/arm/board/fvp_r/fvp_r_bl1_entrypoint.S	\
				plat/arm/board/fvp_r/fvp_r_bl1_exceptions.S	\
				plat/arm/board/fvp_r/fvp_r_bl1_main.c

FVP_R_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

FVP_R_DYNC_CFG_SOURCES	:=	common/fdt_wrappers.c				\
				common/uuid.c					\
				plat/arm/common/arm_dyn_cfg.c			\
				plat/arm/common/arm_dyn_cfg_helpers.c

ifeq (${TRUSTED_BOARD_BOOT},1)
FVP_R_AUTH_SOURCES	:=	drivers/auth/auth_mod.c				\
				drivers/auth/crypto_mod.c			\
				drivers/auth/img_parser_mod.c			\
				lib/fconf/fconf_tbbr_getter.c			\
				bl1/tbbr/tbbr_img_desc.c			\
				plat/arm/common/arm_bl1_fwu.c			\
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
				lib/cpus/errata_report.c			\
				lib/cpus/aarch64/dsu_helpers.S			\
				lib/el3_runtime/aarch64/context.S		\
				lib/el3_runtime/aarch64/context_mgmt.c		\
				lib/fconf/fconf.c				\
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
