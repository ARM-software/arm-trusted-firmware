#
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Default log level to dump the event log (LOG_LEVEL_INFO)
EVENT_LOG_LEVEL         ?= 40

# TPM hash algorithm
TPM_HASH_ALG			:=	sha256

ifeq (${TPM_HASH_ALG}, sha512)
    MBEDTLS_MD_ID		:=	MBEDTLS_MD_SHA512
    TPM_ALG_ID			:=	TPM_ALG_SHA512
    TCG_DIGEST_SIZE		:=	64U
else ifeq (${TPM_HASH_ALG}, sha384)
    MBEDTLS_MD_ID		:=	MBEDTLS_MD_SHA384
    TPM_ALG_ID			:=	TPM_ALG_SHA384
    TCG_DIGEST_SIZE		:=	48U
else
    MBEDTLS_MD_ID		:=	MBEDTLS_MD_SHA256
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
endif

# Event Log length in bytes
EVENT_LOG_SIZE			:= 1024

# Set definitions for mbed TLS library and Measured Boot driver
$(eval $(call add_defines,\
    $(sort \
        MBEDTLS_MD_ID \
        TPM_ALG_ID \
        TCG_DIGEST_SIZE \
        EVENT_LOG_SIZE \
        EVENT_LOG_LEVEL \
)))

ifeq (${HASH_ALG}, sha256)
ifneq (${TPM_HASH_ALG}, sha256)
$(eval $(call add_define,MBEDTLS_SHA512_C))
endif
endif

MEASURED_BOOT_SRC_DIR	:= drivers/measured_boot/

MEASURED_BOOT_SOURCES	:= ${MEASURED_BOOT_SRC_DIR}measured_boot.c	\
    			   ${MEASURED_BOOT_SRC_DIR}event_log.c		\
    			   ${MEASURED_BOOT_SRC_DIR}event_print.c

BL2_SOURCES		+= ${MEASURED_BOOT_SOURCES}
