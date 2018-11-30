#
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP_MIN source files specific to FVP platform
BL32_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c			\
				lib/utils/mem_region.c				\
				plat/arm/board/fvp_ve/aarch32/fvp_ve_helpers.S	\
				drivers/arm/fvp/fvp_pwrc.c			\
				plat/arm/board/fvp_ve/fvp_ve_pm.c		\
				plat/arm/board/fvp_ve/fvp_ve_topology.c		\
				plat/arm/board/fvp_ve/sp_min/fvp_ve_sp_min_setup.c	\
				lib/aarch32/arm32_aeabi_divmod.c		\
				lib/aarch32/arm32_aeabi_divmod_a32.S		\
				plat/arm/common/sp_min/arm_sp_min_setup.c	\
				plat/common/aarch32/platform_mp_stack.S		\
				plat/common/plat_psci_common.c			\
				${FVP_VE_CPU_LIBS}				\
				${FVP_VE_GIC_SOURCES}				\
				${FVP_VE_SECURITY_SOURCES}
