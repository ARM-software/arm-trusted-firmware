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
  else ifeq (${LSI},H3)
    RCAR_LSI:=${RCAR_H3}
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
      else ifeq (${LSI_CUT},20)
        RCAR_LSI_CUT:=10
      else ifeq (${LSI_CUT},30)
        RCAR_LSI_CUT:=20
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},H3N)
    RCAR_LSI:=${RCAR_H3N}
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
  else ifeq (${LSI},M3)
    RCAR_LSI:=${RCAR_M3}
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
  else ifeq (${LSI},M3N)
    RCAR_LSI:=${RCAR_M3N}
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
  else ifeq (${LSI},E3)
    RCAR_LSI:=${RCAR_E3}
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
  else ifeq (${LSI},D3)
    RCAR_LSI:=${RCAR_D3}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      else
        $(error "Error: ${LSI_CUT} is not supported.")
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else ifeq (${LSI},V3M)
    RCAR_LSI:=${RCAR_V3M}
    ifndef LSI_CUT
      # enable compatible function.
      RCAR_LSI_CUT_COMPAT := 1
      $(eval $(call add_define,RCAR_LSI_CUT_COMPAT))
    else
      # disable compatible function.
      ifeq (${LSI_CUT},10)
        RCAR_LSI_CUT:=0
      endif
      ifeq (${LSI_CUT},20)
        RCAR_LSI_CUT:=10
      endif
      $(eval $(call add_define,RCAR_LSI_CUT))
    endif
  else
    $(error "Error: ${LSI} is not supported.")
  endif
  $(eval $(call add_define,RCAR_LSI))
endif

# lock RPC HYPERFLASH access by default
# unlock to repogram the ATF firmware from u-boot
ifndef RCAR_RPC_HYPERFLASH_LOCKED
RCAR_RPC_HYPERFLASH_LOCKED := 1
endif
$(eval $(call add_define,RCAR_RPC_HYPERFLASH_LOCKED))

# Process RCAR_SECURE_BOOT flag
ifndef RCAR_SECURE_BOOT
RCAR_SECURE_BOOT := 1
endif
$(eval $(call add_define,RCAR_SECURE_BOOT))

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
ifeq (${RCAR_AVS_SETTING_ENABLE},0)
AVS_SETTING_ENABLE := 0
else
AVS_SETTING_ENABLE := 1
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

# Process PMIC_ROHM_BD9571 flag
ifndef PMIC_ROHM_BD9571
PMIC_ROHM_BD9571 := 1
endif
$(eval $(call add_define,PMIC_ROHM_BD9571))

# Process PMIC_LEVEL_MODE flag
ifndef PMIC_LEVEL_MODE
PMIC_LEVEL_MODE := 1
endif
$(eval $(call add_define,PMIC_LEVEL_MODE))

# Process RCAR_GEN3_ULCB flag
ifndef RCAR_GEN3_ULCB
RCAR_GEN3_ULCB := 0
endif
ifeq (${RCAR_GEN3_ULCB},1)
 BOARD_DEFAULT := 0x10
 $(eval $(call add_define,BOARD_DEFAULT))
endif
$(eval $(call add_define,RCAR_GEN3_ULCB))

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
RCAR_SYSTEM_SUSPEND := 1
endif
$(eval $(call add_define,RCAR_SYSTEM_SUSPEND))

# SYSTEM_SUSPEND requires power control of PMIC etc.
# When executing SYSTEM_SUSPEND other than Salvator-X, Salvator-XS and Ebisu,
# processing equivalent to that implemented in PMIC_ROHM_BD9571 is necessary.
ifeq (${RCAR_SYSTEM_SUSPEND},1)
  ifeq (${PMIC_ROHM_BD9571},0)
    $(error "Error: When you want RCAR_SYSTEM_SUSPEND to be enable, please also set PMIC_ROHM_BD9571 to enable.")
  endif
endif

# Process RCAR_DRAM_LPDDR4_MEMCONF flag
ifndef RCAR_DRAM_LPDDR4_MEMCONF
RCAR_DRAM_LPDDR4_MEMCONF :=1
endif
$(eval $(call add_define,RCAR_DRAM_LPDDR4_MEMCONF))

