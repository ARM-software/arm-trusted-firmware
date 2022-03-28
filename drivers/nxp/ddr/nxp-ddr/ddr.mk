#
# Copyright 2021-2022 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq ($(PLAT_DDR_PHY), PHY_GEN2)
$(eval $(call add_define, PHY_GEN2))
PLAT_DDR_PHY_DIR		:= phy-gen2
ifeq (${APPLY_MAX_CDD},yes)
$(eval $(call add_define,NXP_APPLY_MAX_CDD))
endif

ifeq (${ERRATA_DDR_A011396}, 1)
$(eval $(call add_define,ERRATA_DDR_A011396))
endif

ifeq (${ERRATA_DDR_A050450}, 1)
$(eval $(call add_define,ERRATA_DDR_A050450))
endif

ifeq (${ERRATA_DDR_A050958}, 1)
$(eval $(call add_define,ERRATA_DDR_A050958))
endif

endif

ifeq ($(PLAT_DDR_PHY), PHY_GEN1)
PLAT_DDR_PHY_DIR		:= phy-gen1

ifeq (${ERRATA_DDR_A008511},1)
$(eval $(call add_define,ERRATA_DDR_A008511))
endif

ifeq (${ERRATA_DDR_A009803},1)
$(eval $(call add_define,ERRATA_DDR_A009803))
endif

ifeq (${ERRATA_DDR_A009942},1)
$(eval $(call add_define,ERRATA_DDR_A009942))
endif

ifeq (${ERRATA_DDR_A010165},1)
$(eval $(call add_define,ERRATA_DDR_A010165))
endif

endif

ifeq ($(DDR_BIST), yes)
$(eval $(call add_define, BIST_EN))
endif

ifeq ($(DDR_DEBUG), yes)
$(eval $(call add_define, DDR_DEBUG))
endif

ifeq ($(DDR_PHY_DEBUG), yes)
$(eval $(call add_define, DDR_PHY_DEBUG))
endif

ifeq ($(DEBUG_PHY_IO), yes)
$(eval $(call add_define, DEBUG_PHY_IO))
endif

ifeq ($(DEBUG_WARM_RESET), yes)
$(eval $(call add_define, DEBUG_WARM_RESET))
endif

ifeq ($(DEBUG_DDR_INPUT_CONFIG), yes)
$(eval $(call add_define, DEBUG_DDR_INPUT_CONFIG))
endif

DDR_CNTLR_SOURCES	:= $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddr.c \
			   $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/ddrc.c \
			   $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/dimm.c \
			   $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/regs.c \
			   $(PLAT_DRIVERS_PATH)/ddr/nxp-ddr/utility.c \
			   $(PLAT_DRIVERS_PATH)/ddr/$(PLAT_DDR_PHY_DIR)/phy.c

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/ddr
