#
# Copyright (c) 2021-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# RME_GPT_BITLOCK_BLOCK is the number of 512MB blocks
# per bit and the value must be power of 2.
BITLOCK_BLOCK_POWER_2=$(shell echo $$(( ${RME_GPT_BITLOCK_BLOCK} & (${RME_GPT_BITLOCK_BLOCK} - 1) )))

# Process RME_GPT_BITLOCK_BLOCK value
ifneq (${BITLOCK_BLOCK_POWER_2}, 0)
    $(error "RME_GPT_BITLOCK_BLOCK must be power of 2. Invalid value ${RME_GPT_BITLOCK_BLOCK}.")
endif

ifeq (${RME_GPT_BITLOCK_BLOCK},0)
    $(info "GPT library uses global spinlock")
endif

# Process the maximum size of supported contiguous blocks
# RME_GPT_MAX_BLOCK
ifeq ($(filter 0 2 32 512, ${RME_GPT_MAX_BLOCK}),)
    $(error "Invalid value for RME_GPT_MAX_BLOCK: ${RME_GPT_MAX_BLOCK}")
endif

GPT_LIB_SRCS	:=	$(addprefix lib/gpt_rme/,        \
			gpt_rme.c)
