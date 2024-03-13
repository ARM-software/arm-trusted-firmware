#
# Copyright (c) 2021-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Process RME_GPT_MAX_BLOCK value
ifeq ($(filter 0 2 32 512, ${RME_GPT_MAX_BLOCK}),)
    $(error "Invalid value for RME_GPT_MAX_BLOCK: ${RME_GPT_MAX_BLOCK}")
endif

GPT_LIB_SRCS	:=	$(addprefix lib/gpt_rme/,        \
			gpt_rme.c)
