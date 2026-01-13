#
# Copyright (c) 2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifneq ($(AARCH32_INSTRUCTION_SET),$(filter $(AARCH32_INSTRUCTION_SET),A32 T32))
         $(error Error: Unknown AArch32 instruction set ${AARCH32_INSTRUCTION_SET})
endif

# Make sure RME configuration is valid
ifeq (${ENABLE_RME},1)
	ifneq (${SEPARATE_CODE_AND_RODATA},1)
                $(error ENABLE_RME requires SEPARATE_CODE_AND_RODATA)
	endif

	ifneq (${ARCH},aarch64)
                $(error ENABLE_RME requires AArch64)
	endif

	ifeq ($(SPMC_AT_EL3),1)
                $(error SPMC_AT_EL3 and ENABLE_RME cannot both be enabled.)
	endif

	ifneq (${SPD}, none)
		ifneq (${SPD}, spmd)
                        $(error ENABLE_RME is incompatible with SPD=${SPD}. Use SPD=spmd)
		endif
	endif
else
	ifeq (${ENABLE_FEAT_RME_GDI},1)
                $(error ENABLE_FEAT_RME_GDI requires ENABLE_RME)
	endif
endif

ifeq (${CTX_INCLUDE_EL2_REGS}, 1)
	ifeq (${SPD},none)
		ifeq (${ENABLE_RME},0)
                        $(error CTX_INCLUDE_EL2_REGS is available only when SPD \
                        or RME is enabled)
		endif
	endif
endif

################################################################################
# Verify FEAT_RME, FEAT_SCTLR2 and FEAT_TCR2 are enabled if FEAT_MEC is enabled.
################################################################################

ifneq (${ENABLE_FEAT_MEC},0)
    ifeq (${ENABLE_RME},0)
        $(error FEAT_RME must be enabled when FEAT_MEC is enabled.)
    endif
    ifeq (${ENABLE_FEAT_TCR2},0)
        $(error FEAT_TCR2 must be enabled when FEAT_MEC is enabled.)
    endif
    ifeq (${ENABLE_FEAT_SCTLR2},0)
        $(error FEAT_SCTLR2 must be enabled when FEAT_MEC is enabled.)
    endif
endif

# Handle all invalid build configurations with SPMD usage.
ifeq (${ENABLE_SPMD_LP}, 1)
ifneq (${SPD},spmd)
        $(error Error: ENABLE_SPMD_LP requires SPD=spmd.)
endif
ifeq ($(SPMC_AT_EL3),1)
        $(error SPMC at EL3 not supported when enabling SPMD Logical partitions.)
endif
endif

ifneq (${SPD},none)
ifeq (${ARCH},aarch32)
        $(error "Error: SPD is incompatible with AArch32.")
endif
ifdef EL3_PAYLOAD_BASE
        $(warning "SPD and EL3_PAYLOAD_BASE are incompatible build options.")
        $(warning "The SPD and its BL32 companion will be present but ignored.")
endif
ifeq (${SPD},spmd)
ifeq ($(SPMD_SPM_AT_SEL2),1)
        ifeq ($(SPMC_AT_EL3),1)
                $(error SPM cannot be enabled in both S-EL2 and EL3.)
        endif
        ifeq ($(CTX_INCLUDE_SVE_REGS),1)
                $(error SVE context management not needed with Hafnium SPMC.)
        endif
endif

ifeq ($(SPMC_AT_EL3_SEL0_SP),1)
        ifneq ($(SPMC_AT_EL3),1)
                $(error SEL0 SP cannot be enabled without SPMC at EL3)
        endif
endif
endif #(SPD=spmd)
endif #(SPD!=none)

# USE_DEBUGFS experimental feature recommended only in debug builds
ifeq (${USE_DEBUGFS},1)
        ifeq (${DEBUG},1)
                $(warning DEBUGFS experimental feature is enabled.)
        else
                $(warning DEBUGFS experimental, recommended in DEBUG builds ONLY)
        endif
endif #(USE_DEBUGFS)

# USE_SPINLOCK_CAS requires AArch64 build
ifeq (${USE_SPINLOCK_CAS},1)
        ifneq (${ARCH},aarch64)
               $(error USE_SPINLOCK_CAS requires AArch64)
        endif
