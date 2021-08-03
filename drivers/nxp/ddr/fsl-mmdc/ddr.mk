#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

# MMDC ddr cntlr driver files

DDR_DRIVERS_PATH	:=	drivers/nxp/ddr

DDR_CNTLR_SOURCES	:=	${PLAT_DRIVERS_PATH}/ddr/fsl-mmdc/fsl_mmdc.c \
				${PLAT_DRIVERS_PATH}/ddr/nxp-ddr/utility.c	\
				${PLAT_DRIVERS_PATH}/ddr/nxp-ddr/ddr.c	\
				${PLAT_DRIVERS_PATH}/ddr/nxp-ddr/ddrc.c

PLAT_INCLUDES		+=	-I$(PLAT_DRIVERS_INCLUDE_PATH)/ddr	\
				-I$(PLAT_DRIVERS_INCLUDE_PATH)/ddr/fsl-mmdc
#------------------------------------------------
