#ifndef __CAVM_CSRS_FUS_H__
#define __CAVM_CSRS_FUS_H__
/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * Read a single fuse bit
 *
 * @param fuse   Fuse number (0-1024)
 *
 * @return fuse value: 0 or 1
 */
int cavm_fuse_read(cavm_node_t node, int fuse);

#endif /* __CAVM_CSRS_FUS_H__ */
