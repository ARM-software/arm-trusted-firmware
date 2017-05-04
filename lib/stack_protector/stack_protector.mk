#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Boolean macro to be used in C code
STACK_PROTECTOR_ENABLED := 0

ifneq (${ENABLE_STACK_PROTECTOR},0)
STACK_PROTECTOR_ENABLED := 1
BL_COMMON_SOURCES	+=	lib/stack_protector/stack_protector.c			\
				lib/stack_protector/${ARCH}/asm_stack_protector.S

TF_CFLAGS		+=	-fstack-protector-${ENABLE_STACK_PROTECTOR}
endif

$(eval $(call add_define,STACK_PROTECTOR_ENABLED))

