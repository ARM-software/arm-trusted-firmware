#
# Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include common/fdt_wrappers.mk

# Include GICv2 driver files
include drivers/arm/gic/v2/gicv2.mk

JUNO_GIC_SOURCES	:=	${GICV2_SOURCES}			\
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

PLAT_INCLUDES		:=	-Iplat/arm/board/juno/include

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
  ifeq (${ETHOSN_NPU_TZMP1},1)
    $(error JUNO_TZMP1 cannot be used together with ETHOSN_NPU_TZMP1)
  else
    $(eval $(call add_define,JUNO_TZMP1))
  endif
endif

TRNG_SUPPORT		:=	1

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
else
  ifeq (${ARCH}, aarch32)
    $(error JUNO_AARCH32_EL3_RUNTIME has to be enabled to build BL32 for AArch32)
  endif
endif

ifeq (${ARCH},aarch64)
BL1_SOURCES		+=	lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a57.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/arm/board/juno/juno_err.c		\
				plat/arm/board/juno/juno_bl1_setup.c	\
				drivers/arm/sp805/sp805.c		\
				${JUNO_INTERCONNECT_SOURCES}		\
				${JUNO_SECURITY_SOURCES}

BL2_SOURCES		+=	drivers/arm/sp805/sp805.c		\
				lib/utils/mem_region.c			\
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
				lib/fconf/fconf.c			\
				lib/fconf/fconf_dyn_cfg_getter.c	\
				plat/arm/board/juno/juno_bl31_setup.c	\
				plat/arm/board/juno/juno_pm.c		\
				plat/arm/board/juno/juno_topology.c	\
				plat/arm/common/arm_nor_psci_mem_protect.c \
				${JUNO_GIC_SOURCES}			\
				${JUNO_INTERCONNECT_SOURCES}		\
				${JUNO_SECURITY_SOURCES}

BL31_SOURCES		+=	${FDT_WRAPPERS_SOURCES}

ifeq (${CSS_USE_SCMI_SDS_DRIVER},1)
BL1_SOURCES		+=	drivers/arm/css/sds/sds.c
endif

ifeq (${TRUSTED_BOARD_BOOT}, 1)
   # Enable Juno specific TBBR images
   $(eval $(call add_define,PLAT_TBBR_IMG_DEF))
   DTC_CPPFLAGS += ${PLAT_INCLUDES}

   BL1_SOURCES		+=	plat/arm/board/juno/juno_trusted_boot.c
   BL2_SOURCES		+=	plat/arm/board/juno/juno_trusted_boot.c

   ifeq (${COT_DESC_IN_DTB},0)
      BL2_SOURCES	+=	plat/arm/board/juno/juno_tbbr_cot_bl2.c
   endif
endif

endif

ifneq (${RESET_TO_BL31},0)
  $(error "Using BL31 as the reset vector is not supported on ${PLAT} platform. \
  Please set RESET_TO_BL31 to 0.")
endif

ifeq ($(USE_ROMLIB),1)
all: $(BUILD_PLAT)/bl1_romlib.bin
endif

$(BUILD_PLAT)/bl1_romlib.bin: $(BUILD_PLAT)/bl1.bin $(BUILD_PLAT)/romlib/romlib.bin
	$(s)echo "Building combined BL1 and ROMLIB binary for Juno $@"
	./lib/romlib/gen_combined_bl1_romlib.sh -o bl1_romlib.bin $(BUILD_PLAT)

# Errata workarounds for Cortex-A53:
ERRATA_A53_819472		:=	1
ERRATA_A53_824069		:=	1
ERRATA_A53_826319		:=	1
ERRATA_A53_827319		:=	1
ERRATA_A53_835769		:=	1
ERRATA_A53_836870		:=	1
ERRATA_A53_843419		:=	1
ERRATA_A53_855873		:=	1

# Errata workarounds for Cortex-A57:
ERRATA_A57_806969		:=	0
ERRATA_A57_813419		:=	1
ERRATA_A57_813420		:=	1
ERRATA_A57_814670		:=	1
ERRATA_A57_817169		:=	1
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

# Enable the dynamic translation tables library.
ifeq (${ARCH},aarch32)
    ifeq (${RESET_TO_SP_MIN},1)
        BL32_CPPFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC
    endif
else
    ifeq (${RESET_TO_BL31},1)
        BL31_CPPFLAGS	+=	-DPLAT_XLAT_TABLES_DYNAMIC
    endif
endif

ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
    ifeq (${JUNO_AARCH32_EL3_RUNTIME}, 1)
        BL32_CPPFLAGS	+=	-DPLAT_RO_XLAT_TABLES
    else
        BL31_CPPFLAGS	+=	-DPLAT_RO_XLAT_TABLES
    endif
endif

BL1_CPPFLAGS += -march=armv8-a+crc
BL2_CPPFLAGS += -march=armv8-a+crc
BL2U_CPPFLAGS += -march=armv8-a+crc
BL31_CPPFLAGS += -march=armv8-a+crc
BL32_CPPFLAGS += -march=armv8-a+crc

# Add the FDT_SOURCES and options for Dynamic Config
FDT_SOURCES		+=	plat/arm/board/juno/fdts/${PLAT}_fw_config.dts	\
				plat/arm/board/juno/fdts/${PLAT}_tb_fw_config.dts \
				fdts/${PLAT}.dts

FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_fw_config.dtb
TB_FW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}_tb_fw_config.dtb
HW_CONFIG		:=	${BUILD_PLAT}/fdts/${PLAT}.dtb

# Add the FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${FW_CONFIG},--fw-config,${FW_CONFIG}))
# Add the TB_FW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${TB_FW_CONFIG},--tb-fw-config,${TB_FW_CONFIG}))
# Add the HW_CONFIG to FIP and specify the same to certtool
$(eval $(call TOOL_ADD_PAYLOAD,${HW_CONFIG},--hw-config,${HW_CONFIG}))

include drivers/arm/ethosn/ethosn_npu.mk
include plat/arm/board/common/board_common.mk
include plat/arm/common/arm_common.mk
include plat/arm/soc/common/soc_css.mk
include plat/arm/css/common/css_common.mk
