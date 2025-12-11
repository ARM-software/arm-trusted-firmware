#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

LIBTPM2_PATH	?= contrib/libtpm
LIBTPM2_NAME		:= tpm2

LIBTPM2_BUILD_DIR	:= $(BUILD_PLAT)/lib$(LIBTPM2_NAME)
LIBTPM2_TARGET		:= $(LIB_DIR)/$(LIBTPM2_NAME).a

LDLIBS		:= -l$(LIBTPM2_NAME) $(LDLIBS)
INCLUDES	+= -I${LIBTPM2_PATH}/include

ifeq ($(DEBUG),1)
LIBTPM2_BUILD_TYPE	:= Debug
else
LIBTPM2_BUILD_TYPE	:= Release
endif

# TPM Hash algorithm, used during Measured Boot
# currently only accepts SHA-256
ifeq (${MBOOT_TPM_HASH_ALG}, sha256)
    TPM_ALG_ID			:=	TPM_ALG_SHA256
    TCG_DIGEST_SIZE		:=	32U
else
    $(error "The selected MBOOT_TPM_HASH_ALG is invalid.")
endif #MBOOT_TPM_HASH_ALG

$(eval $(call add_defines,\
    $(sort \
        TCG_DIGEST_SIZE \
)))

ifeq (${TPM_INTERFACE}, FIFO_SPI)
    $(eval $(call add_define,TPM_INTERFACE_FIFO_SPI))
else
    $(error "The selected TPM_INTERFACE is invalid.")
endif #TPM_INTERFACE

LIBTPM2_CFLAGS ?= $(foreach f,$(TF_CFLAGS),	\
  $(if $(findstring -I,$(f)),			\
    -I$(realpath $(patsubst -I%,%,$(f))),	\
    $(f)))

$(LIBTPM2_TARGET): $(LIB_DIR)/libc.a
	$(s)echo "  CM      $@"
	$(q)cmake -S $(LIBTPM2_PATH) -B $(LIBTPM2_BUILD_DIR) \
		-DTPM_INTERFACE=$(TPM_INTERFACE) \
		-DDEBUG_BACKEND_HEADER="$(abspath $(LIBTPM2_PATH)/include/log_backend_tf.h") \
		-DLOG_LEVEL=$(LOG_LEVEL) \
		-DCMAKE_BUILD_TYPE=$(LIBTPM2_BUILD_TYPE) \
		-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY="$(abspath $(BUILD_PLAT)/lib)" \
		-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
		-DCMAKE_C_COMPILER="$($(ARCH)-cc)" \
		-DCMAKE_C_FLAGS=$(call escape-shell,$(LIBTPM2_CFLAGS)) \
		$(if $(V),, --log-level=ERROR) > /dev/null
	$(q)cmake --build $(LIBTPM2_BUILD_DIR) -- $(if $(V),,-s) > /dev/null

libraries: $(LIBTPM2_TARGET)

