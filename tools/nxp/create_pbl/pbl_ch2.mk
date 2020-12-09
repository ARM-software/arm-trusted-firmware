#
# Copyright 2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

CREATE_PBL	?=	${CREATE_PBL_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${CREATE_PBL_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

.PHONY: pbl
pbl:	${BUILD_PLAT}/bl2.bin
ifeq ($(SECURE_BOOT),yes)
pbl: ${BUILD_PLAT}/bl2.bin
ifeq ($(RCW),"")
	${Q}echo "Platform ${PLAT} requires rcw file. Please set RCW to point to the right RCW file for boot mode ${BOOT_MODE}"
else
	# Generate header for bl2.bin
	$(Q)$(CST_DIR)/create_hdr_isbc --in ${BUILD_PLAT}/bl2.bin --out ${BUILD_PLAT}/hdr_bl2 ${BL2_INPUT_FILE}
	# Compile create_pbl tool
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${CREATE_PBL_TOOL_PATH};\
	# Add bl2.bin to RCW
	${CREATE_PBL} -r ${RCW} -i ${BUILD_PLAT}/bl2.bin -b ${BOOT_MODE} -c ${SOC_NUM} -d ${BL2_BASE} -e ${BL2_BASE}\
			-o ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl ;\
	# Add header to RCW
	${CREATE_PBL} -r ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl -i ${BUILD_PLAT}/hdr_bl2 -b ${BOOT_MODE} -c ${SOC_NUM} \
			-d ${BL2_HDR_LOC} -e ${BL2_HDR_LOC} -o ${BUILD_PLAT}/bl2_${BOOT_MODE}_sec.pbl -s;\
	rm ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl
# Swapping of RCW is required for QSPi Chassis 2 devices
ifeq (${BOOT_MODE}, qspi)
ifeq ($(SWAP),1)
	${Q}echo "Byteswapping RCW for QSPI"
	${BYTE_SWAP} ${BUILD_PLAT}/bl2_${BOOT_MODE}_sec.pbl;
endif # SWAP
endif # BOOT_MODE
	cd ${CREATE_PBL_TOOL_PATH}; ${MAKE} clean ; cd -;
endif
else  # NON SECURE_BOOT
ifeq ($(RCW),"")
	${Q}echo "Platform ${PLAT} requires rcw file. Please set RCW to point to the right RCW file for boot mode ${BOOT_MODE}"
else
	# -a option appends the image for Chassis 3 devices in case of non secure boot
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${CREATE_PBL_TOOL_PATH};
	${CREATE_PBL} -r ${RCW} -i ${BUILD_PLAT}/bl2.bin -b ${BOOT_MODE} -c ${SOC_NUM} -d ${BL2_BASE} -e ${BL2_BASE} \
	-o ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl ;
# Swapping of RCW is required for QSPi Chassis 2 devices
ifeq (${BOOT_MODE}, qspi)
ifeq ($(SWAP),1)
	${Q}echo "Byteswapping RCW for QSPI"
	${BYTE_SWAP} ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl;
endif # SWAP
endif # BOOT_MODE
	cd ${CREATE_PBL_TOOL_PATH}; ${MAKE} clean ; cd -;
endif
endif # SECURE_BOOT



