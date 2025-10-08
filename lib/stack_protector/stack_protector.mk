#
# Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Boolean macro to be used in C code
STACK_PROTECTOR_ENABLED := 0

ifeq (${ENABLE_STACK_PROTECTOR},0)
  ENABLE_STACK_PROTECTOR := none
endif

ifeq (${ENABLE_STACK_PROTECTOR},none)
  cflags-common		+=      -fno-stack-protector
else
  STACK_PROTECTOR_ENABLED := 1
  BL_COMMON_SOURCES	+=	lib/stack_protector/stack_protector.c	\
				lib/stack_protector/${ARCH}/asm_stack_protector.S

  ifeq (${ENABLE_STACK_PROTECTOR},default)
    cflags-common	+=	-fstack-protector
  else
    cflags-common	+=	-fstack-protector-${ENABLE_STACK_PROTECTOR}
  endif
endif

$(eval $(call add_define,STACK_PROTECTOR_ENABLED))
