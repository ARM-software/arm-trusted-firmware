#
# Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include lib/libfdt/libfdt.mk
include lib/xlat_tables_v2/xlat_tables.mk

include drivers/arm/gic/v2/gicv2.mk

ifeq (${TRANSFER_LIST}, 1)
include lib/transfer_list/transfer_list.mk
endif

ifeq (${MEASURED_BOOT},1)
ifeq (${DISCRETE_TPM},1)
TPM2_MK := drivers/tpm/tpm2.mk
$(info Including ${TPM2_MK})
include ${TPM2_MK}
RPI4_PROVISION_TPM	?=	0
$(eval $(call assert_boolean,RPI4_PROVISION_TPM))
$(eval $(call add_define_val,RPI_TPM_PROVISION,$(RPI4_PROVISION_TPM)))
endif

MEASURED_BOOT_MK := drivers/measured_boot/event_log/event_log.mk
$(info Including ${MEASURED_BOOT_MK})
include ${MEASURED_BOOT_MK}

BL1_LIBS += $(LIBEVLOG_LIBS)
BL1_INCLUDE_DIRS += $(LIBEVLOG_INCLUDE_DIRS)

BL2_LIBS += $(LIBEVLOG_LIBS)
BL2_INCLUDE_DIRS += $(LIBEVLOG_INCLUDE_DIRS)

CRYPTO_SOURCES		:=	drivers/auth/crypto_mod.c

BL1_SOURCES		+=	${CRYPTO_SOURCES}
BL2_SOURCES		+=	${CRYPTO_SOURCES}

include drivers/auth/mbedtls/mbedtls_crypto.mk
endif

PLAT_INCLUDES		:=	-Iplat/rpi/common/include		\
				-Iplat/rpi/rpi4/include

PLAT_BL_COMMON_SOURCES	:=	drivers/ti/uart/aarch64/16550_console.S	\
				drivers/arm/pl011/aarch64/pl011_console.S \
				plat/rpi/common/rpi3_common.c		\
				plat/rpi/common/rpi3_console_dual.c	\
				${XLAT_TABLES_LIB_SRCS}

ifeq ($(filter command line environment override,$(origin RESET_TO_BL31)),)
RESET_TO_BL31		:=	1
endif

ifeq (${RESET_TO_BL31}, 1)

PLAT_EXTRA_LD_SCRIPT	:=	1

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a72.S		\
				plat/rpi/common/aarch64/plat_helpers.S	\
				plat/rpi/common/aarch64/armstub8_header.S \
				drivers/delay_timer/delay_timer.c	\
				drivers/gpio/gpio.c			\
				drivers/rpi3/gpio/rpi3_gpio.c		\
				plat/common/plat_gicv2.c                \
				plat/rpi/common/rpi4_bl31_setup.c	\
				plat/rpi/rpi4/rpi4_setup.c		\
				plat/rpi/common/rpi3_pm.c		\
				plat/common/plat_psci_common.c		\
				plat/rpi/common/rpi3_topology.c		\
				common/fdt_fixup.c			\
				common/fdt_wrappers.c			\
				${LIBFDT_SRCS}				\
				${GICV2_SOURCES}

# Add new default target when compiling this platform
all: bl31

else

PLAT_EXTRA_LD_SCRIPT	:=	0
NEED_BL33		:=	no
# Keep these values in sync with PLAT_RPI3_FIP_BASE and
# PLAT_RPI3_FIP_MAX_SIZE in platform_def.h.
RPI4_ARMSTUB_FIP_OFFSET	:=	0x00020000
RPI4_FIP_MAX_SIZE	:=	0x001e0000

BL1_SOURCES		+=	drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				drivers/gpio/gpio.c			\
				drivers/rpi3/gpio/rpi3_gpio.c		\
				lib/cpus/aarch64/cortex_a72.S		\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi/common/aarch64/plat_helpers.S	\
				plat/rpi/common/rpi3_io_storage.c	\
				plat/rpi/rpi4/rpi4_staged_bl1_setup.c

ifeq (${TRANSFER_LIST},1)
BL1_SOURCES		+=	plat/rpi/common/rpi_transfer_list.c
endif

ifeq (${MEASURED_BOOT},1)
BL1_SOURCES		+=	common/measured_boot_helpers.c		\
				plat/rpi/rpi4/rpi4_bl1_mboot.c		\
				plat/rpi/common/rpi3_common_mboot.c
endif

BL2_SOURCES		+=	common/desc_image_load.c		\
				drivers/io/io_fip.c			\
				drivers/io/io_memmap.c			\
				drivers/io/io_storage.c			\
				drivers/delay_timer/delay_timer.c	\
				drivers/delay_timer/generic_delay_timer.c \
				drivers/gpio/gpio.c			\
				drivers/rpi3/gpio/rpi3_gpio.c		\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/rpi/common/aarch64/plat_helpers.S	\
				plat/rpi/common/rpi3_image_load.c	\
				plat/rpi/common/rpi3_io_storage.c	\
				plat/rpi/rpi4/aarch64/rpi4_bl2_mem_params_desc.c \
				plat/rpi/rpi4/rpi4_staged_bl2_setup.c

