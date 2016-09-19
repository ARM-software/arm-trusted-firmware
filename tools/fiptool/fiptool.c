/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/sha.h>

#include "fiptool.h"
#include "firmware_image_package.h"
#include "tbbr_config.h"

#define OPT_TOC_ENTRY 0
#define OPT_PLAT_TOC_FLAGS 1

static int info_cmd(int argc, char *argv[]);
static void info_usage(void);
static int create_cmd(int argc, char *argv[]);
static void create_usage(void);
static int update_cmd(int argc, char *argv[]);
static void update_usage(void);
static int unpack_cmd(int argc, char *argv[]);
static void unpack_usage(void);
static int remove_cmd(int argc, char *argv[]);
static void remove_usage(void);
static int version_cmd(int argc, char *argv[]);
static void version_usage(void);
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

static image_t *images[MAX_IMAGES];
static size_t nr_images;
static uuid_t uuid_null = { 0 };
static int verbose;

static void vlog(int prio, char *msg, va_list ap)
{
	char *prefix[] = { "DEBUG", "WARN", "ERROR" };

	fprintf(stderr, "%s: ", prefix[prio]);
	vfprintf(stderr, msg, ap);
	fputc('\n', stderr);
}

static void log_dbgx(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_DBG, msg, ap);
	va_end(ap);
}

static void log_warnx(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_WARN, msg, ap);
	va_end(ap);
}

static void log_err(char *msg, ...)
{
	char buf[512];
	va_list ap;

	va_start(ap, msg);
	snprintf(buf, sizeof(buf), "%s: %s", msg, strerror(errno));
	vlog(LOG_ERR, buf, ap);
	va_end(ap);
	exit(1);
}

static void log_errx(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vlog(LOG_ERR, msg, ap);
	va_end(ap);
	exit(1);
}

static void add_image(image_t *image)
{
	if (nr_images + 1 > MAX_IMAGES)
		log_errx("Too many images");
	images[nr_images++] = image;
}

static void free_image(image_t *image)
{
	free(image->buffer);
	free(image);
}

static void replace_image(image_t *image_dst, image_t *image_src)
{
	int i;

	for (i = 0; i < nr_images; i++) {
		if (images[i] == image_dst) {
			free_image(images[i]);
			images[i] = image_src;
			break;
		}
	}
	assert(i != nr_images);
}

static void remove_image(image_t *image)
{
	int i;

	for (i = 0; i < nr_images; i++) {
		if (images[i] == image) {
			free_image(images[i]);
			images[i] = NULL;
			break;
		}
	}
	assert(i != nr_images);

	/* Compact array. */
	memmove(&images[i], &images[i + 1],
	    (nr_images - i - 1) * sizeof(*images));
	nr_images--;
}

static void free_images(void)
{
	int i;

	for (i = 0; i < nr_images; i++) {
		free_image(images[i]);
		images[i] = NULL;
	}
}

static toc_entry_t *get_entry_lookup_from_uuid(const uuid_t *uuid)
{
	toc_entry_t *toc_entry = toc_entries;

	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		if (memcmp(&toc_entry->uuid, uuid, sizeof(uuid_t)) == 0)
			return toc_entry;
	return NULL;
}

static int parse_fip(char *filename, fip_toc_header_t *toc_header_out)
{
	struct stat st;
	FILE *fp;
	char *buf, *bufend;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	image_t *image;
	int terminated = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (fstat(fileno(fp), &st) == -1)
		log_err("fstat %s", filename);

	buf = malloc(st.st_size);
	if (buf == NULL)
		log_err("malloc");

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
		/* Found the ToC terminator, we are done. */
		if (memcmp(&toc_entry->uuid, &uuid_null, sizeof(uuid_t)) == 0) {
			terminated = 1;
			break;
		}

		/*
		 * Build a new image out of the ToC entry and add it to the
		 * table of images.
		 */
		image = malloc(sizeof(*image));
		if (image == NULL)
			log_err("malloc");

		memcpy(&image->uuid, &toc_entry->uuid, sizeof(uuid_t));

		image->buffer = malloc(toc_entry->size);
		if (image->buffer == NULL)
			log_err("malloc");

		/* Overflow checks before memory copy. */
		if (toc_entry->size > (uint64_t)-1 - toc_entry->offset_address)
			log_errx("FIP %s is corrupted", filename);
		if (toc_entry->size + toc_entry->offset_address > st.st_size)
			log_errx("FIP %s is corrupted", filename);

		memcpy(image->buffer, buf + toc_entry->offset_address,
		    toc_entry->size);
		image->size = toc_entry->size;

		image->toc_entry = get_entry_lookup_from_uuid(&toc_entry->uuid);
		if (image->toc_entry == NULL) {
			add_image(image);
			toc_entry++;
			continue;
		}

		assert(image->toc_entry->image == NULL);
		/* Link backpointer from lookup entry. */
		image->toc_entry->image = image;
		add_image(image);

		toc_entry++;
	}

	if (terminated == 0)
		log_errx("FIP %s does not have a ToC terminator entry",
		    filename);
	free(buf);
	return 0;
}

