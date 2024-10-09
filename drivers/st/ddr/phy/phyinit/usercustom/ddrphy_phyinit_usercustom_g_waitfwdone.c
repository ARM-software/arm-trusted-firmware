/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include <ddrphy_phyinit.h>

#include <drivers/delay_timer.h>

#include <lib/mmio.h>

#include <platform_def.h>

/* Firmware major messages */
#define FW_MAJ_MSG_TRAINING_SUCCESS	0x0000007U
#define FW_MAJ_MSG_START_STREAMING	0x0000008U
#define FW_MAJ_MSG_TRAINING_FAILED	0x00000FFU

#define PHYINIT_DELAY_1US		1U
#define PHYINIT_DELAY_10US		10U
#define PHYINIT_TIMEOUT_US_1S		1000000U

static int wait_uctwriteprotshadow(bool state)
{
	uint64_t timeout;
	uint16_t read_data;
	uint16_t value = state ? BIT(0) : 0U;

	timeout = timeout_init_us(PHYINIT_TIMEOUT_US_1S);

	do {
		read_data = mmio_read_16((uintptr_t)(DDRPHYC_BASE +
						     (4U * (TAPBONLY | CSR_UCTSHADOWREGS_ADDR))));
		udelay(PHYINIT_DELAY_1US);
		if (timeout_elapsed(timeout)) {
			return -1;
		}
	} while ((read_data & BIT(0)) != value);

	return 0;
}

static int ack_message_receipt(void)
{
	int ret;

	/* Acknowledge the receipt of the message */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_DCTWRITEPROT_ADDR))), 0U);

	udelay(PHYINIT_DELAY_1US);

	ret = wait_uctwriteprotshadow(true);
	if (ret != 0) {
		return ret;
	}

	/* Complete the 4-phase protocol */
	mmio_write_16((uintptr_t)(DDRPHYC_BASE + (4U * (TAPBONLY | CSR_DCTWRITEPROT_ADDR))), 1U);

	udelay(PHYINIT_DELAY_1US);

	return 0;
}

static int get_major_message(uint32_t *msg)
{
	uint16_t message_number;
	int ret;

	ret = wait_uctwriteprotshadow(false);
	if (ret != 0) {
		return ret;
	}

	message_number = mmio_read_16((uintptr_t)(DDRPHYC_BASE +
							    (4U * (TAPBONLY |
								   CSR_UCTWRITEONLYSHADOW_ADDR))));

	ret = ack_message_receipt();
	if (ret != 0) {
		return ret;
	}

	*msg = (uint32_t)message_number;

	return 0;
}

static int get_streaming_message(uint32_t *msg)
{
	uint16_t stream_word_lower_part;
	uint16_t stream_word_upper_part;
	int ret;

	ret = wait_uctwriteprotshadow(false);
	if (ret != 0) {
		return ret;
	}

	stream_word_lower_part = mmio_read_16((uintptr_t)(DDRPHYC_BASE +
							  (4U * (TAPBONLY |
								 CSR_UCTWRITEONLYSHADOW_ADDR))));

	stream_word_upper_part = mmio_read_16((uintptr_t)(DDRPHYC_BASE +
							  (4U * (TAPBONLY |
								 CSR_UCTDATWRITEONLYSHADOW_ADDR))));

	ret = ack_message_receipt();
	if (ret != 0) {
		return ret;
	}

	*msg = (uint32_t)stream_word_lower_part | ((uint32_t)stream_word_upper_part << 16);

	return 0;
}

/*
 * Implements the mechanism to wait for completion of training firmware execution.
 *
 * The purpose of user this function is to wait for firmware to finish training.
 * The user can either implement a counter to wait or implement the polling
 * mechanism (our choice here). The wait time is highly dependent on the training features
 * enabled via sequencectrl input to the message block.
 *
 * The default behavior of this function is to print comments relating to this
 * process. A function call of the same name will be printed in the output text
 * file.
 *
 * The user can choose to leave this function as is, or implement mechanism to
 * trigger mailbox poling event in simulation.
 *
 * \return 0 on success.
 */
int ddrphy_phyinit_usercustom_g_waitfwdone(void)
{
	uint32_t fw_major_message;
	int ret;

	do {
		ret = get_major_message(&fw_major_message);
		if (ret != 0) {
			return ret;
		}

		VERBOSE("fw_major_message = %x\n", (unsigned int)fw_major_message);

		if (fw_major_message == FW_MAJ_MSG_START_STREAMING) {
			uint32_t i;
			uint32_t read_data;
			uint32_t stream_len;

			ret = get_streaming_message(&read_data);
			if (ret != 0) {
				return ret;
			}

			stream_len = read_data & 0xFFFFU;

			for (i = 0U; i < stream_len; i++) {
				ret = get_streaming_message(&read_data);
				if (ret != 0) {
					return ret;
				}

				VERBOSE("streaming message = %x\n", (unsigned int)read_data);
			}
		}
	} while ((fw_major_message != FW_MAJ_MSG_TRAINING_SUCCESS) &&
		 (fw_major_message != FW_MAJ_MSG_TRAINING_FAILED));

	udelay(PHYINIT_DELAY_10US);

	if (fw_major_message == FW_MAJ_MSG_TRAINING_FAILED) {
		ERROR("%s Training has failed.\n", __func__);
		return -1;
	}

	return 0;
}
