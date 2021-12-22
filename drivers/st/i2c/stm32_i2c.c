/*
 * Copyright (c) 2016-2021, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32_i2c.h>
#include <lib/mmio.h>
#include <lib/utils.h>

/* STM32 I2C registers offsets */
#define I2C_CR1			0x00U
#define I2C_CR2			0x04U
#define I2C_OAR1		0x08U
#define I2C_OAR2		0x0CU
#define I2C_TIMINGR		0x10U
#define I2C_TIMEOUTR		0x14U
#define I2C_ISR			0x18U
#define I2C_ICR			0x1CU
#define I2C_PECR		0x20U
#define I2C_RXDR		0x24U
#define I2C_TXDR		0x28U

#define TIMINGR_CLEAR_MASK	0xF0FFFFFFU

#define MAX_NBYTE_SIZE		255U

#define I2C_NSEC_PER_SEC	1000000000L

/* I2C Timing hard-coded value, for I2C clock source is HSI at 64MHz */
#define I2C_TIMING			0x10D07DB5

static void notif_i2c_timeout(struct i2c_handle_s *hi2c)
{
	hi2c->i2c_err |= I2C_ERROR_TIMEOUT;
	hi2c->i2c_mode = I2C_MODE_NONE;
	hi2c->i2c_state = I2C_STATE_READY;
}

/*
 * @brief  Configure I2C Analog noise filter.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C peripheral.
 * @param  analog_filter: New state of the Analog filter
 * @retval 0 if OK, negative value else
 */
static int i2c_config_analog_filter(struct i2c_handle_s *hi2c,
				    uint32_t analog_filter)
{
	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	hi2c->lock = 1;

	hi2c->i2c_state = I2C_STATE_BUSY;

	/* Disable the selected I2C peripheral */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	/* Reset I2Cx ANOFF bit */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_ANFOFF);

	/* Set analog filter bit*/
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR1, analog_filter);

	/* Enable the selected I2C peripheral */
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	hi2c->i2c_state = I2C_STATE_READY;

	hi2c->lock = 0;

	return 0;
}

/*
 * @brief  Get I2C setup information from the device tree and set pinctrl
 *         configuration.
 * @param  fdt: Pointer to the device tree
 * @param  node: I2C node offset
 * @param  init: Ref to the initialization configuration structure
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_get_setup_from_fdt(void *fdt, int node,
				 struct stm32_i2c_init_s *init)
{
	const fdt32_t *cuint;

	cuint = fdt_getprop(fdt, node, "i2c-scl-rising-time-ns", NULL);
	if (cuint == NULL) {
		init->rise_time = STM32_I2C_RISE_TIME_DEFAULT;
	} else {
		init->rise_time = fdt32_to_cpu(*cuint);
	}

	cuint = fdt_getprop(fdt, node, "i2c-scl-falling-time-ns", NULL);
	if (cuint == NULL) {
		init->fall_time = STM32_I2C_FALL_TIME_DEFAULT;
	} else {
		init->fall_time = fdt32_to_cpu(*cuint);
	}

	cuint = fdt_getprop(fdt, node, "clock-frequency", NULL);
	if (cuint == NULL) {
		init->speed_mode = STM32_I2C_SPEED_DEFAULT;
	} else {
		switch (fdt32_to_cpu(*cuint)) {
		case STANDARD_RATE:
			init->speed_mode = I2C_SPEED_STANDARD;
			break;
		case FAST_RATE:
			init->speed_mode = I2C_SPEED_FAST;
			break;
		case FAST_PLUS_RATE:
			init->speed_mode = I2C_SPEED_FAST_PLUS;
			break;
		default:
			init->speed_mode = STM32_I2C_SPEED_DEFAULT;
			break;
		}
	}

	return dt_set_pinctrl_config(node);
}

/*
 * @brief  Initialize the I2C device.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  init_data: Initialization configuration structure
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_init(struct i2c_handle_s *hi2c,
		   struct stm32_i2c_init_s *init_data)
{
	int rc = 0;
	uint32_t timing = I2C_TIMING;

	if (hi2c == NULL) {
		return -ENOENT;
	}

	if (hi2c->i2c_state == I2C_STATE_RESET) {
		hi2c->lock = 0;
	}

	hi2c->i2c_state = I2C_STATE_BUSY;

	clk_enable(hi2c->clock);

	/* Disable the selected I2C peripheral */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	/* Configure I2Cx: Frequency range */
	mmio_write_32(hi2c->i2c_base_addr + I2C_TIMINGR,
		      timing & TIMINGR_CLEAR_MASK);

	/* Disable Own Address1 before set the Own Address1 configuration */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_OAR1, I2C_OAR1_OA1EN);

	/* Configure I2Cx: Own Address1 and ack own address1 mode */
	if (init_data->addressing_mode == I2C_ADDRESSINGMODE_7BIT) {
		mmio_write_32(hi2c->i2c_base_addr + I2C_OAR1,
			      I2C_OAR1_OA1EN | init_data->own_address1);
	} else { /* I2C_ADDRESSINGMODE_10BIT */
		mmio_write_32(hi2c->i2c_base_addr + I2C_OAR1,
			      I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE |
			      init_data->own_address1);
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_CR2, 0);

	/* Configure I2Cx: Addressing Master mode */
	if (init_data->addressing_mode == I2C_ADDRESSINGMODE_10BIT) {
		mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_CR2_ADD10);
	}

	/*
	 * Enable the AUTOEND by default, and enable NACK
	 * (should be disabled only during Slave process).
	 */
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR2,
			I2C_CR2_AUTOEND | I2C_CR2_NACK);

	/* Disable Own Address2 before set the Own Address2 configuration */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_OAR2, I2C_DUALADDRESS_ENABLE);

	/* Configure I2Cx: Dual mode and Own Address2 */
	mmio_write_32(hi2c->i2c_base_addr + I2C_OAR2,
		      init_data->dual_address_mode |
		      init_data->own_address2 |
		      (init_data->own_address2_masks << 8));

	/* Configure I2Cx: Generalcall and NoStretch mode */
	mmio_write_32(hi2c->i2c_base_addr + I2C_CR1,
		      init_data->general_call_mode |
		      init_data->no_stretch_mode);

	/* Enable the selected I2C peripheral */
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	hi2c->i2c_err = I2C_ERROR_NONE;
	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode = I2C_MODE_NONE;

	rc = i2c_config_analog_filter(hi2c, init_data->analog_filter ?
						I2C_ANALOGFILTER_ENABLE :
						I2C_ANALOGFILTER_DISABLE);
	if (rc != 0) {
		ERROR("Cannot initialize I2C analog filter (%d)\n", rc);
		clk_disable(hi2c->clock);
		return rc;
	}

	clk_disable(hi2c->clock);

	return rc;
}

