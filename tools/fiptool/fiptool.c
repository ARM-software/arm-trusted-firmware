/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fiptool.h"
#include "tbbr_config.h"

#define OPT_TOC_ENTRY 0
#define OPT_PLAT_TOC_FLAGS 1
#define OPT_ALIGN 2

static int info_cmd(int argc, char *argv[]);
static void info_usage(int);
static int create_cmd(int argc, char *argv[]);
static void create_usage(int);
static int update_cmd(int argc, char *argv[]);
static void update_usage(int);
static int unpack_cmd(int argc, char *argv[]);
static void unpack_usage(int);
static int remove_cmd(int argc, char *argv[]);
static void remove_usage(int);
static int version_cmd(int argc, char *argv[]);
static void version_usage(int);
static int help_cmd(int argc, char *argv[]);
static void usage(void);

/* Available subcommands. */
static cmd_t cmds[] = {
	{ .name = "info",    .handler = info_cmd,    .usage = info_usage    },
	{ .name = "create",  .handler = create_cmd,  .usage = create_usage  },
	{ .name = "update",  .handler = update_cmd,  .usage = update_usage  },
	{ .name = "unpack",  .handler = unpack_cmd,  .usage = unpack_usage  },
	{ .name = "remove",  .handler = remove_cmd,  .usage = remove_usage  },
	{ .name = "version", .handler = version_cmd, .usage = version_usage },
	{ .name = "help",    .handler = help_cmd,    .usage = NULL          },
};

static image_desc_t *image_desc_head;
static size_t nr_image_descs;
static const uuid_t uuid_null;
static int verbose;

static void vlog(int prio, const char *msg, va_list ap)
{
	char *prefix[] = { "DEBUG", "WARN", "ERROR" };

	fprintf(stderr, "%s: ", prefix[prio]);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
}

static void log_dbgx(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_DBG, msg, ap);
	va_end(ap);
}

static void log_warnx(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_WARN, msg, ap);
	va_end(ap);
}

static void log_err(const char *msg, ...)
{
	char buf[512];
	va_list ap;

	va_start(ap, msg);
	snprintf(buf, sizeof(buf), "%s: %s", msg, strerror(errno));
	vlog(LOG_ERR, buf, ap);
	va_end(ap);
	exit(1);
}

static void log_errx(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_ERR, msg, ap);
	va_end(ap);
	exit(1);
}

static char *xstrdup(const char *s, const char *msg)
{
	char *d;

	d = strdup(s);
	if (d == NULL)
		log_errx("strdup: %s", msg);
	return d;
}

static void *xmalloc(size_t size, const char *msg)
{
	void *d;

	d = malloc(size);
	if (d == NULL)
		log_errx("malloc: %s", msg);
	return d;
}

static void *xzalloc(size_t size, const char *msg)
{
	return memset(xmalloc(size, msg), 0, size);
}

static void xfwrite(void *buf, size_t size, FILE *fp, const char *filename)
{
	if (fwrite(buf, 1, size, fp) != size)
		log_errx("Failed to write %s", filename);
}

static image_desc_t *new_image_desc(const uuid_t *uuid,
    const char *name, const char *cmdline_name)
{
	image_desc_t *desc;

	desc = xzalloc(sizeof(*desc),
	    "failed to allocate memory for image descriptor");
	memcpy(&desc->uuid, uuid, sizeof(uuid_t));
	desc->name = xstrdup(name,
	    "failed to allocate memory for image name");
	desc->cmdline_name = xstrdup(cmdline_name,
	    "failed to allocate memory for image command line name");
	desc->action = DO_UNSPEC;
	return desc;
}

static void set_image_desc_action(image_desc_t *desc, int action,
    const char *arg)
{
	assert(desc != NULL);

	if (desc->action_arg != (char *)DO_UNSPEC)
		free(desc->action_arg);
	desc->action = action;
	desc->action_arg = NULL;
	if (arg != NULL)
		desc->action_arg = xstrdup(arg,
		    "failed to allocate memory for argument");
}

static void free_image_desc(image_desc_t *desc)
{
	free(desc->name);
	free(desc->cmdline_name);
	free(desc->action_arg);
	if (desc->image) {
		free(desc->image->buffer);
		free(desc->image);
	}
	free(desc);
}

static void add_image_desc(image_desc_t *desc)
{
	image_desc_t **p = &image_desc_head;

	while (*p)
		p = &(*p)->next;

	assert(*p == NULL);
	*p = desc;
	nr_image_descs++;
}

