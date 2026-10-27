/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SMMU_CFG_H
#define SMMU_CFG_H

#include <drivers/qti/smmu/smmu.h>

#include <kodiak_def.h>

#define SMMU_CUSTOM_CFG_ADDR                                         (QTI_SMMU_CFG_BASE + 0x002300)
#define SMMU_CUSTOM_CFG                                              0x60480010

#define SMMU_CUSTOM_CFG_SEC_ADDR                                     (QTI_SMMU_CFG_BASE + 0x0022fc)
#define SMMU_CUSTOM_CFG_SEC                                          0x000006f0

#define SMMU_ACR_ADDR                                                (QTI_SMMU_CFG_BASE + 0x000010)
#define SMMU_ACR                                                     0x0000001c

#define SMMU_NSACR_ADDR                                              (QTI_SMMU_CFG_BASE + 0x000410)
#define SMMU_NSACR                                                   0x0000001c

#define SMMU_SAFE_SEC_CFG_ADDR                                       (QTI_SMMU_CFG_BASE + 0x002648)
#define SMMU_SAFE_SEC_CFG                                            0x00000000

#define SMMU_MTLB_DEPTH_ADDR                                         (QTI_SMMU_CFG_BASE + 0x002524)
#define SMMU_MTLB_DEPTH                                              0x000001ff

#define SMMU_PFB_DEPTH_ADDR                                          (QTI_SMMU_CFG_BASE + 0x002564)
#define SMMU_PFB_DEPTH                                               0x000001ff

#define SMMU_SPARE_REG_1_SEC_ADDR                                    (QTI_SMMU_CFG_BASE + 0x1d8030)
#define SMMU_SPARE_REG_1_SEC                                         0x0000080f

#define SMMU_CLOCK_GATING_SEC_ADDR                                   (QTI_SMMU_CFG_BASE + 0x1d8010)
#define SMMU_CLOCK_GATING_SEC                                        0x000400ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_1_SEC_ADDR          (QTI_SMMU_CFG_BASE + 0x1dc010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_1_SEC               0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_2_SEC_ADDR          (QTI_SMMU_CFG_BASE + 0x1e0010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_2_SEC               0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_HF_0_SEC_ADDR       (QTI_SMMU_CFG_BASE + 0x1e4010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_HF_0_SEC            0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_HF_1_SEC_ADDR       (QTI_SMMU_CFG_BASE + 0x1e8010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_HF_1_SEC            0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_COMPUTE_DSP_1_SEC_ADDR   (QTI_SMMU_CFG_BASE + 0x1ec010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_COMPUTE_DSP_1_SEC        0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_COMPUTE_DSP_0_SEC_ADDR   (QTI_SMMU_CFG_BASE + 0x1f0010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_COMPUTE_DSP_0_SEC        0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_LPASS_SEC_ADDR           (QTI_SMMU_CFG_BASE + 0x1f4010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_LPASS_SEC                0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_PCIE_SEC_ADDR       (QTI_SMMU_CFG_BASE + 0x1f8010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_ANOC_PCIE_SEC            0x000103ff

#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_SF_0_SEC_ADDR       (QTI_SMMU_CFG_BASE + 0x1fc010)
#define SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_MNOC_SF_0_SEC            0x000103ff

#define SMMU_CLIENT_DBG_SID_HALT_MNOC_HF_1_SEC_ADDR                  (QTI_SMMU_CFG_BASE + 0x1e8000)
#define SMMU_CLIENT_DBG_SID_HALT_MNOC_HF_1_SEC                       0x80104000

#define SMMU_CLIENT_DBG_SID_HALT_MNOC_HF_0_SEC_ADDR                  (QTI_SMMU_CFG_BASE + 0x1e4000)
#define SMMU_CLIENT_DBG_SID_HALT_MNOC_HF_0_SEC                       0x80104000

#define SMMU_CLIENT_DBG_SID_HALT_ANOC_1_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1dc000)
#define SMMU_CLIENT_DBG_SID_HALT_ANOC_1_SEC                          0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_ANOC_2_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1e0000)
#define SMMU_CLIENT_DBG_SID_HALT_ANOC_2_SEC                          0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_COMPUTE_DSP_1_SEC_ADDR              (QTI_SMMU_CFG_BASE + 0x1ec000)
#define SMMU_CLIENT_DBG_SID_HALT_COMPUTE_DSP_1_SEC                   0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_COMPUTE_DSP_0_SEC_ADDR              (QTI_SMMU_CFG_BASE + 0x1f0000)
#define SMMU_CLIENT_DBG_SID_HALT_COMPUTE_DSP_0_SEC                   0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_LPASS_SEC_ADDR                      (QTI_SMMU_CFG_BASE + 0x1f4000)
#define SMMU_CLIENT_DBG_SID_HALT_LPASS_SEC                           0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_ANOC_PCIE_SEC_ADDR                  (QTI_SMMU_CFG_BASE + 0x1f8000)
#define SMMU_CLIENT_DBG_SID_HALT_ANOC_PCIE_SEC                       0x80100000

