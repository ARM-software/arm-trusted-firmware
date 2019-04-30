#
# Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Boolean macro to be used in C code
STACK_PROTECTOR_ENABLED := 0

ifeq (${ENABLE_STACK_PROTECTOR},0)
  ENABLE_STACK_PROTECTOR := none
endif

ifneq (${ENABLE_STACK_PROTECTOR},none)
  STACK_PROTECTOR_ENABLED := 1
  BL_COMMON_SOURCES	+=	lib/stack_protector/stack_protector.c	\
				lib/stack_protector/${ARCH}/asm_stack_protector.S

  ifeq (${ENABLE_STACK_PROTECTOR},default)
    TF_CFLAGS		+=	-fstack-protector
  else
    TF_CFLAGS		+=	-fstack-protector-${ENABLE_STACK_PROTECTOR}
  endif
endif

$(eval $(call add_define,STACK_PROTECTOR_ENABLED))
