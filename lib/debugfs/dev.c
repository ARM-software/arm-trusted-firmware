/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cdefs.h>
#include <common/debug.h>
#include <lib/debugfs.h>
#include <string.h>

#include "dev.h"

#define NR_MOUNT_POINTS		4

struct mount_point {
	chan_t	*new;
	chan_t	*old;
};

/* This array contains all the available channels of the filesystem.
 * A file descriptor is the index of a specific channel in this array.
 */
static chan_t fdset[NR_CHANS];

/* This array contains all the available mount points of the filesystem. */
static struct mount_point mount_points[NR_MOUNT_POINTS];

/* This variable stores the channel associated to the root directory. */
static chan_t slash_channel;

/* This function creates a channel from a device index and registers
 * it to fdset.
 */
static chan_t *create_new_channel(unsigned char index)
{
	chan_t *channel = NULL;
	int i;

	for (i = 0; i < NR_CHANS; i++) {
		if (fdset[i].index == NODEV) {
			channel = &fdset[i];
			channel->index = index;
			break;
		}
	}

	return channel;
}

/*******************************************************************************
 * This function returns a pointer to an existing channel in fdset from a file
 * descriptor.
 ******************************************************************************/
static chan_t *fd_to_channel(int fd)
{
	if ((fd < 0) || (fd >= NR_CHANS) || (fdset[fd].index == NODEV)) {
		return NULL;
	}

	return &fdset[fd];
}

/*******************************************************************************
 * This function returns a file descriptor from a channel.
 * The caller must be sure that the channel is registered in fdset.
 ******************************************************************************/
static int channel_to_fd(chan_t *channel)
{
	return (channel == NULL) ? -1 : (channel - fdset);
}

/*******************************************************************************
 * This function checks the validity of a mode.
 ******************************************************************************/
static bool is_valid_mode(int mode)
{
	if ((mode & O_READ) && (mode & (O_WRITE | O_RDWR))) {
		return false;
	}
	if ((mode & O_WRITE) && (mode & (O_READ | O_RDWR))) {
		return false;
	}
	if ((mode & O_RDWR) && (mode & (O_READ | O_WRITE))) {
		return false;
	}

	return true;
}

/*******************************************************************************
 * This function extracts the next part of the given path contained and puts it
 * in token. It returns a pointer to the remainder of the path.
 ******************************************************************************/
static const char *next(const char *path, char *token)
{
	int index;
	const char *cursor;

	while (*path == '/') {
		++path;
	}

	index = 0;
	cursor = path;
	if (*path != '\0') {
		while (*cursor != '/' && *cursor != '\0') {
			if (index == NAMELEN) {
				return NULL;
			}
			token[index++] = *cursor++;
		}
	}
	token[index] = '\0';

	return cursor;
}

/*******************************************************************************
 * This function returns the driver index in devtab of the driver
 * identified by id.
 ******************************************************************************/
static int get_device_index(int id)
{
	int index;
	dev_t * const *dp;

	for (index = 0, dp = devtab; *dp && (*dp)->id != id; ++dp) {
		index++;
	}

	if (*dp == NULL) {
		return -1;
	}

	return index;
}

/*******************************************************************************
 * This function clears a given channel fields
 ******************************************************************************/
static void channel_clear(chan_t *channel)
{
	channel->offset = 0;
	channel->qid    = 0;
	channel->index  = NODEV;
	channel->dev    = 0;
	channel->mode   = 0;
}

/*******************************************************************************
 * This function closes the channel pointed to by c.
 ******************************************************************************/
void channel_close(chan_t *channel)
{
	if (channel != NULL) {
		channel_clear(channel);
	}
}

/*******************************************************************************
 * This function copies data from src to dst after applying the offset of the
 * channel c. nbytes bytes are expected to be copied unless the data goes over
 * dst + len.
 * It returns the actual number of bytes that were copied.
 ******************************************************************************/