/*
 * @brief  I2C Tx data register flush process.
 * @param  hi2c: I2C handle
 * @retval None
 */
static void i2c_flush_txdr(struct i2c_handle_s *hi2c)
{
	/*
	 * If a pending TXIS flag is set,
	 * write a dummy data in TXDR to clear it.
	 */
	if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) & I2C_FLAG_TXIS) !=
	    0U) {
		mmio_write_32(hi2c->i2c_base_addr + I2C_TXDR, 0);
	}

	/* Flush TX register if not empty */
	if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) & I2C_FLAG_TXE) ==
	    0U) {
		mmio_setbits_32(hi2c->i2c_base_addr + I2C_ISR,
				I2C_FLAG_TXE);
	}
}

/*
 * @brief  This function handles I2C Communication timeout.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  flag: Specifies the I2C flag to check
 * @param  awaited_value: The awaited bit value for the flag (0 or 1)
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_flag(struct i2c_handle_s *hi2c, uint32_t flag,
			 uint8_t awaited_value, uint64_t timeout_ref)
{
	for ( ; ; ) {
		uint32_t isr = mmio_read_32(hi2c->i2c_base_addr + I2C_ISR);

		if (!!(isr & flag) != !!awaited_value) {
			return 0;
		}

		if (timeout_elapsed(timeout_ref)) {
			notif_i2c_timeout(hi2c);
			hi2c->lock = 0;

			return -EIO;
		}
	}
}

/*
 * @brief  This function handles Acknowledge failed detection during
 *	   an I2C Communication.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_ack_failed(struct i2c_handle_s *hi2c, uint64_t timeout_ref)
{
	if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) & I2C_FLAG_AF) == 0U) {
		return 0;
	}

	/*
	 * Wait until STOP Flag is reset.
	 * AutoEnd should be initiate after AF.
	 */
	while ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		I2C_FLAG_STOPF) == 0U) {
		if (timeout_elapsed(timeout_ref)) {
			notif_i2c_timeout(hi2c);
			hi2c->lock = 0;

			return -EIO;
		}
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_AF);

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

	i2c_flush_txdr(hi2c);

	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_RESET_CR2);

	hi2c->i2c_err |= I2C_ERROR_AF;
	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode = I2C_MODE_NONE;

	hi2c->lock = 0;

	return -EIO;
}

