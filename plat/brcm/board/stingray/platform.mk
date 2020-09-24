#
# Copyright (c) 2019-2021, Broadcom
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Set the toc_flags to 1 for 100% speed operation
# Set the toc_flags to 2 for 50% speed operation
# Set the toc_flags to 3 for 25% speed operation
# Set the toc_flags bit 3 to indicate ignore the fip in UEFI copy mode
PLAT_TOC_FLAGS := 0x0

# Set the IHOST_PLL_FREQ to,
# 1 for full speed
# 2 for 50% speed
# 3 for 25% speed
# 0 for bypass
$(eval $(call add_define_val,IHOST_PLL_FREQ,1))

# Enable workaround for ERRATA_A72_859971
ERRATA_A72_859971 := 1

# Cache Coherency Interconnect Driver needed
DRIVER_CC_ENABLE := 1
$(eval $(call add_define,DRIVER_CC_ENABLE))

# Enable to erase eMMC
INCLUDE_EMMC_DRIVER_ERASE_CODE := 0

ifeq (${INCLUDE_EMMC_DRIVER_ERASE_CODE},1)
$(eval $(call add_define,INCLUDE_EMMC_DRIVER_ERASE_CODE))
endif

# BL31 is in DRAM
ARM_BL31_IN_DRAM	:=	1

ifneq (${USE_EMULATOR},yes)
STINGRAY_EMULATION_SETUP	:=	0
ifeq (${FASTBOOT_TYPE},)
override FASTBOOT_TYPE		:=	0
endif
USE_PAXB := yes
USE_PAXC := yes
USE_CHIMP := yes
endif

USE_CRMU_SRAM := yes

# Disable FS4 clocks - they can be reenabled when needed by linux
FS4_DISABLE_CLOCK := yes

# Enable error logging by default for Stingray
BCM_ELOG := yes

# Enable FRU support by default for Stingray
ifeq (${USE_FRU},)
USE_FRU := no
endif

# Use single cluster
ifeq (${USE_SINGLE_CLUSTER},yes)
$(info Using Single Cluster)
$(eval $(call add_define,USE_SINGLE_CLUSTER))
endif

# Use DDR
ifeq (${USE_DDR},yes)
$(info Using DDR)
$(eval $(call add_define,USE_DDR))
endif

ifeq (${BOARD_CFG},)
BOARD_CFG := bcm958742t
endif

# Use USB
ifeq (${USE_USB},yes)
$(info Using USB)
$(eval $(call add_define,USE_USB))
endif

# Use PAXB
ifeq (${USE_PAXB},yes)
$(info Using PAXB)
$(eval $(call add_define,USE_PAXB))
endif

# Use FS4
ifeq (${USE_FS4},yes)
$(info Using FS4)
$(eval $(call add_define,USE_FS4))
endif

# Use FS6
ifeq (${USE_FS6},yes)
$(info Using FS6)
$(eval $(call add_define,USE_FS6))
endif

# Disable FS4 clock
ifeq (${FS4_DISABLE_CLOCK},yes)
$(info Using FS4_DISABLE_CLOCK)
$(eval $(call add_define,FS4_DISABLE_CLOCK))
endif

ifneq (${NCSI_IO_DRIVE_STRENGTH_MA},)
$(info Using NCSI_IO_DRIVE_STRENGTH_MA)
$(eval $(call add_define,NCSI_IO_DRIVE_STRENGTH_MA))
endif

# Use NAND
ifeq (${USE_NAND},$(filter yes, ${USE_NAND}))
$(info Using NAND)
$(eval $(call add_define,USE_NAND))
endif

# Enable Broadcom error logging support
ifeq (${BCM_ELOG},yes)
$(info Using BCM_ELOG)
$(eval $(call add_define,BCM_ELOG))
endif

# BL31 build for standalone mode
ifeq (${STANDALONE_BL31},yes)
RESET_TO_BL31 := 1
$(info Using RESET_TO_BL31)
endif

# BL31 force full frequency for all CPUs
ifeq (${BL31_FORCE_CPU_FULL_FREQ},yes)
$(info Using BL31_FORCE_CPU_FULL_FREQ)
$(eval $(call add_define,BL31_FORCE_CPU_FULL_FREQ))
endif

# Enable non-secure accesses to CCN registers
ifeq (${BL31_CCN_NONSECURE},yes)
$(info Using BL31_CCN_NONSECURE)
$(eval $(call add_define,BL31_CCN_NONSECURE))
endif

# Use ChiMP
ifeq (${USE_CHIMP},yes)
$(info Using ChiMP)
$(eval $(call add_define,USE_CHIMP))
endif

# Use PAXC
ifeq (${USE_PAXC},yes)
$(info Using PAXC)
$(eval $(call add_define,USE_PAXC))
ifeq (${CHIMPFW_USE_SIDELOAD},yes)
$(info Using ChiMP FW sideload)
$(eval $(call add_define,CHIMPFW_USE_SIDELOAD))
endif
$(eval $(call add_define,FASTBOOT_TYPE))
$(eval $(call add_define,CHIMP_FB1_ENTRY))
endif

ifeq (${DEFAULT_SWREG_CONFIG}, 1)
$(eval $(call add_define,DEFAULT_SWREG_CONFIG))
endif

ifeq (${CHIMP_ALWAYS_NEEDS_QSPI},yes)
$(eval $(call add_define,CHIMP_ALWAYS_NEEDS_QSPI))
endif

# For testing purposes, use memsys stubs.  Remove once memsys is fully tested.
USE_MEMSYS_STUBS := yes

# Default, use BL1_RW area
ifneq (${BL2_USE_BL1_RW},no)
$(eval $(call add_define,USE_BL1_RW))
endif

