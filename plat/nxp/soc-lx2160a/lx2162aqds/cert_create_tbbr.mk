#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Compile time defines used by NXP platforms

PLAT_DEF_OID := yes

ifeq (${PLAT_DEF_OID},yes)

CRTTOOL_DEFINES += PLAT_DEF_OID
CRTTOOL_DEFINES += PDEF_KEYS
CRTTOOL_DEFINES += PDEF_CERTS
CRTTOOL_DEFINES += PDEF_EXTS


CRTTOOL_INCLUDE_DIRS		+=	${PLAT_DIR}/../common/fip_handler/common/

PDEF_CERT_TOOL_PATH		:=	${PLAT_DIR}/cert_create_helper
CRTTOOL_INCLUDE_DIRS		+=	${PDEF_CERT_TOOL_PATH}/include

PLAT_OBJECTS			+=	${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_cert.c \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_ext.c \
					${PDEF_CERT_TOOL_PATH}/src/pdef_tbb_key.c

$(shell rm ${PLAT_OBJECTS})

CRTTOOL_SOURCES			+= ${PLAT_OBJECTS}
endif