int buf_to_channel(chan_t *channel, void *dst, void *src, int nbytes, long len)
{
	const char *addr = src;

	if ((channel == NULL) || (dst == NULL) || (src == NULL)) {
		return 0;
	}

	if (channel->offset >= len) {
		return 0;
	}

	if ((channel->offset + nbytes) > len) {
		nbytes = len - channel->offset;
	}

	memcpy(dst, addr + channel->offset, nbytes);

	channel->offset += nbytes;

	return nbytes;
}

/*******************************************************************************
 * This function checks whether a channel (identified by its device index and
 * qid) is registered as a mount point.
 * Returns a pointer to the channel it is mounted to when found, NULL otherwise.
 ******************************************************************************/
static chan_t *mount_point_to_channel(int index, qid_t qid)
{
	chan_t *channel;
	struct mount_point *mp;

	for (mp = mount_points; mp < &mount_points[NR_MOUNT_POINTS]; mp++) {
		channel = mp->new;
		if (channel == NULL) {
			continue;
		}

		if ((channel->index == index) && (channel->qid == qid)) {
			return mp->old;
		}
	}

	return NULL;
}

/*******************************************************************************
 * This function calls the attach function of the driver identified by id.
 ******************************************************************************/
chan_t *attach(int id, int dev)
{
	/* Get the devtab index for the driver identified by id */
	int index = get_device_index(id);

	if (index < 0) {
		return NULL;
	}

	return devtab[index]->attach(id, dev);
}

/*******************************************************************************
 * This function is the default implementation of the driver attach function.
 * It creates a new channel and returns a pointer to it.
 ******************************************************************************/
chan_t *devattach(int id, int dev)
{
	chan_t *channel;
	int index;

	index = get_device_index(id);
	if (index < 0) {
		return NULL;
	}

	channel = create_new_channel(index);
	if (channel == NULL) {
		return NULL;
	}

	channel->dev = dev;
	channel->qid = CHDIR;

	return channel;
}

/*******************************************************************************
 * This function returns a channel given a path.
 * It goes through the filesystem, from the root namespace ('/') or from a
 * device namespace ('#'), switching channel on mount points.
 ******************************************************************************/
chan_t *path_to_channel(const char *path, int mode)
{
	int i, n;
	const char *path_next;
	chan_t *mnt, *channel;
	char elem[NAMELEN];

	if (path == NULL) {
		return NULL;
	}

	switch (path[0]) {
	case '/':
		channel = clone(&slash_channel, NULL);
		path_next = path;
		break;
	case '#':
		path_next = next(path + 1, elem);
		if (path_next == NULL) {
			goto noent;
		}

		n = 0;
		for (i = 1; (elem[i] >= '0') && (elem[i] <= '9'); i++) {
			n += elem[i] - '0';
		}

		if (elem[i] != '\0') {
			goto noent;
		}

		channel = attach(elem[0], n);
		break;
	default:
		return NULL;
	}

	if (channel == NULL) {
		return NULL;
	}

	for (path_next = next(path_next, elem); *elem;
			path_next = next(path_next, elem)) {
		if ((channel->qid & CHDIR) == 0) {
			goto notfound;
		}

		if (devtab[channel->index]->walk(channel, elem) < 0) {
			channel_close(channel);
			goto notfound;
		}

		mnt = mount_point_to_channel(channel->index, channel->qid);
		if (mnt != NULL) {
			clone(mnt, channel);
		}
	}

	if (path_next == NULL) {
		goto notfound;
	}

	/* TODO: check mode */
	return channel;

notfound:
	channel_close(channel);
noent:
	return NULL;
}

/*******************************************************************************
 * This function calls the clone function of the driver associated to the
 * channel c.
 ******************************************************************************/
chan_t *clone(chan_t *c, chan_t *nc)
{
	if (c->index == NODEV) {
		return NULL;
	}

	return devtab[c->index]->clone(c, nc);
}

