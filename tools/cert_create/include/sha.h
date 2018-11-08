/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SHA_H
#define SHA_H

int sha_file(int md_alg, const char *filename, unsigned char *md);

#endif /* SHA_H */