#define SMMU_CLIENT_DBG_SID_HALT_MNOC_SF_0_SEC_ADDR                  (QTI_SMMU_CFG_BASE + 0x1fc000)
#define SMMU_CLIENT_DBG_SID_HALT_MNOC_SF_0_SEC                       0x80100000

#define SMMU_CLIENT_DBG_WUSER_ANOC_1_SEC_ADDR                        (QTI_SMMU_CFG_BASE + 0x1dc038)
#define SMMU_CLIENT_DBG_WUSER_ANOC_1_SEC                             0x00000000

#define SMMU_CLIENT_DBG_WUSER_ANOC_2_SEC_ADDR                        (QTI_SMMU_CFG_BASE + 0x1e0038)
#define SMMU_CLIENT_DBG_WUSER_ANOC_2_SEC                             0x00000000

#define SMMU_CLIENT_DBG_WUSER_MNOC_HF_0_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1e4038)
#define SMMU_CLIENT_DBG_WUSER_MNOC_HF_0_SEC                          0x00000000

#define SMMU_CLIENT_DBG_WUSER_MNOC_HF_1_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1e8038)
#define SMMU_CLIENT_DBG_WUSER_MNOC_HF_1_SEC                          0x00000000

#define SMMU_CLIENT_DBG_WUSER_COMPUTE_DSP_1_SEC_ADDR                 (QTI_SMMU_CFG_BASE + 0x1ec038)
#define SMMU_CLIENT_DBG_WUSER_COMPUTE_DSP_1_SEC                      0x00000000

#define SMMU_CLIENT_DBG_WUSER_COMPUTE_DSP_0_SEC_ADDR                 (QTI_SMMU_CFG_BASE + 0x1f0038)
#define SMMU_CLIENT_DBG_WUSER_COMPUTE_DSP_0_SEC                      0x00000000

#define SMMU_CLIENT_DBG_WUSER_LPASS_SEC_ADDR                         (QTI_SMMU_CFG_BASE + 0x1f4038)
#define SMMU_CLIENT_DBG_WUSER_LPASS_SEC                              0x00000000

#define SMMU_CLIENT_DBG_WUSER_ANOC_PCIE_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1f8038)
#define SMMU_CLIENT_DBG_WUSER_ANOC_PCIE_SEC                          0x00000000

#define SMMU_CLIENT_DBG_WUSER_MNOC_SF_0_SEC_ADDR                     (QTI_SMMU_CFG_BASE + 0x1fc038)
#define SMMU_CLIENT_DBG_WUSER_MNOC_SF_0_SEC                          0x00000000

#define GPU_GFX_SMMU_CUSTOM_CFG_ADDR                               (QTI_GPU_SMMU_CFG_BASE + 0x42300)
#define GPU_GFX_SMMU_CUSTOM_CFG                                    0x20400053

#define GPU_GFX_SMMU_CUSTOM_CFG_SEC_ADDR                           (QTI_GPU_SMMU_CFG_BASE + 0x422fc)
#define GPU_GFX_SMMU_CUSTOM_CFG_SEC                                0x000007f0

#define GPU_GFX_SMMU_ACR_ADDR                                      (QTI_GPU_SMMU_CFG_BASE + 0x40010)
#define GPU_GFX_SMMU_ACR                                           0x00000004

#define GPU_GFX_SMMU_CLOCK_GATING_SEC_ADDR                         (QTI_GPU_SMMU_CFG_BASE + 0x74010)
#define GPU_GFX_SMMU_CLOCK_GATING_SEC                              0x000000ff

#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC_ADDR (QTI_GPU_SMMU_CFG_BASE + 0x78010)
#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_0_SEC      0x000103ff

#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC_ADDR (QTI_GPU_SMMU_CFG_BASE + 0x7c010)
#define GPU_GFX_SMMU_CLIENT_DBG_SSD_IDX_HYP_HLOS_EN_GFX_1_SEC      0x000103ff

#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC_ADDR            (QTI_GPU_SMMU_CFG_BASE + 0x78000)
#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_0_SEC                   0x80182000

#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC_ADDR            (QTI_GPU_SMMU_CFG_BASE + 0x7c000)
#define GPU_GFX_SMMU_CLIENT_DBG_SID_HALT_GFX_1_SEC                 0x80182000

/*
 * Aperture slot 0, through which the Adreno CP switches the page tables of
 * GPU SMMU context bank 0: the offset of that bank in the GPU register space.
 */
#define GPU_SMMU_APERTURE_S1CB0_ADDR                               (QTI_GPU_SMMU_CFG_BASE + 0x0)
#define GPU_SMMU_APERTURE_S1CB0 \
	((QTI_GPU_SMMU_CFG_BASE + 0x50000) & 0xfffff)
#endif