/*******************************************************************************
 * This function is the default implementation of the driver clone function.
 * It creates a new channel and returns a pointer to it.
 * It clones channel into new_channel.
 ******************************************************************************/
chan_t *devclone(chan_t *channel, chan_t *new_channel)
{
	if (channel == NULL) {
		return NULL;
	}

	if (new_channel == NULL) {
		new_channel = create_new_channel(channel->index);
		if (new_channel == NULL) {
			return NULL;
		}
	}

	new_channel->qid    = channel->qid;
	new_channel->dev    = channel->dev;
	new_channel->mode   = channel->mode;
	new_channel->offset = channel->offset;
	new_channel->index  = channel->index;

	return new_channel;
}

/*******************************************************************************
 * This function is the default implementation of the driver walk function.
 * It goes through all the elements of tab using the gen function until a match
 * is found with name.
 * If a match is found, it copies the qid of the new directory.
 ******************************************************************************/
int devwalk(chan_t *channel, const char *name, const dirtab_t *tab,
	    int ntab, devgen_t *gen)
{
	int i;
	dir_t dir;

	if ((channel == NULL) || (name == NULL) || (gen == NULL)) {
		return -1;
	}

	if ((name[0] == '.') && (name[1] == '\0')) {
		return 1;
	}

	for (i = 0; ; i++) {
		switch ((*gen)(channel, tab, ntab, i, &dir)) {
		case 0:
			/* Intentional fall-through */
		case -1:
			return -1;
		case 1:
			if (strncmp(name, dir.name, NAMELEN) != 0) {
				continue;
			}
			channel->qid = dir.qid;
			return 1;
		}
	}
}

/*******************************************************************************
 * This is a helper function which exposes the content of a directory, element
 * by element. It is meant to be called until the end of the directory is
 * reached or an error occurs.
 * It returns -1 on error, 0 on end of directory and 1 when a new file is found.
 ******************************************************************************/
int dirread(chan_t *channel, dir_t *dir, const dirtab_t *tab,
	int ntab, devgen_t *gen)
{
	int i, ret;

	if ((channel == NULL) || (dir == NULL) || (gen == NULL)) {
		return -1;
	}

	i = channel->offset/sizeof(dir_t);
	ret = (*gen)(channel, tab, ntab, i, dir);
	if (ret == 1) {
		channel->offset += sizeof(dir_t);
	}

	return ret;
}

/*******************************************************************************
 * This function sets the elements of dir.
 ******************************************************************************/
void make_dir_entry(chan_t *channel, dir_t *dir,
	     const char *name, long length, qid_t qid, unsigned int mode)
{
	if ((channel == NULL) || (dir == NULL) || (name == NULL)) {
		return;
	}

	strlcpy(dir->name, name, sizeof(dir->name));
	dir->length = length;
	dir->qid = qid;
	dir->mode = mode;

	if ((qid & CHDIR) != 0) {
		dir->mode |= O_DIR;
	}

	dir->index = channel->index;
	dir->dev   = channel->dev;
}

/*******************************************************************************
 * This function is the default implementation of the internal driver gen
 * function.
 * It copies and formats the information of the nth element of tab into dir.
 ******************************************************************************/
int devgen(chan_t *channel, const dirtab_t *tab, int ntab, int n, dir_t *dir)
{
	const dirtab_t *dp;

	if ((channel == NULL) || (dir == NULL) || (tab == NULL) ||
			(n >= ntab)) {
		return 0;
	}

	dp = &tab[n];
	make_dir_entry(channel, dir, dp->name, dp->length, dp->qid, dp->perm);
	return 1;
}

/*******************************************************************************
 * This function returns a file descriptor identifying the channel associated to
 * the given path.
 ******************************************************************************/
int open(const char *path, int mode)
{
	chan_t *channel;

	if (path == NULL) {
		return -1;
	}

	if (is_valid_mode(mode) == false) {
		return -1;
	}

	channel = path_to_channel(path, mode);

	return channel_to_fd(channel);
}

