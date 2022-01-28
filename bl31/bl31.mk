#
# Copyright (c) 2013-2022, ARM Limited and Contributors. All rights reserved.
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
  $(warning "EL3 SPMC is an experimental feature")
  $(info Including EL3 SPMC makefile)
  include services/std_svc/spm/common/spm.mk
  include services/std_svc/spm/el3_spmc/spmc.mk
endif

include lib/psci/psci_lib.mk

BL31_SOURCES		+=	bl31/bl31_main.c				\
				bl31/interrupt_mgmt.c				\
				bl31/aarch64/bl31_entrypoint.S			\
				bl31/aarch64/crash_reporting.S			\
				bl31/aarch64/ea_delegate.S			\
				bl31/aarch64/runtime_exceptions.S		\
				bl31/bl31_context_mgmt.c			\
				common/runtime_svc.c				\
				lib/cpus/aarch64/dsu_helpers.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				services/arm_arch_svc/arm_arch_svc_setup.c	\
				services/std_svc/std_svc_setup.c		\
				${PSCI_LIB_SOURCES}				\
				${SPMD_SOURCES}					\
				${SPM_MM_SOURCES}				\
				${SPMC_SOURCES}					\
				${SPM_SOURCES}

ifeq (${DISABLE_MTPMU},1)
BL31_SOURCES		+=	lib/extensions/mtpmu/aarch64/mtpmu.S
endif

ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_main.c
endif

include lib/debugfs/debugfs.mk
ifeq (${USE_DEBUGFS},1)
	BL31_SOURCES	+= $(DEBUGFS_SRCS)
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	bl31/ehf.c
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

ifeq (${ENABLE_SPE_FOR_LOWER_ELS},1)
BL31_SOURCES		+=	lib/extensions/spe/spe.c
endif

ifeq (${ENABLE_AMU},1)
BL31_SOURCES		+=	${AMU_SOURCES}
endif

ifeq (${ENABLE_MPMM},1)
BL31_SOURCES		+=	${MPMM_SOURCES}
endif

ifeq (${ENABLE_SME_FOR_NS},1)
BL31_SOURCES		+=	lib/extensions/sme/sme.c
BL31_SOURCES		+=	lib/extensions/sve/sve.c
else
ifeq (${ENABLE_SVE_FOR_NS},1)
BL31_SOURCES		+=	lib/extensions/sve/sve.c
endif
endif

ifeq (${ENABLE_MPAM_FOR_LOWER_ELS},1)
BL31_SOURCES		+=	lib/extensions/mpam/mpam.c
endif

ifeq (${ENABLE_TRBE_FOR_NS},1)
BL31_SOURCES		+=	lib/extensions/trbe/trbe.c
endif

ifeq (${ENABLE_BRBE_FOR_NS},1)
BL31_SOURCES		+=	lib/extensions/brbe/brbe.c
endif

ifeq (${ENABLE_SYS_REG_TRACE_FOR_NS},1)
BL31_SOURCES		+=      lib/extensions/sys_reg_trace/aarch64/sys_reg_trace.c
endif

ifeq (${ENABLE_TRF_FOR_NS},1)
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

BL31_LINKERFILE		:=	bl31/bl31.ld.S

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