ifeq (${TRANSFER_LIST},1)
BL2_SOURCES		+=	plat/rpi/common/rpi_transfer_list.c
endif

ifeq (${MEASURED_BOOT},1)
BL2_SOURCES		+=	common/measured_boot_helpers.c		\
				plat/rpi/rpi4/rpi4_bl2_mboot.c		\
				plat/rpi/common/rpi3_common_mboot.c
endif

BL31_SOURCES		+=	lib/cpus/aarch64/cortex_a72.S		\
				plat/rpi/common/aarch64/plat_helpers.S	\
				drivers/delay_timer/delay_timer.c	\
				drivers/gpio/gpio.c			\
				drivers/rpi3/gpio/rpi3_gpio.c		\
				plat/common/plat_gicv2.c		\
				plat/rpi/common/rpi3_pm.c		\
				plat/common/plat_psci_common.c		\
				plat/rpi/common/rpi3_topology.c		\
				plat/rpi/rpi4/rpi4_staged_bl31_setup.c	\
				plat/rpi/rpi4/rpi4_setup.c		\
				common/fdt_fixup.c			\
				common/fdt_wrappers.c			\
				${LIBFDT_SRCS}				\
				${GICV2_SOURCES}

RPI4_ARMSTUB_HEADER_BIN	:=	${BUILD_PLAT}/armstub8_header.bin
RPI4_BL1_PAD_BIN	:=	${BUILD_PLAT}/bl1_pad.bin
RPI4_ARMSTUB8_BIN	:=	${BUILD_PLAT}/armstub8.bin

all: armstub8

${BUILD_PLAT}/armstub8_header.o: plat/rpi/common/aarch64/armstub8_header.S \
				$(config-header) | $$(@D)/
	$(s)echo "  AS      $<"
	$(q)$($(ARCH)-cc) -x assembler-with-cpp $(TF_CFLAGS) $(ASFLAGS) $(DEFINES) -c $< -o $@

${RPI4_ARMSTUB_HEADER_BIN}: ${BUILD_PLAT}/armstub8_header.o | $$(@D)/
	$(s)echo "  BIN     $@"
	$(q)$($(ARCH)-oc) -O binary $< $@
	$(q)test $$(stat -c%s $@) -eq 4096

armstub8-header: ${RPI4_ARMSTUB_HEADER_BIN}

armstub8: armstub8-header bl1 fip
	$(s)echo "  CAT     ${RPI4_ARMSTUB8_BIN}"
	$(q)size=$$(stat -c%s ${BUILD_PLAT}/fip.bin); \
		max_size=$$(( ${RPI4_FIP_MAX_SIZE} )); \
		[ $$size -le $$max_size ] || { \
			echo "FIP too large ($$size > $$max_size/${RPI4_FIP_MAX_SIZE})"; \
			exit 1; \
		}
	$(q)cat ${RPI4_ARMSTUB_HEADER_BIN} ${BUILD_PLAT}/bl1.bin > ${RPI4_BL1_PAD_BIN}
	$(q)size=$$(stat -c%s ${RPI4_BL1_PAD_BIN}); \
		fip_offset=$$(( ${RPI4_ARMSTUB_FIP_OFFSET} )); \
		[ $$size -le $$fip_offset ] || { \
			echo "BL1 armstub prefix too large ($$size > $$fip_offset/${RPI4_ARMSTUB_FIP_OFFSET})"; \
			exit 1; \
		}
	$(q)truncate --size=$$(( ${RPI4_ARMSTUB_FIP_OFFSET} )) ${RPI4_BL1_PAD_BIN}
	$(q)cat ${RPI4_BL1_PAD_BIN} ${BUILD_PLAT}/fip.bin > ${RPI4_ARMSTUB8_BIN}
ifdef PRELOADED_BL33_BASE
	$(q)size=$$(stat -c%s ${RPI4_ARMSTUB8_BIN}); \
		bl33_base=$$(( ${PRELOADED_BL33_BASE} )); \
		[ $$size -le $$bl33_base ] || { \
			echo "armstub8.bin overlaps BL33 ($$size > $$bl33_base/${PRELOADED_BL33_BASE})"; \
			exit 1; \
		}
endif
	$(s)echo
	$(s)echo "Built ${RPI4_ARMSTUB8_BIN} successfully"
	$(s)echo

endif

# All CPUs enter armstub8.bin.
COLD_BOOT_SINGLE_CPU	:=	0

# Tune compiler for Cortex-A72
ifeq ($($(ARCH)-cc-id),arm-clang)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a72
else ifneq ($(filter %-clang,$($(ARCH)-cc-id)),)
    TF_CFLAGS_aarch64	+=	-mcpu=cortex-a72
else
    TF_CFLAGS_aarch64	+=	-mtune=cortex-a72
endif

# Enable all errata workarounds for Cortex-A72
ERRATA_A72_859971		:= 1

WORKAROUND_CVE_2017_5715	:= 1

# Build config flags
# ------------------

# Disable stack protector by default
ENABLE_STACK_PROTECTOR	 	:= 0

