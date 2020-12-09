#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------

# MMDC ddr cntlr driver files

DDR_DRIVERS_PATH	:=	drivers/nxp/ddr

DDR_CNTLR_SOURCES	:=	${DDR_DRIVERS_PATH}/fsl-mmdc/fsl_mmdc.c \
				${DDR_DRIVERS_PATH}/nxp-ddr/utility.c	\
				${DDR_DRIVERS_PATH}/nxp-ddr/ddr.c	\
				${DDR_DRIVERS_PATH}/nxp-ddr/ddrc.c

PLAT_INCLUDES		+=	-I$(DDR_DRIVERS_PATH)/include	\
				-I$(DDR_DRIVERS_PATH)/fsl-mmdc
#------------------------------------------------
