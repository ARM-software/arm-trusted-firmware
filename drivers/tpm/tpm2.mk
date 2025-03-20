#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TPM2_SRC_DIR	:= drivers/tpm/

TPM2_SOURCES	:= ${TPM2_SRC_DIR}tpm2_cmds.c \
                   ${TPM2_SRC_DIR}tpm2_chip.c

# TPM Hash algorithm, used during Measured Boot
# currently only accepts SHA-256
ifeq (${MBOOT_TPM_HASH_ALG}, sha256)
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
else
    $(error "The selected MBOOT_TPM_HASH_ALG is invalid.")
endif #MBOOT_TPM_HASH_ALG

ifeq (${TPM_INTERFACE}, FIFO_SPI)
    TPM2_SOURCES += ${TPM2_SRC_DIR}tpm2_fifo.c \
                    ${TPM2_SRC_DIR}tpm2_fifo_spi.c
else
    $(error "The selected TPM_INTERFACE is invalid.")
endif #TPM_INTERFACE
