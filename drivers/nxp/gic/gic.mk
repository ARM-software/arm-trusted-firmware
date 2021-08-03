# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the GIC files
#
# -----------------------------------------------------------------------------

ifeq (${ADD_GIC},)
ADD_GIC			:= 1
ifeq ($(GIC), GIC400)
include drivers/arm/gic/v2/gicv2.mk
GIC_SOURCES		+=	${GICV2_SOURCES}
GIC_SOURCES		+=	${PLAT_DRIVERS_PATH}/gic/ls_gicv2.c	\
				plat/common/plat_gicv2.c

PLAT_INCLUDES		+=	-I${PLAT_DRIVERS_INCLUDE_PATH}/gic/gicv2
else
ifeq ($(GIC), GIC500)
include drivers/arm/gic/v3/gicv3.mk
GIC_SOURCES		+=	${GICV3_SOURCES}
GIC_SOURCES		+=	${PLAT_DRIVERS_PATH}/gic/ls_gicv3.c	\
				plat/common/plat_gicv3.c

PLAT_INCLUDES		+=	-I${PLAT_DRIVERS_INCLUDE_PATH}/gic/gicv3
else
    $(error -> GIC type not set!)
endif
endif

ifeq (${BL_COMM_GIC_NEEDED},yes)
BL_COMMON_SOURCES	+= ${GIC_SOURCES}
else
ifeq (${BL2_GIC_NEEDED},yes)
BL2_SOURCES		+= ${GIC_SOURCES}
endif
ifeq (${BL31_GIC_NEEDED},yes)
BL31_SOURCES		+= ${GIC_SOURCES}
endif
endif
endif

# -----------------------------------------------------------------------------
