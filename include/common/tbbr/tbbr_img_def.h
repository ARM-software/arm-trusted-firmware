/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TBBR_IMG_DEF_H
#define TBBR_IMG_DEF_H

#include <export/common/tbbr/tbbr_img_def_exp.h>

#if defined(SPD_spmd)
#define SP_CONTENT_CERT_ID		MAX_IMAGE_IDS
#define MAX_SP_IDS			U(8)
#define MAX_NUMBER_IDS			(MAX_IMAGE_IDS + MAX_SP_IDS + U(1))
#else
#define MAX_NUMBER_IDS			MAX_IMAGE_IDS
#endif

#endif /* TBBR_IMG_DEF_H */
