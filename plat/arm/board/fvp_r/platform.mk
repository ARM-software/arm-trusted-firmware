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
override NEED_BL33	:= no

# Default cluster count for FVP_R
FVP_R_CLUSTER_COUNT	:= 2

# Default number of CPUs per cluster on FVP_R
FVP_R_MAX_CPUS_PER_CLUSTER	:= 4

# Default number of threads per CPU on FVP_R
FVP_R_MAX_PE_PER_CPU	:= 1

# Need to revisit this for FVP_R
FVP_R_DT_PREFIX		:= fvp-base-gicv3-psci

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

PLAT_INCLUDES		:=	-Iplat/arm/board/fvp_r/include

PLAT_BL_COMMON_SOURCES	:=	plat/arm/board/fvp_r/fvp_r_common.c

FVP_R_CPU_LIBS		:=	lib/cpus/${ARCH}/aem_generic.S

# select a different set of CPU files, depending on whether we compile for
# hardware assisted coherency cores or not
ifeq (${HW_ASSISTED_COHERENCY}, 0)
# Cores used without DSU
#	FVP_R_CPU_LIBS	+=	lib/cpus/aarch64/fvp_r.S
else
# Cores used with DSU only
#	FVP_R_CPU_LIBS	+=	lib/cpus/aarch64/fvp_r.S
endif

BL1_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/io/io_semihosting.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp_r/fvp_r_helpers.S		\
				plat/arm/board/fvp_r/fvp_r_bl1_setup.c		\
				plat/arm/board/fvp_r/fvp_r_err.c		\
				plat/arm/board/fvp_r/fvp_r_io_storage.c		\
				${FVP_R_CPU_LIBS}				\
				${FVP_R_INTERCONNECT_SOURCES}

ifeq (${USE_SP804_TIMER},1)
BL1_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
BL1_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif

# Enable Activity Monitor Unit extensions by default
ENABLE_AMU			:=	1

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/board/fvp_r/fvp_r_stack_protector.c
endif

ifeq (${ARCH},aarch32)
    NEED_BL32 := yes
endif

ifneq (${BL2_AT_EL3}, 0)
    override BL1_SOURCES =
endif

# Add the FDT_SOURCES and options for Dynamic Config (only for Unix env)
ifdef UNIX_MK
FVP_R_HW_CONFIG_DTS	:=	fdts/${FVP_R_DT_PREFIX}.dts
FDT_SOURCES		+=	$(addprefix plat/arm/board/fvp_r/fdts/,	\
					${PLAT}_fw_config.dts		\
					${PLAT}_nt_fw_config.dts	\
				)

FVP_R_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
FVP_R_NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_R_FW_CONFIG},--fw-config,${FVP_R_FW_CONFIG}))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_R_NT_FW_CONFIG},--nt-fw-config,${FVP_R_NT_FW_CONFIG}))

FDT_SOURCES		+=	${FVP_R_HW_CONFIG_DTS}
$(eval FVP_R_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(FVP_R_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_R_HW_CONFIG},--hw-config,${FVP_R_HW_CONFIG}))
endif

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk
