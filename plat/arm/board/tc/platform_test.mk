# Copyright (c) 2022-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

$(eval $(call add_define,PLATFORM_TESTS))

ifeq (${PLATFORM_TEST},rse-nv-counters)
    include drivers/arm/rse/rse_comms.mk

    # Test code.
    BL31_SOURCES	+=	plat/arm/board/tc/nv_counter_test.c \
				plat/arm/board/tc/tc_rse_comms.c

    # Code under testing.
    BL31_SOURCES	+=	lib/psa/rse_platform.c \
				${RSE_COMMS_SOURCES}

    PLAT_INCLUDES	+=	-Iinclude/lib/psa

    $(eval $(call add_define,PLATFORM_TEST_NV_COUNTERS))
else ifeq (${PLATFORM_TEST},tfm-testsuite)
    include drivers/arm/rse/rse_comms.mk
    include drivers/measured_boot/rse/qcbor.mk

    # The variables need to be set to compile the platform test:
    ifeq (${TF_M_TESTS_PATH},)
        # Example: ../rse/tf-m-tests
        $(error Error: TF_M_TESTS_PATH not set)
    endif
    ifeq (${TF_M_EXTRAS_PATH},)
        # Example: ../rse/tf-m-extras
        $(error Error: TF_M_EXTRAS_PATH not set)
    endif
    ifeq (${MEASUREMENT_VALUE_SIZE},)
        MEASUREMENT_VALUE_SIZE	:=	32
    endif
    ifeq (${MEASURED_BOOT_HASH_ALG},)
        MEASURED_BOOT_HASH_ALG	:=	"PSA_ALG_SHA_256"
    endif

    DELEGATED_ATTEST_TESTS_PATH	=	$(TF_M_EXTRAS_PATH)/partitions/delegated_attestation/test
    MEASURED_BOOT_TESTS_PATH	=	$(TF_M_EXTRAS_PATH)/partitions/measured_boot/test

    MBEDTLS_CONFIG_FILE		=	"<plat_tc_mbedtls_config.h>"

    LIBMBEDTLS_SRCS		+=	$(addprefix ${MBEDTLS_DIR}/library/,	\
					entropy.c				\
					entropy_poll.c				\
					hmac_drbg.c				\
					psa_crypto.c				\
					psa_crypto_client.c			\
					psa_crypto_driver_wrappers_no_static.c	\
					psa_crypto_hash.c			\
					psa_crypto_rsa.c			\
					psa_crypto_ecp.c			\
					psa_crypto_slot_management.c		\
					psa_util.c				\
					)

    BL31_SOURCES	+=	${RSE_COMMS_SOURCES}				\
				plat/arm/common/arm_dyn_cfg.c			\
				${TC_BASE}/rse_ap_tests.c			\
				${TC_BASE}/rse_ap_testsuites.c			\
				${TC_BASE}/rse_ap_test_stubs.c			\
				${TC_BASE}/tc_rse_comms.c			\
				$(TF_M_TESTS_PATH)/tests_reg/test/framework/test_framework.c \
				$(MEASURED_BOOT_TESTS_PATH)/measured_boot_common.c \
				$(MEASURED_BOOT_TESTS_PATH)/measured_boot_tests_common.c \
				$(DELEGATED_ATTEST_TESTS_PATH)/delegated_attest_test.c \
				drivers/auth/mbedtls/mbedtls_common.c		\
				lib/psa/measured_boot.c				\
				lib/psa/delegated_attestation.c			\
				${QCBOR_SOURCES}

    PLAT_INCLUDES	+=	-I$(TF_M_EXTRAS_PATH)/partitions/measured_boot/interface/include \
				-I$(TF_M_EXTRAS_PATH)/partitions/delegated_attestation/interface/include \
				-I$(TF_M_TESTS_PATH)/tests_reg/test/framework	\
				-I$(TF_M_TESTS_PATH)/tests_reg/test/secure_fw/suites/extra \
				-I$(TF_M_TESTS_PATH)/lib/log			\
				-I$(MEASURED_BOOT_TESTS_PATH)/non_secure	\
				-I$(DELEGATED_ATTEST_TESTS_PATH)		\
				-I$(DELEGATED_ATTEST_TESTS_PATH)/non_secure	\
				-Iplat/arm/board/tc				\
				-Iinclude/drivers/auth/mbedtls			\
				-Iinclude/drivers/arm				\
				-Iinclude/lib/psa				\
				-I${QCBOR_INCLUDES}

    # Some of the PSA functions are declared in multiple header files, that
    # triggers this warning.
    TF_CFLAGS		+=	-Wno-error=redundant-decls

    # TODO: Created patch for warning in tf-m-tests
    TF_CFLAGS		+=	-Wno-error=return-type

    # Define macros that are used by the code coming from the tf-m-extras repo.
    $(eval $(call add_define,MEASUREMENT_VALUE_SIZE))
    $(eval $(call add_define,MEASURED_BOOT_HASH_ALG))
    $(eval $(call add_define,DELEG_ATTEST_DUMP_TOKEN_AND_KEY))

    $(eval $(call add_define,PLATFORM_TEST_TFM_TESTSUITE))
else
    $(error "Unsupported PLATFORM_TEST value")
endif