static void free_image_descs(void)
{
	image_desc_t *desc = image_desc_head, *tmp;

	while (desc != NULL) {
		tmp = desc->next;
		free_image_desc(desc);
		desc = tmp;
		nr_image_descs--;
	}
	assert(nr_image_descs == 0);
}

static void fill_image_descs(void)
{
	toc_entry_t *toc_entry;

	for (toc_entry = toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		image_desc_t *desc;

		desc = new_image_desc(&toc_entry->uuid,
		    toc_entry->name,
		    toc_entry->cmdline_name);
		add_image_desc(desc);
	}
#ifdef PLAT_DEF_FIP_UUID
	for (toc_entry = plat_def_toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		image_desc_t *desc;

		desc = new_image_desc(&toc_entry->uuid,
		    toc_entry->name,
		    toc_entry->cmdline_name);
		add_image_desc(desc);
	}
#endif
}

static image_desc_t *lookup_image_desc_from_uuid(const uuid_t *uuid)
{
	image_desc_t *desc;

	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (memcmp(&desc->uuid, uuid, sizeof(uuid_t)) == 0)
			return desc;
	return NULL;
}

static image_desc_t *lookup_image_desc_from_opt(const char *opt)
{
	image_desc_t *desc;

	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (strcmp(desc->cmdline_name, opt) == 0)
			return desc;
	return NULL;
}

static void uuid_to_str(char *s, size_t len, const uuid_t *u)
{
	assert(len >= (_UUID_STR_LEN + 1));

	snprintf(s, len,
	    "%02X%02X%02X%02X-%02X%02X-%02X%02X-%04X-%04X%04X%04X",
	    u->time_low[0], u->time_low[1], u->time_low[2], u->time_low[3],
	    u->time_mid[0], u->time_mid[1],
	    u->time_hi_and_version[0], u->time_hi_and_version[1],
	    (u->clock_seq_hi_and_reserved << 8) | u->clock_seq_low,
	    (u->node[0] << 8) | u->node[1],
	    (u->node[2] << 8) | u->node[3],
	    (u->node[4] << 8) | u->node[5]);
}

static void uuid_from_str(uuid_t *u, const char *s)
{
	int n;

	if (s == NULL)
		log_errx("UUID cannot be NULL");
	if (strlen(s) != _UUID_STR_LEN)
		log_errx("Invalid UUID: %s", s);

	n = sscanf(s,
	    "%2hhx%2hhx%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
	    &u->time_low[0], &u->time_low[1], &u->time_low[2], &u->time_low[3],
	    &u->time_mid[0], &u->time_mid[1],
	    &u->time_hi_and_version[0], &u->time_hi_and_version[1],
	    &u->clock_seq_hi_and_reserved, &u->clock_seq_low,
	    &u->node[0], &u->node[1],
	    &u->node[2], &u->node[3],
	    &u->node[4], &u->node[5]);
	/*
	 * Given the format specifier above, we expect 16 items to be scanned
	 * for a properly formatted UUID.
	 */
	if (n != 16)
		log_errx("Invalid UUID: %s", s);
}

