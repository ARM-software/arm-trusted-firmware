# Copyright (c) 2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq (${PLATFORM_TEST},1)

    # The variables need to be set to compile the platform test:
    ifeq (${TF_M_TESTS_PATH},)
        # Example: ../rss/tf-m-tests
        $(error Error: TF_M_TESTS_PATH not set)
    endif
    ifeq (${TF_M_EXTRAS_PATH},)
        # Example: ../rss/tf-m-extras
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

    LIBMBEDTLS_SRCS		+= 	$(addprefix ${MBEDTLS_DIR}/library/,	\
					entropy.c				\
					entropy_poll.c				\
					hmac_drbg.c				\
					psa_crypto.c				\
					psa_crypto_client.c			\
					psa_crypto_driver_wrappers.c		\
					psa_crypto_hash.c			\
					psa_crypto_rsa.c			\
					psa_crypto_ecp.c			\
					psa_crypto_slot_management.c		\
					)

    BL31_SOURCES	+=	${RSS_COMMS_SOURCES} 				\
				plat/arm/common/arm_dyn_cfg.c 			\
				${TC_BASE}/rss_ap_tests.c 			\
				${TC_BASE}/rss_ap_testsuites.c 			\
				${TC_BASE}/rss_ap_test_stubs.c			\
				$(TF_M_TESTS_PATH)/test/framework/test_framework.c \
				$(MEASURED_BOOT_TESTS_PATH)/measured_boot_common.c \
				$(MEASURED_BOOT_TESTS_PATH)/measured_boot_tests_common.c \
				$(DELEGATED_ATTEST_TESTS_PATH)/delegated_attest_test.c \
				drivers/auth/mbedtls/mbedtls_common.c 		\
				lib/psa/measured_boot.c 			\
				lib/psa/delegated_attestation.c

    PLAT_INCLUDES	+=	-I$(TF_M_EXTRAS_PATH)/partitions/measured_boot/interface/include \
				-I$(TF_M_EXTRAS_PATH)/partitions/delegated_attestation/interface/include \
				-I$(TF_M_TESTS_PATH)/test/framework 		\
				-I$(TF_M_TESTS_PATH)/log 			\
				-I$(TF_M_TESTS_PATH)/test/secure_fw/suites/extra \
				-I$(MEASURED_BOOT_TESTS_PATH)/non_secure 	\
				-I$(DELEGATED_ATTEST_TESTS_PATH) 		\
				-I$(DELEGATED_ATTEST_TESTS_PATH)/non_secure \
				-Iplat/arm/board/tc 				\
				-Iinclude/drivers/auth/mbedtls 			\
				-Iinclude/drivers/arm

    # Some of the PSA functions are declared in multiple header files, that
    # triggers this warning.
    TF_CFLAGS		+=	-Wno-error=redundant-decls

    # TODO: Created patch for warning in tf-m-tests
    TF_CFLAGS		+=	-Wno-error=return-type

    # Define macros that are used by the code coming from the tf-m-extras repo.
    $(eval $(call add_define,MEASUREMENT_VALUE_SIZE))
    $(eval $(call add_define,MEASURED_BOOT_HASH_ALG))
    $(eval $(call add_define,DELEG_ATTEST_DUMP_TOKEN_AND_KEY))

    $(eval $(call add_define,PLATFORM_TEST))
endif
