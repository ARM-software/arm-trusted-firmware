/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <sspm.h>

static void memcpy_to_sspm(uint32_t dst, uint32_t *src, uint32_t len)
{
	while (len--) {
		mmio_write_32(dst, *src);
		dst += sizeof(uint32_t);
		src++;
	}
}

static void memcpy_from_sspm(uint32_t *dst, uint32_t src, uint32_t len)
{
	while (len--) {
		*dst = mmio_read_32(src);
		dst++;
		src += sizeof(uint32_t);
	}
}

int sspm_mbox_read(uint32_t slot, uint32_t *data, uint32_t len)
{
	if (slot >= 32)	{
		ERROR("%s:slot = %d\n", __func__, slot);
		return -EINVAL;
	}

	if (data)
		memcpy_from_sspm(data,
				 MBOX3_BASE + slot * 4,
				 len);

	return 0;
}

int sspm_mbox_write(uint32_t slot, uint32_t *data, uint32_t len)
{
	if (slot >= 32) {
		ERROR("%s:slot = %d\n", __func__, slot);
		return -EINVAL;
	}

	if (data)
		memcpy_to_sspm(MBOX3_BASE + slot * 4,
			       data,
			       len);

	return 0;
}

static int sspm_ipi_check_ack(uint32_t id)
{
	int ret = 0;

	if (id == IPI_ID_PLATFORM) {
		if ((mmio_read_32(MBOX0_BASE + MBOX_IN_IRQ_OFS) & 0x1) == 0x1)
			ret = -EINPROGRESS;
	} else if (id == IPI_ID_SUSPEND) {
		if ((mmio_read_32(MBOX1_BASE + MBOX_IN_IRQ_OFS) & 0x2) == 0x2)
			ret = -EINPROGRESS;
	} else {
		ERROR("%s: id = %d\n", __func__, id);
		ret = -EINVAL;
	}

	return ret;
}

int sspm_ipi_send_non_blocking(uint32_t id, uint32_t *data)
{
	int ret = 0;

	ret = sspm_ipi_check_ack(id);
	if (ret)
		return ret;

	if (id == IPI_ID_PLATFORM) {
		memcpy_to_sspm(MBOX0_BASE + PINR_OFFSET_PLATFORM * 4,
			       data,
			       PINR_SIZE_PLATFORM);
		dsb();
		mmio_write_32(MBOX0_BASE + MBOX_OUT_IRQ_OFS, 0x1);
	} else if (id == IPI_ID_SUSPEND) {
		memcpy_to_sspm(MBOX1_BASE + PINR_OFFSET_SUSPEND * 4,
			       data,
			       PINR_SIZE_SUSPEND);
		dsb();
		mmio_write_32(MBOX1_BASE + MBOX_OUT_IRQ_OFS,
			      0x2);
	}

	return 0;
}

int sspm_ipi_recv_non_blocking(uint32_t id, uint32_t *data, uint32_t len)
{
	int ret = 0;

	ret = sspm_ipi_check_ack(id);
	if (ret == -EINPROGRESS) {
		if (id == IPI_ID_PLATFORM) {
			memcpy_from_sspm(data,
					 MBOX0_BASE + PINR_OFFSET_PLATFORM * 4,
					 len);
			dsb();
			/* clear interrupt bit*/
			mmio_write_32(MBOX0_BASE + MBOX_IN_IRQ_OFS,
				      0x1);
			ret = 0;
		} else if (id == IPI_ID_SUSPEND) {
			memcpy_from_sspm(data,
					 MBOX1_BASE + PINR_OFFSET_SUSPEND * 4,
					 len);
			dsb();
			/* clear interrupt bit*/
			mmio_write_32(MBOX1_BASE + MBOX_IN_IRQ_OFS,
				      0x2);
			ret = 0;
		}
	} else if (ret == 0) {
		ret = -EBUSY;
	}

	return ret;
}

int sspm_alive_show(void)
{
	uint32_t ipi_data, count;
	int ret = 0;

	count = 5;
	ipi_data = 0xdead;

	if (sspm_ipi_send_non_blocking(IPI_ID_PLATFORM, &ipi_data) != 0) {
		ERROR("sspm init send fail! ret=%d\n", ret);
		return -1;
	}

	while (sspm_ipi_recv_non_blocking(IPI_ID_PLATFORM,
					  &ipi_data,
					  sizeof(ipi_data) / sizeof(uint32_t))
					  && count) {
		mdelay(100);
		count--;
	}

	return (ipi_data == 1) ? 0 : -1;
}