static image_t *read_image_from_file(toc_entry_t *toc_entry, char *filename)
{
	struct stat st;
	image_t *image;
	FILE *fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (fstat(fileno(fp), &st) == -1)
		log_errx("fstat %s", filename);

	image = malloc(sizeof(*image));
	if (image == NULL)
		log_err("malloc");

	memcpy(&image->uuid, &toc_entry->uuid, sizeof(uuid_t));

	image->buffer = malloc(st.st_size);
	if (image->buffer == NULL)
		log_err("malloc");
	if (fread(image->buffer, 1, st.st_size, fp) != st.st_size)
		log_errx("Failed to read %s", filename);
	image->size = st.st_size;
	image->toc_entry = toc_entry;

	fclose(fp);
	return image;
}

static int write_image_to_file(image_t *image, char *filename)
{
	FILE *fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		log_err("fopen");
	if (fwrite(image->buffer, 1, image->size, fp) != image->size)
		log_errx("Failed to write %s", filename);
	fclose(fp);
	return 0;
}

static int fill_common_opts(struct option *opts, int has_arg)
{
	int i;

	for (i = 0; toc_entries[i].cmdline_name != NULL; i++) {
		opts[i].name = toc_entries[i].cmdline_name;
		opts[i].has_arg = has_arg;
		opts[i].flag = NULL;
		opts[i].val = 0;
	}
	return i;
}

static void add_opt(struct option *opts, int idx, char *name,
    int has_arg, int val)
{
	opts[idx].name = name;
	opts[idx].has_arg = has_arg;
	opts[idx].flag = NULL;
	opts[idx].val = val;
}

static void md_print(unsigned char *md, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		printf("%02x", md[i]);
}

static int info_cmd(int argc, char *argv[])
{
	image_t *image;
	uint64_t image_offset;
	uint64_t image_size = 0;
	fip_toc_header_t toc_header;
	int i;

	if (argc != 2)
		info_usage();
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

	image_offset = sizeof(fip_toc_header_t) +
	    (sizeof(fip_toc_entry_t) * (nr_images + 1));

	for (i = 0; i < nr_images; i++) {
		image = images[i];
		if (image->toc_entry != NULL)
			printf("%s: ", image->toc_entry->name);
		else
			printf("Unknown entry: ");
		image_size = image->size;
		printf("offset=0x%llX, size=0x%llX",
		    (unsigned long long)image_offset,
		    (unsigned long long)image_size);
		if (image->toc_entry != NULL)
			printf(", cmdline=\"--%s\"",
			    image->toc_entry->cmdline_name);
		if (verbose) {
			unsigned char md[SHA256_DIGEST_LENGTH];

			SHA256(image->buffer, image_size, md);
			printf(", sha256=");
			md_print(md, sizeof(md));
		}
		putchar('\n');
		image_offset += image_size;
	}

	free_images();
	return 0;
}

static void info_usage(void)
{
	printf("fiptool info FIP_FILENAME\n");
	exit(1);
}