/*
 * @brief  This function handles I2C Communication timeout for specific usage
 *	   of TXIS flag.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_txis(struct i2c_handle_s *hi2c, uint64_t timeout_ref)
{
	while ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		I2C_FLAG_TXIS) == 0U) {
		if (i2c_ack_failed(hi2c, timeout_ref) != 0) {
			return -EIO;
		}

		if (timeout_elapsed(timeout_ref)) {
			notif_i2c_timeout(hi2c);
			hi2c->lock = 0;

			return -EIO;
		}
	}

	return 0;
}

/*
 * @brief  This function handles I2C Communication timeout for specific
 *	   usage of STOP flag.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_stop(struct i2c_handle_s *hi2c, uint64_t timeout_ref)
{
	while ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		 I2C_FLAG_STOPF) == 0U) {
		if (i2c_ack_failed(hi2c, timeout_ref) != 0) {
			return -EIO;
		}

		if (timeout_elapsed(timeout_ref)) {
			notif_i2c_timeout(hi2c);
			hi2c->lock = 0;

			return -EIO;
		}
	}

	return 0;
}

/*
 * @brief  Handles I2Cx communication when starting transfer or during transfer
 *	   (TC or TCR flag are set).
 * @param  hi2c: I2C handle
 * @param  dev_addr: Specifies the slave address to be programmed
 * @param  size: Specifies the number of bytes to be programmed.
 *   This parameter must be a value between 0 and 255.
 * @param  i2c_mode: New state of the I2C START condition generation.
 *   This parameter can be one of the following values:
 *     @arg @ref I2C_RELOAD_MODE: Enable Reload mode.
 *     @arg @ref I2C_AUTOEND_MODE: Enable Automatic end mode.
 *     @arg @ref I2C_SOFTEND_MODE: Enable Software end mode.
 * @param  request: New state of the I2C START condition generation.
 *   This parameter can be one of the following values:
 *     @arg @ref I2C_NO_STARTSTOP: Don't Generate stop and start condition.
 *     @arg @ref I2C_GENERATE_STOP: Generate stop condition
 *                                  (size should be set to 0).
 *     @arg @ref I2C_GENERATE_START_READ: Generate Restart for read request.
 *     @arg @ref I2C_GENERATE_START_WRITE: Generate Restart for write request.
 * @retval None
 */
static void i2c_transfer_config(struct i2c_handle_s *hi2c, uint16_t dev_addr,
				uint16_t size, uint32_t i2c_mode,
				uint32_t request)
{
	uint32_t clr_value, set_value;

	clr_value = (I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD |
		     I2C_CR2_AUTOEND | I2C_CR2_START | I2C_CR2_STOP) |
		(I2C_CR2_RD_WRN & (request >> (31U - I2C_CR2_RD_WRN_OFFSET)));

	set_value = ((uint32_t)dev_addr & I2C_CR2_SADD) |
		(((uint32_t)size << I2C_CR2_NBYTES_OFFSET) & I2C_CR2_NBYTES) |
		i2c_mode | request;

	mmio_clrsetbits_32(hi2c->i2c_base_addr + I2C_CR2, clr_value, set_value);
}

/*
 * @brief  Master sends target device address followed by internal memory
 *	   address for write request.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: Size of internal memory address
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_request_memory_write(struct i2c_handle_s *hi2c,
				    uint16_t dev_addr, uint16_t mem_addr,
				    uint16_t mem_add_size, uint64_t timeout_ref)
{
	i2c_transfer_config(hi2c, dev_addr, mem_add_size, I2C_RELOAD_MODE,
			    I2C_GENERATE_START_WRITE);

	if (i2c_wait_txis(hi2c, timeout_ref) != 0) {
		return -EIO;
	}

	if (mem_add_size == I2C_MEMADD_SIZE_8BIT) {
		/* Send Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	} else {
		/* Send MSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)((mem_addr & 0xFF00U) >> 8));

		if (i2c_wait_txis(hi2c, timeout_ref) != 0) {
			return -EIO;
		}

		/* Send LSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	}

	if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0, timeout_ref) != 0) {
		return -EIO;
	}

	return 0;
}

/*
 * @brief  Master sends target device address followed by internal memory
 *	   address for read request.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: Size of internal memory address
 * @param  timeout_ref: Reference to target timeout
 * @retval 0 if OK, negative value else
 */
