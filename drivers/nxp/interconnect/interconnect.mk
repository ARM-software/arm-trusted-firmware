# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
#------------------------------------------------------------------------------
#
# Select the Interconnect files
#
# -----------------------------------------------------------------------------

ifeq (${ADD_INTERCONNECT},)

ADD_INTERCONNECT	:= 1
PLAT_INCLUDES		+= -I${PLAT_DRIVERS_PATH}/interconnect

ifeq (, $(filter $(INTERCONNECT), CCI400 CCN502 CCN504 CCN508))
    $(error -> Interconnect type not set!)
else
$(eval $(call add_define_val,INTERCONNECT,${INTERCONNECT}))
ifeq ($(INTERCONNECT), $(filter $(INTERCONNECT), CCN502 CCN504 CCN508))
INTERCONNECT_SOURCES	:= 	drivers/arm/ccn/ccn.c 		\
				${PLAT_DRIVERS_PATH}/interconnect/ls_ccn.c
else
ifeq ($(INTERCONNECT), CCI400)
INTERCONNECT_SOURCES	:= 	drivers/arm/cci/cci.c 		\
				${PLAT_DRIVERS_PATH}/interconnect/ls_cci.c
endif
endif
endif

ifeq (${BL_COMM_INTERCONNECT_NEEDED},yes)
BL_COMMON_SOURCES	+= ${INTERCONNECT_SOURCES}
else
ifeq (${BL2_INTERCONNECT_NEEDED},yes)
BL2_SOURCES		+= ${INTERCONNECT_SOURCES}
endif
ifeq (${BL31_INTERCONNECT_NEEDED},yes)
BL31_SOURCES		+= ${INTERCONNECT_SOURCES}
endif
endif
endif

# -----------------------------------------------------------------------------