endif #(USE_SPINLOCK_CAS)

ifdef EL3_PAYLOAD_BASE
	ifdef PRELOADED_BL33_BASE
                $(warning "PRELOADED_BL33_BASE and EL3_PAYLOAD_BASE are \
                incompatible build options. EL3_PAYLOAD_BASE has priority.")
	endif
	ifneq (${GENERATE_COT},0)
                $(error "GENERATE_COT and EL3_PAYLOAD_BASE are incompatible \
                build options.")
	endif
	ifneq (${TRUSTED_BOARD_BOOT},0)
                $(error "TRUSTED_BOARD_BOOT and EL3_PAYLOAD_BASE are \
                incompatible \ build options.")
	endif
endif #(EL3_PAYLOAD_BASE)

ifeq (${NEED_BL33},yes)
	ifdef EL3_PAYLOAD_BASE
                $(warning "BL33 image is not needed when option \
                BL33_PAYLOAD_BASE is used and won't be added to the FIP file.")
	endif
	ifdef PRELOADED_BL33_BASE
                $(warning "BL33 image is not needed when option \
                PRELOADED_BL33_BASE is used and won't be added to the FIP file.")
	endif
endif #(NEED_BL33)

# When building for systems with hardware-assisted coherency, there's no need to
# use USE_COHERENT_MEM. Require that USE_COHERENT_MEM must be set to 0 too.
ifeq ($(HW_ASSISTED_COHERENCY)-$(USE_COHERENT_MEM),1-1)
        $(error USE_COHERENT_MEM cannot be enabled with HW_ASSISTED_COHERENCY)
endif

#For now, BL2_IN_XIP_MEM is only supported when RESET_TO_BL2 is 1.
ifeq ($(RESET_TO_BL2)-$(BL2_IN_XIP_MEM),0-1)
        $(error "BL2_IN_XIP_MEM is only supported when RESET_TO_BL2 is enabled")
endif

# RAS_EXTENSION is deprecated, provide alternate build options
ifeq ($(RAS_EXTENSION),1)
        $(error "RAS_EXTENSION is now deprecated, please use ENABLE_FEAT_RAS \
        and HANDLE_EA_EL3_FIRST_NS instead")
endif


# When FAULT_INJECTION_SUPPORT is used, require that FEAT_RAS is enabled
ifeq ($(FAULT_INJECTION_SUPPORT),1)
	ifeq ($(ENABLE_FEAT_RAS),0)
                $(error For FAULT_INJECTION_SUPPORT, ENABLE_FEAT_RAS must not be 0)
	endif
endif #(FAULT_INJECTION_SUPPORT)

# DYN_DISABLE_AUTH can be set only when TRUSTED_BOARD_BOOT=1
ifeq ($(DYN_DISABLE_AUTH), 1)
	ifeq (${TRUSTED_BOARD_BOOT}, 0)
                $(error "TRUSTED_BOARD_BOOT must be enabled for DYN_DISABLE_AUTH \
                to be set.")
	endif
endif #(DYN_DISABLE_AUTH)

# SDEI_IN_FCONF is only supported when SDEI_SUPPORT is enabled.
ifeq ($(SDEI_SUPPORT)-$(SDEI_IN_FCONF),0-1)
        $(error "SDEI_IN_FCONF is only supported when SDEI_SUPPORT is enabled")
endif

# If pointer authentication is used in the firmware, make sure that all the
# registers associated to it are also saved and restored.
# Not doing it would leak the value of the keys used by EL3 to EL1 and S-EL1.
ifneq ($(ENABLE_PAUTH),0)
	ifeq ($(CTX_INCLUDE_PAUTH_REGS),0)
                $(error Pointer Authentication requires CTX_INCLUDE_PAUTH_REGS to be enabled)
	endif
endif #(ENABLE_PAUTH)

ifneq ($(CTX_INCLUDE_PAUTH_REGS),0)
	ifneq (${ARCH},aarch64)
                $(error CTX_INCLUDE_PAUTH_REGS requires AArch64)
	endif
endif #(CTX_INCLUDE_PAUTH_REGS)

