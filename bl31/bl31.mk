#
# Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

################################################################################
# Include Makefile for the SPM-MM implementation
################################################################################
ifeq (${SUPPORT_UNKNOWN_MPID},1)
  ifeq (${DEBUG},0)
    $(warning WARNING: SUPPORT_UNKNOWN_MPID enabled)
  endif
endif

ifeq (${SPM_MM},1)
  ifeq (${EL3_EXCEPTION_HANDLING},0)
    $(error EL3_EXCEPTION_HANDLING must be 1 for SPM-MM support)
  else
    $(info Including SPM Management Mode (MM) makefile)
    include services/std_svc/spm/common/spm.mk
    include services/std_svc/spm/spm_mm/spm_mm.mk
  endif
endif

include lib/extensions/amu/amu.mk
include lib/mpmm/mpmm.mk

ifeq (${SPMC_AT_EL3},1)
  $(info Including EL3 SPMC makefile)
  include services/std_svc/spm/common/spm.mk
  include services/std_svc/spm/el3_spmc/spmc.mk
endif

include lib/psci/psci_lib.mk

BL31_SOURCES		+=	bl31/bl31_main.c				\
				bl31/interrupt_mgmt.c				\
				bl31/aarch64/bl31_entrypoint.S			\
				bl31/aarch64/crash_reporting.S			\
				bl31/aarch64/runtime_exceptions.S		\
				bl31/bl31_context_mgmt.c			\
				bl31/bl31_traps.c				\
				common/runtime_svc.c				\
				lib/cpus/errata_common.c			\
				lib/cpus/aarch64/dsu_helpers.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				services/arm_arch_svc/arm_arch_svc_setup.c	\
				services/std_svc/std_svc_setup.c		\
				lib/el3_runtime/simd_ctx.c			\
				${PSCI_LIB_SOURCES}				\
				${SPMD_SOURCES}					\
				${SPM_MM_SOURCES}				\
				${SPMC_SOURCES}					\
				${SPM_SOURCES}

VENDOR_EL3_SRCS		+=	services/el3/ven_el3_svc.c

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_main.c				\
				${VENDOR_EL3_SRCS}
endif

include lib/debugfs/debugfs.mk
ifeq (${USE_DEBUGFS},1)
BL31_SOURCES		+=	${DEBUGFS_SRCS}					\
				${VENDOR_EL3_SRCS}
endif

ifeq (${PLATFORM_REPORT_CTX_MEM_USE},1)
BL31_SOURCES		+=	lib/el3_runtime/aarch64/context_debug.c
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	bl31/ehf.c
endif

ifeq (${FFH_SUPPORT},1)
BL31_SOURCES		+=	bl31/aarch64/ea_delegate.S
endif

ifeq (${SDEI_SUPPORT},1)
ifeq (${EL3_EXCEPTION_HANDLING},0)
  $(error EL3_EXCEPTION_HANDLING must be 1 for SDEI support)
endif
BL31_SOURCES		+=	services/std_svc/sdei/sdei_dispatch.S	\
				services/std_svc/sdei/sdei_event.c	\
				services/std_svc/sdei/sdei_intr_mgmt.c	\
				services/std_svc/sdei/sdei_main.c	\
				services/std_svc/sdei/sdei_state.c
endif

ifeq (${TRNG_SUPPORT},1)
BL31_SOURCES		+=	services/std_svc/trng/trng_main.c	\
				services/std_svc/trng/trng_entropy_pool.c
endif

ifneq (${ENABLE_SPE_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/spe/spe.c
endif

ifeq (${ERRATA_ABI_SUPPORT},1)
BL31_SOURCES		+=	services/std_svc/errata_abi/errata_abi_main.c
endif

ifneq (${ENABLE_FEAT_AMU},0)
BL31_SOURCES		+=	${AMU_SOURCES}
endif

ifneq (${ENABLE_FEAT_FGT2},0)
BL31_SOURCES		+=	lib/extensions/fgt/fgt2.c
endif

ifneq (${ENABLE_FEAT_TCR2},0)
BL31_SOURCES		+=	lib/extensions/tcr/tcr2.c
endif

ifeq (${ENABLE_MPMM},1)
BL31_SOURCES		+=	${MPMM_SOURCES}
endif

ifneq (${ENABLE_SME_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/sme/sme.c
endif
ifneq (${ENABLE_SVE_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/sve/sve.c
endif

ifneq (${ENABLE_FEAT_MPAM},0)
BL31_SOURCES		+=	lib/extensions/mpam/mpam.c
endif

ifneq (${ENABLE_FEAT_DEBUGV8P9},0)
BL31_SOURCES		+=	lib/extensions/debug/debugv8p9.c
endif

ifneq (${ENABLE_TRBE_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/trbe/trbe.c
endif

ifneq (${ENABLE_BRBE_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/brbe/brbe.c
endif

ifneq (${ENABLE_SYS_REG_TRACE_FOR_NS},0)
BL31_SOURCES		+=      lib/extensions/sys_reg_trace/aarch64/sys_reg_trace.c
endif

ifneq (${ENABLE_TRF_FOR_NS},0)
BL31_SOURCES		+=	lib/extensions/trf/aarch64/trf.c
endif

ifeq (${WORKAROUND_CVE_2017_5715},1)
BL31_SOURCES		+=	lib/cpus/aarch64/wa_cve_2017_5715_bpiall.S	\
				lib/cpus/aarch64/wa_cve_2017_5715_mmu.S
endif

ifeq ($(SMC_PCI_SUPPORT),1)
BL31_SOURCES		+=	services/std_svc/pci_svc.c
endif

ifeq (${ENABLE_RME},1)
include lib/gpt_rme/gpt_rme.mk

BL31_SOURCES		+=	${GPT_LIB_SRCS}					\
				${RMMD_SOURCES}
endif

ifeq ($(FEATURE_DETECTION),1)
BL31_SOURCES		+=	common/feat_detect.c
endif

ifeq (${DRTM_SUPPORT},1)
BL31_SOURCES		+=	services/std_svc/drtm/drtm_main.c		\
				services/std_svc/drtm/drtm_dma_prot.c		\
				services/std_svc/drtm/drtm_res_address_map.c	\
				services/std_svc/drtm/drtm_measurements.c	\
				services/std_svc/drtm/drtm_remediation.c	\
				${MBEDTLS_SOURCES}
endif

ifeq ($(CROS_WIDEVINE_SMC),1)
BL31_SOURCES		+=	services/oem/chromeos/widevine_smc_handlers.c
endif

BL31_DEFAULT_LINKER_SCRIPT_SOURCE := bl31/bl31.ld.S

ifeq ($($(ARCH)-ld-id),gnu-gcc)
        BL31_LDFLAGS	+=	-Wl,--sort-section=alignment
else ifneq ($(filter llvm-lld gnu-ld,$($(ARCH)-ld-id)),)
        BL31_LDFLAGS	+=	--sort-section=alignment
endif

# Flag used to indicate if Crash reporting via console should be included
# in BL31. This defaults to being present in DEBUG builds only
ifndef CRASH_REPORTING
CRASH_REPORTING		:=	$(DEBUG)
endif

$(eval $(call assert_booleans,\
    $(sort \
	CRASH_REPORTING \
	EL3_EXCEPTION_HANDLING \
	SDEI_SUPPORT \
)))

$(eval $(call add_defines,\
    $(sort \
        CRASH_REPORTING \
        EL3_EXCEPTION_HANDLING \
        SDEI_SUPPORT \
)))
