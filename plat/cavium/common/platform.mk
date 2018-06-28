#/*
# * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
# * Copyright (c) 2018     , Facebook, Inc.
# *
# * SPDX-License-Identifier: BSD-3-Clause
# */

LIBFDT_DIR		:=	lib/libfdt

PLAT_INCLUDES		:=	-Iplat/cavium/common/csr/include		\
				-Iinclude/common/tbbr				\
				-Iinclude/lib/libfdt 				\
				-Iinclude/lib/stdlib				\
				-Iplat/cavium/common/include

PLAT_BL_COMMON_SOURCES	:=	drivers/arm/pl011/aarch64/pl011_console.S	\
				lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/aarch64/xlat_tables.c		\
				drivers/cavium/thunder_twsi.c			\
				drivers/cavium/thunder_mmc.c			\
				${LIBFDT_DIR}/fdt.c				\
				${LIBFDT_DIR}/fdt_ro.c				\
				${LIBFDT_DIR}/fdt_rw.c				\
				plat/cavium/common/thunder_common.c		\
				plat/cavium/common/thunder_dram.c		\
				plat/cavium/common/thunder_dt.c			\
				plat/cavium/common/thunder_io_storage.c		\
				plat/cavium/common/thunder_console.c

BL31_SOURCES		+=	drivers/arm/gic/common/gic_common.c		\
				drivers/arm/gic/v3/gicv3_main.c			\
				drivers/arm/gic/v3/gicv3_helpers.c		\
				drivers/delay_timer/delay_timer.c		\
				drivers/cavium/thunder_spi.c			\
				drivers/timers/timers.c				\
				lib/cpus/aarch64/thunder.S			\
				plat/cavium/common/aarch64/thunder_helpers.S	\
				plat/cavium/common/bl31_thunder_setup.c		\
				plat/cavium/common/thunder_gicv3.c		\
				plat/cavium/common/thunder_pm.c			\
				plat/cavium/common/thunder_pwrc.c		\
				plat/cavium/common/thunder_timers.c		\
				plat/cavium/common/thunder_topology.c		\
				plat/common/plat_psci_common.c			\
				plat/common/plat_gicv3.c			\
				plat/cavium/common/params_setup.c		\
				plat/cavium/common/thunder_security.c           \
				plat/cavium/common/thunder_gti.c		\
				plat/cavium/common/thunder_svc.c

ENABLE_PLAT_COMPAT	:=	0

CRASH_REPORTING		:=	1

ASM_ASSERTION		:=	1

MULTI_CONSOLE_API	:=	1

ERROR_DEPRECATED	:=	1

CTX_INCLUDE_AARCH32_REGS	:=	0

ifeq (${SECURE_BOOT},1)
    include drivers/auth/mbedtls/mbedtls_common.mk

    ifeq (${ARM_ROTPK_LOCATION}, regs)
        ARM_ROTPK_LOCATION_ID = ARM_ROTPK_REGS_ID
    else
        $(error "Unsupported ARM_ROTPK_LOCATION value '${ARM_ROTPK_LOCATION}'")
    endif

    # KEY_ALG and MBEDTLS_KEY_ALG are used for authentication purposes
    KEY_ALG                := ecdsa
    MBEDTLS_KEY_ALG        := ${KEY_ALG}

    $(eval $(call add_define,ARM_ROTPK_LOCATION_ID))
    PLAT_BL_COMMON_SOURCES += plat/cavium/common/thunder_tbbr_cot.c                \
                              plat/cavium/common/thunder_trusted_boot.c

    IMG_PARSER_LIB_MK := drivers/auth/mbedtls/mbedtls_x509.mk

    $(info Including ${CRYPTO_LIB_MK})
    include ${CRYPTO_LIB_MK}

    $(info Including ${IMG_PARSER_LIB_MK})
    include ${IMG_PARSER_LIB_MK}
endif
