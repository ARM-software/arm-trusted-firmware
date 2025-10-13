#
# Copyright (c) 2020-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBEVLOG_PATH		?= contrib/libeventlog
LIBEVLOG_NAME		:= eventlog

LIBEVLOG_BUILD_DIR	:= $(BUILD_PLAT)/lib$(LIBEVLOG_NAME)
LIBEVLOG_INSTALL_DIR	:= $(BUILD_PLAT)/$(LIBEVLOG_NAME)-install
LIBEVLOG_TARGET		:= $(LIBEVLOG_INSTALL_DIR)/lib/lib$(LIBEVLOG_NAME).a

# We currently support up to SHA512, use that as a sane default that platforms
# may override.
LIBEVLOG_MAX_DIGEST_SIZE	?=	64U
LIBEVLOG_MAX_HASH_COUNT		?=	3U

ifeq ($(DEBUG),1)
LIBEVLOG_BUILD_TYPE	:= Debug
else
LIBEVLOG_BUILD_TYPE	:= Release
endif

LIBEVLOG_LIBS		:= $(LIBEVLOG_TARGET)
LIBEVLOG_INCLUDE_DIRS	:= $(LIBEVLOG_INSTALL_DIR)/include

LIBEVLOG_DIRS_TO_CHECK	+= $(LIBEVLOG_INSTALL_DIR)/include

# When using a TPM, adopt the TPM's hash algorithm for
# measurements through the Event Log mechanism, ensuring
# the TPM uses the same algorithm for measurements and
# extends the PCR accordingly, allowing for comparison
# between PCR value and Event Log measurements required
# for attestation.
ifdef MBOOT_TPM_HASH_ALG
    MBOOT_EL_HASH_ALG		:=	${MBOOT_TPM_HASH_ALG}
endif

# Legacy support only. Do NOT set by default.
# Valid only if explicitly provided by old build environments.
ifdef MBOOT_EL_HASH_ALG
$(warning MBOOT_EL_HASH_ALG is supported solely for backward compatibility. \
Please configure the hash algorithm at runtime instead.)
ifeq ($(MBOOT_EL_HASH_ALG),sha256)
    TPM_ALG_ID					:= TPM_ALG_SHA256
else ifeq ($(MBOOT_EL_HASH_ALG),sha384)
    TPM_ALG_ID					:= TPM_ALG_SHA384
else ifeq ($(MBOOT_EL_HASH_ALG),sha512)
    TPM_ALG_ID					:= TPM_ALG_SHA512
else
    $(error Unsupported legacy MBOOT_EL_HASH_ALG '$(MBOOT_EL_HASH_ALG)'. Expected: sha256, sha384, sha512)
endif

LIBEVLOG_MAX_HASH_COUNT	:= 1U
$(eval $(call add_define,TPM_ALG_ID))
endif # MBOOT_EL_HASH_ALG

LIBEVLOG_CFLAGS ?= $(filter-out -I%,$(TF_CFLAGS))
LIBEVLOG_CFLAGS += $(patsubst %,-I%,$(call include-dirs,$(TF_CFLAGS)))

$(LIBEVLOG_INSTALL_DIR)/% $(LIBEVLOG_INSTALL_DIR)/%/: $(LIBEVLOG_TARGET) ;
$(LIBEVLOG_TARGET) $(LIBEVLOG_INSTALL_DIR)/ &: $(LIB_DIR)/libc.a
	$(s)echo "  CM      $@"
	$(q)cmake -S $(LIBEVLOG_PATH) -B $(LIBEVLOG_BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(LIBEVLOG_BUILD_TYPE) \
		-DCMAKE_SYSTEM_NAME=Generic \
		-DCMAKE_SYSTEM_VERSION= \
		-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
		-DCMAKE_C_COMPILER=$(call shell-quote,$(call shell-join,$($(ARCH)-cc-program),;)) \
		-DCMAKE_C_COMPILER_LAUNCHER=$(call shell-quote,$(call shell-join,$($(ARCH)-cc-wrapper),;)) \
		-DCMAKE_C_FLAGS=$(call escape-shell,$(LIBEVLOG_CFLAGS)) \
		-DDEBUG_BACKEND_HEADER="log_backend_tf.h" \
		-DMAX_DIGEST_SIZE=${LIBEVLOG_MAX_DIGEST_SIZE} \
		-DMAX_HASH_COUNT=${LIBEVLOG_MAX_HASH_COUNT} \
		$(if $(V),, --log-level=ERROR) > /dev/null
	$(q)cmake --build $(LIBEVLOG_BUILD_DIR) -- $(if $(V),,-s) > /dev/null
	$(q)cmake --install $(LIBEVLOG_BUILD_DIR) \
		--prefix $(LIBEVLOG_INSTALL_DIR) \
		--config $(LIBEVLOG_BUILD_TYPE) > /dev/null