static int i2c_request_memory_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
				   uint16_t mem_addr, uint16_t mem_add_size,
				   uint64_t timeout_ref)
{
	i2c_transfer_config(hi2c, dev_addr, mem_add_size, I2C_SOFTEND_MODE,
			    I2C_GENERATE_START_WRITE);

	if (i2c_wait_txis(hi2c, timeout_ref) != 0) {
		return -EIO;
	}

	if (mem_add_size == I2C_MEMADD_SIZE_8BIT) {
		/* Send Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	} else {
		/* Send MSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)((mem_addr & 0xFF00U) >> 8));

		if (i2c_wait_txis(hi2c, timeout_ref) != 0) {
			return -EIO;
		}

		/* Send LSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	}

	if (i2c_wait_flag(hi2c, I2C_FLAG_TC, 0, timeout_ref) != 0) {
		return -EIO;
	}

	return 0;
}
/*
 * @brief  Generic function to write an amount of data in blocking mode
 *         (for Memory Mode and Master Mode)
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address (if Memory Mode)
 * @param  mem_add_size: Size of internal memory address (if Memory Mode)
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @param  mode: Communication mode
 * @retval 0 if OK, negative value else
 */
static int i2c_write(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		     uint16_t mem_addr, uint16_t mem_add_size,
		     uint8_t *p_data, uint16_t size, uint32_t timeout_ms,
		     enum i2c_mode_e mode)
{
	uint64_t timeout_ref;
	int rc = -EIO;
	uint8_t *p_buff = p_data;
	uint32_t xfer_size;
	uint32_t xfer_count = size;

	if ((mode != I2C_MODE_MASTER) && (mode != I2C_MODE_MEM)) {
		return -1;
	}

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	if ((p_data == NULL) || (size == 0U)) {
		return -EINVAL;
	}

	clk_enable(hi2c->clock);

	hi2c->lock = 1;

	timeout_ref = timeout_init_us(I2C_TIMEOUT_BUSY_MS * 1000);
	if (i2c_wait_flag(hi2c, I2C_FLAG_BUSY, 1, timeout_ref) != 0) {
		goto bail;
	}

	hi2c->i2c_state = I2C_STATE_BUSY_TX;
	hi2c->i2c_mode = mode;
	hi2c->i2c_err = I2C_ERROR_NONE;

	timeout_ref = timeout_init_us(timeout_ms * 1000);

	if (mode == I2C_MODE_MEM) {
		/* In Memory Mode, Send Slave Address and Memory Address */
		if (i2c_request_memory_write(hi2c, dev_addr, mem_addr,
					     mem_add_size, timeout_ref) != 0) {
			goto bail;
		}

		if (xfer_count > MAX_NBYTE_SIZE) {
			xfer_size = MAX_NBYTE_SIZE;
			i2c_transfer_config(hi2c, dev_addr, xfer_size,
					    I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
		} else {
			xfer_size = xfer_count;
			i2c_transfer_config(hi2c, dev_addr, xfer_size,
					    I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
		}
	} else {
		/* In Master Mode, Send Slave Address */
		if (xfer_count > MAX_NBYTE_SIZE) {
			xfer_size = MAX_NBYTE_SIZE;
			i2c_transfer_config(hi2c, dev_addr, xfer_size,
					    I2C_RELOAD_MODE,
					    I2C_GENERATE_START_WRITE);
		} else {
			xfer_size = xfer_count;
			i2c_transfer_config(hi2c, dev_addr, xfer_size,
					    I2C_AUTOEND_MODE,
					    I2C_GENERATE_START_WRITE);
		}
	}

	do {
		if (i2c_wait_txis(hi2c, timeout_ref) != 0) {
			goto bail;
		}

		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR, *p_buff);
		p_buff++;
		xfer_count--;
		xfer_size--;

		if ((xfer_count != 0U) && (xfer_size == 0U)) {
			/* Wait until TCR flag is set */
			if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0,
					  timeout_ref) != 0) {
				goto bail;
			}

			if (xfer_count > MAX_NBYTE_SIZE) {
				xfer_size = MAX_NBYTE_SIZE;
				i2c_transfer_config(hi2c, dev_addr,
						    xfer_size,
						    I2C_RELOAD_MODE,
						    I2C_NO_STARTSTOP);
			} else {
				xfer_size = xfer_count;
				i2c_transfer_config(hi2c, dev_addr,
						    xfer_size,
						    I2C_AUTOEND_MODE,
						    I2C_NO_STARTSTOP);
			}
		}

	} while (xfer_count > 0U);

	/*
	 * No need to Check TC flag, with AUTOEND mode the stop
	 * is automatically generated.
	 * Wait until STOPF flag is reset.
	 */
	if (i2c_wait_stop(hi2c, timeout_ref) != 0) {
		goto bail;
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_RESET_CR2);

	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode  = I2C_MODE_NONE;

	rc = 0;

bail:
	hi2c->lock = 0;
	clk_disable(hi2c->clock);

	return rc;
}

/*
 * @brief  Write an amount of data in blocking mode to a specific memory
 *         address.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: Size of internal memory address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_mem_write(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			uint16_t mem_addr, uint16_t mem_add_size,
			uint8_t *p_data, uint16_t size, uint32_t timeout_ms)
{
	return i2c_write(hi2c, dev_addr, mem_addr, mem_add_size,
			 p_data, size, timeout_ms, I2C_MODE_MEM);
}

/*
 * @brief  Transmits in master mode an amount of data in blocking mode.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_master_transmit(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			      uint8_t *p_data, uint16_t size,
			      uint32_t timeout_ms)
{
	return i2c_write(hi2c, dev_addr, 0, 0,
			 p_data, size, timeout_ms, I2C_MODE_MASTER);
}

/*
 * @brief  Generic function to read an amount of data in blocking mode
 *         (for Memory Mode and Master Mode)
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address (if Memory Mode)
 * @param  mem_add_size: Size of internal memory address (if Memory Mode)
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @param  mode: Communication mode
 * @retval 0 if OK, negative value else
 */
static int i2c_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		    uint16_t mem_addr, uint16_t mem_add_size,
		    uint8_t *p_data, uint16_t size, uint32_t timeout_ms,
		    enum i2c_mode_e mode)
{
	uint64_t timeout_ref;
	int rc = -EIO;
	uint8_t *p_buff = p_data;
	uint32_t xfer_count = size;
	uint32_t xfer_size;