# Have different sections for code and rodata
SEPARATE_CODE_AND_RODATA	:= 1

# Use Coherent memory
USE_COHERENT_MEM		:= 1

# Platform build flags
# --------------------

# There is not much else than a Linux kernel to load at the moment.
RPI3_DIRECT_LINUX_BOOT		:= 1

# BL33 images are in AArch64 by default
RPI3_BL33_IN_AARCH32		:= 0

# UART to use at runtime. -1 means the runtime UART is disabled.
# Any other value means the default UART will be used.
RPI3_RUNTIME_UART		:= 0

# Use normal memory mapping for ROM, FIP, SRAM and DRAM
RPI3_USE_UEFI_MAP		:= 0

# SMCCC PCI support (should be enabled for ACPI builds)
SMC_PCI_SUPPORT            	:= 0

# Process platform flags
# ----------------------

$(eval $(call add_define,RPI3_BL33_IN_AARCH32))
$(eval $(call add_define,RPI3_DIRECT_LINUX_BOOT))
ifdef RPI3_PRELOADED_DTB_BASE
$(eval $(call add_define,RPI3_PRELOADED_DTB_BASE))
endif
$(eval $(call add_define,RPI3_RUNTIME_UART))
$(eval $(call add_define,RPI3_USE_UEFI_MAP))
ifeq (${SPD},spmd)
ifeq (${SPMC_OPTEE},1)
$(eval $(call add_define,SPMC_OPTEE))
endif
endif

ifeq (${ARCH},aarch32)
  $(error Error: AArch32 not supported on rpi4)
endif

ifeq (${MEASURED_BOOT},1)
ifneq (${RESET_TO_BL31},0)
  $(error Error: rpi4 measured boot requires RESET_TO_BL31=0)
endif
ifneq (${TRANSFER_LIST},1)
  $(error Error: rpi4 measured boot requires TRANSFER_LIST=1)
endif
ifneq (${RPI3_DIRECT_LINUX_BOOT},0)
  $(error Error: rpi4 measured boot requires RPI3_DIRECT_LINUX_BOOT=0)
endif
endif

ifeq (${SPD},spmd)
ifneq (${SPMD_SPM_AT_SEL2},0)
  $(error Error: rpi4 SPMD requires SPMD_SPM_AT_SEL2=0)
endif
ifneq (${SPMC_OPTEE},1)
  $(error Error: rpi4 SPMD requires SPMC_OPTEE=1)
endif
ifneq (${SPMC_AT_EL3},0)
  $(error Error: rpi4 SPMD does not support SPMC_AT_EL3=1)
endif
ifneq (${RESET_TO_BL31},0)
  $(error Error: rpi4 SPMD requires RESET_TO_BL31=0)
endif
ifneq (${TRANSFER_LIST},1)
  $(error Error: rpi4 SPMD requires TRANSFER_LIST=1)
endif
endif

ifneq ($(ENABLE_STACK_PROTECTOR), 0)
PLAT_BL_COMMON_SOURCES	+=	drivers/rpi3/rng/rpi3_rng.c		\
				plat/rpi/common/rpi3_stack_protector.c
endif

ifeq (${MEASURED_BOOT},1)
ifeq (${DISCRETE_TPM},1)
BL1_SOURCES		+=	$(TPM2_SOURCES)			\
				plat/rpi/common/rpi_tpm.c
BL2_SOURCES		+=	$(TPM2_SOURCES)			\
				plat/rpi/common/rpi_tpm.c
ifeq (${TPM_INTERFACE},FIFO_SPI)
BL1_SOURCES		+=	drivers/gpio/gpio_spi.c		\
				drivers/tpm/tpm2_slb9670/slb9670_gpio.c
BL2_SOURCES		+=	drivers/gpio/gpio_spi.c		\
				drivers/tpm/tpm2_slb9670/slb9670_gpio.c
endif
endif
endif

ifeq (${SPD},spmd)
ifeq (${SPMC_OPTEE},1)
BL2_SOURCES		+=	lib/optee/optee_utils.c		\
				plat/rpi/common/rpi_optee.c
endif
endif

ifeq (${SPD},spmd)
BL31_SOURCES		+=	plat/common/plat_spmd_manifest.c

ifeq (${ARM_SPMC_MANIFEST_DTS},)
ARM_SPMC_MANIFEST_DTS	:=	plat/rpi/rpi4/fdts/rpi4_spmc_el1_optee_manifest.dts
endif

FDT_SOURCES		+=	${ARM_SPMC_MANIFEST_DTS}
RPI4_TOS_FW_CONFIG	:=	${BUILD_PLAT}/fdts/$(notdir $(basename ${ARM_SPMC_MANIFEST_DTS})).dtb

$(eval $(call TOOL_ADD_PAYLOAD,${RPI4_TOS_FW_CONFIG},--tos-fw-config,${RPI4_TOS_FW_CONFIG}))

ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif
endif

ifeq ($(SMC_PCI_SUPPORT), 1)
BL31_SOURCES            +=      plat/rpi/common/rpi_pci_svc.c
endif
