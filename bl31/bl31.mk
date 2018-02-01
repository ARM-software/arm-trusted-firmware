#
# Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

################################################################################
# Include SPM Makefile
################################################################################
ifeq (${ENABLE_SPM},1)
$(info Including SPM makefile)
include services/std_svc/spm/spm.mk
endif


include lib/psci/psci_lib.mk

BL31_SOURCES		+=	bl31/bl31_main.c				\
				bl31/interrupt_mgmt.c				\
				bl31/aarch64/bl31_entrypoint.S			\
				bl31/aarch64/runtime_exceptions.S		\
				bl31/aarch64/crash_reporting.S			\
				bl31/bl31_context_mgmt.c			\
				common/runtime_svc.c				\
				plat/common/aarch64/platform_mp_stack.S		\
				services/arm_arch_svc/arm_arch_svc_setup.c	\
				services/std_svc/std_svc_setup.c		\
				${PSCI_LIB_SOURCES}				\
				${SPM_SOURCES}					\


ifeq (${ENABLE_PMF}, 1)
BL31_SOURCES		+=	lib/pmf/pmf_main.c
endif

ifeq (${EL3_EXCEPTION_HANDLING},1)
BL31_SOURCES		+=	bl31/ehf.c
endif

ifeq (${SDEI_SUPPORT},1)
ifeq (${EL3_EXCEPTION_HANDLING},0)
  $(error EL3_EXCEPTION_HANDLING must be 1 for SDEI support)
endif
BL31_SOURCES		+=	services/std_svc/sdei/sdei_event.c	\
				services/std_svc/sdei/sdei_intr_mgmt.c	\
				services/std_svc/sdei/sdei_main.c	\
				services/std_svc/sdei/sdei_state.c
endif

ifeq (${ENABLE_SPE_FOR_LOWER_ELS},1)
BL31_SOURCES		+=	lib/extensions/spe/spe.c
endif

ifeq (${ENABLE_AMU},1)
BL31_SOURCES		+=	lib/extensions/amu/aarch64/amu.c		\
				lib/extensions/amu/aarch64/amu_helpers.S
endif

ifeq (${ENABLE_SVE_FOR_NS},1)
BL31_SOURCES		+=	lib/extensions/sve/sve.c
endif

ifeq (${WORKAROUND_CVE_2017_5715},1)
BL31_SOURCES		+=	lib/cpus/aarch64/workaround_cve_2017_5715_mmu.S		\
				lib/cpus/aarch64/workaround_cve_2017_5715_bpiall.S
endif

BL31_LINKERFILE		:=	bl31/bl31.ld.S

# Flag used to indicate if Crash reporting via console should be included
# in BL31. This defaults to being present in DEBUG builds only
ifndef CRASH_REPORTING
CRASH_REPORTING		:=	$(DEBUG)
endif

$(eval $(call assert_boolean,CRASH_REPORTING))
$(eval $(call assert_boolean,EL3_EXCEPTION_HANDLING))
$(eval $(call assert_boolean,SDEI_SUPPORT))

$(eval $(call add_define,CRASH_REPORTING))
$(eval $(call add_define,EL3_EXCEPTION_HANDLING))
$(eval $(call add_define,SDEI_SUPPORT))
