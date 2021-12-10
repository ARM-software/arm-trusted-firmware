#
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default log level to dump the event log (LOG_LEVEL_INFO)
EVENT_LOG_LEVEL         ?= 40

# TPM hash algorithm.
# SHA-256 (or stronger) is required for all devices that are TPM 2.0 compliant.
TPM_HASH_ALG			:=	sha256

ifeq (${TPM_HASH_ALG}, sha512)
    TPM_ALG_ID			:=	TPM_ALG_SHA512
    TCG_DIGEST_SIZE		:=	64U
else ifeq (${TPM_HASH_ALG}, sha384)
    TPM_ALG_ID			:=	TPM_ALG_SHA384
    TCG_DIGEST_SIZE		:=	48U
else
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
endif #TPM_HASH_ALG

# Set definitions for Measured Boot driver.
$(eval $(call add_defines,\
    $(sort \
        TPM_ALG_ID \
        TCG_DIGEST_SIZE \
        EVENT_LOG_LEVEL \
)))

MEASURED_BOOT_SRC_DIR	:= drivers/measured_boot/event_log/

MEASURED_BOOT_SOURCES	:= ${MEASURED_BOOT_SRC_DIR}event_log.c		\
			   ${MEASURED_BOOT_SRC_DIR}event_print.c

BL2_SOURCES		+= ${MEASURED_BOOT_SOURCES}
BL1_SOURCES             += ${MEASURED_BOOT_SOURCES}