static int parse_fip(const char *filename, fip_toc_header_t *toc_header_out)
{
	struct BLD_PLAT_STAT st;
	FILE *fp;
	char *buf, *bufend;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	int terminated = 0;

	fp = fopen(filename, "rb");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (fstat(fileno(fp), &st) == -1)
		log_err("fstat %s", filename);

	buf = xmalloc(st.st_size, "failed to load file into memory");
	if (fread(buf, 1, st.st_size, fp) != st.st_size)
		log_errx("Failed to read %s", filename);
	bufend = buf + st.st_size;
	fclose(fp);

	if (st.st_size < sizeof(fip_toc_header_t))
		log_errx("FIP %s is truncated", filename);

	toc_header = (fip_toc_header_t *)buf;
	toc_entry = (fip_toc_entry_t *)(toc_header + 1);

	if (toc_header->name != TOC_HEADER_NAME)
		log_errx("%s is not a FIP file", filename);

	/* Return the ToC header if the caller wants it. */
	if (toc_header_out != NULL)
		*toc_header_out = *toc_header;

	/* Walk through each ToC entry in the file. */
	while ((char *)toc_entry + sizeof(*toc_entry) - 1 < bufend) {
		image_t *image;
		image_desc_t *desc;

		/* Found the ToC terminator, we are done. */
		if (memcmp(&toc_entry->uuid, &uuid_null, sizeof(uuid_t)) == 0) {
			terminated = 1;
			break;
		}

		/*
		 * Build a new image out of the ToC entry and add it to the
		 * table of images.
		 */
		image = xzalloc(sizeof(*image),
		    "failed to allocate memory for image");
		image->toc_e = *toc_entry;
		image->buffer = xmalloc(toc_entry->size,
		    "failed to allocate image buffer, is FIP file corrupted?");
		/* Overflow checks before memory copy. */
		if (toc_entry->size > (uint64_t)-1 - toc_entry->offset_address)
			log_errx("FIP %s is corrupted", filename);
		if (toc_entry->size + toc_entry->offset_address > st.st_size)
			log_errx("FIP %s is corrupted", filename);

		memcpy(image->buffer, buf + toc_entry->offset_address,
		    toc_entry->size);

		/* If this is an unknown image, create a descriptor for it. */
		desc = lookup_image_desc_from_uuid(&toc_entry->uuid);
		if (desc == NULL) {
			char name[_UUID_STR_LEN + 1], filename[PATH_MAX];

			uuid_to_str(name, sizeof(name), &toc_entry->uuid);
			snprintf(filename, sizeof(filename), "%s%s",
			    name, ".bin");
			desc = new_image_desc(&toc_entry->uuid, name, "blob");
			desc->action = DO_UNPACK;
			desc->action_arg = xstrdup(filename,
			    "failed to allocate memory for blob filename");
			add_image_desc(desc);
		}

		assert(desc->image == NULL);
		desc->image = image;

		toc_entry++;
	}

	if (terminated == 0)
		log_errx("FIP %s does not have a ToC terminator entry",
		    filename);
	free(buf);
	return 0;
}

static image_t *read_image_from_file(const uuid_t *uuid, const char *filename)
{
	struct BLD_PLAT_STAT st;
	image_t *image;
	FILE *fp;

	assert(uuid != NULL);
	assert(filename != NULL);

	fp = fopen(filename, "rb");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (fstat(fileno(fp), &st) == -1)
		log_errx("fstat %s", filename);

	image = xzalloc(sizeof(*image), "failed to allocate memory for image");
	image->toc_e.uuid = *uuid;
	image->buffer = xmalloc(st.st_size, "failed to allocate image buffer");
	if (fread(image->buffer, 1, st.st_size, fp) != st.st_size)
		log_errx("Failed to read %s", filename);
	image->toc_e.size = st.st_size;

	fclose(fp);
	return image;
}

static int write_image_to_file(const image_t *image, const char *filename)
{
	FILE *fp;

	fp = fopen(filename, "wb");
	if (fp == NULL)
		log_err("fopen");
	xfwrite(image->buffer, image->toc_e.size, fp, filename);
	fclose(fp);
	return 0;
}

static struct option *add_opt(struct option *opts, size_t *nr_opts,
    const char *name, int has_arg, int val)
{
	opts = realloc(opts, (*nr_opts + 1) * sizeof(*opts));
	if (opts == NULL)
		log_err("realloc");
	opts[*nr_opts].name = name;
	opts[*nr_opts].has_arg = has_arg;
	opts[*nr_opts].flag = NULL;
	opts[*nr_opts].val = val;
	++*nr_opts;
	return opts;
}

static struct option *fill_common_opts(struct option *opts, size_t *nr_opts,
    int has_arg)
{
	image_desc_t *desc;

	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		opts = add_opt(opts, nr_opts, desc->cmdline_name, has_arg,
		    OPT_TOC_ENTRY);
	return opts;
}

static void md_print(const unsigned char *md, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		printf("%02x", md[i]);
}

