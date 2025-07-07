#
# Copyright (c) 2023-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

PLAT_DEF_OID := 1

ifeq (${PLAT_DEF_OID},1)
  ifeq (${ETHOSN_NPU_DRIVER},1)
    CRTTOOL_DEFINES += PLAT_DEF_OID
    CRTTOOL_DEFINES += PDEF_CERTS
    CRTTOOL_DEFINES += PDEF_EXTS
    CRTTOOL_DEFINES += PDEF_KEYS

    CRTTOOL_INCLUDE_DIRS		+=	${PLAT_DIR}/certificate/include \
						../../include/drivers/arm

    CRTTOOL_SOURCES			+=	${PLAT_DIR}certificate/src/juno_tbb_cert.c \
						${PLAT_DIR}certificate/src/juno_tbb_ext.c \
						${PLAT_DIR}certificate/src/juno_tbb_key.c
  endif
endif
