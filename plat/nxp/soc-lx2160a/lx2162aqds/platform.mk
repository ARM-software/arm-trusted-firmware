#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters

BOOT_MODE	?= 	flexspi_nor
BOARD		?=	lx2162aqds
POVDD_ENABLE	:=	no
NXP_COINED_BB	:=	no

 # DDR Compilation Configs
NUM_OF_DDRC	:=	1
DDRC_NUM_DIMM	:=	1
DDRC_NUM_CS	:=	2
DDR_ECC_EN	:=	yes
 #enable address decoding feature
DDR_ADDR_DEC	:=	yes
APPLY_MAX_CDD	:=	yes

# DDR Errata
ERRATA_DDR_A011396	:= 1
ERRATA_DDR_A050450	:= 1


 # On-Board Flash Details
FLASH_TYPE	:=	MT35XU512A
XSPI_FLASH_SZ	:=	0x10000000
NXP_XSPI_NOR_UNIT_SIZE		:=	0x20000
BL2_BIN_XSPI_NOR_END_ADDRESS	:=	0x100000
# CONFIG_FSPI_ERASE_4K is required to erase 4K sector sizes. This
# config is enabled for future use cases.
FSPI_ERASE_4K	:= 0

 # Platform specific features.
WARM_BOOT	:=	yes

 # Adding platform specific defines

$(eval $(call add_define_val,BOARD,'"${BOARD}"'))

ifeq (${POVDD_ENABLE},yes)
$(eval $(call add_define,CONFIG_POVDD_ENABLE))
endif

ifneq (${FLASH_TYPE},)
$(eval $(call add_define,CONFIG_${FLASH_TYPE}))
endif

ifneq (${XSPI_FLASH_SZ},)
$(eval $(call add_define_val,NXP_FLEXSPI_FLASH_SIZE,${XSPI_FLASH_SZ}))
endif

ifneq (${FSPI_ERASE_4K},)
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${FSPI_ERASE_4K}))
endif

ifneq (${NUM_OF_DDRC},)
$(eval $(call add_define_val,NUM_OF_DDRC,${NUM_OF_DDRC}))
endif

ifneq (${DDRC_NUM_DIMM},)
$(eval $(call add_define_val,DDRC_NUM_DIMM,${DDRC_NUM_DIMM}))
endif

ifneq (${DDRC_NUM_CS},)
$(eval $(call add_define_val,DDRC_NUM_CS,${DDRC_NUM_CS}))
endif

ifeq (${DDR_ADDR_DEC},yes)
$(eval $(call add_define,CONFIG_DDR_ADDR_DEC))
endif

ifeq (${DDR_ECC_EN},yes)
$(eval $(call add_define,CONFIG_DDR_ECC_EN))
endif

# Platform can control the base address for non-volatile storage.
#$(eval $(call add_define_val,NV_STORAGE_BASE_ADDR,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - 2 * ${NXP_XSPI_NOR_UNIT_SIZE}'))

ifeq (${WARM_BOOT},yes)
$(eval $(call add_define_val,PHY_TRAINING_REGS_ON_FLASH,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - ${NXP_XSPI_NOR_UNIT_SIZE}'))
endif

 # Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c\
			${BOARD_PATH}/platform.c

 # Adding SoC build info
include plat/nxp/soc-lx2160a/soc.mk
