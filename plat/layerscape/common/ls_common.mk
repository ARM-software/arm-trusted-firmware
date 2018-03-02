#
# Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#


# For the original power-state parameter format, the State-ID can be encoded
# according to the recommended encoding or zero. This flag determines which
# State-ID encoding to be parsed.
ARM_RECOM_STATE_ID_ENC := 0

# If the PSCI_EXTENDED_STATE_ID is set, then ARM_RECOM_STATE_ID_ENC need to
# be set. Else throw a build error.
ifeq (${PSCI_EXTENDED_STATE_ID}, 1)
  ifeq (${ARM_RECOM_STATE_ID_ENC}, 0)
    $(error Build option ARM_RECOM_STATE_ID_ENC needs to be set if \
            PSCI_EXTENDED_STATE_ID is set for ARM platforms)
  endif
endif

# Process ARM_RECOM_STATE_ID_ENC flag
$(eval $(call assert_boolean,ARM_RECOM_STATE_ID_ENC))
$(eval $(call add_define,ARM_RECOM_STATE_ID_ENC))

# Process LS1043_DISABLE_TRUSTED_WDOG flag
# TODO:Temparally disabled it on development phase, not implemented yet
LS1043_DISABLE_TRUSTED_WDOG	:=	1

# Process ARM_PLAT_MT flag
ARM_PLAT_MT			:=	0

# Use translation tables library v2 by default
ARM_XLAT_TABLES_LIB_V1		:=	0

# Enable PSCI_STAT_COUNT/RESIDENCY APIs on ARM platforms
#ENABLE_PSCI_STAT		:=	1
ENABLE_PMF			:=	1

# On ARM platforms, separate the code and read-only data sections to allow
# mapping the former as executable and the latter as execute-never.
SEPARATE_CODE_AND_RODATA	:=	1

# Enable new version of image loading on ARM platforms
LOAD_IMAGE_V2			:=	1

# Use generic OID definition (tbbr_oid.h)
USE_TBBR_DEFS			:=	1


PLAT_INCLUDES		+=	-Iinclude/common/tbbr

PLAT_BL_COMMON_SOURCES	+=	plat/layerscape/common/${ARCH}/ls_helpers.S		\
				plat/layerscape/common/ls_common.c

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
PLAT_BL_COMMON_SOURCES	+=	lib/xlat_tables/xlat_tables_common.c		\
				lib/xlat_tables/${ARCH}/xlat_tables.c
else
include lib/xlat_tables_v2/xlat_tables.mk

PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}
endif

BL1_SOURCES		+=			\
				drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_bl1_setup.c			\
				plat/layerscape/common/ls_io_storage.c

BL2_SOURCES		+=	drivers/io/io_fip.c				\
				drivers/io/io_memmap.c				\
				drivers/io/io_storage.c				\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_bl2_setup.c			\
				plat/layerscape/common/ls_io_storage.c
ifeq (${LOAD_IMAGE_V2},1)
BL2_SOURCES		+=	plat/layerscape/common/${ARCH}/ls_bl2_mem_params_desc.c
BL2_SOURCES		+=	plat/layerscape/common/ls_image_load.c		\
					common/desc_image_load.c
endif


BL31_SOURCES		+=	plat/layerscape/common/ls_bl31_setup.c		\
				plat/layerscape/common/ls_timer.c			\
				plat/layerscape/common/ls_topology.c			\
				plat/layerscape/common/ns_access.c		\
				plat/common/plat_psci_common.c
