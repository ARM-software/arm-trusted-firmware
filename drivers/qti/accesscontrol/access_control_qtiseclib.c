/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/qti/accesscontrol/accesscontrol.h>
#include <qtiseclib_interface.h>

/*
 * QTISECLIB memory-assignment backend. The actual stage-2 protection is
 * performed by QTISECLIB, so this glue forwards the request to
 * qtiseclib_mem_assign().
 */
uint64_t
qti_accesscontrol_mem_assign(const qti_accesscontrol_mem_t *mem,
			     uint32_t mem_len,
			     const uint32_t *src, uint32_t src_len,
			     const qti_accesscontrol_perm_t *perm,
			     uint32_t perm_len)
{
	memprot_dst_vm_perm_info_t *dst;
	memprot_info_t *mem_info;

	dst = (memprot_dst_vm_perm_info_t *)(void *)perm;
	mem_info = (memprot_info_t *)(void *)mem;

	return qtiseclib_mem_assign(mem_info, mem_len, src, src_len, dst,
				    perm_len);
}

void qti_accesscontrol_init(void)
{
}
