/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <lib/debugfs.h>

#include "blobs.h"
#include "dev.h"

/*******************************************************************************
 * This array contains the directories available from the root directory.
 ******************************************************************************/
static const dirtab_t dirtab[] = {
	{"dev",   CHDIR | DEV_ROOT_QDEV,   0, O_READ},
	{"blobs", CHDIR | DEV_ROOT_QBLOBS, 0, O_READ},
	{"fip",   CHDIR | DEV_ROOT_QFIP,   0, O_READ}
};

static const dirtab_t devfstab[] = {
};

/*******************************************************************************
 * This function exposes the elements of the root directory.
 * It also exposes the content of the dev and blobs directories.
 ******************************************************************************/
static int rootgen(chan_t *channel, const dirtab_t *tab, int ntab,
		   int n, dir_t *dir)
{
	switch (channel->qid & ~CHDIR) {
	case DEV_ROOT_QROOT:
		tab = dirtab;
		ntab = NELEM(dirtab);
		break;
	case DEV_ROOT_QDEV:
		tab = devfstab;
		ntab = NELEM(devfstab);
		break;
	case DEV_ROOT_QBLOBS:
		tab = blobtab;
		ntab = NELEM(blobtab);
		break;
	default:
		return 0;
	}

	return devgen(channel, tab, ntab, n, dir);
}

static int rootwalk(chan_t *channel, const char *name)
{
	return devwalk(channel, name, NULL, 0, rootgen);
}

/*******************************************************************************
 * This function copies at most n bytes from the element referred by c into buf.
 ******************************************************************************/
static int rootread(chan_t *channel, void *buf, int size)
{
	const dirtab_t *dp;
	dir_t *dir;

	if ((channel->qid & CHDIR) != 0) {
		if (size < sizeof(dir_t)) {
			return -1;
		}

		dir = buf;
		return dirread(channel, dir, NULL, 0, rootgen);
	}

	/* Only makes sense when using debug language */
	assert(channel->qid != DEV_ROOT_QBLOBCTL);

	dp = &blobtab[channel->qid - DEV_ROOT_QBLOBCTL];
	return buf_to_channel(channel, buf, dp->data, size, dp->length);
}

static int rootstat(chan_t *channel, const char *file, dir_t *dir)
{
	return devstat(channel, file, dir, NULL, 0, rootgen);
}

const dev_t rootdevtab = {
	.id = '/',
	.stat = rootstat,
	.clone = devclone,
	.attach = devattach,
	.walk = rootwalk,
	.read = rootread,
	.write = deverrwrite,
	.mount = deverrmount,
	.seek = devseek
};
