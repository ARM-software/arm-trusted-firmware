#
# Copyright (c) 2020-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

EVENT_LOG_SRC_DIR	:= drivers/measured_boot/event_log/

# Default log level to dump the event log (LOG_LEVEL_INFO)
EVENT_LOG_LEVEL         ?= 40

# When using a TPM, adopt the TPM's hash algorithm for
# measurements through the Event Log mechanism, ensuring
# the TPM uses the same algorithm for measurements and
# extends the PCR accordingly, allowing for comparison
# between PCR value and Event Log measurements required
# for attestation.
ifdef MBOOT_TPM_HASH_ALG
    MBOOT_EL_HASH_ALG		:=	${MBOOT_TPM_HASH_ALG}
else
    MBOOT_EL_HASH_ALG		:=	sha256
endif

# Measured Boot hash algorithm.
# SHA-256 (or stronger) is required for all devices that are TPM 2.0 compliant.
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

INCLUDES		+= -Iinclude/drivers/measured_boot/event_log \
				-Iinclude/drivers/auth

EVENT_LOG_SOURCES	:= ${EVENT_LOG_SRC_DIR}event_log.c		\
			   ${EVENT_LOG_SRC_DIR}event_print.c


ifeq (${TRANSFER_LIST}, 1)
EVENT_LOG_SOURCES	+= ${EVENT_LOG_SRC_DIR}/event_handoff.c
endif
