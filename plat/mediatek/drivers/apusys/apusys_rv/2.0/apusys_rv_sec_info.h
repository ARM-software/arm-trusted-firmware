/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_SECURE_INFO_H
#define APUSYS_RV_SECURE_INFO_H

#define CE_MAIN_MAGIC		(0xCEC0DE88)
#define CE_SUB_MAGIC		(0xCEC0DE00)

struct apusys_secure_info_t {
	unsigned int total_sz;
	unsigned int up_code_buf_ofs;
	unsigned int up_code_buf_sz;

	unsigned int up_fw_ofs;
	unsigned int up_fw_sz;
	unsigned int up_xfile_ofs;
	unsigned int up_xfile_sz;
	unsigned int mdla_fw_boot_ofs;
	unsigned int mdla_fw_boot_sz;
	unsigned int mdla_fw_main_ofs;
	unsigned int mdla_fw_main_sz;
	unsigned int mdla_xfile_ofs;
	unsigned int mdla_xfile_sz;
	unsigned int mvpu_fw_ofs;
	unsigned int mvpu_fw_sz;
	unsigned int mvpu_xfile_ofs;
	unsigned int mvpu_xfile_sz;
	unsigned int mvpu_sec_fw_ofs;
	unsigned int mvpu_sec_fw_sz;
	unsigned int mvpu_sec_xfile_ofs;
	unsigned int mvpu_sec_xfile_sz;

	unsigned int up_coredump_ofs;
	unsigned int up_coredump_sz;
	unsigned int mdla_coredump_ofs;
	unsigned int mdla_coredump_sz;
	unsigned int mvpu_coredump_ofs;
	unsigned int mvpu_coredump_sz;
	unsigned int mvpu_sec_coredump_ofs;
	unsigned int mvpu_sec_coredump_sz;

	unsigned int ce_bin_ofs;
	unsigned int ce_bin_sz;
};

struct ce_main_hdr_t {
	unsigned int magic;         /* magic number*/
	unsigned int hdr_size;      /* header size */
	unsigned int img_size;      /* img size */
	unsigned int bin_count;     /* bin count */
};

struct ce_sub_hdr_t {
	unsigned int magic;         /* magic number */
	unsigned int bin_offset;    /* binary offset */
	unsigned int bin_size;      /* binary size */
	unsigned int ce_enum;       /* ce enum */
	char job_name[8];           /* job name */
	unsigned int mem_st;        /* ce enum */
	unsigned int hw_entry;      /* hw entry */
	unsigned int hw_entry_bit;  /* hw entry bit */
	unsigned int hw_entry_mask; /* hw entry mask */
	unsigned int hw_entry_val;  /* hw entry val*/
	unsigned int user_info;     /* user_info */
};

#endif /* APUSYS_RV_SECURE_INFO_H */
