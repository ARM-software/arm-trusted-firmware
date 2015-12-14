/*
 * Copyright (c) 2014-2015, ARM Limited and Contributors. All rights reserved.
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

#include <errno.h>
#include <getopt.h> /* getopt_long() is a GNU extention */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fip_create.h"
#include "firmware_image_package.h"

/* Values returned by getopt() as part of the command line parsing */
#define OPT_TOC_ENTRY 0
#define OPT_DUMP 1
#define OPT_HELP 2

file_info_t files[MAX_FILES];
unsigned file_info_count = 0;
uuid_t uuid_null = {0};

/*
 * TODO: Add ability to specify and flag different file types.
 * Add flags to the toc_entry?
 * const char* format_type_str[] = { "RAW", "ELF", "PIC" };
 */

/* The images used depends on the platform. */
static entry_lookup_list_t toc_entry_lookup_list[] = {
	{ "SCP Firmware Updater Configuration FWU SCP_BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_SCP_BL2U,
	  "scp_bl2u", NULL, FLAG_FILENAME },
	{ "AP Firmware Updater Configuration BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_BL2U,
	  "bl2u", NULL, FLAG_FILENAME },
	{ "Firmware Updater NS_BL2U", UUID_TRUSTED_UPDATE_FIRMWARE_NS_BL2U,
	  "ns_bl2u", NULL, FLAG_FILENAME },
	{ "Non-Trusted Firmware Updater certificate", UUID_TRUSTED_FWU_CERT,
	  "fwu-cert", NULL, FLAG_FILENAME},
	{ "Trusted Boot Firmware BL2", UUID_TRUSTED_BOOT_FIRMWARE_BL2,
	  "bl2", NULL, FLAG_FILENAME },
	{ "SCP Firmware SCP_BL2", UUID_SCP_FIRMWARE_SCP_BL2,
	  "scp_bl2", NULL, FLAG_FILENAME},
	{ "EL3 Runtime Firmware BL31", UUID_EL3_RUNTIME_FIRMWARE_BL31,
	  "bl31", NULL, FLAG_FILENAME},
	{ "Secure Payload BL32 (Trusted OS)", UUID_SECURE_PAYLOAD_BL32,
	  "bl32", NULL, FLAG_FILENAME},
	{ "Non-Trusted Firmware BL33", UUID_NON_TRUSTED_FIRMWARE_BL33,
	  "bl33", NULL, FLAG_FILENAME},
	/* Key Certificates */
	{ "Root Of Trust key certificate", UUID_ROT_KEY_CERT,
	  "rot-cert", NULL, FLAG_FILENAME },
	{ "Trusted key certificate", UUID_TRUSTED_KEY_CERT,
	  "trusted-key-cert", NULL, FLAG_FILENAME},
	{ "SCP Firmware key certificate", UUID_SCP_FW_KEY_CERT,
	  "scp-fw-key-cert", NULL, FLAG_FILENAME},
	{ "SoC Firmware key certificate", UUID_SOC_FW_KEY_CERT,
	  "soc-fw-key-cert", NULL, FLAG_FILENAME},
	{ "Trusted OS Firmware key certificate", UUID_TRUSTED_OS_FW_KEY_CERT,
	  "tos-fw-key-cert", NULL, FLAG_FILENAME},
	{ "Non-Trusted Firmware key certificate", UUID_NON_TRUSTED_FW_KEY_CERT,
	  "nt-fw-key-cert", NULL, FLAG_FILENAME},
	/* Content certificates */
	{ "Trusted Boot Firmware BL2 certificate", UUID_TRUSTED_BOOT_FW_CERT,
	  "tb-fw-cert", NULL, FLAG_FILENAME },
	{ "SCP Firmware content certificate", UUID_SCP_FW_CONTENT_CERT,
	  "scp-fw-cert", NULL, FLAG_FILENAME},
	{ "SoC Firmware content certificate", UUID_SOC_FW_CONTENT_CERT,
	  "soc-fw-cert", NULL, FLAG_FILENAME},
	{ "Trusted OS Firmware content certificate", UUID_TRUSTED_OS_FW_CONTENT_CERT,
	  "tos-fw-cert", NULL, FLAG_FILENAME},
	{ "Non-Trusted Firmware content certificate", UUID_NON_TRUSTED_FW_CONTENT_CERT,
	  "nt-fw-cert", NULL, FLAG_FILENAME},
	{ NULL, {0}, 0 }
};


