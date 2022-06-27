#
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default log level to dump the event log (LOG_LEVEL_INFO)
EVENT_LOG_LEVEL         ?= 40

# Measured Boot hash algorithm.
# SHA-256 (or stronger) is required for all devices that are TPM 2.0 compliant.
ifdef TPM_HASH_ALG
    $(warning "TPM_HASH_ALG is deprecated. Please use MBOOT_EL_HASH_ALG instead.")
    MBOOT_EL_HASH_ALG		:=	${TPM_HASH_ALG}
else
    MBOOT_EL_HASH_ALG		:=	sha256
endif

ifeq (${MBOOT_EL_HASH_ALG}, sha512)
    TPM_ALG_ID			:=	TPM_ALG_SHA512
    TCG_DIGEST_SIZE		:=	64U
else ifeq (${MBOOT_EL_HASH_ALG}, sha384)
    TPM_ALG_ID			:=	TPM_ALG_SHA384
    TCG_DIGEST_SIZE		:=	48U
else
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
endif #MBOOT_EL_HASH_ALG

# Set definitions for Measured Boot driver.
$(eval $(call add_defines,\
    $(sort \
        TPM_ALG_ID \
        TCG_DIGEST_SIZE \
        EVENT_LOG_LEVEL \
)))

EVENT_LOG_SRC_DIR	:= drivers/measured_boot/event_log/

EVENT_LOG_SOURCES	:= ${EVENT_LOG_SRC_DIR}event_log.c		\
			   ${EVENT_LOG_SRC_DIR}event_print.c
