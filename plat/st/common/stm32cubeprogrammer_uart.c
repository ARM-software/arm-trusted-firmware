/*
 * Copyright (c) 2021-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_iwdg.h>
#include <drivers/st/stm32_uart.h>
#include <drivers/st/stm32_uart_regs.h>
#include <lib/mmio.h>
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>
#include <stm32cubeprogrammer.h>

/* USART bootloader protocol version V4.0 */
#define USART_BL_VERSION	0x40U

/* Command definition */
#define GET_CMD_COMMAND		0x00U
#define GET_VER_COMMAND		0x01U
#define GET_ID_COMMAND		0x02U
#define PHASE_COMMAND		0x03U
#define READ_PART_COMMAND	0x12U
#define START_COMMAND		0x21U
#define DOWNLOAD_COMMAND	0x31U

/* Answer defines */
#define INIT_BYTE		0x7FU
#define ACK_BYTE		0x79U
#define NACK_BYTE		0x1FU
#define ABORT			0x5FU

#define UNDEFINED_DOWN_ADDR	U(0xFFFFFFFF)
#define PROGRAMMER_TIMEOUT_US	20000U

static const uint8_t command_tab[] = {
	GET_CMD_COMMAND,
	GET_VER_COMMAND,
	GET_ID_COMMAND,
	PHASE_COMMAND,
	START_COMMAND,
	DOWNLOAD_COMMAND
};

/* STM32CubeProgrammer over UART handle */
struct stm32prog_uart_handle_s {
	struct stm32_uart_handle_s uart;
	uint32_t packet;
	uint8_t *addr;
	uint32_t len;
	uint8_t phase;
	/* Error msg buffer: max 255 in UART protocol, reduced in TF-A */
	uint8_t error[64];
} handle;

/* Trace and handle unrecoverable UART protocol error */
#define STM32PROG_ERROR(...) \
	{ \
		ERROR(__VA_ARGS__); \
		if (handle.phase != PHASE_RESET) { \
			snprintf((char *)&handle.error, sizeof(handle.error), __VA_ARGS__); \
			handle.phase = PHASE_RESET; \
			handle.addr = (uint8_t *)UNDEFINED_DOWN_ADDR; \
			handle.len = 0U; \
			handle.packet = 0U; \
		} \
	}

static int uart_write(const uint8_t *addr, uint16_t size)
{
	while (size != 0U) {
		if (stm32_uart_putc(&handle.uart, *addr) != 0) {
			return -EIO;
		}
		size--;
		addr++;
	}

	return 0;
}

static int uart_write_8(uint8_t byte)
{
	return stm32_uart_putc(&handle.uart, byte);
}

static int uart_write_32(uint32_t value)
{
	return uart_write((uint8_t *)&value, 4U);
}

static int uart_read_8(uint8_t *byte)
{
	int ret;
	uint64_t timeout_ref = timeout_init_us(PROGRAMMER_TIMEOUT_US);

	do {
		ret = stm32_uart_getc(&handle.uart);
		if (ret == -EAGAIN) {
			if (timeout_elapsed(timeout_ref)) {
				return -ETIMEDOUT;
			}
		} else if (ret < 0) {
			return ret;
		}
	} while (ret == -EAGAIN);

	*byte = (uint8_t)ret;

	return 0;
}

static int uart_send_result(uint8_t byte)
{
	int ret;

	/* Always flush fifo before to send result = read all pending data */
	do {
		ret = stm32_uart_getc(&handle.uart);
	} while (ret >= 0);

	return uart_write_8(byte);
}

static bool is_valid_header(fip_toc_header_t *header)
{
	return (header->name == TOC_HEADER_NAME) &&
	       (header->serial_number != 0U);
}

static int uart_receive_command(uint8_t *command)
{
	uint8_t byte = 0U;
	uint8_t xor = 0U;
	unsigned int count;
	bool found = false;
	int ret;

	/* Repeat read until something is received */
	do {
		stm32_iwdg_refresh();
		ret = uart_read_8(&byte);
	} while (ret == -ETIMEDOUT);

	if (ret != 0) {
		return ret;
	}

	/* Handle reconnection request */
	if (byte == INIT_BYTE) {
		*command = byte;
		return 0;
	}

	for (count = 0U; count < ARRAY_SIZE(command_tab); count++) {
		if (command_tab[count] == byte) {
			found = true;
			break;
		}
	}
	if (!found) {
		VERBOSE("UART: Command unknown (byte=0x%x)\n", byte);
		return -EPROTO;
	}

	ret = uart_read_8(&xor);
	if (ret != 0) {
		return ret;
	}
	if ((byte ^ xor) != 0xFF) {
		VERBOSE("UART: Command XOR check fail (byte=0x%x, xor=0x%x)\n",
			byte, xor);
		return -EPROTO;
	}

	*command = byte;

	return 0;
}

