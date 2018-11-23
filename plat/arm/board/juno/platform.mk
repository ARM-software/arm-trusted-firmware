#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

JUNO_GIC_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c

JUNO_INTERCONNECT_SOURCES	:=	drivers/arm/cci/cci.c		\
					plat/arm/common/arm_cci.c

JUNO_SECURITY_SOURCES	:=	drivers/arm/tzc/tzc400.c		\
				plat/arm/board/juno/juno_security.c	\
				plat/arm/board/juno/juno_trng.c		\
				plat/arm/common/arm_tzc400.c

ifneq (${ENABLE_STACK_PROTECTOR}, 0)
JUNO_SECURITY_SOURCES	+=	plat/arm/board/juno/juno_stack_protector.c
endif

# Select SCMI/SDS drivers instead of SCPI/BOM driver for communicating with the
# SCP during power management operations and for SCP RAM Firmware transfer.
CSS_USE_SCMI_SDS_DRIVER		:=	1

PLAT_INCLUDES		:=	-Iplat/arm/board/juno/include		\
				-Iplat/arm/css/drivers/scmi		\
				-Iplat/arm/css/drivers/sds

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/juno/${ARCH}/juno_helpers.S \
				plat/arm/board/juno/juno_common.c

# Flag to enable support for AArch32 state on JUNO
JUNO_AARCH32_EL3_RUNTIME	:=	0
$(eval $(call assert_boolean,JUNO_AARCH32_EL3_RUNTIME))
$(eval $(call add_define,JUNO_AARCH32_EL3_RUNTIME))

# Flag to enable support for TZMP1 on JUNO
JUNO_TZMP1		:=	0
$(eval $(call assert_boolean,JUNO_TZMP1))
ifeq (${JUNO_TZMP1}, 1)
$(eval $(call add_define,JUNO_TZMP1))
endif

ifeq (${JUNO_AARCH32_EL3_RUNTIME}, 1)
# Include BL32 in FIP
NEED_BL32		:= yes
# BL31 is not required
override BL31_SOURCES =

# The BL32 needs to be built separately invoking the AARCH32 compiler and
# be specifed via `BL32` build option.
  ifneq (${ARCH}, aarch32)
    override BL32_SOURCES =
  endif
endif

ifeq (${ARCH},aarch64)
BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/arm/board/juno/juno_err.c		\
				plat/arm/board/juno/juno_bl1_setup.c	\
				${JUNO_INTERCONNECT_SOURCES}		\
				${JUNO_SECURITY_SOURCES}

BL2_SOURCES		+=	lib/utils/mem_region.c			\
				plat/arm/board/juno/juno_err.c		\
				plat/arm/board/juno/juno_bl2_setup.c	\
				plat/arm/common/arm_nor_psci_mem_protect.c \
				${JUNO_SECURITY_SOURCES}

BL2U_SOURCES		+=	${JUNO_SECURITY_SOURCES}

BL31_SOURCES		+=	drivers/cfi/v2m/v2m_flash.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/utils/mem_region.c			\
				plat/arm/board/juno/juno_pm.c		\
				plat/arm/board/juno/juno_topology.c	\
				plat/arm/common/arm_nor_psci_mem_protect.c \
				${JUNO_GIC_SOURCES}			\
				${JUNO_INTERCONNECT_SOURCES}		\
				${JUNO_SECURITY_SOURCES}

ifeq (${CSS_USE_SCMI_SDS_DRIVER},1)
BL1_SOURCES		+=	plat/arm/css/drivers/sds/sds.c
endif

endif

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on ${PLATFORM} platform. \
  Please set RESET_TO_BL31 to 0.")
endif

# Errata workarounds for Cortex-A53:
ERRATA_A53_826319		:=	1
ERRATA_A53_835769		:=	1
ERRATA_A53_836870		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

# Errata workarounds for Cortex-A57:
ERRATA_A57_806969		:=	0
ERRATA_A57_813419		:=	1
ERRATA_A57_813420		:=	1
ERRATA_A57_826974		:=	1
ERRATA_A57_826977		:=	1
ERRATA_A57_828024		:=	1
ERRATA_A57_829520		:=	1
ERRATA_A57_833471		:=	1
ERRATA_A57_859972		:=	0

# Errata workarounds for Cortex-A72:
ERRATA_A72_859971		:=	0

# Enable option to skip L1 data cache flush during the Cortex-A57 cluster
# power down sequence
SKIP_A57_L1_FLUSH_PWR_DWN	:=	 1

# Do not enable SVE
ENABLE_SVE_FOR_NS		:=	0

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/css/common/css_common.mk

