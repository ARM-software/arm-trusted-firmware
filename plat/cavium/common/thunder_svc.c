/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <psci.h>
#include <runtime_svc.h>
#include <thunder_svc.h>
#include <stdint.h>
#include <uuid.h>
#include <string.h>
#include <thunder_private.h>
#include <errno.h>
#include <libfdt.h>

#define PAR_EL1_F	(1 << 0)

extern int spi_nor_init(void);
extern int spi_nor_rw_data(int write, unsigned long addr, int size, void *buf, int buf_size);

/* Cavium OEM Service UUID */
DEFINE_SVC_UUID2(thunder_svc_uid,
		0x6ff498cf,  0x9cfa, 0x5a4e, 0xa4, 0x3a,
		0x2a, 0x2f, 0x05, 0xb1, 0x45, 0x59);

/* Setup Standard Services */
static int32_t thunder_svc_setup(void)
{
	return 0;
}

/*
 * Top-level OEM Service SMC handler
 */
uintptr_t thunder_svc_smc_handler(uint32_t smc_fid,
				 u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *cookie,
				 void *handle,
				 u_register_t flags)
{
	unsigned int write;
	uintptr_t offset, user_buf, size, xfer_len;
	int64_t ret = 0;
	uint64_t buffer[512 / sizeof(uint64_t)];

	VERBOSE("ThunderX Service Call: 0x%x\n", smc_fid);

	switch (smc_fid) {
	case THUNDERX_SVC_CALL_COUNT:
		/*
		 * Return the number of Service Calls.
		 */
		SMC_RET1(handle, THUNDERX_NUM_CALLS);

	case THUNDERX_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, thunder_svc_uid);

	case THUNDERX_SVC_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, THUNDERX_VERSION_MAJOR, THUNDERX_VERSION_MINOR);

	case THUNDERX_NOR_WRITE:
	case THUNDERX_NOR_READ:
		spi_nor_init();

		offset = x1;
		size = x2;
		user_buf = x3;

		write = (smc_fid == THUNDERX_NOR_WRITE) ? 1 : 0;

		while (size > 0) {
			xfer_len = size < sizeof(buffer) ? size : sizeof(buffer);

			if (write) {
				memcpy((void *)buffer, (void *)user_buf, xfer_len);
			}

			ret = spi_nor_rw_data(write, offset, xfer_len, buffer, xfer_len);

			if (ret < 0)
				break;

			if (!write) {
				memcpy((void *)user_buf, (void *)buffer, xfer_len);
			}

			offset += xfer_len;
			user_buf += xfer_len;
			size -= xfer_len;
		}

		ret = (ret == 0) ? x2 : ret;

		SMC_RET1(handle, ret);

	case THUNDERX_NOR_ERASE:
		spi_nor_init();
		offset = x1;
		ret = spi_nor_erase_sect(offset);
		SMC_RET1(handle, ret);

	case THUNDERX_DRAM_SIZE:
		ret = thunder_dram_size_node(x1);
		WARN("DRAM size for %lx: %llx\n", x1, ret);
		SMC_RET1(handle, ret);

	case THUNDERX_NODE_COUNT:
		ret = thunder_get_node_count();
		SMC_RET1(handle, ret);

	case THUNDERX_PUTC:
		putchar(x1);
		SMC_RET1(handle, 0);
	default:
		WARN("Unimplemented ThunderX Service Call: 0x%x \n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/* Register Standard Service Calls as runtime service */
DECLARE_RT_SVC(
		thunder_svc,
		OEN_OEM_START,
		OEN_OEM_END,
		SMC_TYPE_FAST,
		thunder_svc_setup,
		thunder_svc_smc_handler
);