/* Return 0 for equal uuids */
static inline int compare_uuids(const uuid_t *uuid1, const uuid_t *uuid2)
{
	return memcmp(uuid1, uuid2, sizeof(uuid_t));
}


static inline void copy_uuid(uuid_t *to_uuid, const uuid_t *from_uuid)
{
	memcpy(to_uuid, from_uuid, sizeof(uuid_t));
}


static void print_usage(void)
{
	entry_lookup_list_t *entry = toc_entry_lookup_list;

	printf("Usage: fip_create [options] FIP_FILENAME\n\n");
	printf("\tThis tool is used to create a Firmware Image Package.\n\n");
	printf("Options:\n");
	printf("\t--help: Print this help message and exit\n");
	printf("\t--dump: Print contents of FIP\n\n");
	printf("\tComponents that can be added/updated:\n");
	for (; entry->command_line_name != NULL; entry++) {
		printf("\t--%s%s\t\t%s",
		       entry->command_line_name,
		       (entry->flags & FLAG_FILENAME) ? " FILENAME" : "",
		       entry->name);
		printf("\n");
	}
}


static entry_lookup_list_t *get_entry_lookup_from_uuid(const uuid_t *uuid)
{
	unsigned int lookup_index = 0;

	while (toc_entry_lookup_list[lookup_index].command_line_name != NULL) {
		if (compare_uuids(&toc_entry_lookup_list[lookup_index].name_uuid,
		    uuid) == 0) {
			return &toc_entry_lookup_list[lookup_index];
		}
		lookup_index++;
	}
	return NULL;
}


static file_info_t *find_file_info_from_uuid(const uuid_t *uuid)
{
	int index;

	for (index = 0; index < file_info_count; index++) {
		if (compare_uuids(&files[index].name_uuid, uuid) == 0) {
			return &files[index];
		}
	}
	return NULL;
}


static int add_file_info_entry(entry_lookup_list_t *lookup_entry, char *filename)
{
	file_info_t *file_info_entry;
	int error;
	struct stat file_status;
	bool is_new_entry = false;

	/* Check if the file already exists in the array */
	file_info_entry = find_file_info_from_uuid(&lookup_entry->name_uuid);
	if (file_info_entry == NULL) {
		/* The file does not exist in the current list; take the next
		 * one available in the file_info list. 'file_info_count' is
		 * incremented in case of successful update at the end of the
		 * function.
		 */
		file_info_entry = &files[file_info_count];
		is_new_entry = true;

		/* Copy the uuid for the new entry */
		copy_uuid(&file_info_entry->name_uuid,
			  &lookup_entry->name_uuid);
	}

	/* Get the file information for entry */
	error = stat(filename, &file_status);
	if (error != 0) {
		printf("Error: Cannot get information for file \"%s\": %s\n",
			filename, strerror(errno));
		return errno;
	}
	file_info_entry->filename = filename;
	file_info_entry->size = (unsigned int)file_status.st_size;
	file_info_entry->entry = lookup_entry;

	/* Increment the file_info counter on success if it is new file entry */
	if (is_new_entry) {
		file_info_count++;

		/* Ensure we do not overflow */
		if (file_info_count > MAX_FILES) {
			printf("ERROR: Too many files in Package\n");
			return 1;
		}
	}

	return 0;
}


static int write_memory_to_file(const uint8_t *start, const char *filename,
		unsigned int size)
{
	FILE *stream;
	unsigned int bytes_written;

	/* Write the packed file out to the filesystem */
	stream = fopen(filename, "r+");
	if (stream == NULL) {
		stream = fopen(filename, "w");
		if (stream == NULL) {
			printf("Error: Cannot create output file \"%s\": %s\n",
			       filename, strerror(errno));
			return errno;
		} else {
			printf("Creating \"%s\"\n", filename);
		}
	} else {
		printf("Updating \"%s\"\n", filename);
	}

	bytes_written = fwrite(start, sizeof(uint8_t), size, stream);
	fclose(stream);

	if (bytes_written != size) {
		printf("Error: Incorrect write for file \"%s\": Size=%u,"
			"Written=%u bytes.\n", filename, size, bytes_written);
		return EIO;
	}

	return 0;
}