# Check ENABLE_FEAT_PAUTH_LR
ifneq (${ENABLE_FEAT_PAUTH_LR},0)

# Make sure PAUTH is enabled
ifeq (${ENABLE_PAUTH},0)
        $(error Error: PAUTH_LR cannot be used without PAUTH (see BRANCH_PROTECTION))
endif

# Make sure SCTLR2 is enabled
ifeq (${ENABLE_FEAT_SCTLR2},0)
        $(error Error: PAUTH_LR cannot be used without ENABLE_FEAT_SCTLR2)
endif

# FEAT_PAUTH_LR is only supported in aarch64 state
ifneq (${ARCH},aarch64)
        $(error ENABLE_FEAT_PAUTH_LR requires AArch64)
endif

endif # ${ENABLE_FEAT_PAUTH_LR}

ifeq ($(FEATURE_DETECTION),1)
        $(info FEATURE_DETECTION is an experimental feature)
endif #(FEATURE_DETECTION)

ifneq ($(ENABLE_SME2_FOR_NS), 0)
	ifeq (${ENABLE_SME_FOR_NS}, 0)
                $(warning "ENABLE_SME2_FOR_NS requires ENABLE_SME_FOR_NS also \
                to be set")
                $(warning "Forced ENABLE_SME_FOR_NS=1")
                override ENABLE_SME_FOR_NS	:= 1
	endif
endif #(ENABLE_SME2_FOR_NS)

ifeq (${ARM_XLAT_TABLES_LIB_V1}, 1)
	ifeq (${ALLOW_RO_XLAT_TABLES}, 1)
                $(error "ALLOW_RO_XLAT_TABLES requires translation tables \
                library v2")
	endif
endif #(ARM_XLAT_TABLES_LIB_V1)

ifneq (${DECRYPTION_SUPPORT},none)
	ifeq (${TRUSTED_BOARD_BOOT}, 0)
                $(error TRUSTED_BOARD_BOOT must be enabled for DECRYPTION_SUPPORT \
                to be set)
	endif
endif #(DECRYPTION_SUPPORT)

# Ensure that no Aarch64-only features are enabled in Aarch32 build
ifeq (${ARCH},aarch32)
        ifneq (${ENABLE_LTO},0)
                $(error "ENABLE_LTO is not supported with ARCH=aarch32")
        endif
        ifneq (${EL3_EXCEPTION_HANDLING},0)
                $(error "EL3_EXCEPTION_HANDLING is not supported outside BL31")
        endif

        ifeq (${CRASH_REPORTING},1)
                $(error "CRASH_REPORTING is not supported with ARCH=aarch32")
        endif

	# SME/SVE only supported on AArch64
	ifneq (${ENABLE_SME_FOR_NS},0)
                $(error "ENABLE_SME_FOR_NS cannot be used with ARCH=aarch32")
	endif

	ifneq (${ENABLE_SVE_FOR_NS},0)
                $(error "ENABLE_SVE_FOR_NS cannot be used with ARCH=aarch32")
	endif

	ifneq (${ENABLE_SPE_FOR_NS},0)
                $(error "ENABLE_SPE_FOR_NS cannot be used with ARCH=aarch32")
	endif

	# BRBE is not supported in AArch32
	ifneq (${ENABLE_BRBE_FOR_NS},0)
                $(error "ENABLE_BRBE_FOR_NS cannot be used with ARCH=aarch32")
	endif

	# FEAT_RNG_TRAP is not supported in AArch32
	ifneq (${ENABLE_FEAT_RNG_TRAP},0)
                $(error "ENABLE_FEAT_RNG_TRAP cannot be used with ARCH=aarch32")
	endif

	ifneq (${ENABLE_FEAT_FPMR},0)
                $(error "ENABLE_FEAT_FPMR cannot be used with ARCH=aarch32")
	endif

	ifeq (${ARCH_FEATURE_AVAILABILITY},1)
                $(error "ARCH_FEATURE_AVAILABILITY cannot be used with ARCH=aarch32")
	endif
	# FEAT_MOPS is only supported on AArch64
	ifneq (${ENABLE_FEAT_MOPS},0)
                $(error "ENABLE_FEAT_MOPS cannot be used with ARCH=aarch32")
	endif
	ifneq (${ENABLE_FEAT_GCIE},0)
                $(error "ENABLE_FEAT_GCIE cannot be used with ARCH=aarch32")
	endif
	ifneq (${ENABLE_FEAT_CPA2},0)
                $(error "ENABLE_FEAT_CPA2 cannot be used with ARCH=aarch32")
	endif
        ifneq (${USE_SPINLOCK_CAS},0)
                $(error "USE_SPINLOCK_CAS is not supported with ARCH=aarch32")
        endif
	ifneq (${PLATFORM_NODE_COUNT},1)
                $(error "NUMA AWARE PER CPU is not supported with ARCH=aarch32")
	endif
	ifneq (${ENABLE_FEAT_MPAM},0)
                $(error "ENABLE_FEAT_MPAM cannot be used with ARCH=aarch32")
	endif
	ifneq (${ENABLE_FEAT_UINJ},0)
		$(error "ENABLE_FEAT_UINJ cannot be used with ARCH=aarch32")
	endif
