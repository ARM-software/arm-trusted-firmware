#
# Copyright (c) 2019, ARM Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# SP_MIN source files specific to A5DS platform
BL32_SOURCES	+=	drivers/cfi/v2m/v2m_flash.c			\
			lib/utils/mem_region.c				\
			lib/aarch32/arm32_aeabi_divmod.c		\
			lib/aarch32/arm32_aeabi_divmod_a32.S		\
			plat/arm/board/a5ds/aarch32/a5ds_helpers.S	\
			plat/arm/board/a5ds/a5ds_pm.c			\
			plat/arm/board/a5ds/a5ds_topology.c		\
			plat/arm/board/a5ds/sp_min/a5ds_sp_min_setup.c	\
			plat/arm/common/sp_min/arm_sp_min_setup.c	\
			plat/common/aarch32/platform_mp_stack.S		\
			plat/common/plat_psci_common.c			\
			${A5DS_CPU_LIBS}				\
			${A5DS_GIC_SOURCES}				\
			${A5DS_SECURITY_SOURCES}