static int read_file_to_memory(void *memory, const file_info_t *info)
{
	FILE *stream;
	unsigned int bytes_read;

	/* If the file_info is defined by its filename we need to load it */
	if (info->filename) {
		/* Read image from filesystem */
		stream = fopen(info->filename, "r");
		if (stream == NULL) {
			printf("Error: Cannot open file \"%s\": %s\n",
				info->filename, strerror(errno));
			return errno;
		}

		bytes_read = (unsigned int)fread(memory, sizeof(uint8_t),
						 info->size, stream);
		fclose(stream);
		if (bytes_read != info->size) {
			printf("Error: Incomplete read for file \"%s\":"
				"Size=%u, Read=%u bytes.\n", info->filename,
				info->size, bytes_read);
			return EIO;
		}
	} else {
		if (info->image_buffer == NULL) {
			printf("ERROR: info->image_buffer = NULL\n");
			return EIO;
		}
		/* Copy the file_info buffer (extracted from the existing
		 * image package) into the new buffer.
		 */
		memcpy(memory, info->image_buffer, info->size);
	}

	return 0;
}


/* Create the image package file */
static int pack_images(const char *fip_filename)
{
	int status;
	uint8_t *fip_base_address;
	void *entry_address;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	unsigned int entry_index;
	unsigned int toc_size;
	unsigned int fip_size;
	unsigned int entry_offset_address;
	unsigned int payload_size = 0;

	/* Validate filename */
	if ((fip_filename == NULL) || (strcmp(fip_filename, "") == 0)) {
		return EINVAL;
	}

	/* Payload size calculation */
	for (entry_index = 0; entry_index < file_info_count; entry_index++) {
		payload_size += files[entry_index].size;
	}

	/* Allocate memory for entire package, including the final null entry */
	toc_size = (sizeof(fip_toc_header_t) +
		    (sizeof(fip_toc_entry_t) * (file_info_count + 1)));
	fip_size = toc_size + payload_size;
	fip_base_address = malloc(fip_size);
	if (fip_base_address == NULL) {
		printf("Error: Can't allocate enough memory to create package."
		       "Process aborted.\n");
		return ENOMEM;
	}
	memset(fip_base_address, 0, fip_size);

	/* Create ToC Header */
	toc_header = (fip_toc_header_t *)fip_base_address;
	toc_header->name = TOC_HEADER_NAME;
	toc_header->serial_number = TOC_HEADER_SERIAL_NUMBER;
	toc_header->flags = 0;

	toc_entry = (fip_toc_entry_t *)(fip_base_address +
				      sizeof(fip_toc_header_t));

	/* Calculate the starting address of the first image, right after the
	 * toc header.
	 */
	entry_offset_address = toc_size;
	entry_index = 0;

	/* Create the package in memory. */
	for (entry_index = 0; entry_index < file_info_count; entry_index++) {
		entry_address = (fip_base_address + entry_offset_address);
		status = read_file_to_memory(entry_address,
					     &files[entry_index]);
		if (status != 0) {
			printf("Error: While reading \"%s\" from filesystem.\n",
				files[entry_index].filename);
			return status;
		}

		copy_uuid(&toc_entry->uuid, &files[entry_index].name_uuid);
		toc_entry->offset_address = entry_offset_address;
		toc_entry->size = files[entry_index].size;
		toc_entry->flags = 0;
		entry_offset_address += toc_entry->size;
		toc_entry++;
	}

	/* Add a null uuid entry to mark the end of toc entries */
	copy_uuid(&toc_entry->uuid, &uuid_null);
	toc_entry->offset_address = entry_offset_address;
	toc_entry->size = 0;
	toc_entry->flags = 0;

	/* Save the package to file */
	status = write_memory_to_file(fip_base_address, fip_filename, fip_size);
	if (status != 0) {
		printf("Error: Failed while writing package to file \"%s\" "
			"with status=%d.\n", fip_filename, status);
		return status;
	}
	return 0;
}


static void dump_toc(void)
{
	unsigned int index = 0;
	unsigned int image_offset;
	unsigned int image_size = 0;

	image_offset = sizeof(fip_toc_header_t) +
		(sizeof(fip_toc_entry_t) * (file_info_count + 1));

	printf("Firmware Image Package ToC:\n");
	printf("---------------------------\n");
	for (index = 0; index < file_info_count; index++) {
		if (files[index].entry) {
			printf("- %s: ", files[index].entry->name);
		} else {
			printf("- Unknown entry: ");
		}
		image_size = files[index].size;

		printf("offset=0x%X, size=0x%X\n", image_offset, image_size);
		image_offset += image_size;

		if (files[index].filename) {
			printf("  file: '%s'\n", files[index].filename);
		}
	}
	printf("---------------------------\n");
}


