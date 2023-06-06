#
# Copyright (c) 2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Hash algorithm for DICE Protection Environment
# SHA-256 (or stronger) is required.
DPE_HASH_ALG	:=	sha256

ifeq (${DPE_HASH_ALG}, sha512)
    DPE_ALG_ID		:=	DPE_ALG_SHA512
    DPE_DIGEST_SIZE	:=	64U
else ifeq (${DPE_HASH_ALG}, sha384)
    DPE_ALG_ID		:=	DPE_ALG_SHA384
    DPE_DIGEST_SIZE	:=	48U
else
    DPE_ALG_ID		:=	DPE_ALG_SHA256
    DPE_DIGEST_SIZE	:=	32U
endif #DPE_HASH_ALG

# Set definitions for DICE Protection Environment
$(eval $(call add_defines,\
    $(sort \
        DPE_ALG_ID \
        DPE_DIGEST_SIZE \
)))

DPE_SOURCES	+=	drivers/measured_boot/rss/dice_prot_env.c
