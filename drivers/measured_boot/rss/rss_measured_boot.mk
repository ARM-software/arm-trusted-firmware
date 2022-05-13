#
# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Hash algorithm for measured boot
# SHA-256 (or stronger) is required.
# TODO: The measurement algorithm incorrectly suggests that the TPM backend
#       is used which may not be the case. It is currently being worked on and
#       soon TPM_HASH_ALG will be replaced by a more generic name.
TPM_HASH_ALG			:=	sha256

ifeq (${TPM_HASH_ALG}, sha512)
    MBOOT_ALG_ID		:=	MBOOT_ALG_SHA512
    MBOOT_DIGEST_SIZE		:=	64U
else ifeq (${TPM_HASH_ALG}, sha384)
    MBOOT_ALG_ID		:=	MBOOT_ALG_SHA384
    MBOOT_DIGEST_SIZE		:=	48U
else
    MBOOT_ALG_ID		:=	MBOOT_ALG_SHA256
    MBOOT_DIGEST_SIZE		:=	32U
endif #TPM_HASH_ALG

# Set definitions for Measured Boot driver.
$(eval $(call add_defines,\
    $(sort \
        MBOOT_ALG_ID \
        MBOOT_DIGEST_SIZE \
        MBOOT_RSS_BACKEND \
)))

MEASURED_BOOT_SRC_DIR	:= drivers/measured_boot/rss/

MEASURED_BOOT_SOURCES	+= ${MEASURED_BOOT_SRC_DIR}rss_measured_boot.c