/* Read and load existing package into memory. */
static int parse_fip(const char *fip_filename)
{
	FILE *fip;
	char *fip_buffer;
	char *fip_buffer_end;
	int fip_size, read_fip_size;
	fip_toc_header_t *toc_header;
	fip_toc_entry_t *toc_entry;
	bool found_last_toc_entry = false;
	file_info_t *file_info_entry;
	int status = -1;
	struct stat st;

	fip = fopen(fip_filename, "r");
	if (fip == NULL) {
		/* If the fip does not exist just return, it should not be
		 * considered as an error. The package will be created later
		 */
		status = 0;
		goto parse_fip_return;
	}

	if (stat(fip_filename, &st) != 0) {
		status = errno;
		goto parse_fip_fclose;
	} else {
		fip_size = (int)st.st_size;
	}

	/* Allocate a buffer to read the package */
	fip_buffer = (char *)malloc(fip_size);
	if (fip_buffer == NULL) {
		printf("ERROR: Cannot allocate %d bytes.\n", fip_size);
		status = errno;
		goto parse_fip_fclose;
	}
	fip_buffer_end = fip_buffer + fip_size;

	/* Read the file */
	read_fip_size = fread(fip_buffer, sizeof(char), fip_size, fip);
	if (read_fip_size != fip_size) {
		printf("ERROR: Cannot read the FIP.\n");
		status = EIO;
		goto parse_fip_free;
	}
	fclose(fip);
	fip = NULL;

	/* The package must at least contain the ToC Header */
	if (fip_size < sizeof(fip_toc_header_t)) {
		printf("ERROR: Given FIP is smaller than the ToC header.\n");
		status = EINVAL;
		goto parse_fip_free;
	}
	/* Set the ToC Header at the base of the buffer */
	toc_header = (fip_toc_header_t *)fip_buffer;
	/* The first toc entry should be just after the ToC header */
	toc_entry = (fip_toc_entry_t *)(toc_header + 1);

	/* While the ToC entry is contained into the buffer */
	int cnt = 0;
	while (((char *)toc_entry + sizeof(fip_toc_entry_t)) < fip_buffer_end) {
		cnt++;
		/* Check if the ToC Entry is the last one */
		if (compare_uuids(&toc_entry->uuid, &uuid_null) == 0) {
			found_last_toc_entry = true;
			status = 0;
			break;
		}

		/* Add the entry into file_info */

		/* Get the new entry in the array and clear it */
		file_info_entry = &files[file_info_count++];
		memset(file_info_entry, 0, sizeof(file_info_t));

		/* Copy the info from the ToC entry */
		copy_uuid(&file_info_entry->name_uuid, &toc_entry->uuid);
		file_info_entry->image_buffer = fip_buffer +
		  toc_entry->offset_address;
		file_info_entry->size = toc_entry->size;

		/* Check if there is a corresponding entry in lookup table */
		file_info_entry->entry =
		  get_entry_lookup_from_uuid(&toc_entry->uuid);

		/* Go to the next ToC entry */
		toc_entry++;
	}

	if (!found_last_toc_entry) {
		printf("ERROR: Given FIP does not have an end ToC entry.\n");
		status = EINVAL;
		goto parse_fip_free;
	} else {
		/* All is well, we should not free any of the loaded images */
		goto parse_fip_fclose;
	}

 parse_fip_free:
	if (fip_buffer != NULL) {
		free(fip_buffer);
		fip_buffer = NULL;
	}

 parse_fip_fclose:
	if (fip != NULL) {
		fclose(fip);
	}

 parse_fip_return:
	return status;
}


/* Parse all command-line options and return the FIP name if present. */
static char *get_filename(int argc, char **argv, struct option *options)
{
	int c;
	char *filename = NULL;

	/* Reset option pointer so we parse all args. starts at 1.
	 * The filename is the only argument that does not have an option flag.
	 */
	optind = 1;
	while (1) {
		c = getopt_long(argc, argv, "", options, NULL);
		if (c == -1)
			break;

		if (c == '?') {
			/* Failed to parse an option. Fail. */
			return NULL;
		}
	}

	/* Only one argument left then it is the filename.
	 * We dont expect any other options
	 */
	if (optind + 1 == argc)
		filename = argv[optind];

	return filename;
}


