#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# NXP Non-Volatile data flag storage used and then cleared by SW on boot-up

$(eval $(call add_define,NXP_NV_SW_MAINT_LAST_EXEC_DATA))

ifeq ($(NXP_COINED_BB),yes)
$(eval $(call add_define,NXP_COINED_BB))
# BL2 : To read the reset cause from LP SECMON GPR register
# BL31: To write the reset cause to LP SECMON GPR register
$(eval $(call SET_NXP_MAKE_FLAG,SNVS_NEEDED,BL_COMM))

# BL2: DDR training data is stored on Flexspi NOR.
ifneq (${BOOT_MODE},flexspi_nor)
$(eval $(call SET_NXP_MAKE_FLAG,XSPI_NEEDED,BL2))
endif

else
$(eval $(call add_define_val,DEFAULT_NV_STORAGE_BASE_ADDR,'${BL2_BIN_XSPI_NOR_END_ADDRESS} - 2 * ${NXP_XSPI_NOR_UNIT_SIZE}'))
$(eval $(call SET_NXP_MAKE_FLAG,XSPI_NEEDED,BL_COMM))
endif

NV_STORAGE_INCLUDES	+=  -I${PLAT_COMMON_PATH}/nv_storage

NV_STORAGE_SOURCES	+=  ${PLAT_COMMON_PATH}/nv_storage/plat_nv_storage.c