# Process RCAR_DRAM_MEMRANK flag
ifndef RCAR_DRAM_MEMRANK
RCAR_DRAM_MEMRANK :=0
endif
$(eval $(call add_define,RCAR_DRAM_MEMRANK))

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

ifndef RCAR_GEN3_BL33_GZIP
RCAR_GEN3_BL33_GZIP := 0
endif
$(eval $(call add_define,RCAR_GEN3_BL33_GZIP))

# RCAR_SYSTEM_RESET_KEEPON_DDR requires power control of PMIC etc.
# When executing SYSTEM_SUSPEND other than Salvator-X, Salvator-XS and Ebisu,
# processing equivalent to that implemented in PMIC_ROHM_BD9571 is necessary.
# Also, it is necessary to enable RCAR_SYSTEM_SUSPEND.
ifeq (${RCAR_SYSTEM_RESET_KEEPON_DDR},1)
  ifeq (${PMIC_ROHM_BD9571},0)
    $(error "Error: When you want RCAR_SYSTEM_RESET_KEEPON_DDR to be enable, please also set PMIC_ROHM_BD9571 to enable.")
  endif
  ifeq (${RCAR_SYSTEM_SUSPEND},0)
    $(error "Error: When you want RCAR_SYSTEM_RESET_KEEPON_DDR to be enable, please also set RCAR_SYSTEM_SUSPEND to enable.")
  endif
endif

include drivers/renesas/common/ddr/ddr.mk
include drivers/renesas/rcar/qos/qos.mk
include drivers/renesas/rcar/pfc/pfc.mk
include lib/libfdt/libfdt.mk

PLAT_INCLUDES	+=	-Idrivers/renesas/common/ddr		\
			-Idrivers/renesas/rcar/qos		\
			-Idrivers/renesas/rcar/board		\
			-Idrivers/renesas/rcar/cpld/		\
			-Idrivers/renesas/common		\
			-Idrivers/renesas/common/iic_dvfs	\
			-Idrivers/renesas/common/avs		\
			-Idrivers/renesas/common/delay		\
			-Idrivers/renesas/common/rom		\
			-Idrivers/renesas/common/scif		\
			-Idrivers/renesas/common/emmc		\
			-Idrivers/renesas/common/pwrc		\
			-Idrivers/renesas/common/io

BL2_SOURCES	+=	plat/renesas/rcar/bl2_plat_setup.c	\
			drivers/renesas/rcar/board/board.c

ifeq (${RCAR_GEN3_BL33_GZIP},1)
include lib/zlib/zlib.mk

BL2_SOURCES	+=	common/image_decompress.c               \
			$(ZLIB_SOURCES)
endif

ifeq (${RCAR_GEN3_ULCB},1)
BL31_SOURCES		+=	drivers/renesas/rcar/cpld/ulcb_cpld.c
endif

# build the layout images for the bootrom and the necessary srecords
rcar: rcar_layout_tool rcar_srecord
distclean realclean clean: clean_layout_tool clean_srecord

# layout images
LAYOUT_TOOLPATH ?= tools/renesas/rcar_layout_create

clean_layout_tool:
	@echo "clean layout tool"
	${Q}${MAKE} -C ${LAYOUT_TOOLPATH} clean

.PHONY: rcar_layout_tool
rcar_layout_tool:
	@echo "generating layout srecs"
	${Q}${MAKE} CPPFLAGS="-D=AARCH64" --no-print-directory -C ${LAYOUT_TOOLPATH}

# srecords
SREC_PATH	= ${BUILD_PLAT}
BL2_ELF_SRC	= ${SREC_PATH}/bl2/bl2.elf
BL31_ELF_SRC	= ${SREC_PATH}/bl31/bl31.elf

clean_srecord:
	@echo "clean bl2 and bl31 srecs"
	rm -f ${SREC_PATH}/bl2.srec ${SREC_PATH}/bl31.srec

.PHONY: rcar_srecord
rcar_srecord: $(BL2_ELF_SRC) $(BL31_ELF_SRC)
	@echo "generating srec: ${SREC_PATH}/bl2.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL2_ELF_SRC}  ${SREC_PATH}/bl2.srec
	@echo "generating srec: ${SREC_PATH}/bl31.srec"
	$(Q)$(OC) -O srec --srec-forceS3 ${BL31_ELF_SRC} ${SREC_PATH}/bl31.srec

