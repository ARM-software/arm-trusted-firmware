#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${SFP_ADDED},)

SFP_ADDED		:= 1
$(eval $(call add_define, NXP_SFP_ENABLED))

PLAT_INCLUDES		+= -I$(PLAT_DRIVERS_INCLUDE_PATH)/sfp

SFP_SOURCES		+= $(PLAT_DRIVERS_PATH)/sfp/sfp.c

ifeq (${FUSE_PROG}, 1)
SFP_BL2_SOURCES		+= $(PLAT_DRIVERS_PATH)/sfp/fuse_prov.c
endif

ifeq (${BL_COMM_SFP_NEEDED},yes)
BL_COMMON_SOURCES	+= ${SFP_SOURCES}
BL2_SOURCES		+= ${SFP_BL2_SOURCES}
else
ifeq (${BL2_SFP_NEEDED},yes)
BL2_SOURCES		+= ${SFP_SOURCES}\
			   ${SFP_BL2_SOURCES}
endif
ifeq (${BL31_SFP_NEEDED},yes)
BL31_SOURCES		+= ${SFP_SOURCES}
endif
endif
endif
#------------------------------------------------
