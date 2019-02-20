#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

DDR_DRIVERS_PATH	:=	${PLAT_DRIVERS_PATH}/ddr

DDR_CNTLR_SOURCES	=  $(DDR_DRIVERS_PATH)/nxp-ddr/ddr.c \
			   $(DDR_DRIVERS_PATH)/nxp-ddr/ddrc.c \
			   $(DDR_DRIVERS_PATH)/nxp-ddr/dimm.c \
			   $(DDR_DRIVERS_PATH)/nxp-ddr/regs.c \
			   $(DDR_DRIVERS_PATH)/nxp-ddr/utility.c \
			   $(DDR_DRIVERS_PATH)/$(PLAT_DDR_PHY)/phy.c

ifeq ($(DDR_BIST), yes)
$(eval $(call add_define, BIST_EN))
endif

ifeq ($(DDR_DEBUG), yes)
$(eval $(call add_define, DDR_DEBUG))
endif
