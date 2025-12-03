#
# Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Cortex A720 is ARMv9.2A and will enable all features we want
ARM_ARCH_MAJOR			:= 9
ARM_ARCH_MINOR			:= 2

PROGRAMMABLE_RESET_ADDRESS	:= 0
COLD_BOOT_SINGLE_CPU		:= 1
ARM_CCI_PRODUCT_ID		:= 500
TRUSTED_BOARD_BOOT		:= 1
RESET_TO_BL31			:= 1
GENERATE_COT			:= 1
ENABLE_SVE_FOR_NS		:= 1
ENABLE_SVE_FOR_SWD		:= 0
MULTI_CONSOLE_API		:= 1
INIT_UNUSED_NS_EL2		:= 1

ENABLE_FEAT_AMU			:= 1
ENABLE_FEAT_AMUv1p1		:= 1
ENABLE_AMU_AUXILIARY_COUNTERS	:= 1

ENABLE_PAUTH			:= 1
CTX_INCLUDE_PAUTH_REGS		:= 1

CRASH_REPORTING			:= 1
HANDLE_EA_EL3_FIRST_NS		:= 1
ENABLE_STACK_PROTECTOR		:= strong

CTX_INCLUDE_AARCH32_REGS	:= 0

# Process SET_SCMI_PARAM flag
# 0:Disable(default), 1:Enable
ifndef SET_SCMI_PARAM
    SET_SCMI_PARAM := 0
    $(eval $(call add_define,SET_SCMI_PARAM))
else
    ifeq (${SET_SCMI_PARAM}, 0)
        $(eval $(call add_define,SET_SCMI_PARAM))
    else ifeq (${SET_SCMI_PARAM},1)
        $(eval $(call add_define,SET_SCMI_PARAM))
    else
        $(error "Error:SET_SCMI_PARAM=${SET_SCMI_PARAM} is not supported.")
    endif
endif

ifeq (${SPD},none)
  SPD_NONE:=1
  $(eval $(call add_define,SPD_NONE))
endif

# LSI setting common define
RCAR_X5H:=10
RCAR_AUTO:=99
$(eval $(call add_define,RCAR_X5H))
$(eval $(call add_define,RCAR_AUTO))
$(eval $(call add_define,PLAT_EXTRA_LD_SCRIPT))

ifndef LSI
  $(error "Error: Unknown LSI. Please use LSI=<LSI name> to specify the LSI")
else
  ifeq (${LSI},AUTO)
    RCAR_LSI:=${RCAR_AUTO}
  else ifeq (${LSI},X5H)
    RCAR_LSI:=${RCAR_X5H}
  else
    $(error "Error: ${LSI} is not supported.")
  endif
  $(eval $(call add_define,RCAR_LSI))
endif

# Disable workarounds unnecessary for Cortex-A720AE
WORKAROUND_CVE_2017_5715	:= 0
WORKAROUND_CVE_2022_23960	:= 0
ERRATA_A720_AE_3699562		:= 1

USE_COHERENT_MEM := 0
HW_ASSISTED_COHERENCY := 1

PLAT_INCLUDES	:=	-Iplat/renesas/rcar_gen5/include		\
			-Iplat/renesas/rcar_gen5			\
			-Iplat/renesas/common/include			\
			-Idrivers/renesas/common/scif			\
			-Idrivers/renesas/common/timer			\
			-Idrivers/renesas/rcar_gen5/pwrc		\
			-Idrivers/arm/css/scmi				\
			-Iinclude/drivers

ifneq (${ENABLE_STACK_PROTECTOR},0)
BL_COMMON_SOURCES	+=	plat/renesas/common/rcar_stack_protector.c
endif

# R-Car Gen5 platform uses Arm GIC-Fainlight-AE,
# which is successor of GIC-700 based on GICv4.1
GIC_ENABLE_V4_EXTN	:= 1
GIC_EXT_INTID		:= 1

# GIC-600 configuration
GICV3_SUPPORT_GIC600	:= 1
# Include GICv3 driver files
USE_GIC_DRIVER		:= 3

BL31_SOURCES	+=	${RCAR_GIC_SOURCES}				\
			lib/cpus/aarch64/cortex_a720_ae.S		\
			plat/common/plat_psci_common.c			\
			plat/renesas/rcar_gen5/plat_topology.c		\
			plat/renesas/rcar_gen5/aarch64/plat_helpers.S	\
			plat/renesas/rcar_gen5/aarch64/platform_common.c \
			plat/renesas/rcar_gen5/bl31_plat_setup.c	\
			plat/renesas/rcar_gen5/plat_pm.c		\
			plat/renesas/rcar_gen5/plat_pm_scmi.c		\
			plat/renesas/rcar_gen5/rcar_common.c		\
			drivers/delay_timer/delay_timer.c		\
			drivers/delay_timer/generic_delay_timer.c	\
			drivers/renesas/common/scif/scif-common.c	\
			drivers/renesas/common/scif/scif_helpers.S	\
			drivers/renesas/common/timer/timer.c		\
			drivers/renesas/rcar_gen5/pwrc/pwrc.c		\
			drivers/renesas/rcar_gen5/scif/scif.c		\
			drivers/arm/cci/cci.c				\
			drivers/arm/css/scmi/scmi_common.c		\
			drivers/arm/css/scmi/scmi_pwr_dmn_proto.c	\
			drivers/arm/css/scmi/scmi_sys_pwr_proto.c

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}

# Pointer Authentication sources
ifeq (${ENABLE_PAUTH}, 1)
PLAT_BL_COMMON_SOURCES	+=	plat/arm/common/aarch64/arm_pauth.c
endif

# build the layout images for the bootrom and the necessary srecords
rcar: rcar_srecord
distclean realclean clean: clean_srecord

# srecords
SREC_PATH	= ${BUILD_PLAT}
BL31_ELF_SRC	= ${SREC_PATH}/bl31/bl31.elf

clean_srecord:
	@echo "clean bl31 srecs"
	rm -f ${SREC_PATH}/bl31.srec

.PHONY: rcar_srecord
rcar_srecord: $(BL31_ELF_SRC)
	@echo "generating srec: ${SREC_PATH}/bl31.srec"
	$(Q)$($(ARCH)-oc) -O srec --srec-forceS3 ${BL31_ELF_SRC} ${SREC_PATH}/bl31.srec

