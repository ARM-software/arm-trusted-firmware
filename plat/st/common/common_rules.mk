#
# Copyright (c) 2023, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Compilation rules
.PHONY: check_dtc_version stm32image clean_stm32image check_boot_device
.SUFFIXES:

all: check_dtc_version stm32image ${STM32_TF_STM32}

distclean realclean clean: clean_stm32image

bl2: check_boot_device

check_boot_device:
	@if [ ${STM32MP_EMMC} != 1 ] && \
	    [ ${STM32MP_SDMMC} != 1 ] && \
	    [ ${STM32MP_RAW_NAND} != 1 ] && \
	    [ ${STM32MP_SPI_NAND} != 1 ] && \
	    [ ${STM32MP_SPI_NOR} != 1 ] && \
	    [ ${STM32MP_UART_PROGRAMMER} != 1 ] && \
	    [ ${STM32MP_USB_PROGRAMMER} != 1 ]; then \
		echo "No boot device driver is enabled"; \
		false; \
	fi

stm32image: ${STM32IMAGE}

${STM32IMAGE}: ${STM32IMAGE_SRC}
	${Q}${MAKE} CPPFLAGS="" --no-print-directory -C ${STM32IMAGEPATH}

clean_stm32image:
	${Q}${MAKE} --no-print-directory -C ${STM32IMAGEPATH} clean

check_dtc_version:
	@if [ ${DTC_VERSION} -lt 10407 ]; then \
		echo "dtc version too old (${DTC_V}), you need at least version 1.4.7"; \
		false; \
	fi

# Create DTB file for BL2
${BUILD_PLAT}/fdts/%-bl2.dts: fdts/%.dts fdts/${BL2_DTSI} | ${BUILD_PLAT} fdt_dirs
	@echo '#include "$(patsubst fdts/%,%,$<)"' > $@
	@echo '#include "${BL2_DTSI}"' >> $@

${BUILD_PLAT}/fdts/%-bl2.dtb: ${BUILD_PLAT}/fdts/%-bl2.dts

${BUILD_PLAT}/$(PLAT)-%.o: ${BUILD_PLAT}/fdts/%-bl2.dtb $(STM32_BINARY_MAPPING) bl2
	@echo "  AS      $${PLAT}.S"
	${Q}${AS} ${ASFLAGS} ${TF_CFLAGS} \
		-DDTB_BIN_PATH=\"$<\" \
		-c $(word 2,$^) -o $@

$(eval $(call MAKE_LD,${STM32_TF_LINKERFILE},$(STM32_LD_FILE),bl2))

tf-a-%.elf: $(PLAT)-%.o ${STM32_TF_LINKERFILE}
	@echo "  LDS     $<"
ifneq ($(findstring gcc,$(notdir $(LD))),)
	${Q}${LD} -o $@ $(subst --,-Wl$(comma)--,${STM32_TF_ELF_LDFLAGS}) -nostartfiles -Wl,-Map=$(@:.elf=.map) -Wl,-dT ${STM32_TF_LINKERFILE} $<
else
	${Q}${LD} -o $@ ${STM32_TF_ELF_LDFLAGS} -Map=$(@:.elf=.map) --script ${STM32_TF_LINKERFILE} $<
endif

tf-a-%.bin: tf-a-%.elf
	${Q}${OC} -O binary $< $@
	@echo
	@echo "Built $@ successfully"
	@echo

tf-a-%.stm32: tf-a-%.bin ${STM32_DEPS}
	@echo
	@echo "Generate $@"
	$(eval LOADADDR = $(shell cat $(@:.stm32=.map) | grep '^RAM' | awk '{print $$2}'))
	$(eval ENTRY = $(shell cat $(@:.stm32=.map) | grep "__BL2_IMAGE_START" | awk '{print $$1}'))
	${Q}${STM32IMAGE} -s $< -d $@ \
		-l $(LOADADDR) -e ${ENTRY} \
		-v ${STM32_TF_VERSION} \
		-m ${STM32_HEADER_VERSION_MAJOR} \
		-n ${STM32_HEADER_VERSION_MINOR} \
		-b ${STM32_HEADER_BL2_BINARY_TYPE}
	@echo
