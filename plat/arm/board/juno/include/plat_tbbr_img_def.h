/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef JUNO_IMG_DEF_H
#define JUNO_IMG_DEF_H

#if ETHOSN_NPU_TZMP1
/* Arm(R) Ethos(TM)-N NPU images */
#define ETHOSN_NPU_FW_KEY_CERT_ID		U(MAX_IMG_IDS_WITH_SPMDS + 1)
#define ETHOSN_NPU_FW_CONTENT_CERT_ID		U(MAX_IMG_IDS_WITH_SPMDS + 2)
#define ETHOSN_NPU_FW_IMAGE_ID			U(MAX_IMG_IDS_WITH_SPMDS + 3)
#define MAX_NUMBER_IDS				U(MAX_IMG_IDS_WITH_SPMDS + 4)
#endif /* ETHOSN_NPU_TZMP1 */

#endif	/* JUNO_IMG_DEF_H */