	if ((mode != I2C_MODE_MASTER) && (mode != I2C_MODE_MEM)) {
		return -1;
	}

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	if ((p_data == NULL) || (size == 0U)) {
		return  -EINVAL;
	}

	clk_enable(hi2c->clock);

	hi2c->lock = 1;

	timeout_ref = timeout_init_us(I2C_TIMEOUT_BUSY_MS * 1000);
	if (i2c_wait_flag(hi2c, I2C_FLAG_BUSY, 1, timeout_ref) != 0) {
		goto bail;
	}

	hi2c->i2c_state = I2C_STATE_BUSY_RX;
	hi2c->i2c_mode = mode;
	hi2c->i2c_err = I2C_ERROR_NONE;

	if (mode == I2C_MODE_MEM) {
		/* Send Memory Address */
		if (i2c_request_memory_read(hi2c, dev_addr, mem_addr,
					    mem_add_size, timeout_ref) != 0) {
			goto bail;
		}
	}

	/*
	 * Send Slave Address.
	 * Set NBYTES to write and reload if xfer_count > MAX_NBYTE_SIZE
	 * and generate RESTART.
	 */
	if (xfer_count > MAX_NBYTE_SIZE) {
		xfer_size = MAX_NBYTE_SIZE;
		i2c_transfer_config(hi2c, dev_addr, xfer_size,
				    I2C_RELOAD_MODE, I2C_GENERATE_START_READ);
	} else {
		xfer_size = xfer_count;
		i2c_transfer_config(hi2c, dev_addr, xfer_size,
				    I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);
	}

	do {
		if (i2c_wait_flag(hi2c, I2C_FLAG_RXNE, 0, timeout_ref) != 0) {
			goto bail;
		}

		*p_buff = mmio_read_8(hi2c->i2c_base_addr + I2C_RXDR);
		p_buff++;
		xfer_size--;
		xfer_count--;

		if ((xfer_count != 0U) && (xfer_size == 0U)) {
			if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0,
					  timeout_ref) != 0) {
				goto bail;
			}

			if (xfer_count > MAX_NBYTE_SIZE) {
				xfer_size = MAX_NBYTE_SIZE;
				i2c_transfer_config(hi2c, dev_addr,
						    xfer_size,
						    I2C_RELOAD_MODE,
						    I2C_NO_STARTSTOP);
			} else {
				xfer_size = xfer_count;
				i2c_transfer_config(hi2c, dev_addr,
						    xfer_size,
						    I2C_AUTOEND_MODE,
						    I2C_NO_STARTSTOP);
			}
		}
	} while (xfer_count > 0U);

	/*
	 * No need to Check TC flag, with AUTOEND mode the stop
	 * is automatically generated.
	 * Wait until STOPF flag is reset.
	 */
	if (i2c_wait_stop(hi2c, timeout_ref) != 0) {
		goto bail;
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_RESET_CR2);

	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode = I2C_MODE_NONE;

	rc = 0;