static int pack_images(char *filename, uint64_t toc_flags)
{
	FILE *fp;
	image_t *image;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	char *buf;
	uint64_t entry_offset, buf_size, payload_size;
	int i;

	/* Calculate total payload size and allocate scratch buffer. */
	payload_size = 0;
	for (i = 0; i < nr_images; i++)
		payload_size += images[i]->size;

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
	for (i = 0; i < nr_images; i++) {
		image = images[i];
		memcpy(&toc_entry->uuid, &image->uuid, sizeof(uuid_t));
		toc_entry->offset_address = entry_offset;
		toc_entry->size = image->size;
		toc_entry->flags = 0;
		entry_offset += toc_entry->size;
		toc_entry++;
	}

	/* Append a null uuid entry to mark the end of ToC entries. */
	memcpy(&toc_entry->uuid, &uuid_null, sizeof(uuid_t));
	toc_entry->offset_address = entry_offset;
	toc_entry->size = 0;
	toc_entry->flags = 0;

	/* Generate the FIP file. */
	fp = fopen(filename, "w");
	if (fp == NULL)
		log_err("fopen %s", filename);

	if (verbose)
		log_dbgx("Metadata size: %zu bytes", buf_size);

	if (fwrite(buf, 1, buf_size, fp) != buf_size)
		log_errx("Failed to write image to %s", filename);
	free(buf);

	if (verbose)
		log_dbgx("Payload size: %zu bytes", payload_size);

	for (i = 0; i < nr_images; i++) {
		image = images[i];
		if (fwrite(image->buffer, 1, image->size, fp) != image->size)
			log_errx("Failed to write image to %s", filename);
	}

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
	toc_entry_t *toc_entry;
	image_t *image;

	/* Add or replace images in the FIP file. */
	for (toc_entry = toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		if (toc_entry->action != DO_PACK)
			continue;

		image = read_image_from_file(toc_entry, toc_entry->action_arg);
		if (toc_entry->image != NULL) {
			if (verbose)
				log_dbgx("Replacing image %s.bin with %s",
				    toc_entry->cmdline_name,
				    toc_entry->action_arg);
			replace_image(toc_entry->image, image);
		} else {
			if (verbose)
				log_dbgx("Adding image %s",
				    toc_entry->action_arg);
			add_image(image);
		}
		/* Link backpointer from lookup entry. */
		toc_entry->image = image;

		free(toc_entry->action_arg);
		toc_entry->action_arg = NULL;
	}
}

static void parse_plat_toc_flags(char *arg, unsigned long long *toc_flags)
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

