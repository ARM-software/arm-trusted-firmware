/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEV_H
#define DEV_H

#include <cdefs.h>
#include <lib/debugfs.h>
#include <stddef.h>

/* FIXME: need configurability */
#define NR_CHANS  10
#define NR_CONSS  1
#define NR_BINDS  4
#define NR_FILES  18

#define NODEV     255
#define CHDIR     (1 << 15)

#define SYNCDEV   0
#define SYNCALL   1

typedef struct dev dev_t;
typedef struct chan chan_t;
typedef struct dirtab dirtab_t;
typedef int devgen_t(chan_t *, const dirtab_t *, int, int, dir_t *);
typedef struct attr attr_t;

enum {
	DEV_ROOT_QROOT,
	DEV_ROOT_QDEV,
	DEV_ROOT_QFIP,
	DEV_ROOT_QBLOBS,
	DEV_ROOT_QBLOBCTL,
	DEV_ROOT_QPSCI
};

/*******************************************************************************
 * This structure contains the necessary information to represent a directory
 * of the filesystem.
 ******************************************************************************/
struct dirtab {
	char		name[NAMELEN];
	qid_t		qid;
	long		length;
	unsigned char	perm;
	void		*data;
};

/*******************************************************************************
 * This structure defines the interface of device drivers.
 * Each driver must implement a subset of those functions.
 * It is possible to redirect to default implementations defined in dev.c.
 ******************************************************************************/
/* FIXME: comments for the callbacks */
struct dev {
	char id;
	int (*stat)(chan_t *c, const char *file, dir_t *dir);
	int (*walk)(chan_t *c, const char *name);
	int (*read)(chan_t *c, void *buf, int n);
	int (*write)(chan_t *c, void *buf, int n);
	int (*seek)(chan_t *c, long off, int whence);
	chan_t *(*clone)(chan_t *c, chan_t *nc);
	chan_t *(*attach)(int id, int dev);
	chan_t *(*mount)(chan_t *c, const char *spec);
};

/*******************************************************************************
 * This structure defines the channel structure.
 * A channel is a handle on an element of the filesystem.
 ******************************************************************************/
struct chan {
	long		offset;
	qid_t		qid;
	unsigned char	index;	/* device index in devtab */
	unsigned char	dev;
	unsigned char	mode;
};

/*******************************************************************************
 * This structure defines an abstract argument passed to physical drivers from
 * the configuration file.
 ******************************************************************************/
struct attr {
	char	*key;
	char	*value;
};

chan_t *path_to_channel(const char *path, int mode);
chan_t *clone(chan_t *c, chan_t *nc);
chan_t *attach(int id, int dev);
void channel_close(chan_t *c);
int buf_to_channel(chan_t *c, void *dst, void *src, int nbytes, long len);
int dirread(chan_t *c, dir_t *dir, const dirtab_t *tab,
	    int ntab, devgen_t *gen);
void make_dir_entry(chan_t *c, dir_t *dir, const char *name, long length,
		    qid_t qid, unsigned int mode);
void devlink(void);

chan_t *devattach(int id, int dev);
int devseek(chan_t *c, long off, int whence);
chan_t *devclone(chan_t *c, chan_t *nc);
int devgen(chan_t *c, const dirtab_t *tab, int ntab, int n, dir_t *dir);
int devwalk(chan_t *c, const char *name, const dirtab_t *tab, int ntab,
		   devgen_t *gen);
int devstat(chan_t *dirc, const char *file, dir_t *dir,
		   const dirtab_t *tab, int ntab, devgen_t *gen);

chan_t *deverrmount(chan_t *c, const char *spec);
int deverrwrite(chan_t *c, void *buf, int n);
int deverrseek(chan_t *c, long off, int whence);

extern dev_t *const devtab[];

void __dead2 devpanic(const char *cause);

#endif /* DEV_H */
