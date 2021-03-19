#
# Copyright (c) 2018-2021, Renesas Electronics Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include plat/renesas/common/common.mk

ifndef LSI
  $(error "Error: Unknown LSI. Please use LSI=<LSI name> to specify the LSI")
else
  ifeq (${LSI},AUTO)
    RCAR_LSI:=${RCAR_AUTO}
  else ifeq (${LSI},G2M)
    RCAR_LSI:=${RZ_G2M}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else ifeq (${LSI_CUT},13)
        RCAR_LSI_CUT:=3
      else ifeq (${LSI_CUT},30)
        RCAR_LSI_CUT:=20
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2H)
    RCAR_LSI:=${RZ_G2H}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},30)
        RCAR_LSI_CUT:=20
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2N)
    RCAR_LSI:=${RZ_G2N}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},G2E)
    RCAR_LSI:=${RZ_G2E}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else ifeq (${LSI_CUT},11)
        RCAR_LSI_CUT:=1
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else
    $(error "Error: ${LSI} is not supported.")
  endif
  $(eval $(call add_define,RCAR_LSI))
endif

# Process RZG_LCS_STATE_DETECTION_ENABLE flag
# Enable to get LCS state information
ifndef RZG_LCS_STATE_DETECTION_ENABLE
RZG_LCS_STATE_DETECTION_ENABLE := 0
endif
$(eval $(call add_define,RZG_LCS_STATE_DETECTION_ENABLE))

# Process RCAR_SECURE_BOOT flag
ifndef RCAR_SECURE_BOOT
RCAR_SECURE_BOOT := 0
endif
$(eval $(call add_define,RCAR_SECURE_BOOT))

# LCS state of RZ/G2 Chip is all CM.
# However certain chips(RZ/G2M and RZ/G2E) have incorrect factory Fuse settings
# which results in getting incorrect LCS states
# if need to enable RCAR_SECURE_BOOT, make sure the chip has proper factory Fuse settings.
ifeq (${RCAR_SECURE_BOOT},1)
  ifeq (${RZG_LCS_STATE_DETECTION_ENABLE},0)
    $(error "Error: Please check the chip has proper factory Fuse settings and set RZG_LCS_STATE_DETECTION_ENABLE to enable.")
  endif
endif

# lock RPC HYPERFLASH access by default
# unlock to repogram the ATF firmware from u-boot
ifndef RCAR_RPC_HYPERFLASH_LOCKED
RCAR_RPC_HYPERFLASH_LOCKED := 1
endif
$(eval $(call add_define,RCAR_RPC_HYPERFLASH_LOCKED))

# Process RCAR_QOS_TYPE flag
ifndef RCAR_QOS_TYPE
RCAR_QOS_TYPE := 0
endif
$(eval $(call add_define,RCAR_QOS_TYPE))

# Process RCAR_DRAM_SPLIT flag
ifndef RCAR_DRAM_SPLIT
RCAR_DRAM_SPLIT := 0
endif
$(eval $(call add_define,RCAR_DRAM_SPLIT))

# Process RCAR_BL33_EXECUTION_EL flag
ifndef RCAR_BL33_EXECUTION_EL
RCAR_BL33_EXECUTION_EL := 0
endif
$(eval $(call add_define,RCAR_BL33_EXECUTION_EL))

# Process RCAR_AVS_SETTING_ENABLE flag
ifndef AVS_SETTING_ENABLE
AVS_SETTING_ENABLE := 0
endif
$(eval $(call add_define,AVS_SETTING_ENABLE))

# Process RCAR_LOSSY_ENABLE flag
ifndef RCAR_LOSSY_ENABLE
RCAR_LOSSY_ENABLE := 0
endif
$(eval $(call add_define,RCAR_LOSSY_ENABLE))

# Process LIFEC_DBSC_PROTECT_ENABLE flag
ifndef LIFEC_DBSC_PROTECT_ENABLE
LIFEC_DBSC_PROTECT_ENABLE := 1
endif
$(eval $(call add_define,LIFEC_DBSC_PROTECT_ENABLE))

# Process RCAR_GEN3_ULCB flag
ifndef RCAR_GEN3_ULCB
RCAR_GEN3_ULCB := 0
endif

# Process RCAR_REF_INT flag
ifndef RCAR_REF_INT
RCAR_REF_INT :=0
endif
$(eval $(call add_define,RCAR_REF_INT))

# Process RCAR_REWT_TRAINING flag
ifndef RCAR_REWT_TRAINING
RCAR_REWT_TRAINING := 1
endif
$(eval $(call add_define,RCAR_REWT_TRAINING))