static int get_cmd_command(void)
{
	const uint8_t msg[2] = {
		sizeof(command_tab), /* Length of data - 1 */
		USART_BL_VERSION
	};
	int ret;

	ret = uart_write(msg, sizeof(msg));
	if (ret != 0) {
		return ret;
	}

	return uart_write(command_tab, sizeof(command_tab));
}

static int get_version_command(void)
{
	return uart_write_8(STM32_TF_VERSION);
}

static int get_id_command(void)
{
	uint8_t msg[3] = {
		sizeof(msg) - 1 /* Length of data - 1 */
	};
	uint32_t chip_id = stm32mp_get_chip_dev_id();

	be16enc(&msg[1], chip_id);

	return uart_write(msg, sizeof(msg));
}

static int uart_send_phase(uint32_t address)
{
	int ret;
	uint8_t msg_size = 5U; /* Length of data - 1 */
	uint8_t error_size = 0U;

	/* Additional information only for RESET phase */
	if (handle.phase == PHASE_RESET) {
		error_size = strnlen((char *)&handle.error, sizeof(handle.error));
	}
	ret = uart_write_8(msg_size + error_size);
	if (ret != 0) {
		return ret;
	}

	/* Send the ID of next partition */
	ret = uart_write_8(handle.phase);
	if (ret != 0) {
		return ret;
	}

	/* Destination address */
	ret = uart_write_32(address);
	if (ret != 0) {
		return ret;
	}

	ret = uart_write_8(error_size);
	if (ret != 0) {
		return ret;
	}

	/* Additional information: message error */
	if (error_size > 0U) {
		ret = uart_write(handle.error, error_size);
	}

	return ret;
}

static int uart_download_part(void)
{
	uint8_t operation = 0U;
	uint8_t xor;
	uint8_t byte = 0U;
	uint32_t packet_number = 0U;
	uint32_t packet_size = 0U;
	uint32_t i = 0U;
	int ret;

	/* Get operation number */
	ret = uart_read_8(&operation);
	if (ret != 0) {
		return ret;
	}

	xor = operation;

	/* Get packet number */
	for (i = 3U; i != 0U; i--) {
		ret = uart_read_8(&byte);
		if (ret != 0) {
			return ret;
		}

		xor ^= byte;
		packet_number = (packet_number << 8) | byte;
	}

	if (packet_number != handle.packet) {
		WARN("UART: Bad packet number receive: %u, expected %u\n",
		     packet_number, handle.packet);
		return -EPROTO;
	}

	/* Checksum */
	ret = uart_read_8(&byte);
	if (ret != 0) {
		return ret;
	}
	if (xor != byte) {
		VERBOSE("UART: Download Command checksum xor: %x, received %x\n",
			xor, byte);
		return -EPROTO;
	}

	ret = uart_send_result(ACK_BYTE);
	if (ret != 0) {
		return ret;
	}

	ret = uart_read_8(&byte);
	if (ret != 0) {
		return ret;
	}
	xor = byte;
	packet_size = byte + 1U;
	if (handle.len < packet_size) {
		STM32PROG_ERROR("Download overflow at %p\n", handle.addr + packet_size);
		return 0;
	}

	for (i = 0U; i < packet_size; i++) {
		ret = uart_read_8(&byte);
		if (ret != 0) {
			return ret;
		}

		*(handle.addr + i) = byte;
		xor ^= byte;
	}

	/* Checksum */
	ret = uart_read_8(&byte) != 0;
	if (ret != 0) {
		return ret;
	}
	if (xor != byte) {
		VERBOSE("UART: Download Data checksum xor: %x, received %x\n",
			xor, byte);
		return -EPROTO;
	}

	/* Packet treated */
	handle.packet++;
	handle.addr += packet_size;
	handle.len -= packet_size;

	return 0;
}

