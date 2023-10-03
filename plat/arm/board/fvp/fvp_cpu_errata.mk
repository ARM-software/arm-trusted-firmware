#
# Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


#/*
# * TODO: below lines of code to be removed
# * after abi and framework are synchronized
# */

ifeq (${ERRATA_ABI_SUPPORT}, 1)
# enable the cpu macros for errata abi interface
ifeq (${ARCH}, aarch64)
ifeq (${HW_ASSISTED_COHERENCY}, 0)
CORTEX_A35_H_INC	:= 1
CORTEX_A53_H_INC	:= 1
CORTEX_A57_H_INC	:= 1
CORTEX_A72_H_INC	:= 1
CORTEX_A73_H_INC	:= 1
$(eval $(call add_define, CORTEX_A35_H_INC))
$(eval $(call add_define, CORTEX_A53_H_INC))
$(eval $(call add_define, CORTEX_A57_H_INC))
$(eval $(call add_define, CORTEX_A72_H_INC))
$(eval $(call add_define, CORTEX_A73_H_INC))
else
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 0)
CORTEX_A76_H_INC	:= 1
CORTEX_A77_H_INC	:= 1
CORTEX_A78_H_INC	:= 1
NEOVERSE_N1_H_INC	:= 1
NEOVERSE_N2_H_INC	:= 1
NEOVERSE_V1_H_INC	:= 1
CORTEX_A78_AE_H_INC	:= 1
CORTEX_A510_H_INC	:= 1
CORTEX_A710_H_INC	:= 1
CORTEX_A715_H_INC 	:= 1
CORTEX_A78C_H_INC	:= 1
CORTEX_X2_H_INC		:= 1
$(eval $(call add_define, CORTEX_A76_H_INC))
$(eval $(call add_define, CORTEX_A77_H_INC))
$(eval $(call add_define, CORTEX_A78_H_INC))
$(eval $(call add_define, NEOVERSE_N1_H_INC))
$(eval $(call add_define, NEOVERSE_N2_H_INC))
$(eval $(call add_define, NEOVERSE_V1_H_INC))
$(eval $(call add_define, CORTEX_A78_AE_H_INC))
$(eval $(call add_define, CORTEX_A510_H_INC))
$(eval $(call add_define, CORTEX_A710_H_INC))
$(eval $(call add_define, CORTEX_A715_H_INC))
$(eval $(call add_define, CORTEX_A78C_H_INC))
$(eval $(call add_define, CORTEX_X2_H_INC))
endif
CORTEX_A55_H_INC	:= 1
CORTEX_A75_H_INC	:= 1
$(eval $(call add_define, CORTEX_A55_H_INC))
$(eval $(call add_define, CORTEX_A75_H_INC))
endif
else
CORTEX_A32_H_INC	:= 1
$(eval $(call add_define, CORTEX_A32_H_INC))
endif
endif
