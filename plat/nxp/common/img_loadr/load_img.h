/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef LOAD_IMAGE_H
#define LOAD_IMAGE_H

int load_img(unsigned int image_id, uintptr_t *image_base,
		      uint32_t *image_size);

#endif /* LOAD_IMAGE_H */
