#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Use the GICv3 driver on the FVP by default
FVP_USE_GIC_DRIVER	:= FVP_GICV3

# Use the SP804 timer instead of the generic one
FVP_USE_SP804_TIMER	:= 0

# Default cluster count for FVP
FVP_CLUSTER_COUNT	:= 2

# Default number of threads per CPU on FVP
FVP_MAX_PE_PER_CPU	:= 1

$(eval $(call assert_boolean,FVP_USE_SP804_TIMER))
$(eval $(call add_define,FVP_USE_SP804_TIMER))

# The FVP platform depends on this macro to build with correct GIC driver.
$(eval $(call add_define,FVP_USE_GIC_DRIVER))

# Pass FVP_CLUSTER_COUNT to the build system.
$(eval $(call add_define,FVP_CLUSTER_COUNT))

# Pass FVP_MAX_PE_PER_CPU to the build system.
$(eval $(call add_define,FVP_MAX_PE_PER_CPU))

# Sanity check the cluster count and if FVP_CLUSTER_COUNT <= 2,
# choose the CCI driver , else the CCN driver
ifeq ($(FVP_CLUSTER_COUNT), 0)
$(error "Incorrect cluster count specified for FVP port")
else ifeq ($(FVP_CLUSTER_COUNT),$(filter $(FVP_CLUSTER_COUNT),1 2))
FVP_INTERCONNECT_DRIVER := FVP_CCI
else
FVP_INTERCONNECT_DRIVER := FVP_CCN
endif

$(eval $(call add_define,FVP_INTERCONNECT_DRIVER))

FVP_GICV3_SOURCES	:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				plat/common/plat_gicv3.c		\
				plat/arm/common/arm_gicv3.c

# Choose the GIC sources depending upon the how the FVP will be invoked
ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV3)
FVP_GIC_SOURCES		:=	${FVP_GICV3_SOURCES}			\
				drivers/arm/gic/v3/gic500.c
else ifeq (${FVP_USE_GIC_DRIVER},FVP_GIC600)
FVP_GIC_SOURCES		:=	${FVP_GICV3_SOURCES}			\
				drivers/arm/gic/v3/gic600.c
else ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV2)
FVP_GIC_SOURCES		:=	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				plat/arm/common/arm_gicv2.c
else ifeq (${FVP_USE_GIC_DRIVER}, FVP_GICV3_LEGACY)
  ifeq (${ARCH}, aarch32)
    $(error "GICV3 Legacy driver not supported for AArch32 build")
  endif
FVP_GIC_SOURCES		:=	drivers/arm/gic/arm_gic.c		\
				drivers/arm/gic/gic_v2.c		\
				drivers/arm/gic/gic_v3.c		\
				plat/common/plat_gic.c			\
				plat/arm/common/arm_gicv3_legacy.c
else
$(error "Incorrect GIC driver chosen on FVP port")
endif

ifeq (${FVP_INTERCONNECT_DRIVER}, FVP_CCI)
FVP_INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c
else ifeq (${FVP_INTERCONNECT_DRIVER}, FVP_CCN)
FVP_INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c		\
					plat/arm/common/arm_ccn.c
else
$(error "Incorrect CCN driver chosen on FVP port")
endif

FVP_SECURITY_SOURCES	:=	drivers/arm/tzc/tzc400.c		\
				plat/arm/board/fvp/fvp_security.c	\
				plat/arm/common/arm_tzc400.c


PLAT_INCLUDES		:=	-Iplat/arm/board/fvp/include


PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/fvp/fvp_common.c

FVP_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

ifeq (${ARCH}, aarch64)
FVP_CPU_LIBS		+=	lib/cpus/aarch64/cortex_a35.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a55.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				lib/cpus/aarch64/cortex_a73.S			\
				lib/cpus/aarch64/cortex_a75.S
else
FVP_CPU_LIBS		+=	lib/cpus/aarch32/cortex_a32.S
endif

BL1_SOURCES		+=	drivers/io/io_semihosting.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/${ARCH}/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl1_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_trusted_boot.c		\
				${FVP_CPU_LIBS}					\
				${FVP_INTERCONNECT_SOURCES}


BL2_SOURCES		+=	drivers/io/io_semihosting.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/fvp_bl2_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_trusted_boot.c		\
				${FVP_SECURITY_SOURCES}

ifeq (${BL2_AT_EL3},1)
BL2_SOURCES		+=	plat/arm/board/fvp/${ARCH}/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl2_el3_setup.c		\
				${FVP_CPU_LIBS}					\
				${FVP_INTERCONNECT_SOURCES}
endif

ifeq (${FVP_USE_SP804_TIMER},1)
BL2_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
endif

BL2U_SOURCES		+=	plat/arm/board/fvp/fvp_bl2u_setup.c		\
				${FVP_SECURITY_SOURCES}

BL31_SOURCES		+=	drivers/arm/smmu/smmu_v3.c			\
				plat/arm/board/fvp/fvp_bl31_setup.c		\
				plat/arm/board/fvp/fvp_pm.c			\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/board/fvp/drivers/pwrc/fvp_pwrc.c	\
				${FVP_CPU_LIBS}					\
				${FVP_GIC_SOURCES}				\
				${FVP_INTERCONNECT_SOURCES}			\
				${FVP_SECURITY_SOURCES}

# Disable the PSCI platform compatibility layer
ENABLE_PLAT_COMPAT	:= 	0

# Enable Activity Monitor Unit extensions by default
ENABLE_AMU			:=	1

ifeq (${ENABLE_AMU},1)
BL31_SOURCES		+= lib/cpus/aarch64/cortex_a75_pubsub.c
endif

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/board/fvp/fvp_stack_protector.c
endif

ifeq (${ARCH},aarch32)
    NEED_BL32 := yes
endif

# Add support for platform supplied linker script for BL31 build
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifneq (${BL2_AT_EL3}, 0)
    override BL1_SOURCES =
endif

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk
