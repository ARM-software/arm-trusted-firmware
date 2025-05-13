#
# Copyright (c) 2020-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBEVLOG_PATH		?= contrib/libeventlog
LIBEVLOG_NAME		:= eventlog

LIBEVLOG_BUILD_DIR	:= $(BUILD_PLAT)/lib$(LIBEVLOG_NAME)
LIBEVLOG_TARGET		:= $(LIB_DIR)/$(LIBEVLOG_NAME).a

ifeq ($(DEBUG),1)
LIBEVLOG_BUILD_TYPE	:= Debug
else
LIBEVLOG_BUILD_TYPE	:= Release
endif

LDLIBS		:= -l$(LIBEVLOG_NAME) $(LDLIBS)
INCLUDES	+= -I$(LIBEVLOG_PATH)/include

LIBEVLOG_DIRS_TO_CHECK	+= $(LIBEVLOG_PATH)/include

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
    CRYPTO_MD_ID		:=	CRYPTO_MD_SHA512
else ifeq (${MBOOT_EL_HASH_ALG}, sha384)
    TPM_ALG_ID			:=	TPM_ALG_SHA384
    TCG_DIGEST_SIZE		:=	48U
    CRYPTO_MD_ID		:=	CRYPTO_MD_SHA384
else
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
    CRYPTO_MD_ID		:=	CRYPTO_MD_SHA256
endif #MBOOT_EL_HASH_ALG

# Set definitions for Measured Boot driver.
$(eval $(call add_defines,\
    $(sort \
        TPM_ALG_ID \
        TCG_DIGEST_SIZE \
        EVENT_LOG_LEVEL \
        CRYPTO_MD_ID \
)))

LIBEVLOG_CFLAGS ?= $(filter-out -I%,$(TF_CFLAGS))
LIBEVLOG_CFLAGS += $(patsubst %,-I%,$(call include-dirs,$(TF_CFLAGS)))

$(LIBEVLOG_TARGET): $(LIB_DIR)/libc.a
	$(s)echo "  CM      $@"
	$(q)cmake -S $(LIBEVLOG_PATH) -B $(LIBEVLOG_BUILD_DIR) \
		-DHASH_ALGORITHM=$(call uppercase,$(MBOOT_EL_HASH_ALG)) \
		-DCMAKE_BUILD_TYPE=$(LIBEVLOG_BUILD_TYPE) \
		-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$(abspath $(BUILD_PLAT)/lib)" \
		-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
		-DCMAKE_C_COMPILER="$($(ARCH)-cc)" \
		-DCMAKE_C_FLAGS=$(call escape-shell,$(LIBEVLOG_CFLAGS)) \
		-DDEBUG_BACKEND_HEADER="log_backend_tf.h" \
		$(if $(V),, --log-level=ERROR) > /dev/null
	$(q)cmake --build $(LIBEVLOG_BUILD_DIR) -- $(if $(V),,-s) > /dev/null

libraries: $(LIBEVLOG_TARGET)
