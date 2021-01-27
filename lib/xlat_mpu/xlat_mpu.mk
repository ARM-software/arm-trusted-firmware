#
# Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

XLAT_MPU_LIB_V1_SRCS	:=	$(addprefix lib/xlat_mpu/,		\
				${ARCH}/enable_mpu.S			\
				${ARCH}/xlat_mpu_arch.c			\
				xlat_mpu_context.c			\
				xlat_mpu_core.c				\
				xlat_mpu_utils.c)

XLAT_MPU_LIB_V1	:=	1
$(eval $(call add_define,XLAT_MPU_LIB_V1))

ifeq (${ALLOW_XLAT_MPU}, 1)
    include lib/xlat_mpu_v2/ro_xlat_mpu.mk
endif
