# Copyright 2020-2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Adding SoC specific defines

ifneq (${CACHE_LINE},)
$(eval $(call add_define_val,PLATFORM_CACHE_LINE_SHIFT,${CACHE_LINE}))
$(eval CACHE_WRITEBACK_GRANULE=$(shell echo $$((1 << $(CACHE_LINE)))))
$(eval $(call add_define_val,CACHE_WRITEBACK_GRANULE,$(CACHE_WRITEBACK_GRANULE)))
endif

ifneq (${INTERCONNECT},)
$(eval $(call add_define,NXP_HAS_${INTERCONNECT}))
ifeq (${INTERCONNECT}, CCI400)
ICNNCT_ID := 0x420
$(eval $(call add_define,ICNNCT_ID))
endif
endif

ifneq (${CHASSIS},)
$(eval $(call add_define,CONFIG_CHASSIS_${CHASSIS}))
endif

ifneq (${PLAT_DDR_PHY},)
$(eval $(call add_define,NXP_DDR_${PLAT_DDR_PHY}))
endif

ifneq (${PHYS_SYS},)
$(eval $(call add_define,CONFIG_PHYS_64BIT))
endif

ifneq (${CSF_HDR_SZ},)
$(eval $(call add_define_val,CSF_HDR_SZ,${CSF_HDR_SZ}))
endif

ifneq (${OCRAM_START_ADDR},)
$(eval $(call add_define_val,NXP_OCRAM_ADDR,${OCRAM_START_ADDR}))
endif

ifneq (${OCRAM_SIZE},)
$(eval $(call add_define_val,NXP_OCRAM_SIZE,${OCRAM_SIZE}))
endif

ifneq (${NXP_ROM_RSVD},)
$(eval $(call add_define_val,NXP_ROM_RSVD,${NXP_ROM_RSVD}))
endif

ifneq (${BL2_BASE},)
$(eval $(call add_define_val,BL2_BASE,${BL2_BASE}))
endif

ifeq (${SEC_MEM_NON_COHERENT},yes)
$(eval $(call add_define,SEC_MEM_NON_COHERENT))
endif

ifneq (${NXP_ESDHC_ENDIANNESS},)
$(eval $(call add_define,NXP_ESDHC_${NXP_ESDHC_ENDIANNESS}))
endif

ifneq (${NXP_SFP_VER},)
$(eval $(call add_define,NXP_SFP_VER_${NXP_SFP_VER}))
endif

ifneq (${NXP_SFP_ENDIANNESS},)
$(eval $(call add_define,NXP_SFP_${NXP_SFP_ENDIANNESS}))
endif

ifneq (${NXP_GPIO_ENDIANNESS},)
$(eval $(call add_define,NXP_GPIO_${NXP_GPIO_ENDIANNESS}))
endif

ifneq (${NXP_SNVS_ENDIANNESS},)
$(eval $(call add_define,NXP_SNVS_${NXP_SNVS_ENDIANNESS}))
endif

ifneq (${NXP_GUR_ENDIANNESS},)
$(eval $(call add_define,NXP_GUR_${NXP_GUR_ENDIANNESS}))
endif

ifneq (${NXP_FSPI_ENDIANNESS},)
$(eval $(call add_define,NXP_FSPI_${NXP_FSPI_ENDIANNESS}))
endif

ifneq (${NXP_SEC_ENDIANNESS},)
$(eval $(call add_define,NXP_SEC_${NXP_SEC_ENDIANNESS}))
endif

ifneq (${NXP_DDR_ENDIANNESS},)
$(eval $(call add_define,NXP_DDR_${NXP_DDR_ENDIANNESS}))
endif

ifneq (${NXP_QSPI_ENDIANNESS},)
$(eval $(call add_define,NXP_QSPI_${NXP_QSPI_ENDIANNESS}))
endif

ifneq (${NXP_SCFG_ENDIANNESS},)
$(eval $(call add_define,NXP_SCFG_${NXP_SCFG_ENDIANNESS}))
endif

ifneq (${NXP_IFC_ENDIANNESS},)
$(eval $(call add_define,NXP_IFC_${NXP_IFC_ENDIANNESS}))
endif

ifneq (${NXP_DDR_INTLV_256B},)
$(eval $(call add_define,NXP_DDR_INTLV_256B))
endif

ifneq (${PLAT_XLAT_TABLES_DYNAMIC},)
$(eval $(call add_define,PLAT_XLAT_TABLES_DYNAMIC))
endif

ifeq (${OCRAM_ECC_EN},yes)
$(eval $(call add_define,CONFIG_OCRAM_ECC_EN))
include ${PLAT_COMMON_PATH}/ocram/ocram.mk
endif