/*******************************************************************************
 * This function closes the channel identified by the file descriptor fd.
 ******************************************************************************/
int close(int fd)
{
	chan_t *channel;

	channel = fd_to_channel(fd);
	if (channel == NULL) {
		return -1;
	}

	channel_close(channel);
	return 0;
}

/*******************************************************************************
 * This function is the default implementation of the driver stat function.
 * It goes through all the elements of tab using the gen function until a match
 * is found with file.
 * If a match is found, dir contains the information file.
 ******************************************************************************/
int devstat(chan_t *dirc, const char *file, dir_t *dir,
	    const dirtab_t *tab, int ntab, devgen_t *gen)
{
	int i, r = 0;
	chan_t *c, *mnt;

	if ((dirc == NULL) || (dir == NULL) || (gen == NULL)) {
		return -1;
	}

	c = path_to_channel(file, O_STAT);
	if (c == NULL) {
		return -1;
	}

	for (i = 0; ; i++) {
		switch ((*gen)(dirc, tab, ntab, i, dir)) {
		case 0:
			/* Intentional fall-through */
		case -1:
			r = -1;
			goto leave;
		case 1:
			mnt = mount_point_to_channel(dir->index, dir->qid);
			if (mnt != NULL) {
				dir->qid = mnt->qid;
				dir->index = mnt->index;
			}

			if ((dir->qid != c->qid) || (dir->index != c->index)) {
				continue;
			}

			goto leave;
		}
	}

leave:
	channel_close(c);
	return r;
}

/*******************************************************************************
 * This function calls the stat function of the driver associated to the parent
 * directory of the file in path.
 * The result is stored in dir.
 ******************************************************************************/
int stat(const char *path, dir_t *dir)
{
	int r;
	size_t len;
	chan_t *channel;
	char *p, dirname[PATHLEN];

	if ((path == NULL) || (dir == NULL)) {
		return -1;
	}

	len = strlen(path);
	if ((len + 1) > sizeof(dirname)) {
		return -1;
	}

	memcpy(dirname, path, len);
	for (p = dirname + len; p > dirname; --p) {
		if (*p != '/') {
			break;
		}
	}

	p = memrchr(dirname, '/', p - dirname);
	if (p == NULL) {
		return -1;
	}

	dirname[p - dirname + 1] = '\0';

	channel = path_to_channel(dirname, O_STAT);
	if (channel == NULL) {
		return -1;
	}

	r = devtab[channel->index]->stat(channel, path, dir);
	channel_close(channel);

	return r;
}

/*******************************************************************************
 * This function calls the read function of the driver associated to fd.
 * It fills buf with at most n bytes.
 * It returns the number of bytes that were actually read.
 ******************************************************************************/
int read(int fd, void *buf, int n)
{
	chan_t *channel;

	if (buf == NULL) {
		return -1;
	}

	channel = fd_to_channel(fd);
	if (channel == NULL) {
		return -1;
	}

	if (((channel->qid & CHDIR) != 0) && (n < sizeof(dir_t))) {
		return -1;
	}

	return devtab[channel->index]->read(channel, buf, n);
}

/*******************************************************************************
 * This function calls the write function of the driver associated to fd.
 * It writes at most n bytes of buf.
 * It returns the number of bytes that were actually written.
 ******************************************************************************/
int write(int fd, void *buf, int n)
{
	chan_t *channel;

	if (buf == NULL) {
		return -1;
	}

	channel = fd_to_channel(fd);
	if (channel == NULL) {
		return -1;
	}

	if ((channel->qid & CHDIR) != 0) {
		return -1;
	}

	return devtab[channel->index]->write(channel, buf, n);
}

/*******************************************************************************
 * This function calls the seek function of the driver associated to fd.
 * It applies the offset off according to the strategy whence.
 ******************************************************************************/
