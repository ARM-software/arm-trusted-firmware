#
# Copyright (c) 2013-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Use the GICv3 driver on the FVP by default
FVP_USE_GIC_DRIVER	:= FVP_GICV3

# Use the SP804 timer instead of the generic one
FVP_USE_SP804_TIMER	:= 0

# Default cluster count for FVP
FVP_CLUSTER_COUNT	:= 2

# Default number of CPUs per cluster on FVP
FVP_MAX_CPUS_PER_CLUSTER	:= 4

# Default number of threads per CPU on FVP
FVP_MAX_PE_PER_CPU	:= 1

FVP_DT_PREFIX		:= fvp-base-gicv3-psci

$(eval $(call assert_boolean,FVP_USE_SP804_TIMER))
$(eval $(call add_define,FVP_USE_SP804_TIMER))

# The FVP platform depends on this macro to build with correct GIC driver.
$(eval $(call add_define,FVP_USE_GIC_DRIVER))

# Pass FVP_CLUSTER_COUNT to the build system.
$(eval $(call add_define,FVP_CLUSTER_COUNT))

# Pass FVP_MAX_CPUS_PER_CLUSTER to the build system.
$(eval $(call add_define,FVP_MAX_CPUS_PER_CLUSTER))

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

FVP_DT_PREFIX		:=	fvp-base-gicv2-psci
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

# select a different set of CPU files, depending on whether we compile for
# hardware assisted coherency cores or not
ifeq (${HW_ASSISTED_COHERENCY}, 0)
# Cores used without DSU
	FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a35.S			\
				lib/cpus/aarch64/cortex_a53.S			\
				lib/cpus/aarch64/cortex_a57.S			\
				lib/cpus/aarch64/cortex_a72.S			\
				lib/cpus/aarch64/cortex_a73.S
else
# Cores used with DSU only
	ifeq (${CTX_INCLUDE_AARCH32_REGS}, 0)
	# AArch64-only cores
		FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a76.S		\
					lib/cpus/aarch64/cortex_a76ae.S		\
					lib/cpus/aarch64/cortex_a77.S		\
					lib/cpus/aarch64/neoverse_n1.S		\
					lib/cpus/aarch64/neoverse_e1.S		\
					lib/cpus/aarch64/neoverse_zeus.S	\
					lib/cpus/aarch64/cortex_hercules.S	\
					lib/cpus/aarch64/cortex_hercules_ae.S	\
					lib/cpus/aarch64/cortex_a65.S		\
					lib/cpus/aarch64/cortex_a65ae.S
	endif
	# AArch64/AArch32 cores
	FVP_CPU_LIBS	+=	lib/cpus/aarch64/cortex_a55.S		\
				lib/cpus/aarch64/cortex_a75.S
endif

else
FVP_CPU_LIBS		+=	lib/cpus/aarch32/cortex_a32.S
endif

BL1_SOURCES		+=	drivers/arm/smmu/smmu_v3.c			\
				drivers/arm/sp805/sp805.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/io/io_semihosting.c			\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/${ARCH}/fvp_helpers.S	\
				plat/arm/board/fvp/fvp_bl1_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_trusted_boot.c		\
				${FVP_CPU_LIBS}					\
				${FVP_INTERCONNECT_SOURCES}

ifeq (${FVP_USE_SP804_TIMER},1)
BL1_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
BL1_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif


BL2_SOURCES		+=	drivers/arm/sp805/sp805.c			\
				drivers/io/io_semihosting.c			\
				lib/utils/mem_region.c				\
				lib/semihosting/semihosting.c			\
				lib/semihosting/${ARCH}/semihosting_call.S	\
				plat/arm/board/fvp/fvp_bl2_setup.c		\
				plat/arm/board/fvp/fvp_err.c			\
				plat/arm/board/fvp/fvp_io_storage.c		\
				plat/arm/board/fvp/fvp_trusted_boot.c		\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
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

ifeq (${FVP_USE_SP804_TIMER},1)
BL2U_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
endif

BL31_SOURCES		+=	drivers/arm/fvp/fvp_pwrc.c			\
				drivers/arm/smmu/smmu_v3.c			\
				drivers/delay_timer/delay_timer.c		\
				drivers/cfi/v2m/v2m_flash.c			\
				lib/utils/mem_region.c				\
				plat/arm/board/fvp/fvp_bl31_setup.c		\
				plat/arm/board/fvp/fvp_pm.c			\
				plat/arm/board/fvp/fvp_topology.c		\
				plat/arm/board/fvp/aarch64/fvp_helpers.S	\
				plat/arm/common/arm_nor_psci_mem_protect.c	\
				${FVP_CPU_LIBS}					\
				${FVP_GIC_SOURCES}				\
				${FVP_INTERCONNECT_SOURCES}			\
				${FVP_SECURITY_SOURCES}

