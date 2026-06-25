#
# Copyright 2026 Free Mobile - Vincent Jardin
#
# SPDX-License-Identifier: BSD-3-Clause
#

# board-specific build parameters
# Nodebox v3 CPU Module (COM-HPC Server, LX2160A)

BOOT_MODE	?=	flexspi_nor
BOARD		?=	nbxv3

# TA_PROG_SFP is strapped to ground on this board, so software cannot
# raise POVDD on demand. Secure-boot fuse provisioning needs an
# external fuse-burn jig. See board_enable_povdd() in platform.c.
POVDD_ENABLE	:=	no
NXP_COINED_BB	:=	no

# DDR Compilation Configs
# 2x DDR controllers, 1 SODIMM each, timings from SPD over I2C.
NUM_OF_DDRC	:=	2
DDRC_NUM_DIMM	:=	1
DDRC_NUM_CS	:=	4
DDR_ECC_EN	:=	yes
DDR_ADDR_DEC	:=	yes
APPLY_MAX_CDD	:=	yes

# DDR Errata
ERRATA_DDR_A011396	:= 1
ERRATA_DDR_A050450	:= 1

# Cortex-A72 erratum 1319367 (AT-instruction speculation): applies to
# every Cortex-A72 revision; cost is ~10 extra insns + 2 ISBs per EL3
# entry. Avoids hard-to-debug PA mistranslations on PSCI / SMCCC paths.
ERRATA_A72_1319367	:= 1

# 256 MB FlexSPI NOR (GigaDevice GD55LB02GF, 1.8 V, JEDEC 0xC8 0x60 0x1C)
# on XSPI1 Flash-A CS0. Supports 4 KB / 32 KB / 64 KB erase only --
# no 128 KB. Matching flash_info.h block keys off CONFIG_GD55LB02GF.
FLASH_TYPE	:=	GD55LB02GF
XSPI_FLASH_SZ	:=	0x10000000

# nbxv3 pairs two same-MPN DIMMs from different production lots whose
# SPDs differ only in lot-stamped bytes (encoding revision, mfg info)
# but match on every timing-relevant byte. Opt in to the tolerant SPD
# compare; the -D is emitted by drivers/nxp/ddr/nxp-ddr/ddr.mk when
# this is 'yes' -- we only set the make variable here.
NXP_DDR_DUAL_DIMM_TOLERANT	:= yes

# fspi.c JEDEC RDID / SFDP / SR1 probe helpers and the xspi_nor_diag
# module are gated by NXP_XSPI_DIAG; flash_bootstrap uses
# fspi_set_serclk_max_hz() and bring-up diag uses xspi_nor_diag.
# The -D is emitted by drivers/nxp/flexspi/nor/flexspi_nor.mk when
# this is 1, we only set the make variable here.
NXP_XSPI_DIAG	:= 1
# 64 KiB matches F_SECTOR_64K (the chip's largest erase block) so
# carve-outs at the top of the BL2 region stay erase-aligned.
NXP_XSPI_NOR_UNIT_SIZE		:=	0x10000
BL2_BIN_XSPI_NOR_END_ADDRESS	:=	0x100000
# Keep at 0: 64 KB erase wins on every production workload
# (provisioning, env save, FIT update); 4 KB only helps the rare
# NV-counter rewrite path.
FSPI_ERASE_4K	:= 0

# Platform specific features.
WARM_BOOT	:=	no

# Adding Platform files build files
BL2_SOURCES	+=	${BOARD_PATH}/ddr_init.c\
			${BOARD_PATH}/platform.c

# Unified BL2 source tree, two linker layouts. The C/asm source is
# the same for both: nbxv3_runtime_mode() classifies into PROD/
# FLASH_BOOTSTRAP/FLASH_NO_HOST/DDR_DEBUG at boot based on
# RCWSR0.MEM_PLL_RAT and DCFG_SCRATCHRW2 magic.
# It implies XIP settings.
#
#   NBXV3_BL2_BOOTSTRAP=0 (default): XIP BL2. .text/.rodata in NOR
#       AHB window at 0x20009000, .data/.bss/.stack in OCRAM. SP's shall
#       PBI releases cpu.0 directly to the NOR AHB address; no Block
#       Copy needed.
#
#   NBXV3_BL2_BOOTSTRAP=1: OCRAM-only BL2. Everything in OCRAM at
#       BL2_BASE=0x1800D000.
NBXV3_BL2_BOOTSTRAP ?= 0
$(eval $(call add_define_val,NBXV3_BL2_BOOTSTRAP,${NBXV3_BL2_BOOTSTRAP}))

ifeq (${NBXV3_BL2_BOOTSTRAP},0)
BL2_IN_XIP_MEM	:=	1
$(eval $(call add_define,BL2_IN_XIP_MEM))
endif

PLAT_INCLUDES	+=	-Iinclude/drivers/nxp/io

SUPPORTED_BOOT_MODE	:=	flexspi_nor	\
				semihosting

# Adding platform board build info
include plat/nxp/common/plat_make_helper/plat_common_def.mk

# nbxv3-specific BL31 verbose External-Abort / SError trap.
BL31_SOURCES	+=	${BOARD_PATH}/nbxv3_ea_handler.c

# nbxv3 override of plat_soc_early_init_hook() (SMMU bypass).
BL2_SOURCES	+=	${BOARD_PATH}/plat_soc_hooks.c

# JSON manifest parser.
BL2_SOURCES	+=	${BOARD_PATH}/json_tokens.c		\
			${BOARD_PATH}/manifest_parse.c

# Manifest runner + SHA-256 verifier + UART progress bar.
BL2_SOURCES	+=	${BOARD_PATH}/manifest_runner.c	\
			${BOARD_PATH}/progress_bar.c		\
			${BOARD_PATH}/sha256.c

# Runtime mode classifier (PROD / FLASH_BOOTSTRAP / FLASH_NO_HOST).
BL2_SOURCES	+=	${BOARD_PATH}/nbxv3_runtime_mode.c

# Bootstrap orchestrator + io_xspi_nor backend.
BL2_SOURCES	+=	${BOARD_PATH}/flash_bootstrap.c	\
			drivers/nxp/io/io_xspi_nor.c

$(eval $(call SET_NXP_MAKE_FLAG,XSPI_NEEDED,BL2))
BL2_SOURCES	+=	drivers/io/io_semihosting.c			\
			lib/semihosting/semihosting.c			\
			lib/semihosting/${ARCH}/semihosting_call.S


# Adding SoC build info
include plat/nxp/soc-lx2160a/soc.mk