endif #(ARCH=aarch32)

ifneq (${ENABLE_FEAT_FPMR},0)
	ifeq (${ENABLE_FEAT_FGT},0)
                $(error "ENABLE_FEAT_FPMR requires ENABLE_FEAT_FGT")
	endif
	ifeq (${ENABLE_FEAT_HCX},0)
                $(error "ENABLE_FEAT_FPMR requires ENABLE_FEAT_HCX")
	endif
endif #(ENABLE_FEAT_FPMR)

ifneq (${ENABLE_FEAT_CPA2},0)
	ifeq (${ENABLE_FEAT_SCTLR2},0)
                $(error "Error: ENABLE_FEAT_CPA2 cannot be used without ENABLE_FEAT_SCTLR2")
	endif
endif #${ENABLE_FEAT_CPA2}

ifneq (${ENABLE_SME_FOR_NS},0)
	ifeq (${ENABLE_SVE_FOR_NS},0)
                $(error "ENABLE_SME_FOR_NS requires ENABLE_SVE_FOR_NS")
	endif
endif #(ENABLE_SME_FOR_NS)

# Secure SME/SVE requires the non-secure component as well
ifeq (${ENABLE_SME_FOR_SWD},1)
	ifeq (${ENABLE_SME_FOR_NS},0)
                $(error "ENABLE_SME_FOR_SWD requires ENABLE_SME_FOR_NS")
	endif
	ifeq (${ENABLE_SVE_FOR_SWD},0)
                $(error "ENABLE_SME_FOR_SWD requires ENABLE_SVE_FOR_SWD")
	endif
endif #(ENABLE_SME_FOR_SWD)

# Enabling SVE for SWD requires enabling SVE for NWD due to ENABLE_FEAT
# mechanism.
ifeq (${ENABLE_SVE_FOR_SWD},1)
    ifeq (${ENABLE_SVE_FOR_NS},0)
        $(error "ENABLE_SVE_FOR_SWD requires ENABLE_SVE_FOR_NS")
    endif
endif

# Enabling FEAT_MOPS requires access to hcrx_el2 registers which is
# available only when FEAT_HCX is enabled.
ifneq (${ENABLE_FEAT_MOPS},0)
    ifeq (${ENABLE_FEAT_HCX},0)
        $(error "ENABLE_FEAT_MOPS requires ENABLE_FEAT_HCX")
    endif
endif

# Enabling SVE for both the worlds typically requires the context
# management of SVE registers. The only exception being SPMC at S-EL2.
ifeq (${ENABLE_SVE_FOR_SWD}, 1)
    ifneq (${ENABLE_SVE_FOR_NS}, 0)
        ifeq (${CTX_INCLUDE_SVE_REGS}-$(SPMD_SPM_AT_SEL2),0-0)
            $(warning "ENABLE_SVE_FOR_SWD and ENABLE_SVE_FOR_NS together require CTX_INCLUDE_SVE_REGS")
        endif
    endif
endif