static int info_cmd(int argc, char *argv[])
{
	image_desc_t *desc;
	fip_toc_header_t toc_header;

	if (argc != 2)
		info_usage(EXIT_FAILURE);
	argc--, argv++;

	parse_fip(argv[0], &toc_header);

	if (verbose) {
		log_dbgx("toc_header[name]: 0x%llX",
		    (unsigned long long)toc_header.name);
		log_dbgx("toc_header[serial_number]: 0x%llX",
		    (unsigned long long)toc_header.serial_number);
		log_dbgx("toc_header[flags]: 0x%llX",
		    (unsigned long long)toc_header.flags);
	}

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;

		if (image == NULL)
			continue;
		printf("%s: offset=0x%llX, size=0x%llX, cmdline=\"--%s\"",
		       desc->name,
		       (unsigned long long)image->toc_e.offset_address,
		       (unsigned long long)image->toc_e.size,
		       desc->cmdline_name);
#ifndef _MSC_VER	/* We don't have SHA256 for Visual Studio. */
		if (verbose) {
			unsigned char md[SHA256_DIGEST_LENGTH];

			SHA256(image->buffer, image->toc_e.size, md);
			printf(", sha256=");
			md_print(md, sizeof(md));
		}
#endif
		putchar('\n');
	}

	return 0;
}

static void info_usage(int exit_status)
{
	printf("fiptool info FIP_FILENAME\n");
	exit(exit_status);
}

static int pack_images(const char *filename, uint64_t toc_flags, unsigned long align)
{
	FILE *fp;
	image_desc_t *desc;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	char *buf;
	uint64_t entry_offset, buf_size, payload_size = 0, pad_size;
	size_t nr_images = 0;

	for (desc = image_desc_head; desc != NULL; desc = desc->next)
		if (desc->image != NULL)
			nr_images++;

	buf_size = sizeof(fip_toc_header_t) +
	    sizeof(fip_toc_entry_t) * (nr_images + 1);
	buf = calloc(1, buf_size);
	if (buf == NULL)
		log_err("calloc");

	/* Build up header and ToC entries from the image table. */
	toc_header = (fip_toc_header_t *)buf;
	toc_header->name = TOC_HEADER_NAME;
	toc_header->serial_number = TOC_HEADER_SERIAL_NUMBER;
	toc_header->flags = toc_flags;

	toc_entry = (fip_toc_entry_t *)(toc_header + 1);

	entry_offset = buf_size;
	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;

		if (image == NULL || (image->toc_e.size == 0ULL))
			continue;
		payload_size += image->toc_e.size;
		entry_offset = (entry_offset + align - 1) & ~(align - 1);
		image->toc_e.offset_address = entry_offset;
		*toc_entry++ = image->toc_e;
		entry_offset += image->toc_e.size;
	}

	/*
	 * Append a null uuid entry to mark the end of ToC entries.
	 * NOTE the offset address for the last toc_entry must match the fip
	 * size.
	 */
	memset(toc_entry, 0, sizeof(*toc_entry));
	toc_entry->offset_address = (entry_offset + align - 1) & ~(align - 1);

	/* Generate the FIP file. */
	fp = fopen(filename, "wb");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (verbose)
		log_dbgx("Metadata size: %zu bytes", buf_size);

	xfwrite(buf, buf_size, fp, filename);

	if (verbose)
		log_dbgx("Payload size: %zu bytes", payload_size);

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image = desc->image;

		if (image == NULL)
			continue;
		if (fseek(fp, image->toc_e.offset_address, SEEK_SET))
			log_errx("Failed to set file position");

		xfwrite(image->buffer, image->toc_e.size, fp, filename);
	}

	if (fseek(fp, entry_offset, SEEK_SET))
		log_errx("Failed to set file position");

	pad_size = toc_entry->offset_address - entry_offset;
	while (pad_size--)
		fputc(0x0, fp);

	free(buf);
	fclose(fp);
	return 0;
}

/*
 * This function is shared between the create and update subcommands.
 * The difference between the two subcommands is that when the FIP file
 * is created, the parsing of an existing FIP is skipped.  This results
 * in update_fip() creating the new FIP file from scratch because the
 * internal image table is not populated.
 */
static void update_fip(void)
{
	image_desc_t *desc;

	/* Add or replace images in the FIP file. */
	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		image_t *image;

		if (desc->action != DO_PACK)
			continue;

		image = read_image_from_file(&desc->uuid,
		    desc->action_arg);
		if (desc->image != NULL) {
			if (verbose) {
				log_dbgx("Replacing %s with %s",
				    desc->cmdline_name,
				    desc->action_arg);
			}
			free(desc->image);
			desc->image = image;
		} else {
			if (verbose)
				log_dbgx("Adding image %s",
				    desc->action_arg);
			desc->image = image;
		}
	}
}

