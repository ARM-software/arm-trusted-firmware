/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 *
 * Functions for dealing with multiple chips organized into a
 * NUMA cluster.
 *
 * <hr>$Revision: 49448 $<hr>
 */

typedef enum
{
    NODE_0 = 0,
    NODE_1 = 1,
    NODE_2 = 2,
    NODE_3 = 3,
    NUMA_MAX_NODES = 4
} cavm_node_t;

/**
 * Return the local node number
 *
 * @return Node number
 */
static inline cavm_node_t cavm_numa_local(void) __attribute__ ((always_inline, pure));
static inline cavm_node_t cavm_numa_local(void)
{
#ifndef CAVM_BUILD_HOST
    int mpidr_el1;
    MRS_NV(MPIDR_EL1, mpidr_el1);
    int result;
    result = (mpidr_el1 >> 16) & 0xff;
    return NODE_0 + result;
#else
    return NODE_0; /* FIXME: choose remote node */
#endif
}

/**
 * Return the master node number
 *
 * @return Node number
 */
static inline cavm_node_t cavm_numa_master(void)
{
    extern int __cavm_numa_master_node;
    return __cavm_numa_master_node;
}

/**
 * Get a bitmask of the nodes that exist
 *
 * @return bitmask
 */
extern uint64_t cavm_numa_get_exists_mask(void);

/**
 * Add a node to the exists mask
 *
 * @param node   Node to add
 */
extern void cavm_numa_set_exists(cavm_node_t node);

/**
 * Return true if a node exists
 *
 * @param node   Node to check
 *
 * @return Non zero if the node exists
 */
extern int cavm_numa_exists(cavm_node_t node);

/**
 * Return true if there is only one node
 *
 * @return
 */
extern int cavm_numa_is_only_one(void);

/**
 * Given a physical address without a node, return the proper physical address
 * for the given node.
 *
 * @param node   Node to create address for
 * @param pa     Base physical address
 *
 * @return Node specific address
 */
static inline uint64_t cavm_numa_get_address(cavm_node_t node, uint64_t pa)
{
    if (pa & (1ull << 47))
        pa |= (uint64_t)(node&3) << 44;
    else
        pa |= (uint64_t)(node&3) << 40;
    return pa;
}