static int uart_start_cmd(uintptr_t buffer)
{
	uint8_t byte = 0U;
	uint8_t xor = 0U;
	uint32_t i;
	uint32_t start_address = 0U;
	int ret;

	/* Get address */
	for (i = 4U; i != 0U; i--) {
		ret = uart_read_8(&byte);
		if (ret != 0U) {
			return ret;
		}

		xor ^= byte;
		start_address = (start_address << 8) | byte;
	}

	/* Checksum */
	ret = uart_read_8(&byte);
	if (ret != 0) {
		return ret;
	}

	if (xor != byte) {
		VERBOSE("UART: Start Command checksum xor: %x, received %x\n",
			xor, byte);
		return -EPROTO;
	}

	if (start_address != UNDEFINED_DOWN_ADDR) {
		STM32PROG_ERROR("Invalid start at %x, for phase %u\n",
				start_address, handle.phase);
		return 0;
	}

	if (!is_valid_header((fip_toc_header_t *)buffer)) {
		STM32PROG_ERROR("FIP Header check failed %lx, for phase %u\n",
				buffer, handle.phase);
		return -EIO;
	}
	VERBOSE("FIP header looks OK.\n");

	return 0;
}

static int uart_read(uint8_t id, uintptr_t buffer, size_t length)
{
	bool start_done = false;
	int ret;
	uint8_t command = 0U;

	handle.phase = id;
	handle.packet = 0U;
	handle.addr = (uint8_t *)buffer;
	handle.len = length;

	INFO("UART: read phase %u at 0x%lx size 0x%x\n",
	     id, buffer, length);
	while (!start_done) {
		ret = uart_receive_command(&command);
		if (ret != 0) {
			/* Delay to wait STM32CubeProgrammer end of transmission */
			mdelay(3);

			ret = uart_send_result(NACK_BYTE);
			if (ret != 0U) {
				return ret;
			}

			continue;
		}

		uart_send_result(ACK_BYTE);

		switch (command) {
		case INIT_BYTE:
			INFO("UART: Connected\n");
			/* Nothing to do */
			continue;

		case GET_CMD_COMMAND:
			ret = get_cmd_command();
			break;

		case GET_VER_COMMAND:
			ret = get_version_command();
			break;

		case GET_ID_COMMAND:
			ret = get_id_command();
			break;

		case PHASE_COMMAND:
			ret = uart_send_phase((uint32_t)buffer);
			if ((ret == 0) && (handle.phase == PHASE_RESET)) {
				start_done = true;
				INFO("UART: Reset\n");
			}
			break;

		case DOWNLOAD_COMMAND:
			ret = uart_download_part();
			break;

		case START_COMMAND:
			ret = uart_start_cmd(buffer);
			if ((ret == 0) && (handle.phase == id)) {
				INFO("UART: Start phase %u\n", handle.phase);
				start_done = true;
			}
			break;

		default:
			WARN("UART: Unknown command\n");
			ret = -EINVAL;
			break;
		}

		if (ret == 0) {
			ret = uart_send_result(ACK_BYTE);
		} else {
			ret = uart_send_result(NACK_BYTE);
		}
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/* Init UART: 115200, 8bit 1stop parity even and enable FIFO mode */
const struct stm32_uart_init_s init = {
	.baud_rate = STM32MP_UART_BAUDRATE,
	.word_length = STM32_UART_WORDLENGTH_9B,
	.stop_bits = STM32_UART_STOPBITS_1,
	.parity = STM32_UART_PARITY_EVEN,
	.hw_flow_control = STM32_UART_HWCONTROL_NONE,
	.mode = STM32_UART_MODE_TX_RX,
	.fifo_mode = STM32_UART_FIFOMODE_EN,
};

int stm32cubeprog_uart_load(uintptr_t instance, uintptr_t base, size_t len)
{
	int ret;

	if (stm32_uart_init(&handle.uart, instance, &init) != 0) {
		return -EIO;
	}

	/*
	 * The following NACK_BYTE is written because STM32CubeProgrammer has
	 * already sent its command before TF-A has reached this point, and
	 * because FIFO was not configured by BootROM.
	 * The byte in the UART_RX register is then the checksum and not the
	 * command. NACK_BYTE has to be written, so that the programmer will
	 * re-send the good command.
	 */
	ret = uart_send_result(NACK_BYTE);
	if (ret != 0) {
		return ret;
	}

	return uart_read(PHASE_SSBL, base, len);
}
