/*
 * Copyright (c) 2019, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <plat/common/platform.h>

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy.
 *
 * This function is not supported at this time
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	return -ENOTSUP;
}