static void parse_plat_toc_flags(const char *arg, unsigned long long *toc_flags)
{
	unsigned long long flags;
	char *endptr;

	errno = 0;
	flags = strtoull(arg, &endptr, 16);
	if (*endptr != '\0' || flags > UINT16_MAX || errno != 0)
		log_errx("Invalid platform ToC flags: %s", arg);
	/* Platform ToC flags is a 16-bit field occupying bits [32-47]. */
	*toc_flags |= flags << 32;
}

static int is_power_of_2(unsigned long x)
{
	return x && !(x & (x - 1));
}

static unsigned long get_image_align(char *arg)
{
	char *endptr;
	unsigned long align;

	errno = 0;
	align = strtoul(arg, &endptr, 0);
	if (*endptr != '\0' || !is_power_of_2(align) || errno != 0)
		log_errx("Invalid alignment: %s", arg);

	return align;
}

static void parse_blob_opt(char *arg, uuid_t *uuid, char *filename, size_t len)
{
	char *p;

	for (p = strtok(arg, ","); p != NULL; p = strtok(NULL, ",")) {
		if (strncmp(p, "uuid=", strlen("uuid=")) == 0) {
			p += strlen("uuid=");
			uuid_from_str(uuid, p);
		} else if (strncmp(p, "file=", strlen("file=")) == 0) {
			p += strlen("file=");
			snprintf(filename, len, "%s", p);
		}
	}
}

static int create_cmd(int argc, char *argv[])
{
	struct option *opts = NULL;
	size_t nr_opts = 0;
	unsigned long long toc_flags = 0;
	unsigned long align = 1;

	if (argc < 2)
		create_usage(EXIT_FAILURE);

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while (1) {
		int c, opt_index = 0;

		c = getopt_long(argc, argv, "b:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_PACK, optarg);
			break;
		}
		case OPT_PLAT_TOC_FLAGS:
			parse_plat_toc_flags(optarg, &toc_flags);
			break;
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				create_usage(EXIT_FAILURE);

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_PACK, filename);
			break;
		}
		default:
			create_usage(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;
	free(opts);

	if (argc == 0)
		create_usage(EXIT_SUCCESS);

	update_fip();

	pack_images(argv[0], toc_flags, align);
	return 0;
}

static void create_usage(int exit_status)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool create [opts] FIP_FILENAME\n");
	printf("\n");
	printf("Options:\n");
	printf("  --align <value>\t\tEach image is aligned to <value> (default: 1).\n");
	printf("  --blob uuid=...,file=...\tAdd an image with the given UUID pointed to by file.\n");
	printf("  --plat-toc-flags <value>\t16-bit platform specific flag field occupying bits 32-47 in 64-bit ToC header.\n");
	printf("\n");
	printf("Specific images are packed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#ifdef PLAT_DEF_FIP_UUID
	toc_entry = plat_def_toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#endif
	exit(exit_status);
}

static int update_cmd(int argc, char *argv[])
{
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outfile[PATH_MAX] = { 0 };
	fip_toc_header_t toc_header = { 0 };
	unsigned long long toc_flags = 0;
	unsigned long align = 1;
	int pflag = 0;

	if (argc < 2)
		update_usage(EXIT_FAILURE);

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while (1) {
		int c, opt_index = 0;

		c = getopt_long(argc, argv, "b:o:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_PACK, optarg);
			break;
		}
		case OPT_PLAT_TOC_FLAGS:
			parse_plat_toc_flags(optarg, &toc_flags);
			pflag = 1;
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				update_usage(EXIT_FAILURE);

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_PACK, filename);
			break;
		}
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			update_usage(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;
	free(opts);

	if (argc == 0)
		update_usage(EXIT_SUCCESS);

	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);

	if (access(argv[0], F_OK) == 0)
		parse_fip(argv[0], &toc_header);

	if (pflag)
		toc_header.flags &= ~(0xffffULL << 32);
	toc_flags = (toc_header.flags |= toc_flags);

	update_fip();

	pack_images(outfile, toc_flags, align);
	return 0;
}

static void update_usage(int exit_status)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool update [opts] FIP_FILENAME\n");
	printf("\n");
	printf("Options:\n");
	printf("  --align <value>\t\tEach image is aligned to <value> (default: 1).\n");
	printf("  --blob uuid=...,file=...\tAdd or update an image with the given UUID pointed to by file.\n");
	printf("  --out FIP_FILENAME\t\tSet an alternative output FIP file.\n");
	printf("  --plat-toc-flags <value>\t16-bit platform specific flag field occupying bits 32-47 in 64-bit ToC header.\n");
	printf("\n");
	printf("Specific images are packed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#ifdef PLAT_DEF_FIP_UUID
	toc_entry = plat_def_toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#endif
	exit(exit_status);
}

