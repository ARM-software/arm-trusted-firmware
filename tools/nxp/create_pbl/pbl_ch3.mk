#
# Copyright 2018-2020 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#
#
SHELL=/bin/bash

CREATE_PBL	?=	${CREATE_PBL_TOOL_PATH}/create_pbl${BIN_EXT}
BYTE_SWAP	?=	${CREATE_PBL_TOOL_PATH}/byte_swap${BIN_EXT}

HOST_GCC	:= gcc

BL2_SRC_OFFSET ?= 0x9000
BL2_HDR_SRC_OFFSET ?= 0x5000
bl2_hdr_loc=$(shell echo $$(( $(BL2_HDR_SRC_OFFSET) / 1024 )))
bl2_loc=$(shell echo $$(( $(BL2_SRC_OFFSET) / 1024 )))

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

	# Add Block Copy command for bl2.bin to RCW
	${CREATE_PBL} -r ${RCW} -i ${BUILD_PLAT}/bl2.bin -b ${BOOT_MODE} -c ${SOC_NUM} -d ${BL2_BASE} -e ${BL2_BASE}\
			-o ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl -f ${BL2_SRC_OFFSET};\

	# Add Block Copy command and Load CSF header command to RCW
	${CREATE_PBL} -r ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl -i ${BUILD_PLAT}/hdr_bl2 -b ${BOOT_MODE} -c ${SOC_NUM} \
			-d ${BL2_HDR_LOC} -e ${BL2_HDR_LOC} -s -f ${BL2_HDR_SRC_OFFSET}	\
			-o ${BUILD_PLAT}/rcw_sec.pbl

	# Sign and add "Load Security Header command to PBI commands
	$(Q)$(CST_DIR)/create_hdr_pbi --out ${BUILD_PLAT}/bl2_${BOOT_MODE}_sec.pbl --in ${BUILD_PLAT}/rcw_sec.pbl ${PBI_INPUT_FILE}

	# Append the bl2_hdr to the RCW image
	@echo "${bl2_hdr_loc}"
	dd if=${BUILD_PLAT}/hdr_bl2 of=${BUILD_PLAT}/bl2_${BOOT_MODE}_sec.pbl bs=1K seek=${bl2_hdr_loc}

	# Append the bl2.bin to the RCW image
	@echo "${bl2_loc}"
	dd if=${BUILD_PLAT}/bl2.bin of=${BUILD_PLAT}/bl2_${BOOT_MODE}_sec.pbl bs=1K seek=${bl2_loc}

	rm ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl
	cd ${CREATE_PBL_TOOL_PATH}; ${MAKE} clean ; cd -;
endif
else  #SECURE_BOOT
ifeq ($(RCW),"")
	${Q}echo "Platform ${PLAT} requires rcw file. Please set RCW to point to the right RCW file for boot mode ${BOOT_MODE}"
else
	${Q}${MAKE} CPPFLAGS="-DVERSION='\"${VERSION_STRING}\"'" --no-print-directory -C ${CREATE_PBL_TOOL_PATH};

	# Add Block Copy command and populate boot loc ptrfor bl2.bin to RCW
	${CREATE_PBL} -r ${RCW} -i ${BUILD_PLAT}/bl2.bin -b ${BOOT_MODE} -c ${SOC_NUM} -d ${BL2_BASE} -e ${BL2_BASE} \
	-o ${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl -f ${BL2_SRC_OFFSET};

	# Append the bl2.bin to the RCW image
	@echo "bl2_loc is ${bl2_offset}"
	dd if=${BUILD_PLAT}/bl2.bin of=${BUILD_PLAT}/bl2_${BOOT_MODE}.pbl bs=1K seek=${bl2_loc}

	cd ${CREATE_PBL_TOOL_PATH}; ${MAKE} clean ; cd -;
endif
endif # SECURE_BOOT