int seek(int fd, long off, int whence)
{
	chan_t *channel;

	channel = fd_to_channel(fd);
	if (channel == NULL) {
		return -1;
	}

	if ((channel->qid & CHDIR) != 0) {
		return -1;
	}

	return devtab[channel->index]->seek(channel, off, whence);
}

/*******************************************************************************
 * This function is the default error implementation of the driver mount
 * function.
 ******************************************************************************/
chan_t *deverrmount(chan_t *channel, const char *spec)
{
	return NULL;
}

/*******************************************************************************
 * This function is the default error implementation of the driver write
 * function.
 ******************************************************************************/
int deverrwrite(chan_t *channel, void *buf, int n)
{
	return -1;
}

/*******************************************************************************
 * This function is the default error implementation of the driver seek
 * function.
 ******************************************************************************/
int deverrseek(chan_t *channel, long off, int whence)
{
	return -1;
}

/*******************************************************************************
 * This function is the default implementation of the driver seek function.
 * It applies the offset off according to the strategy whence to the channel c.
 ******************************************************************************/
int devseek(chan_t *channel, long off, int whence)
{
	switch (whence) {
	case KSEEK_SET:
		channel->offset = off;
		break;
	case KSEEK_CUR:
		channel->offset += off;
		break;
	case KSEEK_END:
		/* Not implemented */
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * This function registers the channel associated to the path new as a mount
 * point for the channel c.
 ******************************************************************************/
static int add_mount_point(chan_t *channel, const char *new)
{
	int i;
	chan_t *cn;
	struct mount_point *mp;

	if (new == NULL) {
		goto err0;
	}

	cn = path_to_channel(new, O_READ);
	if (cn == NULL) {
		goto err0;
	}

	if ((cn->qid & CHDIR) == 0) {
		goto err1;
	}

	for (i = NR_MOUNT_POINTS - 1; i >= 0; i--) {
		mp = &mount_points[i];
		if (mp->new == NULL) {
			break;
		}
	}

	if (i < 0) {
		goto err1;
	}

	mp->new = cn;
	mp->old = channel;

	return 0;

err1:
	channel_close(cn);
err0:
	return -1;
}

/*******************************************************************************
 * This function registers the path new as a mount point for the path old.
 ******************************************************************************/
int bind(const char *old, const char *new)
{
	chan_t *channel;

	channel = path_to_channel(old, O_BIND);
	if (channel == NULL) {
		return -1;
	}

	if (add_mount_point(channel, new) < 0) {
		channel_close(channel);
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * This function calls the mount function of the driver associated to the path
 * srv.
 * It mounts the path srv on the path where.
 ******************************************************************************/
int mount(const char *srv, const char *where, const char *spec)
{
	chan_t *channel, *mount_point_chan;
	int ret;

	channel = path_to_channel(srv, O_RDWR);
	if (channel == NULL) {
		goto err0;
	}

	mount_point_chan = devtab[channel->index]->mount(channel, spec);
	if (mount_point_chan == NULL) {
		goto err1;
	}

	ret = add_mount_point(mount_point_chan, where);
	if (ret < 0) {
		goto err2;
	}

	channel_close(channel);

	return 0;

err2:
	channel_close(mount_point_chan);
err1:
	channel_close(channel);
err0:
	return -1;
}

/*******************************************************************************
 * This function initializes the device environment.
 * It creates the '/' channel.
 * It links the device drivers to the physical drivers.
 ******************************************************************************/
void debugfs_init(void)
{
	chan_t *channel, *cloned_channel;

	for (channel = fdset; channel < &fdset[NR_CHANS]; channel++) {
		channel_clear(channel);
	}

	channel = devattach('/', 0);
	if (channel == NULL) {
		panic();
	}

	cloned_channel = clone(channel, &slash_channel);
	if (cloned_channel == NULL) {
		panic();
	}

	channel_close(channel);
	devlink();
}

__dead2 void devpanic(const char *cause)
{
	panic();
}
