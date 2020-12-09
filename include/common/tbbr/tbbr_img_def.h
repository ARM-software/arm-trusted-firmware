/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_IMG_DEF_H
#define TBBR_IMG_DEF_H

#include <export/common/tbbr/tbbr_img_def_exp.h>

#if defined(SPD_spmd)
#define SIP_SP_CONTENT_CERT_ID		MAX_IMAGE_IDS
#define PLAT_SP_CONTENT_CERT_ID		(MAX_IMAGE_IDS + 1)
#define SP_PKG1_ID			(MAX_IMAGE_IDS + 2)
#define SP_PKG2_ID			(MAX_IMAGE_IDS + 3)
#define SP_PKG3_ID			(MAX_IMAGE_IDS + 4)
#define SP_PKG4_ID			(MAX_IMAGE_IDS + 5)
#define SP_PKG5_ID			(MAX_IMAGE_IDS + 6)
#define SP_PKG6_ID			(MAX_IMAGE_IDS + 7)
#define SP_PKG7_ID			(MAX_IMAGE_IDS + 8)
#define SP_PKG8_ID			(MAX_IMAGE_IDS + 9)
#define MAX_SP_IDS			U(8)
#define MAX_IMG_IDS_WITH_SPMDS		(MAX_IMAGE_IDS + MAX_SP_IDS + U(2))
#else
#define MAX_IMG_IDS_WITH_SPMDS		MAX_IMAGE_IDS
#endif

#ifdef PLAT_TBBR_IMG_DEF
#include <plat_tbbr_img_def.h>
#endif

#ifndef MAX_NUMBER_IDS
#define MAX_NUMBER_IDS			MAX_IMG_IDS_WITH_SPMDS
#endif

#endif /* TBBR_IMG_DEF_H */
