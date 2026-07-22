#
# Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# This file enables all supported optional architectural features in runtime detection mode.

# Always check that hardware matches the codebase's expectations.
FEATURE_DETECTION			:= 1

ENABLE_FEAT_AMU				:= 2
ifneq (${ENABLE_FEAT_AMU},0)
    ENABLE_FEAT_AMUv1p1			:= 2
endif
ENABLE_FEAT_HCX				:= 2
ENABLE_FEAT_RNG				:= 2
ENABLE_FEAT_TWED			:= 2
ENABLE_FEAT_GCS				:= 2
ENABLE_FEAT_RAS				:= 2
ENABLE_FEAT_SB				:= 2

ifeq (${ARCH},aarch64)
    ifeq (${SPM_MM},0)
        ifeq (${CTX_INCLUDE_FPREGS},0)
            ENABLE_SME_FOR_NS		:= 2
            ENABLE_SME2_FOR_NS		:= 2
        else
            ENABLE_SVE_FOR_NS		:= 0
            ENABLE_SME_FOR_NS		:= 0
            ENABLE_SME2_FOR_NS		:= 0
        endif
    endif

    ENABLE_BRBE_FOR_NS			:= 2
    ENABLE_TRBE_FOR_NS			:= 2
    ENABLE_FEAT_D128			:= 2
    ENABLE_FEAT_FPMR			:= 2
    ENABLE_FEAT_MOPS			:= 2
    ENABLE_FEAT_FGWTE3			:= 2
    ifneq (${ENABLE_FEAT_MPAM},0)
        ENABLE_FEAT_MPAM_PE_BW_CTRL	:= 2
    endif
    ENABLE_FEAT_CPA2			:= 2
    ENABLE_FEAT_UINJ			:= 2
    ENABLE_FEAT_STEP2			:= 2
    ENABLE_FEAT_HDBSS			:= 2
    ENABLE_FEAT_HACDBS			:= 2
    ENABLE_FEAT_SPEV1P5			:= 2
    ENABLE_FEAT_SRMASK			:= 2
    ENABLE_FEAT_BRBEV1P1		:= 2
    ENABLE_FEAT_TRBE_EXC		:= 2
endif

ENABLE_SYS_REG_TRACE_FOR_NS		:= 2
ENABLE_FEAT_CSV2_2			:= 2
ENABLE_FEAT_CSV2_3			:= 2
ENABLE_FEAT_CLRBHB			:= 2
ENABLE_FEAT_DEBUGV8P9			:= 2
ENABLE_FEAT_DIT				:= 2
ENABLE_FEAT_PAN				:= 2
ENABLE_FEAT_VHE				:= 2
CTX_INCLUDE_NEVE_REGS			:= 2
ENABLE_FEAT_SEL2			:= 2
ENABLE_TRF_FOR_NS			:= 2
ENABLE_FEAT_ECV				:= 2
ENABLE_FEAT_FGT				:= 2
ENABLE_FEAT_FGT2			:= 2
ENABLE_FEAT_THE				:= 2
ENABLE_FEAT_TCR2			:= 2
ENABLE_FEAT_S2PIE			:= 2
ENABLE_FEAT_S1PIE			:= 2
ENABLE_FEAT_S2POE			:= 2
ENABLE_FEAT_S1POE			:= 2
ENABLE_FEAT_SCTLR2			:= 2
ENABLE_FEAT_MTE2			:= 2
ENABLE_FEAT_LS64_ACCDATA		:= 2
ENABLE_FEAT_AIE				:= 2
ENABLE_FEAT_PFAR			:= 2
ENABLE_FEAT_AxERR			:= 2
ENABLE_FEAT_EBEP			:= 2

ifeq (${ENABLE_RMM},1)
    ENABLE_FEAT_MEC			:= 2
endif
