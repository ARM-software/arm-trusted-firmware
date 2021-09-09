/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if STM32MP_USE_STM32IMAGE
#ifdef BL32_BIN_PATH
.section .bl32_image
.incbin BL32_BIN_PATH
#endif
#endif /* STM32MP_USE_STM32IMAGE */

.section .bl2_image
.incbin BL2_BIN_PATH

.section .dtb_image
.incbin DTB_BIN_PATH