static int create_cmd(int argc, char *argv[])
{
	struct option opts[toc_entries_len + 1];
	unsigned long long toc_flags = 0;
	int i;

	if (argc < 2)
		create_usage();

	i = fill_common_opts(opts, required_argument);
	add_opt(opts, i, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	add_opt(opts, ++i, NULL, 0, 0);

	while (1) {
		int c, opt_index;

		c = getopt_long(argc, argv, "o:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			toc_entry_t *toc_entry;

			toc_entry = &toc_entries[opt_index];
			toc_entry->action = DO_PACK;
			toc_entry->action_arg = strdup(optarg);
			if (toc_entry->action_arg == NULL)
				log_err("strdup");
			break;
		}
		case OPT_PLAT_TOC_FLAGS:
			parse_plat_toc_flags(optarg, &toc_flags);
			break;
		default:
			create_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		create_usage();

	update_fip();

	pack_images(argv[0], toc_flags);
	free_images();
	return 0;
}

static void create_usage(void)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool create [--plat-toc-flags <value>] [opts] FIP_FILENAME\n");
	printf("  --plat-toc-flags <value>\t16-bit platform specific flag field "
	    "occupying bits 32-47 in 64-bit ToC header.\n");
	fputc('\n', stderr);
	printf("Specific images are packed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
	exit(1);
}

static int update_cmd(int argc, char *argv[])
{
	struct option opts[toc_entries_len + 2];
	char outfile[FILENAME_MAX] = { 0 };
	fip_toc_header_t toc_header = { 0 };
	unsigned long long toc_flags = 0;
	int pflag = 0;
	int i;

	if (argc < 2)
		update_usage();

	i = fill_common_opts(opts, required_argument);
	add_opt(opts, i, "out", required_argument, 'o');
	add_opt(opts, ++i, "plat-toc-flags", required_argument,
	    OPT_PLAT_TOC_FLAGS);
	add_opt(opts, ++i, NULL, 0, 0);

	while (1) {
		int c, opt_index;

		c = getopt_long(argc, argv, "o:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY: {
			toc_entry_t *toc_entry;

			toc_entry = &toc_entries[opt_index];
			toc_entry->action = DO_PACK;
			toc_entry->action_arg = strdup(optarg);
			if (toc_entry->action_arg == NULL)
				log_err("strdup");
			break;
		}
		case OPT_PLAT_TOC_FLAGS: {
			parse_plat_toc_flags(optarg, &toc_flags);
			pflag = 1;
			break;
		}
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			update_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		update_usage();

	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);

	if (access(outfile, F_OK) == 0)
		parse_fip(argv[0], &toc_header);

	if (pflag)
		toc_header.flags &= ~(0xffffULL << 32);
	toc_flags = (toc_header.flags |= toc_flags);

	update_fip();

	pack_images(outfile, toc_flags);
	free_images();
	return 0;
}

static void update_usage(void)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool update [--out FIP_FILENAME] "
	    "[--plat-toc-flags <value>] [opts] FIP_FILENAME\n");
	printf("  --out FIP_FILENAME\t\tSet an alternative output FIP file.\n");
	printf("  --plat-toc-flags <value>\t16-bit platform specific flag field "
	    "occupying bits 32-47 in 64-bit ToC header.\n");
	fputc('\n', stderr);
	printf("Specific images are packed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
	exit(1);
}

static int unpack_cmd(int argc, char *argv[])
{
	struct option opts[toc_entries_len + 3];
	char file[FILENAME_MAX], outdir[PATH_MAX] = { 0 };
	toc_entry_t *toc_entry;
	int fflag = 0;
	int unpack_all = 1;
	int i;

	if (argc < 2)
		unpack_usage();

	i = fill_common_opts(opts, required_argument);
	add_opt(opts, i, "force", no_argument, 'f');
	add_opt(opts, ++i, "out", required_argument, 'o');
	add_opt(opts, ++i, NULL, 0, 0);

	while (1) {
		int c, opt_index;

		c = getopt_long(argc, argv, "fo:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY:
			unpack_all = 0;
			toc_entry = &toc_entries[opt_index];
			toc_entry->action = DO_UNPACK;
			toc_entry->action_arg = strdup(optarg);
			if (toc_entry->action_arg == NULL)
				log_err("strdup");
			break;
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outdir, sizeof(outdir), "%s", optarg);
			break;
		default:
			unpack_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		unpack_usage();

	parse_fip(argv[0], NULL);

	if (outdir[0] != '\0')
		if (chdir(outdir) == -1)
			log_err("chdir %s", outdir);

	/* Mark all images to be unpacked. */
	if (unpack_all) {
		for (toc_entry = toc_entries;
		     toc_entry->cmdline_name != NULL;
		     toc_entry++) {
			if (toc_entry->image != NULL) {
				toc_entry->action = DO_UNPACK;
				toc_entry->action_arg = NULL;
			}
		}
	}

	/* Unpack all specified images. */
	for (toc_entry = toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		if (toc_entry->action != DO_UNPACK)
			continue;

		/* Build filename. */
		if (toc_entry->action_arg == NULL)
			snprintf(file, sizeof(file), "%s.bin",
			    toc_entry->cmdline_name);
		else
			snprintf(file, sizeof(file), "%s",
			    toc_entry->action_arg);

		if (toc_entry->image == NULL) {
			log_warnx("Requested image %s is not in %s",
			    file, argv[0]);
			free(toc_entry->action_arg);
			toc_entry->action_arg = NULL;
			continue;
		}

		if (access(file, F_OK) != 0 || fflag) {
			if (verbose)
				log_dbgx("Unpacking %s", file);
			write_image_to_file(toc_entry->image, file);
		} else {
			log_warnx("File %s already exists, use --force to overwrite it",
			    file);
		}

		free(toc_entry->action_arg);
		toc_entry->action_arg = NULL;
	}

	free_images();
	return 0;
}

static void unpack_usage(void)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool unpack [--force] [--out <path>] [opts] FIP_FILENAME\n");
	printf("  --force\tIf the output file already exists, use --force to "
	    "overwrite it.\n");
	printf("  --out path\tSet the output directory path.\n");
	fputc('\n', stderr);
	printf("Specific images are unpacked with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s FILENAME\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
	fputc('\n', stderr);
	printf("If no options are provided, all images will be unpacked.\n");
	exit(1);
}

static int remove_cmd(int argc, char *argv[])
{
	struct option opts[toc_entries_len + 2];
	char outfile[FILENAME_MAX] = { 0 };
	fip_toc_header_t toc_header;
	toc_entry_t *toc_entry;
	int fflag = 0;
	int i;

	if (argc < 2)
		remove_usage();

	i = fill_common_opts(opts, no_argument);
	add_opt(opts, i, "force", no_argument, 'f');
	add_opt(opts, ++i, "out", required_argument, 'o');
	add_opt(opts, ++i, NULL, 0, 0);

	while (1) {
		int c, opt_index;

		c = getopt_long(argc, argv, "fo:", opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY:
			toc_entry = &toc_entries[opt_index];
			toc_entry->action = DO_REMOVE;
			break;
		case 'f':
			fflag = 1;
			break;
		case 'o':
			snprintf(outfile, sizeof(outfile), "%s", optarg);
			break;
		default:
			remove_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		remove_usage();

	if (outfile[0] != '\0' && access(outfile, F_OK) == 0 && !fflag)
		log_errx("File %s already exists, use --force to overwrite it",
		    outfile);

	if (outfile[0] == '\0')
		snprintf(outfile, sizeof(outfile), "%s", argv[0]);

	parse_fip(argv[0], &toc_header);

	for (toc_entry = toc_entries;
	     toc_entry->cmdline_name != NULL;
	     toc_entry++) {
		if (toc_entry->action != DO_REMOVE)
			continue;
		if (toc_entry->image != NULL) {
			if (verbose)
				log_dbgx("Removing %s.bin",
				    toc_entry->cmdline_name);
			remove_image(toc_entry->image);
		} else {
			log_warnx("Requested image %s.bin is not in %s",
			    toc_entry->cmdline_name, argv[0]);
		}
	}

	pack_images(outfile, toc_header.flags);
	free_images();
	return 0;
}

static void remove_usage(void)
{
	toc_entry_t *toc_entry = toc_entries;

	printf("fiptool remove [--force] [--out FIP_FILENAME] [opts] FIP_FILENAME\n");
	printf("  --force\t\tIf the output FIP file already exists, use --force to "
	    "overwrite it.\n");
	printf("  --out FIP_FILENAME\tSet an alternative output FIP file.\n");
	fputc('\n', stderr);
	printf("Specific images are removed with the following options:\n");
	for (; toc_entry->cmdline_name != NULL; toc_entry++)
		printf("  --%-16s\t%s\n", toc_entry->cmdline_name,
		    toc_entry->name);
	exit(1);
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

static void version_usage(void)
{
	printf("fiptool version\n");
	exit(1);
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
			cmds[i].usage();
	}
	if (i == NELEM(cmds))
		printf("No help for subcommand '%s'\n", argv[0]);
	return 0;
}

static void usage(void)
{
	printf("usage: [--verbose] fiptool <command> [<args>]\n");
	printf("Global options supported:\n");
	printf("  --verbose\tEnable verbose output for all commands.\n");
	fputc('\n', stderr);
	printf("Commands supported:\n");
	printf("  info\t\tList images contained in FIP.\n");
	printf("  create\tCreate a new FIP with the given images.\n");
	printf("  update\tUpdate an existing FIP with the given images.\n");
	printf("  unpack\tUnpack images from FIP.\n");
	printf("  remove\tRemove images from FIP.\n");
	printf("  version\tShow fiptool version.\n");
	printf("  help\t\tShow help for given command.\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	int i, ret = 0;

	if (argc < 2)
		usage();
	argc--, argv++;

	if (strcmp(argv[0], "-v") == 0 ||
	    strcmp(argv[0], "--verbose") == 0) {
		verbose = 1;
		argc--, argv++;
	}

	for (i = 0; i < NELEM(cmds); i++) {
		if (strcmp(cmds[i].name, argv[0]) == 0) {
			ret = cmds[i].handler(argc, argv);
			break;
		}
	}
	if (i == NELEM(cmds))
		usage();
	return ret;
}