# Process RCAR_SYSTEM_SUSPEND flag
ifndef RCAR_SYSTEM_SUSPEND
RCAR_SYSTEM_SUSPEND := 0
endif
$(eval $(call add_define,RCAR_SYSTEM_SUSPEND))

# Process RCAR_DRAM_LPDDR4_MEMCONF flag
ifndef RCAR_DRAM_LPDDR4_MEMCONF
RCAR_DRAM_LPDDR4_MEMCONF :=1
endif
$(eval $(call add_define,RCAR_DRAM_LPDDR4_MEMCONF))

# Process RCAR_DRAM_DDR3L_MEMCONF flag
ifndef RCAR_DRAM_DDR3L_MEMCONF
RCAR_DRAM_DDR3L_MEMCONF :=1
endif
$(eval $(call add_define,RCAR_DRAM_DDR3L_MEMCONF))

# Process RCAR_DRAM_DDR3L_MEMDUAL flag
ifndef RCAR_DRAM_DDR3L_MEMDUAL
RCAR_DRAM_DDR3L_MEMDUAL :=1
endif
$(eval $(call add_define,RCAR_DRAM_DDR3L_MEMDUAL))

# Process RCAR_BL33_ARG0 flag
ifdef RCAR_BL33_ARG0
$(eval $(call add_define,RCAR_BL33_ARG0))
endif

#Process RCAR_BL2_DCACHE flag
ifndef RCAR_BL2_DCACHE
RCAR_BL2_DCACHE := 0
endif
$(eval $(call add_define,RCAR_BL2_DCACHE))

# Process RCAR_DRAM_CHANNEL flag
ifndef RCAR_DRAM_CHANNEL
RCAR_DRAM_CHANNEL :=15
endif
$(eval $(call add_define,RCAR_DRAM_CHANNEL))

#Process RCAR_SYSTEM_RESET_KEEPON_DDR flag
ifndef RCAR_SYSTEM_RESET_KEEPON_DDR
RCAR_SYSTEM_RESET_KEEPON_DDR := 0
endif
$(eval $(call add_define,RCAR_SYSTEM_RESET_KEEPON_DDR))

RZG_SOC :=1
$(eval $(call add_define,RZG_SOC))

include drivers/renesas/common/ddr/ddr.mk
include drivers/renesas/rzg/qos/qos.mk
include drivers/renesas/rzg/pfc/pfc.mk
include lib/libfdt/libfdt.mk

PLAT_INCLUDES	+=	-Idrivers/renesas/common/ddr		\
			-Idrivers/renesas/rzg/qos		\
			-Idrivers/renesas/rzg/board		\
			-Idrivers/renesas/common		\
			-Idrivers/renesas/common/iic_dvfs	\
			-Idrivers/renesas/common/avs		\
			-Idrivers/renesas/common/delay		\
			-Idrivers/renesas/common/rom		\
			-Idrivers/renesas/common/scif		\
			-Idrivers/renesas/common/emmc		\
			-Idrivers/renesas/common/pwrc		\
			-Idrivers/renesas/common/io

BL2_SOURCES	+=	plat/renesas/rzg/bl2_plat_setup.c	\
			drivers/renesas/rzg/board/board.c

# build the layout images for the bootrom and the necessary srecords
rzg: rzg_layout_create rzg_srecord
distclean realclean clean: clean_layout_tool clean_srecord

# layout images
LAYOUT_TOOLPATH ?= tools/renesas/rzg_layout_create

clean_layout_tool:
	@echo "clean layout tool"
	${Q}${MAKE} -C ${LAYOUT_TOOLPATH} clean

.PHONY: rzg_layout_create
rzg_layout_create:
	@echo "generating layout srecs"
	${Q}${MAKE} CPPFLAGS="-D=AARCH64" --no-print-directory -C ${LAYOUT_TOOLPATH}

# srecords
SREC_PATH	= ${BUILD_PLAT}
BL2_ELF_SRC	= ${SREC_PATH}/bl2/bl2.elf
BL31_ELF_SRC	= ${SREC_PATH}/bl31/bl31.elf

clean_srecord:
	@echo "clean bl2 and bl31 srecs"
	rm -f ${SREC_PATH}/bl2.srec ${SREC_PATH}/bl31.srec

.PHONY: rzg_srecord
rzg_srecord: $(BL2_ELF_SRC) $(BL31_ELF_SRC)
	@echo "generating srec: ${SREC_PATH}/bl2.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL2_ELF_SRC}  ${SREC_PATH}/bl2.srec
	@echo "generating srec: ${SREC_PATH}/bl31.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL31_ELF_SRC} ${SREC_PATH}/bl31.srec
