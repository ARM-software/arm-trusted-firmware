/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <lib/debugfs.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <smccc_helpers.h>

#define MAX_PATH_LEN	256

#define MOUNT		0
#define CREATE		1
#define OPEN		2
#define CLOSE		3
#define READ		4
#define WRITE		5
#define SEEK		6
#define BIND		7
#define STAT		8
#define INIT		10
#define VERSION		11

/* This is the virtual address to which we map the NS shared buffer */
#define DEBUGFS_SHARED_BUF_VIRT		((void *)0x81000000U)

static union debugfs_parms {
	struct {
		char fname[MAX_PATH_LEN];
	} open;

	struct {
		char srv[MAX_PATH_LEN];
		char where[MAX_PATH_LEN];
		char spec[MAX_PATH_LEN];
	} mount;

	struct {
		char path[MAX_PATH_LEN];
		dir_t dir;
	} stat;

	struct {
		char oldpath[MAX_PATH_LEN];
		char newpath[MAX_PATH_LEN];
	} bind;
} parms;

/* debugfs_access_lock protects shared buffer and internal */
/* FS functions from concurrent acccesses.                 */
static spinlock_t debugfs_access_lock;

static bool debugfs_initialized;

uintptr_t debugfs_smc_handler(unsigned int smc_fid,
			      u_register_t cmd,
			      u_register_t arg2,
			      u_register_t arg3,
			      u_register_t arg4,
			      void *cookie,
			      void *handle,
			      u_register_t flags)
{
	int64_t smc_ret = DEBUGFS_E_INVALID_PARAMS, smc_resp = 0;
	int ret;

	/* Allow calls from non-secure only */
	if (is_caller_secure(flags)) {
		SMC_RET1(handle, DEBUGFS_E_DENIED);
	}

	/* Expect a SiP service fast call */
	if ((GET_SMC_TYPE(smc_fid) != SMC_TYPE_FAST) ||
		(GET_SMC_OEN(smc_fid) != OEN_SIP_START)) {
		SMC_RET1(handle, SMC_UNK);
	}

	/* Truncate parameters if 32b SMC convention call */
	if (GET_SMC_CC(smc_fid) == SMC_32) {
		arg2 &= 0xffffffff;
		arg3 &= 0xffffffff;
		arg4 &= 0xffffffff;
	}

	spin_lock(&debugfs_access_lock);

	if (debugfs_initialized == true) {
		/* Copy NS shared buffer to internal secure location */
		memcpy(&parms, (void *)DEBUGFS_SHARED_BUF_VIRT,
		       sizeof(union debugfs_parms));
	}

	switch (cmd) {
	case INIT:
		if (debugfs_initialized == false) {
			/* TODO: check PA validity e.g. whether */
			/* it is an NS region.                  */
			ret = mmap_add_dynamic_region(arg2,
				(uintptr_t)DEBUGFS_SHARED_BUF_VIRT,
				PAGE_SIZE_4KB,
				MT_MEMORY | MT_RW | MT_NS);
			if (ret == 0) {
				debugfs_initialized = true;
				smc_ret = SMC_OK;
				smc_resp = 0;
			}
		}
		break;

	case VERSION:
		smc_ret = SMC_OK;
		smc_resp = DEBUGFS_VERSION;
		break;

	case MOUNT:
		ret = mount(parms.mount.srv,
			    parms.mount.where,
			    parms.mount.spec);
		if (ret == 0) {
			smc_ret = SMC_OK;
			smc_resp = 0;
		}
		break;

	case OPEN:
		ret = open(parms.open.fname, arg2);
		if (ret >= 0) {
			smc_ret = SMC_OK;
			smc_resp = ret;
		}
		break;

	case CLOSE:
		ret = close(arg2);
		if (ret == 0) {
			smc_ret = SMC_OK;
			smc_resp = 0;
		}
		break;

	case READ:
		ret = read(arg2, DEBUGFS_SHARED_BUF_VIRT, arg3);
		if (ret >= 0) {
			smc_ret = SMC_OK;
			smc_resp = ret;
		}
		break;

	case SEEK:
		ret = seek(arg2, arg3, arg4);
		if (ret == 0) {
			smc_ret = SMC_OK;
			smc_resp = 0;
		}
		break;

	case BIND:
		ret = bind(parms.bind.oldpath, parms.bind.newpath);
		if (ret == 0) {
			smc_ret = SMC_OK;
			smc_resp = 0;
		}
		break;

	case STAT:
		ret = stat(parms.stat.path, &parms.stat.dir);
		if (ret == 0) {
			memcpy((void *)DEBUGFS_SHARED_BUF_VIRT, &parms,
			       sizeof(union debugfs_parms));
			smc_ret = SMC_OK;
			smc_resp = 0;
		}
		break;

	/* Not implemented */
	case CREATE:
		/* Intentional fall-through */

	/* Not implemented */
	case WRITE:
		/* Intentional fall-through */

	default:
		smc_ret = SMC_UNK;
		smc_resp = 0;
	}

	spin_unlock(&debugfs_access_lock);

	SMC_RET2(handle, smc_ret, smc_resp);

	/* Not reached */
	return smc_ret;
}

int debugfs_smc_setup(void)
{
	debugfs_initialized = false;
	debugfs_access_lock.lock = 0;

	return 0;
}