static int unpack_cmd(int argc, char *argv[])
{
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outdir[PATH_MAX] = { 0 };
	image_desc_t *desc;
	int fflag = 0;
	int unpack_all = 1;

	if (argc < 2)
		unpack_usage(EXIT_FAILURE);

	opts = fill_common_opts(opts, &nr_opts, required_argument);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "force", no_argument, 'f');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while (1) {
		int c, opt_index = 0;

		c = getopt_long(argc, argv, "b:fo:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_UNPACK, optarg);
			unpack_all = 0;
			break;
		}
		case 'b': {
			char name[_UUID_STR_LEN + 1];
			char filename[PATH_MAX] = { 0 };
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0 ||
			    filename[0] == '\0')
				unpack_usage(EXIT_FAILURE);

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_UNPACK, filename);
			unpack_all = 0;
			break;
		}
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outdir, sizeof(outdir), "%s", optarg);
			break;
		default:
			unpack_usage(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;
	free(opts);

	if (argc == 0)
		unpack_usage(EXIT_SUCCESS);

	parse_fip(argv[0], NULL);

	if (outdir[0] != '\0')
		if (chdir(outdir) == -1)
			log_err("chdir %s", outdir);

	/* Unpack all specified images. */
	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		char file[PATH_MAX];
		image_t *image = desc->image;

		if (!unpack_all && desc->action != DO_UNPACK)
			continue;

		/* Build filename. */
		if (desc->action_arg == NULL)
			snprintf(file, sizeof(file), "%s.bin",
			    desc->cmdline_name);
		else
			snprintf(file, sizeof(file), "%s",
			    desc->action_arg);

		if (image == NULL) {
			if (!unpack_all)
				log_warnx("%s does not exist in %s",
				    file, argv[0]);
			continue;
		}

		if (access(file, F_OK) != 0 || fflag) {
			if (verbose)
				log_dbgx("Unpacking %s", file);
			write_image_to_file(image, file);
		} else {
			log_warnx("File %s already exists, use --force to overwrite it",
			    file);
		}
	}

	return 0;
}

static void unpack_usage(int exit_status)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool unpack [opts] FIP_FILENAME\n");
	printf("\n");
	printf("Options:\n");
	printf("  --blob uuid=...,file=...\tUnpack an image with the given UUID to file.\n");
	printf("  --force\t\t\tIf the output file already exists, use --force to overwrite it.\n");
	printf("  --out path\t\t\tSet the output directory path.\n");
	printf("\n");
	printf("Specific images are unpacked with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#ifdef PLAT_DEF_FIP_UUID
	toc_entry = plat_def_toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#endif
	printf("\n");
	printf("If no options are provided, all images will be unpacked.\n");
	exit(exit_status);
}