bail:
	hi2c->lock = 0;
	clk_disable(hi2c->clock);

	return rc;
}

/*
 * @brief  Read an amount of data in blocking mode from a specific memory
 *	   address.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: Size of internal memory address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_mem_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		       uint16_t mem_addr, uint16_t mem_add_size,
		       uint8_t *p_data, uint16_t size, uint32_t timeout_ms)
{
	return i2c_read(hi2c, dev_addr, mem_addr, mem_add_size,
			p_data, size, timeout_ms, I2C_MODE_MEM);
}

/*
 * @brief  Receives in master mode an amount of data in blocking mode.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout_ms: Timeout duration in milliseconds
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_master_receive(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			     uint8_t *p_data, uint16_t size,
			     uint32_t timeout_ms)
{
	return i2c_read(hi2c, dev_addr, 0, 0,
			p_data, size, timeout_ms, I2C_MODE_MASTER);
}

/*
 * @brief  Checks if target device is ready for communication.
 * @note   This function is used with Memory devices
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  trials: Number of trials
 * @param  timeout_ms: Timeout duration in milliseconds
 * @retval True if device is ready, false else
 */
bool stm32_i2c_is_device_ready(struct i2c_handle_s *hi2c,
			       uint16_t dev_addr, uint32_t trials,
			       uint32_t timeout_ms)
{
	uint32_t i2c_trials = 0U;
	bool rc = false;

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return rc;
	}

	clk_enable(hi2c->clock);

	hi2c->lock = 1;
	hi2c->i2c_mode = I2C_MODE_NONE;

	if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) & I2C_FLAG_BUSY) !=
	    0U) {
		goto bail;
	}

	hi2c->i2c_state = I2C_STATE_BUSY;
	hi2c->i2c_err = I2C_ERROR_NONE;

	do {
		uint64_t timeout_ref;

		/* Generate Start */
		if ((mmio_read_32(hi2c->i2c_base_addr + I2C_OAR1) &
		     I2C_OAR1_OA1MODE) == 0) {
			mmio_write_32(hi2c->i2c_base_addr + I2C_CR2,
				      (((uint32_t)dev_addr & I2C_CR2_SADD) |
				       I2C_CR2_START | I2C_CR2_AUTOEND) &
				      ~I2C_CR2_RD_WRN);
		} else {
			mmio_write_32(hi2c->i2c_base_addr + I2C_CR2,
				      (((uint32_t)dev_addr & I2C_CR2_SADD) |
				       I2C_CR2_START | I2C_CR2_ADD10) &
				      ~I2C_CR2_RD_WRN);
		}

		/*
		 * No need to Check TC flag, with AUTOEND mode the stop
		 * is automatically generated.
		 * Wait until STOPF flag is set or a NACK flag is set.
		 */
		timeout_ref = timeout_init_us(timeout_ms * 1000);
		do {
			if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
			     (I2C_FLAG_STOPF | I2C_FLAG_AF)) != 0U) {
				break;
			}

			if (timeout_elapsed(timeout_ref)) {
				notif_i2c_timeout(hi2c);
				goto bail;
			}
		} while (true);

		if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		     I2C_FLAG_AF) == 0U) {
			if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0,
					  timeout_ref) != 0) {
				goto bail;
			}

			mmio_write_32(hi2c->i2c_base_addr + I2C_ICR,
				      I2C_FLAG_STOPF);

			hi2c->i2c_state = I2C_STATE_READY;

			rc = true;
			goto bail;
		}

		if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0, timeout_ref) != 0) {
			goto bail;
		}

		mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_AF);

		mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

		if (i2c_trials == trials) {
			mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR2,
					I2C_CR2_STOP);

			if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0,
					  timeout_ref) != 0) {
				goto bail;
			}

			mmio_write_32(hi2c->i2c_base_addr + I2C_ICR,
				      I2C_FLAG_STOPF);
		}

		i2c_trials++;
	} while (i2c_trials < trials);

	notif_i2c_timeout(hi2c);

bail:
	hi2c->lock = 0;
	clk_disable(hi2c->clock);

	return rc;
}

