#
# Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq (${ARCH}, aarch32)
	$(error SP_MIN is only supported on AArch32 platforms)
endif

include lib/extensions/amu/amu.mk
include lib/psci/psci_lib.mk

INCLUDES		+=	-Iinclude/bl32/sp_min

BL32_SOURCES		+=	bl32/sp_min/sp_min_main.c			\
				bl32/sp_min/aarch32/entrypoint.S		\
				common/runtime_svc.c				\
				plat/common/aarch32/plat_sp_min_common.c	\
				services/arm_arch_svc/arm_arch_svc_setup.c	\
				services/std_svc/std_svc_setup.c		\
				${PSCI_LIB_SOURCES}

ifeq (${ENABLE_PMF}, 1)
BL32_SOURCES		+=	services/el3/ven_el3_svc.c			\
				lib/pmf/pmf_main.c
endif

ifneq (${ENABLE_FEAT_AMU},0)
BL32_SOURCES		+=	${AMU_SOURCES}
endif

ifeq (${WORKAROUND_CVE_2017_5715},1)
BL32_SOURCES		+=	bl32/sp_min/wa_cve_2017_5715_bpiall.S	\
				bl32/sp_min/wa_cve_2017_5715_icache_inv.S
else
ifeq (${WORKAROUND_CVE_2022_23960},1)
BL32_SOURCES		+=	bl32/sp_min/wa_cve_2017_5715_icache_inv.S
endif
endif

ifeq (${TRNG_SUPPORT},1)
BL32_SOURCES		+=	services/std_svc/trng/trng_main.c	\
				services/std_svc/trng/trng_entropy_pool.c
endif

ifeq (${ERRATA_ABI_SUPPORT}, 1)
BL32_SOURCES		+=	services/std_svc/errata_abi/errata_abi_main.c
endif

ifneq (${ENABLE_SYS_REG_TRACE_FOR_NS},0)
BL32_SOURCES		+=	lib/extensions/sys_reg_trace/aarch32/sys_reg_trace.c
endif

ifneq (${ENABLE_TRF_FOR_NS},0)
BL32_SOURCES		+=	lib/extensions/trf/aarch32/trf.c
endif

BL32_DEFAULT_LINKER_SCRIPT_SOURCE := bl32/sp_min/sp_min.ld.S

ifeq ($($(ARCH)-ld-id),gnu-gcc)
        BL32_LDFLAGS	+=	-Wl,--sort-section=alignment
else ifneq ($(filter llvm-lld gnu-ld,$($(ARCH)-ld-id)),)
        BL32_LDFLAGS	+=	--sort-section=alignment
endif

# Include the platform-specific SP_MIN Makefile
# If no platform-specific SP_MIN Makefile exists, it means SP_MIN is not supported
# on this platform.
SP_MIN_PLAT_MAKEFILE := $(wildcard ${PLAT_DIR}/sp_min/sp_min-${PLAT}.mk)
ifeq (,${SP_MIN_PLAT_MAKEFILE})
  $(error SP_MIN is not supported on platform ${PLAT})
else
  include ${SP_MIN_PLAT_MAKEFILE}
endif

RESET_TO_SP_MIN	:= 0
$(eval $(call add_define,RESET_TO_SP_MIN))
$(eval $(call assert_boolean,RESET_TO_SP_MIN))

# Flag to allow SP_MIN to handle FIQ interrupts in monitor mode. The platform
# port is free to override this value. It is default disabled.
SP_MIN_WITH_SECURE_FIQ 	?= 0
$(eval $(call add_define,SP_MIN_WITH_SECURE_FIQ))
$(eval $(call assert_boolean,SP_MIN_WITH_SECURE_FIQ))