# Enabling SVE in either world while enabling CTX_INCLUDE_FPREGS requires
# CTX_INCLUDE_SVE_REGS to be enabled due to architectural dependency between FP
# and SVE registers.
ifeq (${CTX_INCLUDE_FPREGS}, 1)
    ifneq (${ENABLE_SVE_FOR_NS},0)
        ifeq (${CTX_INCLUDE_SVE_REGS},0)
            # Warning instead of error due to CI dependency on this
            $(warning "CTX_INCLUDE_FPREGS and ENABLE_SVE_FOR_NS together require CTX_INCLUDE_SVE_REGS")
            $(warning "Forced ENABLE_SVE_FOR_NS=0")
            override ENABLE_SVE_FOR_NS	:= 0
        endif
    endif
endif #(CTX_INCLUDE_FPREGS)

# SVE context management is only required if secure world has access to SVE/FP
# functionality.
# Enabling CTX_INCLUDE_SVE_REGS requires CTX_INCLUDE_FPREGS to be enabled due
# to architectural dependency between FP and SVE registers.
ifeq (${CTX_INCLUDE_SVE_REGS},1)
    ifeq (${ENABLE_SVE_FOR_SWD},0)
        $(error "CTX_INCLUDE_SVE_REGS requires ENABLE_SVE_FOR_SWD to also be enabled")
    endif
    ifeq (${CTX_INCLUDE_FPREGS},0)
        $(error "CTX_INCLUDE_SVE_REGS requires CTX_INCLUDE_FPREGS to also be enabled")
    endif #(CTX_INCLUDE_FPREGS)
endif #(CTX_INCLUDE_SVE_REGS)

# SME cannot be used with CTX_INCLUDE_FPREGS since SPM does its own context
# management including FPU registers.
ifeq (${CTX_INCLUDE_FPREGS},1)
    ifneq (${ENABLE_SME_FOR_NS},0)
        $(error "ENABLE_SME_FOR_NS cannot be used with CTX_INCLUDE_FPREGS")
    endif
endif #(CTX_INCLUDE_FPREGS)

ifeq ($(DRTM_SUPPORT),1)
        $(info DRTM_SUPPORT is an experimental feature)
endif

ifeq (${HOB_LIST},1)
        $(warning HOB_LIST is an experimental feature)
endif

ifeq (${TRANSFER_LIST},1)
        $(info TRANSFER_LIST is an experimental feature)
endif

ifeq ($(PSA_CRYPTO),1)
        $(info PSA_CRYPTO is an experimental feature)
endif

ifeq ($(DICE_PROTECTION_ENVIRONMENT),1)
        $(info DICE_PROTECTION_ENVIRONMENT is an experimental feature)
endif

ifeq (${LFA_SUPPORT},1)
        $(warning LFA_SUPPORT is an experimental feature)
endif #(LFA_SUPPORT)

ifneq (${ENABLE_FEAT_MPAM_PE_BW_CTRL},0)
        ifeq (${ENABLE_FEAT_MPAM},0)
                $(error "ENABLE_FEAT_MPAM_PW_BW_CTRL requires ENABLE_FEAT_MPAM")
        endif
endif #(ENABLE_FEAT_MPAM_PE_BW_CTRL)

ifneq (${DYNAMIC_WORKAROUND_CVE_2018_3639},0)
        ifeq (${WORKAROUND_CVE_2018_3639},0)
                $(error Error: WORKAROUND_CVE_2018_3639 must be 1 if DYNAMIC_WORKAROUND_CVE_2018_3639 is 1)
        endif
endif

ifneq ($(ENABLE_FEAT_MORELLO),0)
        ifneq ($($(ARCH)-cc-id),llvm-clang)
                $(error ENABLE_FEAT_MORELLO requires Clang toolchain)
        endif
        $(warning Morello capability is an experimental feature)
endif

# Handle all deprecated build options.
ifeq (${ERROR_DEPRECATED}, 1)
    ifneq (${NS_TIMER_SWITCH},0)
        $(error "NS_TIMER_SWITCH breaks Linux preemption model, hence deprecated")
    endif
    ifneq (${SPM_MM},0)
        $(error "SPM_MM build option is deprecated")
    endif
endif

ifneq (${ENABLE_FEAT_IDTE3},0)
        $(info FEAT_IDTE3 is an experimental feature)
endif #(ENABLE_FEAT_IDTE3)