/* Work through command-line options */
static int parse_cmdline(int argc, char **argv, struct option *options,
			 int *do_pack)
{
	int c;
	int status = 0;
	int option_index = 0;
	entry_lookup_list_t *lookup_entry;
	int do_dump = 0;

	/* restart parse to process all options. starts at 1. */
	optind = 1;
	while (1) {
		c = getopt_long(argc, argv, "", options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case OPT_TOC_ENTRY:
			if (optarg) {
				/* Does the option expect a filename. */
				lookup_entry = &toc_entry_lookup_list[option_index];
				if (lookup_entry->flags & FLAG_FILENAME) {
					status = add_file_info_entry(lookup_entry, optarg);
					if (status != 0) {
						printf("Failed to process %s\n",
						       options[option_index].name);
						return status;
					} else {
						/* Update package */
						*do_pack = 1;
					}
				}
			}
			break;

		case OPT_DUMP:
			do_dump = 1;
			continue;

		case OPT_HELP:
			print_usage();
			exit(0);

		default:
			/* Unrecognised options are caught in get_filename() */
			break;
		}
	}


	/* Do not dump toc if we have an error as it could hide the error */
	if ((status == 0) && (do_dump)) {
		dump_toc();
	}

	return status;

}

int main(int argc, char **argv)
{
	int i;
	int status;
	char *fip_filename;
	int do_pack = 0;

	/* Clear file list table. */
	memset(files, 0, sizeof(files));

	/* Initialise for getopt_long().
	 * Use image table as defined at top of file to get options.
	 * Add 'dump' option, 'help' option and end marker.
	 */
	static struct option long_options[(sizeof(toc_entry_lookup_list)/
					   sizeof(entry_lookup_list_t)) + 2];

	for (i = 0;
	     /* -1 because we dont want to process end marker in toc table */
	     i < sizeof(toc_entry_lookup_list)/sizeof(entry_lookup_list_t) - 1;
	     i++) {
		long_options[i].name = toc_entry_lookup_list[i].command_line_name;
		/* The only flag defined at the moment is for a FILENAME */
		long_options[i].has_arg = toc_entry_lookup_list[i].flags ? 1 : 0;
		long_options[i].flag = 0;
		long_options[i].val = OPT_TOC_ENTRY;
	}

	/* Add '--dump' option */
	long_options[i].name = "dump";
	long_options[i].has_arg = 0;
	long_options[i].flag = 0;
	long_options[i].val = OPT_DUMP;

	/* Add '--help' option */
	long_options[++i].name = "help";
	long_options[i].has_arg = 0;
	long_options[i].flag = 0;
	long_options[i].val = OPT_HELP;

	/* Zero the last entry (required) */
	long_options[++i].name = 0;
	long_options[i].has_arg = 0;
	long_options[i].flag = 0;
	long_options[i].val = 0;

#ifdef DEBUG
	/* Print all supported options */
	for (i = 0; i < sizeof(long_options)/sizeof(struct option); i++) {
		printf("long opt (%d) : name = %s\n", i, long_options[i].name);
	}
#endif /* DEBUG */

	/* As the package may already exist and is to be updated we need to get
	 * the filename from the arguments and load from it.
	 * NOTE: As this is the first function to look at the program arguments
	 * it causes a failure if bad options were provided.
	 */
	fip_filename = get_filename(argc, argv, long_options);

	/* Try to open the file and load it into memory */
	if (fip_filename != NULL) {
		status = parse_fip(fip_filename);
		if (status != 0) {
			return status;
		}
	}

	/* Work through provided program arguments and perform actions */
	status = parse_cmdline(argc, argv, long_options, &do_pack);
	if (status != 0) {
		return status;
	};

	if (fip_filename == NULL) {
		printf("ERROR: Missing FIP filename\n");
		print_usage();
		return 0;
	}

	/* Processed all command line options. Create/update the package if
	 * required.
	 */
	if (do_pack) {
		status = pack_images(fip_filename);
		if (status != 0) {
			printf("Failed to create package (status = %d).\n",
			       status);
		}
	}

	return status;
}