static int remove_cmd(int argc, char *argv[])
{
	struct option *opts = NULL;
	size_t nr_opts = 0;
	char outfile[PATH_MAX] = { 0 };
	fip_toc_header_t toc_header;
	image_desc_t *desc;
	unsigned long align = 1;
	int fflag = 0;

	if (argc < 2)
		remove_usage(EXIT_FAILURE);

	opts = fill_common_opts(opts, &nr_opts, no_argument);
	opts = add_opt(opts, &nr_opts, "align", required_argument, OPT_ALIGN);
	opts = add_opt(opts, &nr_opts, "blob", required_argument, 'b');
	opts = add_opt(opts, &nr_opts, "force", no_argument, 'f');
	opts = add_opt(opts, &nr_opts, "out", required_argument, 'o');
	opts = add_opt(opts, &nr_opts, NULL, 0, 0);

	while (1) {
		int c, opt_index = 0;

		c = getopt_long(argc, argv, "b:fo:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			image_desc_t *desc;

			desc = lookup_image_desc_from_opt(opts[opt_index].name);
			set_image_desc_action(desc, DO_REMOVE, NULL);
			break;
		}
		case OPT_ALIGN:
			align = get_image_align(optarg);
			break;
		case 'b': {
			char name[_UUID_STR_LEN + 1], filename[PATH_MAX];
			uuid_t uuid = uuid_null;
			image_desc_t *desc;

			parse_blob_opt(optarg, &uuid,
			    filename, sizeof(filename));

			if (memcmp(&uuid, &uuid_null, sizeof(uuid_t)) == 0)
				remove_usage(EXIT_FAILURE);

			desc = lookup_image_desc_from_uuid(&uuid);
			if (desc == NULL) {
				uuid_to_str(name, sizeof(name), &uuid);
				desc = new_image_desc(&uuid, name, "blob");
				add_image_desc(desc);
			}
			set_image_desc_action(desc, DO_REMOVE, NULL);
			break;
		}
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			remove_usage(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;
	free(opts);

	if (argc == 0)
		remove_usage(EXIT_SUCCESS);

	if (outfile[0] != '\0' && access(outfile, F_OK) == 0 && !fflag)
		log_errx("File %s already exists, use --force to overwrite it",
		    outfile);

	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);

	parse_fip(argv[0], &toc_header);

	for (desc = image_desc_head; desc != NULL; desc = desc->next) {
		if (desc->action != DO_REMOVE)
			continue;

		if (desc->image != NULL) {
			if (verbose)
				log_dbgx("Removing %s",
				    desc->cmdline_name);
			free(desc->image);
			desc->image = NULL;
		} else {
			log_warnx("%s does not exist in %s",
			    desc->cmdline_name, argv[0]);
		}
	}

	pack_images(outfile, toc_header.flags, align);
	return 0;
}

static void remove_usage(int exit_status)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool remove [opts] FIP_FILENAME\n");
	printf("\n");
	printf("Options:\n");
	printf("  --align <value>\tEach image is aligned to <value> (default: 1).\n");
	printf("  --blob uuid=...\tRemove an image with the given UUID.\n");
	printf("  --force\t\tIf the output FIP file already exists, use --force to overwrite it.\n");
	printf("  --out FIP_FILENAME\tSet an alternative output FIP file.\n");
	printf("\n");
	printf("Specific images are removed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#ifdef PLAT_DEF_FIP_UUID
	toc_entry = plat_def_toc_entries;
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
#endif
	exit(exit_status);
}

static int version_cmd(int argc, char *argv[])
{
#ifdef VERSION
	puts(VERSION);
#else
	/* If built from fiptool directory, VERSION is not set. */
	puts("Unknown version");
#endif
	return 0;
}

static void version_usage(int exit_status)
{
	printf("fiptool version\n");
	exit(exit_status);
}

static int help_cmd(int argc, char *argv[])
{
	int i;

	if (argc < 2)
		usage();
	argc--, argv++;

	for (i = 0; i < NELEM(cmds); i++) {
		if (strcmp(cmds[i].name, argv[0]) == 0 &&
		    cmds[i].usage != NULL)
			cmds[i].usage(EXIT_SUCCESS);
	}
	if (i == NELEM(cmds))
		printf("No help for subcommand '%s'\n", argv[0]);
	return 0;
}

static void usage(void)
{
	printf("usage: fiptool [--verbose] <command> [<args>]\n");
	printf("Global options supported:\n");
	printf("  --verbose\tEnable verbose output for all commands.\n");
	printf("\n");
	printf("Commands supported:\n");
	printf("  info\t\tList images contained in FIP.\n");
	printf("  create\tCreate a new FIP with the given images.\n");
	printf("  update\tUpdate an existing FIP with the given images.\n");
	printf("  unpack\tUnpack images from FIP.\n");
	printf("  remove\tRemove images from FIP.\n");
	printf("  version\tShow fiptool version.\n");
	printf("  help\t\tShow help for given command.\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int i, ret = 0;

	while (1) {
		int c, opt_index = 0;
		static struct option opts[] = {
			{ "verbose", no_argument, NULL, 'v' },
			{ NULL, no_argument, NULL, 0 }
		};

		/*
		 * Set POSIX mode so getopt stops at the first non-option
		 * which is the subcommand.
		 */
		c = getopt_long(argc, argv, "+v", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	/* Reset optind for subsequent getopt processing. */
	optind = 0;

	if (argc == 0)
		usage();

	fill_image_descs();
	for (i = 0; i < NELEM(cmds); i++) {
		if (strcmp(cmds[i].name, argv[0]) == 0) {
			ret = cmds[i].handler(argc, argv);
			break;
		}
	}
	if (i == NELEM(cmds))
		usage();
	free_image_descs();
	return ret;
}