ifeq (${FVP_USE_SP804_TIMER},1)
BL31_SOURCES		+=	drivers/arm/sp804/sp804_delay_timer.c
else
BL31_SOURCES		+=	drivers/delay_timer/generic_delay_timer.c
endif

# Add the FDT_SOURCES and options for Dynamic Config (only for Unix env)
ifdef UNIX_MK
FVP_HW_CONFIG_DTS	:=	fdts/${FVP_DT_PREFIX}.dts
FDT_SOURCES		+=	$(addprefix plat/arm/board/fvp/fdts/,	\
					${PLAT}_tb_fw_config.dts	\
					${PLAT}_soc_fw_config.dts	\
					${PLAT}_nt_fw_config.dts	\
				)

FVP_TB_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
FVP_SOC_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_soc_fw_config.dtb
FVP_NT_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_nt_fw_config.dtb

ifeq (${SPD},tspd)
FDT_SOURCES		+=	plat/arm/board/fvp/fdts/${PLAT}_tsp_fw_config.dts
FVP_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/${PLAT}_tsp_fw_config.dtb

# Add the TOS_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TOS_FW_CONFIG},--tos-fw-config))
endif

# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_TB_FW_CONFIG},--tb-fw-config))
# Add the SOC_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_SOC_FW_CONFIG},--soc-fw-config))
# Add the NT_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_NT_FW_CONFIG},--nt-fw-config))

FDT_SOURCES		+=	${FVP_HW_CONFIG_DTS}
$(eval FVP_HW_CONFIG	:=	${BUILD_PLAT}/$(patsubst %.dts,%.dtb,$(FVP_HW_CONFIG_DTS)))

# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FVP_HW_CONFIG},--hw-config))
endif

# Enable Activity Monitor Unit extensions by default
ENABLE_AMU			:=	1

# Enable dynamic mitigation support by default
DYNAMIC_WORKAROUND_CVE_2018_3639	:=	1

ifneq (${RESET_TO_BL31},1)
# Enable reclaiming of BL31 initialisation code for secondary cores stacks for
# FVP. We cannot enable PIE for this case because the overlayed init section
# creates some dynamic relocations which cannot be handled by the fixup
# logic currently.
RECLAIM_INIT_CODE	:=	1
else
# Enable PIE support when RESET_TO_BL31=1
ENABLE_PIE		:=	1
endif

ifeq (${ENABLE_AMU},1)
BL31_SOURCES		+=	lib/cpus/aarch64/cpuamu.c		\
				lib/cpus/aarch64/cpuamu_helpers.S

ifeq (${HW_ASSISTED_COHERENCY}, 1)
BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a75_pubsub.c	\
				lib/cpus/aarch64/neoverse_n1_pubsub.c
endif
endif

ifeq (${RAS_EXTENSION},1)
BL31_SOURCES		+=	plat/arm/board/fvp/aarch64/fvp_ras.c
endif

ifneq (${ENABLE_STACK_PROTECTOR},0)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/board/fvp/fvp_stack_protector.c
endif

ifeq (${ARCH},aarch32)
    NEED_BL32 := yes
endif

# Enable the dynamic translation tables library.
ifeq (${ARCH},aarch32)
    ifeq (${RESET_TO_SP_MIN},1)
        BL32_CFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC=1
    endif
else # if AArch64
    ifeq (${RESET_TO_BL31},1)
        BL31_CFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC=1
    endif
    ifeq (${ENABLE_SPM},1)
        ifeq (${SPM_MM},0)
            BL31_CFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC=1
        endif
    endif
    ifeq (${SPD},trusty)
        BL31_CFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC=1
    endif
endif

# Add support for platform supplied linker script for BL31 build
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifneq (${BL2_AT_EL3}, 0)
    override BL1_SOURCES =
endif

include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk

# FVP being a development platform, enable capability to disable Authentication
# dynamically if TRUSTED_BOARD_BOOT is set.
ifeq (${TRUSTED_BOARD_BOOT}, 1)
        DYN_DISABLE_AUTH	:=	1
endif
