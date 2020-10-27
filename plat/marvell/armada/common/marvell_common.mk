# Copyright (C) 2018 Marvell International Ltd.
#
# SPDX-License-Identifier:     BSD-3-Clause
# https://spdx.org/licenses

MARVELL_PLAT_BASE		:= plat/marvell/armada
MARVELL_PLAT_INCLUDE_BASE	:= include/plat/marvell/armada

include plat/marvell/version.mk

VERSION_STRING			+=(Marvell-${SUBVERSION})

SEPARATE_CODE_AND_RODATA	:= 1

# flag to switch from PLL to ARO
ARO_ENABLE			:= 0
$(eval $(call add_define,ARO_ENABLE))

# Convert LLC to secure SRAM
LLC_SRAM			:= 0
$(eval $(call add_define,LLC_SRAM))

# Enable/Disable LLC
LLC_ENABLE			:= 1
$(eval $(call add_define,LLC_ENABLE))

include lib/xlat_tables_v2/xlat_tables.mk

PLAT_INCLUDES		+=	-I$(MARVELL_PLAT_INCLUDE_BASE)/common	\
				-I$(MARVELL_PLAT_INCLUDE_BASE)/common/aarch64


PLAT_BL_COMMON_SOURCES  += ${XLAT_TABLES_LIB_SRCS} \
				$(MARVELL_PLAT_BASE)/common/aarch64/marvell_common.c	\
				$(MARVELL_PLAT_BASE)/common/aarch64/marvell_helpers.S	\
				$(MARVELL_COMMON_BASE)/marvell_console.c

BL1_SOURCES		+=	drivers/delay_timer/delay_timer.c			\
				drivers/io/io_fip.c					\
				drivers/io/io_memmap.c					\
				drivers/io/io_storage.c					\
				$(MARVELL_PLAT_BASE)/common/marvell_bl1_setup.c		\
				$(MARVELL_PLAT_BASE)/common/marvell_io_storage.c	\
				$(MARVELL_PLAT_BASE)/common/plat_delay_timer.c

ifdef EL3_PAYLOAD_BASE
# Need the arm_program_trusted_mailbox() function to release secondary CPUs from
# their holding pen
endif

BL2_SOURCES		+=	drivers/io/io_fip.c					\
				drivers/io/io_memmap.c					\
				drivers/io/io_storage.c					\
				common/desc_image_load.c				\
				$(MARVELL_PLAT_BASE)/common/marvell_bl2_setup.c		\
				$(MARVELL_PLAT_BASE)/common/marvell_io_storage.c	\
				$(MARVELL_PLAT_BASE)/common/aarch64/marvell_bl2_mem_params_desc.c	\
				$(MARVELL_PLAT_BASE)/common/marvell_image_load.c

ifeq (${SPD},opteed)
PLAT_INCLUDES		+=	-Iinclude/lib
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

BL31_SOURCES		+=	$(MARVELL_PLAT_BASE)/common/marvell_bl31_setup.c	\
				$(MARVELL_PLAT_BASE)/common/marvell_pm.c		\
				$(MARVELL_PLAT_BASE)/common/marvell_topology.c		\
				plat/common/plat_psci_common.c				\
				$(MARVELL_PLAT_BASE)/common/plat_delay_timer.c		\
				drivers/delay_timer/delay_timer.c

# PSCI functionality
$(eval $(call add_define,CONFIG_ARM64))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,bl32_extra1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,bl32_extra2,--tos-fw-extra2))
endif

# MSS (SCP) build
ifeq (${MSS_SUPPORT}, 1)
include $(MARVELL_PLAT_BASE)/common/mss/mss_common.mk
endif
