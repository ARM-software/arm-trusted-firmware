#
# Copyright 2021 NXP
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${TRUSTED_BOARD_BOOT},0)

ifeq (${GENERATE_COT},0)

DDR_FIP_ARGS += --ddr-immem-udimm-1d ${DDR_IMEM_UDIMM_1D}.sb \
		--ddr-immem-udimm-2d ${DDR_IMEM_UDIMM_2D}.sb \
		--ddr-dmmem-udimm-1d ${DDR_DMEM_UDIMM_1D}.sb \
		--ddr-dmmem-udimm-2d ${DDR_DMEM_UDIMM_2D}.sb \
		--ddr-immem-rdimm-1d ${DDR_IMEM_RDIMM_1D}.sb \
		--ddr-immem-rdimm-2d ${DDR_IMEM_RDIMM_2D}.sb \
		--ddr-dmmem-rdimm-1d ${DDR_DMEM_RDIMM_1D}.sb \
		--ddr-dmmem-rdimm-2d ${DDR_DMEM_RDIMM_2D}.sb
endif

UDIMM_DEPS = ${DDR_IMEM_UDIMM_1D}.sb ${DDR_IMEM_UDIMM_2D}.sb ${DDR_DMEM_UDIMM_1D}.sb ${DDR_DMEM_UDIMM_2D}.sb
RDIMM_DEPS = ${DDR_IMEM_RDIMM_1D}.sb ${DDR_IMEM_RDIMM_2D}.sb ${DDR_DMEM_RDIMM_1D}.sb ${DDR_DMEM_RDIMM_2D}.sb
DDR_FIP_DEPS += ${UDIMM_DEPS}
DDR_FIP_DEPS += ${RDIMM_DEPS}

# Max Size of CSF header (CSF_HDR_SZ = 0x3000).
# Image will be appended at this offset of the header.
# Path to CST directory is required to generate the CSF header,
# and prepend it to image before fip image gets generated
ifeq (${CST_DIR},)
  $(error Error: CST_DIR not set)
endif

ifeq (${DDR_INPUT_FILE},)
DDR_INPUT_FILE:= drivers/nxp/auth/csf_hdr_parser/${CSF_FILE}
endif

%.sb: %
	@echo " Generating CSF Header for $@ $<"
	$(CST_DIR)/create_hdr_esbc --in $< --out $@ --app_off ${CSF_HDR_SZ} \
					--app $< ${DDR_INPUT_FILE}

endif
