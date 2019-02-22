#
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

CREATE_PBL	?=	${PLAT_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${PLAT_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

.PHONY: pbl
pbl:	${BUILD_PLAT}/bl2.bin
ifeq ($(RCW),"")
	${Q}echo "Platform ${PLAT} requires rcw file. Please set RCW to point to the right RCW file for boot mode ${BOOT_MODE}"
else
	# -a option appends the image for Chassis 3 devices in case of non secure boot
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${PLAT_TOOL_PATH};
	${CREATE_PBL} -r ${RCW} -i ${BUILD_PLAT}/bl2.bin -b ${BOOT_MODE} -c ${SOC_NUM} -d ${BL2_BASE} -e ${BL2_BASE} \
	-o ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl ;
# Swapping of RCW is required for QSPi Chassis 2 devices
ifeq (${BOOT_MODE}, qspi)
ifeq ($(SWAP),1)
	${Q}echo "Byteswapping RCW for QSPI"
	${BYTE_SWAP} ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl;
endif # SWAP
endif # BOOT_MODE
	cd ${PLAT_TOOL_PATH}; ${MAKE} clean ; cd -;
endif
