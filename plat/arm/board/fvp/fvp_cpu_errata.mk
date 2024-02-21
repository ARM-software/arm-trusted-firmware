#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Flags to enable the cpu structures in the Errata ABI file
# file: services/std_svc/errata_abi/errata_abi_main.c. This is specifically
# for platforms that need to enable errata based on non-arm interconnect IP.

ifeq (${ERRATA_ABI_SUPPORT}, 1)
ifeq (${ERRATA_NON_ARM_INTERCONNECT}, 1)
ifeq (${CTX_INCLUDE_AARCH32_REGS}, 0)
CORTEX_A78_H_INC	:= 1
NEOVERSE_N2_H_INC	:= 1
NEOVERSE_V1_H_INC	:= 1
CORTEX_A78_AE_H_INC	:= 1
CORTEX_A710_H_INC	:= 1
CORTEX_X3_H_INC		:= 1
CORTEX_A78C_H_INC	:= 1
CORTEX_X2_H_INC		:= 1
$(eval $(call add_define, CORTEX_A78_H_INC))
$(eval $(call add_define, NEOVERSE_N2_H_INC))
$(eval $(call add_define, NEOVERSE_V1_H_INC))
$(eval $(call add_define, CORTEX_A78_AE_H_INC))
$(eval $(call add_define, CORTEX_A710_H_INC))
$(eval $(call add_define, CORTEX_X3_H_INC))
$(eval $(call add_define, CORTEX_A78C_H_INC))
$(eval $(call add_define, CORTEX_X2_H_INC))
endif
endif
endif
