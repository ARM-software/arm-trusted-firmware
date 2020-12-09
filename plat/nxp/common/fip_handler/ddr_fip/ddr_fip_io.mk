#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-----------------------------------------------------------------------------
ifeq (${DDR_FIP_IO_STORAGE_ADDED},)

$(eval $(call add_define, PLAT_DEF_FIP_UUID))
$(eval $(call add_define, PLAT_TBBR_IMG_DEF))
$(eval $(call SET_NXP_MAKE_FLAG,IMG_LOADR_NEEDED,BL2))

DDR_FIP_IO_STORAGE_ADDED	:= 1
$(eval $(call add_define,CONFIG_DDR_FIP_IMAGE))

FIP_HANDLER_PATH	:=  ${PLAT_COMMON_PATH}/fip_handler
FIP_HANDLER_COMMON_PATH	:=  ${FIP_HANDLER_PATH}/common
DDR_FIP_IO_STORAGE_PATH	:=  ${FIP_HANDLER_PATH}/ddr_fip

PLAT_INCLUDES		+= -I${FIP_HANDLER_COMMON_PATH}\
			   -I$(DDR_FIP_IO_STORAGE_PATH)

DDR_FIP_IO_SOURCES	+= $(DDR_FIP_IO_STORAGE_PATH)/ddr_io_storage.c

$(shell cp tools/nxp/plat_fiptool/plat_fiptool.mk ${PLAT_DIR})

ifeq (${BL_COMM_DDR_FIP_IO_NEEDED},yes)
BL_COMMON_SOURCES	+= ${DDR_FIP_IO_SOURCES}
else
ifeq (${BL2_DDR_FIP_IO_NEEDED},yes)
BL2_SOURCES		+= ${DDR_FIP_IO_SOURCES}
endif
ifeq (${BL31_DDR_FIP_IO_NEEDED},yes)
BL31_SOURCES		+= ${DDR_FIP_IO_SOURCES}
endif
endif
endif
#------------------------------------------------
