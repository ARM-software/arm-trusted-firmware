#
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Process RME_GPT_BITLOCK_BLOCK value
ifeq ($(filter 0 1 2 4 8 16 32 64 128 256 512, ${RME_GPT_BITLOCK_BLOCK}),)
    $(error "Invalid value for RME_GPT_BITLOCK_BLOCK: ${RME_GPT_BITLOCK_BLOCK}")
endif

ifeq (${RME_GPT_BITLOCK_BLOCK},0)
    $(warning "GPT library uses global spinlock")
endif

# Process RME_GPT_MAX_BLOCK value
ifeq ($(filter 0 2 32 512, ${RME_GPT_MAX_BLOCK}),)
    $(error "Invalid value for RME_GPT_MAX_BLOCK: ${RME_GPT_MAX_BLOCK}")
endif

GPT_LIB_SRCS	:=	$(addprefix lib/gpt_rme/,        \
			gpt_rme.c)
