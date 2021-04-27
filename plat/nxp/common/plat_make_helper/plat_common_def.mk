# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Include build macros, for example: SET_NXP_MAKE_FLAG
include plat/nxp/common/plat_make_helper/plat_build_macros.mk

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

ifneq (${QSPI_FLASH_SZ},)
$(eval $(call add_define_val,NXP_QSPI_FLASH_SIZE,${QSPI_FLASH_SZ}))
endif

ifneq (${NOR_FLASH_SZ},)
$(eval $(call add_define_val,NXP_NOR_FLASH_SIZE,${NOR_FLASH_SZ}))
endif


ifneq (${FSPI_ERASE_4K},)
$(eval $(call add_define_val,CONFIG_FSPI_ERASE_4K,${FSPI_ERASE_4K}))
endif

ifneq (${NUM_OF_DDRC},)
$(eval $(call add_define_val,NUM_OF_DDRC,${NUM_OF_DDRC}))
endif

ifeq (${CONFIG_DDR_NODIMM},1)
$(eval $(call add_define,CONFIG_DDR_NODIMM))
DDRC_NUM_DIMM := 1
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

ifeq (${CONFIG_STATIC_DDR},1)
$(eval $(call add_define,CONFIG_STATIC_DDR))
endif

# Platform can control the base address for non-volatile storage.
#$(eval $(call add_define_val,NV_STORAGE_BASE_ADDR,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - 2 * ${NXP_XSPI_NOR_UNIT_SIZE}'))

ifeq (${WARM_BOOT},yes)
$(eval $(call add_define_val,PHY_TRAINING_REGS_ON_FLASH,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - ${NXP_XSPI_NOR_UNIT_SIZE}'))
endif

# Selecting Boot Source for the TFA images.
define add_boot_mode_define
    ifeq ($(1),qspi)
        $$(eval $$(call SET_NXP_MAKE_FLAG,QSPI_NEEDED,BL2))
        $$(eval $$(call add_define,QSPI_BOOT))
    else ifeq ($(1),sd)
        $$(eval $$(call SET_NXP_MAKE_FLAG,SD_MMC_NEEDED,BL2))
        $$(eval $$(call add_define,SD_BOOT))
    else ifeq ($(1),emmc)
        $$(eval $$(call SET_NXP_MAKE_FLAG,SD_MMC_NEEDED,BL2))
        $$(eval $$(call add_define,EMMC_BOOT))
    else ifeq ($(1),nor)
        $$(eval $$(call SET_NXP_MAKE_FLAG,IFC_NOR_NEEDED,BL2))
        $$(eval $$(call add_define,NOR_BOOT))
    else ifeq ($(1),nand)
        $$(eval $$(call SET_NXP_MAKE_FLAG,IFC_NAND_NEEDED,BL2))
        $$(eval $$(call add_define,NAND_BOOT))
    else ifeq ($(1),flexspi_nor)
        $$(eval $$(call SET_NXP_MAKE_FLAG,XSPI_NEEDED,BL2))
        $$(eval $$(call add_define,FLEXSPI_NOR_BOOT))
    else
        $$(error $(PLAT) Cannot Support Boot Mode: $(BOOT_MODE))
    endif
endef

ifneq (,$(findstring $(BOOT_MODE),$(SUPPORTED_BOOT_MODE)))
    $(eval $(call add_boot_mode_define,$(strip $(BOOT_MODE))))
else
    $(error $(PLAT) Un-supported Boot Mode = $(BOOT_MODE))
endif