# Default soft reset is L3
$(eval $(call add_define,CONFIG_SOFT_RESET_L3))

# Enable Chip OTP driver
DRIVER_OCOTP_ENABLE := 1

ifneq (${WARMBOOT_DDR_S3_SUPPORT},)
DRIVER_SPI_ENABLE := 1
endif

include plat/brcm/board/common/board_common.mk

SOC_DIR			:= 	brcm/board/stingray

PLAT_INCLUDES		+=	-Iplat/${SOC_DIR}/include/ \
				-Iinclude/plat/brcm/common/ \
				-Iplat/brcm/common/

PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch64/cortex_a72.S \
				plat/${SOC_DIR}/aarch64/plat_helpers.S \
				drivers/ti/uart/aarch64/16550_console.S \
				plat/${SOC_DIR}/src/tz_sec.c \
				drivers/arm/tzc/tzc400.c \
				plat/${SOC_DIR}/driver/plat_emmc.c \
				plat/${SOC_DIR}/src/topology.c \
				drivers/brcm/mdio/mdio.c

ifeq (${USE_CHIMP},yes)
PLAT_BL_COMMON_SOURCES	+=	drivers/brcm/chimp.c
endif

ifeq (${USE_USB},yes)
PLAT_BL_COMMON_SOURCES	+=	plat/${SOC_DIR}/driver/usb.c \
				plat/${SOC_DIR}/driver/usb_phy.c
endif

BL2_SOURCES		+=	plat/${SOC_DIR}/driver/ihost_pll_config.c \
				plat/${SOC_DIR}/src/bl2_setup.c \
				plat/${SOC_DIR}/driver/swreg.c

ifeq (${USE_DDR},yes)
PLAT_INCLUDES		+=	-Iplat/${SOC_DIR}/driver/ddr/soc/include
else
PLAT_INCLUDES		+=	-Iplat/${SOC_DIR}/driver/ext_sram_init
BL2_SOURCES		+=	plat/${SOC_DIR}/driver/ext_sram_init/ext_sram_init.c
endif

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

BRCM_GIC_SOURCES	:=	${GICV3_SOURCES}		\
				plat/common/plat_gicv3.c	\
				plat/brcm/common/brcm_gicv3.c

BL31_SOURCES		+=	\
				drivers/arm/ccn/ccn.c \
				plat/brcm/board/common/timer_sync.c \
				plat/brcm/common/brcm_ccn.c \
				plat/common/plat_psci_common.c \
				plat/${SOC_DIR}/driver/ihost_pll_config.c \
				plat/${SOC_DIR}/src/bl31_setup.c \
				plat/${SOC_DIR}/src/fsx.c \
				plat/${SOC_DIR}/src/iommu.c \
				plat/${SOC_DIR}/src/sdio.c \
				${BRCM_GIC_SOURCES}

ifneq (${NCSI_IO_DRIVE_STRENGTH_MA},)
BL31_SOURCES   +=      plat/${SOC_DIR}/src/ncsi.c
endif

ifeq (${USE_PAXB},yes)
BL31_SOURCES   +=      plat/${SOC_DIR}/src/paxb.c
BL31_SOURCES   +=      plat/${SOC_DIR}/src/sr_paxb_phy.c
endif

ifeq (${USE_PAXC},yes)
BL31_SOURCES   +=      plat/${SOC_DIR}/src/paxc.c
endif

ifdef SCP_BL2
PLAT_INCLUDES		+=	-Iplat/brcm/common/

BL2_SOURCES		+=	plat/brcm/common/brcm_mhu.c \
				plat/brcm/common/brcm_scpi.c \
				plat/${SOC_DIR}/src/scp_utils.c \
				plat/${SOC_DIR}/src/scp_cmd.c \
				drivers/brcm/scp.c

BL31_SOURCES		+=	plat/brcm/common/brcm_mhu.c \
				plat/brcm/common/brcm_scpi.c \
				plat/${SOC_DIR}/src/brcm_pm_ops.c
else
BL31_SOURCES		+=	plat/${SOC_DIR}/src/ihost_pm.c \
				plat/${SOC_DIR}/src/pm.c
endif

ifeq (${ELOG_SUPPORT},1)
ifeq (${ELOG_STORE_MEDIA},DDR)
BL2_SOURCES		+=	plat/brcm/board/common/bcm_elog_ddr.c
endif
endif

ifeq (${BL31_BOOT_PRELOADED_SCP}, 1)
ifdef SCP_BL2
SCP_CFG_DIR=$(dir ${SCP_BL2})
PLAT_INCLUDES	+=	-I${SCP_CFG_DIR}
endif
PLAT_INCLUDES	+=	-Iplat/brcm/common/

# By default use OPTEE Assigned memory
PRELOADED_SCP_BASE ?= 0x8E000000
PRELOADED_SCP_SIZE ?= 0x10000
$(eval $(call add_define,PRELOADED_SCP_BASE))
$(eval $(call add_define,PRELOADED_SCP_SIZE))
$(eval $(call add_define,BL31_BOOT_PRELOADED_SCP))
BL31_SOURCES += plat/${SOC_DIR}/src/scp_utils.c \
		plat/${SOC_DIR}/src/scp_cmd.c \
		drivers/brcm/scp.c
endif

# Do not execute the startup code on warm reset.
PROGRAMMABLE_RESET_ADDRESS	:=	1

# Nitro FW, config and Crash log uses secure DDR memory
# Inaddition to above, Nitro master and slave is also secure
ifneq ($(NITRO_SECURE_ACCESS),)
$(eval $(call add_define,NITRO_SECURE_ACCESS))
$(eval $(call add_define,DDR_NITRO_SECURE_REGION_START))
$(eval $(call add_define,DDR_NITRO_SECURE_REGION_END))
endif
