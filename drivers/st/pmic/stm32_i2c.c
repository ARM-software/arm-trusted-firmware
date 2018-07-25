/*
 * Copyright (c) 2016-2018, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stm32_i2c.h>

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

#define MAX_DELAY		0xFFFFFFFFU

/* I2C TIMING clear register Mask */
#define TIMING_CLEAR_MASK	0xF0FFFFFFU
/* Timeout 25 ms */
#define I2C_TIMEOUT_BUSY	25U

#define MAX_NBYTE_SIZE		255U

static int i2c_request_memory_write(struct i2c_handle_s *hi2c,
				    uint16_t dev_addr, uint16_t mem_addr,
				    uint16_t mem_add_size, uint32_t timeout,
				    uint32_t tick_start);
static int i2c_request_memory_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
				   uint16_t mem_addr, uint16_t mem_add_size,
				   uint32_t timeout, uint32_t tick_start);

/* Private functions to handle flags during polling transfer */
static int i2c_wait_flag(struct i2c_handle_s *hi2c, uint32_t flag,
			 uint8_t awaited_value, uint32_t timeout,
			 uint32_t tick_start);
static int i2c_wait_txis(struct i2c_handle_s *hi2c, uint32_t timeout,
			 uint32_t tick_start);
static int i2c_wait_stop(struct i2c_handle_s *hi2c, uint32_t timeout,
			 uint32_t tick_start);
static int i2c_ack_failed(struct i2c_handle_s *hi2c, uint32_t timeout,
			  uint32_t tick_start);

/* Private function to flush TXDR register */
static void i2c_flush_txdr(struct i2c_handle_s *hi2c);

/* Private function to start, restart or stop a transfer */
static void i2c_transfer_config(struct i2c_handle_s *hi2c, uint16_t dev_addr,
				uint16_t size, uint32_t i2c_mode,
				uint32_t request);

/*
 * @brief  Initialize the I2C device.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_init(struct i2c_handle_s *hi2c)
{
	if (hi2c == NULL) {
		return -ENOENT;
	}

	if (hi2c->i2c_state == I2C_STATE_RESET) {
		hi2c->lock = 0;
	}

	hi2c->i2c_state = I2C_STATE_BUSY;

	/* Disable the selected I2C peripheral */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	/* Configure I2Cx: Frequency range */
	mmio_write_32(hi2c->i2c_base_addr + I2C_TIMINGR,
		      hi2c->i2c_init.timing & TIMING_CLEAR_MASK);

	/* Disable Own Address1 before set the Own Address1 configuration */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_OAR1, I2C_OAR1_OA1EN);

	/* Configure I2Cx: Own Address1 and ack own address1 mode */
	if (hi2c->i2c_init.addressing_mode == I2C_ADDRESSINGMODE_7BIT) {
		mmio_write_32(hi2c->i2c_base_addr + I2C_OAR1,
			      I2C_OAR1_OA1EN | hi2c->i2c_init.own_address1);
	} else { /* I2C_ADDRESSINGMODE_10BIT */
		mmio_write_32(hi2c->i2c_base_addr + I2C_OAR1,
			      I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE |
			      hi2c->i2c_init.own_address1);
	}

	/* Configure I2Cx: Addressing Master mode */
	if (hi2c->i2c_init.addressing_mode == I2C_ADDRESSINGMODE_10BIT) {
		mmio_write_32(hi2c->i2c_base_addr + I2C_CR2, I2C_CR2_ADD10);
	}

	/*
	 * Enable the AUTOEND by default, and enable NACK
	 * (should be disable only during Slave process)
	 */
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR2,
			I2C_CR2_AUTOEND | I2C_CR2_NACK);

	/* Disable Own Address2 before set the Own Address2 configuration */
	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_OAR2, I2C_DUALADDRESS_ENABLE);

	/* Configure I2Cx: Dual mode and Own Address2 */
	mmio_write_32(hi2c->i2c_base_addr + I2C_OAR2,
		      hi2c->i2c_init.dual_address_mode |
		      hi2c->i2c_init.own_address2 |
		      (hi2c->i2c_init.own_address2_masks << 8));

	/* Configure I2Cx: Generalcall and NoStretch mode */
	mmio_write_32(hi2c->i2c_base_addr + I2C_CR1,
		      hi2c->i2c_init.general_call_mode |
		      hi2c->i2c_init.no_stretch_mode);

	/* Enable the selected I2C peripheral */
	mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR1, I2C_CR1_PE);

	hi2c->i2c_err = I2C_ERROR_NONE;
	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode = I2C_MODE_NONE;

	return 0;
}

/*
 * @brief  Write an amount of data in blocking mode to a specific memory address
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: size of internal memory address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout: timeout duration
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_mem_write(struct i2c_handle_s *hi2c, uint16_t dev_addr,
			uint16_t mem_addr, uint16_t mem_add_size,
			uint8_t *p_data, uint16_t size, uint32_t timeout)
{
	uint32_t tickstart;

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	if ((p_data == NULL) || (size == 0U)) {
		return -EINVAL;
	}

	hi2c->lock = 1;

	tickstart = (uint32_t)read_cntpct_el0();

	if (i2c_wait_flag(hi2c, I2C_FLAG_BUSY, 1, I2C_TIMEOUT_BUSY,
			  tickstart) != 0) {
		return -EIO;
	}

	hi2c->i2c_state     = I2C_STATE_BUSY_TX;
	hi2c->i2c_mode      = I2C_MODE_MEM;
	hi2c->i2c_err = I2C_ERROR_NONE;

	hi2c->p_buff  = p_data;
	hi2c->xfer_count = size;

	/* Send Slave Address and Memory Address */
	if (i2c_request_memory_write(hi2c, dev_addr, mem_addr, mem_add_size,
				     timeout, tickstart) != 0) {
		hi2c->lock = 0;
		return -EIO;
	}

	/*
	 * Set NBYTES to write and reload
	 * if hi2c->xfer_count > MAX_NBYTE_SIZE
	 */
	if (hi2c->xfer_count > MAX_NBYTE_SIZE) {
		hi2c->xfer_size = MAX_NBYTE_SIZE;
		i2c_transfer_config(hi2c, dev_addr, hi2c->xfer_size,
				    I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
	} else {
		hi2c->xfer_size = hi2c->xfer_count;
		i2c_transfer_config(hi2c, dev_addr, hi2c->xfer_size,
				    I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
	}

	do {
		if (i2c_wait_txis(hi2c, timeout, tickstart) != 0) {
			return -EIO;
		}

		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR, *hi2c->p_buff);
		hi2c->p_buff++;
		hi2c->xfer_count--;
		hi2c->xfer_size--;

		if ((hi2c->xfer_count != 0U) && (hi2c->xfer_size == 0U)) {
			/* Wait until TCR flag is set */
			if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0, timeout,
					  tickstart) != 0) {
				return -EIO;
		}

			if (hi2c->xfer_count > MAX_NBYTE_SIZE) {
				hi2c->xfer_size = MAX_NBYTE_SIZE;
				i2c_transfer_config(hi2c, dev_addr,
						    hi2c->xfer_size,
						    I2C_RELOAD_MODE,
						    I2C_NO_STARTSTOP);
			} else {
				hi2c->xfer_size = hi2c->xfer_count;
				i2c_transfer_config(hi2c, dev_addr,
						    hi2c->xfer_size,
						    I2C_AUTOEND_MODE,
						    I2C_NO_STARTSTOP);
			}
		}

	} while (hi2c->xfer_count > 0U);

	/*
	 * No need to Check TC flag, with AUTOEND mode the stop
	 * is automatically generated.
	 * Wait until STOPF flag is reset.
	 */
	if (i2c_wait_stop(hi2c, timeout, tickstart) != 0) {
		return -EIO;
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_RESET_CR2);

	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode  = I2C_MODE_NONE;

	hi2c->lock = 0;

	return 0;
}

/*
 * @brief  Read an amount of data in blocking mode from a specific memory
 *	   address
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: size of internal memory address
 * @param  p_data: Pointer to data buffer
 * @param  size: Amount of data to be sent
 * @param  timeout: timeout duration
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_mem_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
		       uint16_t mem_addr, uint16_t mem_add_size,
		       uint8_t *p_data, uint16_t size, uint32_t timeout)
{
	uint32_t tickstart;

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	if ((p_data == NULL) || (size == 0U)) {
		return  -EINVAL;
	}

	hi2c->lock = 1;

	tickstart = (uint32_t)read_cntpct_el0();

	if (i2c_wait_flag(hi2c, I2C_FLAG_BUSY, 1, I2C_TIMEOUT_BUSY,
			  tickstart) != 0) {
		return -EIO;
	}

	hi2c->i2c_state     = I2C_STATE_BUSY_RX;
	hi2c->i2c_mode      = I2C_MODE_MEM;
	hi2c->i2c_err = I2C_ERROR_NONE;

	hi2c->p_buff  = p_data;
	hi2c->xfer_count = size;

	/* Send Slave Address and Memory Address */
	if (i2c_request_memory_read(hi2c, dev_addr, mem_addr, mem_add_size,
				    timeout, tickstart) != 0) {
		hi2c->lock = 0;
		return -EIO;
	}

	/*
	 * Send Slave Address.
	 * Set NBYTES to write and reload if hi2c->xfer_count > MAX_NBYTE_SIZE
	 * and generate RESTART.
	 */
	if (hi2c->xfer_count > MAX_NBYTE_SIZE) {
		hi2c->xfer_size = MAX_NBYTE_SIZE;
		i2c_transfer_config(hi2c, dev_addr, hi2c->xfer_size,
				    I2C_RELOAD_MODE, I2C_GENERATE_START_READ);
	} else {
		hi2c->xfer_size = hi2c->xfer_count;
		i2c_transfer_config(hi2c, dev_addr, hi2c->xfer_size,
				    I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);
	}

	do {
		if (i2c_wait_flag(hi2c, I2C_FLAG_RXNE, 0, timeout,
				  tickstart) != 0) {
			return -EIO;
		}

		*hi2c->p_buff = mmio_read_8(hi2c->i2c_base_addr + I2C_RXDR);
		hi2c->p_buff++;
		hi2c->xfer_size--;
		hi2c->xfer_count--;

		if ((hi2c->xfer_count != 0U) && (hi2c->xfer_size == 0U)) {
			if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0, timeout,
					  tickstart) != 0) {
				return -EIO;
			}

			if (hi2c->xfer_count > MAX_NBYTE_SIZE) {
				hi2c->xfer_size = MAX_NBYTE_SIZE;
				i2c_transfer_config(hi2c, dev_addr,
						    hi2c->xfer_size,
						    I2C_RELOAD_MODE,
						    I2C_NO_STARTSTOP);
			} else {
				hi2c->xfer_size = hi2c->xfer_count;
				i2c_transfer_config(hi2c, dev_addr,
						    hi2c->xfer_size,
						    I2C_AUTOEND_MODE,
						    I2C_NO_STARTSTOP);
			}
		}
	} while (hi2c->xfer_count > 0U);

	/*
	 * No need to Check TC flag, with AUTOEND mode the stop
	 * is automatically generated
	 * Wait until STOPF flag is reset
	 */
	if (i2c_wait_stop(hi2c, timeout, tickstart) != 0) {
		return -EIO;
	}

	mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

	mmio_clrbits_32(hi2c->i2c_base_addr + I2C_CR2, I2C_RESET_CR2);

	hi2c->i2c_state = I2C_STATE_READY;
	hi2c->i2c_mode  = I2C_MODE_NONE;

	hi2c->lock = 0;

	return 0;
}

/*
 * @brief  Checks if target device is ready for communication.
 * @note   This function is used with Memory devices
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  trials: Number of trials
 * @param  timeout: timeout duration
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_is_device_ready(struct i2c_handle_s *hi2c,
			      uint16_t dev_addr, uint32_t trials,
			      uint32_t timeout)
{
	uint32_t i2c_trials = 0U;

	if ((hi2c->i2c_state != I2C_STATE_READY) || (hi2c->lock != 0U)) {
		return -EBUSY;
	}

	if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) & I2C_FLAG_BUSY) !=
	    0U) {
		return -EBUSY;
	}

	hi2c->lock = 1;

	hi2c->i2c_state = I2C_STATE_BUSY;
	hi2c->i2c_err = I2C_ERROR_NONE;

	do {
		uint32_t tickstart;

		/* Generate Start */
		if (hi2c->i2c_init.addressing_mode == I2C_ADDRESSINGMODE_7BIT) {
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
		 * is automatically generated
		 * Wait until STOPF flag is set or a NACK flag is set
		 */
		tickstart = (uint32_t)read_cntpct_el0();
		while (((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
			 (I2C_FLAG_STOPF | I2C_FLAG_AF)) == 0U) &&
		       (hi2c->i2c_state != I2C_STATE_TIMEOUT)) {
			if (timeout != MAX_DELAY) {
				if ((((uint32_t)read_cntpct_el0() - tickstart) >
				     timeout) || (timeout == 0U)) {
					hi2c->i2c_state = I2C_STATE_READY;

					hi2c->i2c_err |=
						I2C_ERROR_TIMEOUT;

					hi2c->lock = 0;

					return -EIO;
				}
			}
		}

		/* Check if the NACKF flag has not been set */
		if ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		     I2C_FLAG_AF) == 0U) {
			if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0, timeout,
					  tickstart) != 0) {
				return -EIO;
			}

			mmio_write_32(hi2c->i2c_base_addr + I2C_ICR,
				      I2C_FLAG_STOPF);

			hi2c->i2c_state = I2C_STATE_READY;

			hi2c->lock = 0;

			return 0;
		}

		if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0, timeout,
				  tickstart) != 0) {
			return -EIO;
		}

		mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_AF);

		mmio_write_32(hi2c->i2c_base_addr + I2C_ICR, I2C_FLAG_STOPF);

		if (i2c_trials == trials) {
			mmio_setbits_32(hi2c->i2c_base_addr + I2C_CR2,
					I2C_CR2_STOP);

			if (i2c_wait_flag(hi2c, I2C_FLAG_STOPF, 0, timeout,
					  tickstart) != 0) {
				return -EIO;
			}

			mmio_write_32(hi2c->i2c_base_addr + I2C_ICR,
				      I2C_FLAG_STOPF);
		}

		i2c_trials++;
	} while (i2c_trials < trials);

	hi2c->i2c_state = I2C_STATE_READY;

	hi2c->i2c_err |= I2C_ERROR_TIMEOUT;

	hi2c->lock = 0;

	return -EIO;
}

/*
 * @brief  Master sends target device address followed by internal memory
 *	   address for write request.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  dev_addr: Target device address
 * @param  mem_addr: Internal memory address
 * @param  mem_add_size: size of internal memory address
 * @param  timeout: timeout duration
 * @param  tick_start Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_request_memory_write(struct i2c_handle_s *hi2c,
				    uint16_t dev_addr, uint16_t mem_addr,
				    uint16_t mem_add_size, uint32_t timeout,
				    uint32_t tick_start)
{
	i2c_transfer_config(hi2c, dev_addr, mem_add_size, I2C_RELOAD_MODE,
			    I2C_GENERATE_START_WRITE);

	if (i2c_wait_txis(hi2c, timeout, tick_start) != 0) {
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

		/* Wait until TXIS flag is set */
		if (i2c_wait_txis(hi2c, timeout, tick_start) != 0) {
			return -EIO;
		}

		/* Send LSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	}

	if (i2c_wait_flag(hi2c, I2C_FLAG_TCR, 0, timeout, tick_start) !=
	    0) {
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
 * @param  mem_add_size: size of internal memory address
 * @param  timeout: timeout duration
 * @param  tick_start Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_request_memory_read(struct i2c_handle_s *hi2c, uint16_t dev_addr,
				   uint16_t mem_addr, uint16_t mem_add_size,
				   uint32_t timeout, uint32_t tick_start)
{
	i2c_transfer_config(hi2c, dev_addr, mem_add_size, I2C_SOFTEND_MODE,
			    I2C_GENERATE_START_WRITE);

	if (i2c_wait_txis(hi2c, timeout, tick_start) != 0) {
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

		/* Wait until TXIS flag is set */
		if (i2c_wait_txis(hi2c, timeout, tick_start) != 0) {
			return -EIO;
		}

		/* Send LSB of Memory Address */
		mmio_write_8(hi2c->i2c_base_addr + I2C_TXDR,
			     (uint8_t)(mem_addr & 0x00FFU));
	}

	if (i2c_wait_flag(hi2c, I2C_FLAG_TC, 0, timeout, tick_start) != 0) {
		return -EIO;
	}

	return 0;
}

/*
 * @brief  I2C Tx data register flush process.
 * @param  hi2c: I2C handle.
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
 * @param  flag: Specifies the I2C flag to check.
 * @param  awaited_value: The awaited bit value for the flag (0 or 1).
 * @param  timeout: timeout duration
 * @param  tick_start: Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_flag(struct i2c_handle_s *hi2c, uint32_t flag,
			 uint8_t awaited_value, uint32_t timeout,
			 uint32_t tick_start)
{
	uint8_t flag_check;

	do {
		flag_check = ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
			       flag) == flag) ? 1U : 0U;

		if (timeout != MAX_DELAY) {
			if ((((uint32_t)read_cntpct_el0() - tick_start) >
			     timeout) || (timeout == 0U)) {
				hi2c->i2c_err |= I2C_ERROR_TIMEOUT;
				hi2c->i2c_state = I2C_STATE_READY;
				hi2c->i2c_mode = I2C_MODE_NONE;

				hi2c->lock = 0;
				return -EIO;
			}
		}
	} while (flag_check == awaited_value);

	return 0;
}

/*
 * @brief  This function handles I2C Communication timeout for specific usage
 *	   of TXIS flag.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout: timeout duration
 * @param  tick_start: Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_txis(struct i2c_handle_s *hi2c, uint32_t timeout,
			 uint32_t tick_start)
{
	while ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		I2C_FLAG_TXIS) == 0U) {
		if (i2c_ack_failed(hi2c, timeout, tick_start) != 0) {
			return -EIO;
		}

		if (timeout != MAX_DELAY) {
			if ((((uint32_t)read_cntpct_el0() - tick_start) >
			     timeout) || (timeout == 0U)) {
				hi2c->i2c_err |= I2C_ERROR_TIMEOUT;
				hi2c->i2c_state = I2C_STATE_READY;
				hi2c->i2c_mode = I2C_MODE_NONE;

				hi2c->lock = 0;

				return -EIO;
			}
		}
	}

	return 0;
}

/*
 * @brief  This function handles I2C Communication timeout for specific
 *	   usage of STOP flag.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout: timeout duration
 * @param  tick_start: Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_wait_stop(struct i2c_handle_s *hi2c, uint32_t timeout,
			 uint32_t tick_start)
{
	while ((mmio_read_32(hi2c->i2c_base_addr + I2C_ISR) &
		 I2C_FLAG_STOPF) == 0U) {
		if (i2c_ack_failed(hi2c, timeout, tick_start) != 0) {
			return -EIO;
		}

		if ((((uint32_t)read_cntpct_el0() - tick_start) > timeout) ||
		    (timeout == 0U)) {
			hi2c->i2c_err |= I2C_ERROR_TIMEOUT;
			hi2c->i2c_state = I2C_STATE_READY;
			hi2c->i2c_mode = I2C_MODE_NONE;

			hi2c->lock = 0;

			return -EIO;
		}
	}

	return 0;
}

/*
 * @brief  This function handles Acknowledge failed detection during
 *	   an I2C Communication.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2C.
 * @param  timeout: timeout duration
 * @param  tick_start: Tick start value
 * @retval 0 if OK, negative value else
 */
static int i2c_ack_failed(struct i2c_handle_s *hi2c, uint32_t timeout,
			  uint32_t tick_start)
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
		if (timeout != MAX_DELAY) {
			if ((((uint32_t)read_cntpct_el0() - tick_start) >
			     timeout) || (timeout == 0U)) {
				hi2c->i2c_err |= I2C_ERROR_TIMEOUT;
				hi2c->i2c_state = I2C_STATE_READY;
				hi2c->i2c_mode = I2C_MODE_NONE;

				hi2c->lock = 0;

				return -EIO;
			}
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
 * @brief  Handles I2Cx communication when starting transfer or during transfer
 *	   (TC or TCR flag are set).
 * @param  hi2c: I2C handle.
 * @param  dev_addr: Specifies the slave address to be programmed.
 * @param  size: Specifies the number of bytes to be programmed.
 *   This parameter must be a value between 0 and 255.
 * @param  i2c_mode: New state of the I2C START condition generation.
 *   This parameter can be one of the following values:
 *     @arg @ref I2C_RELOAD_MODE: Enable Reload mode .
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
 * @brief  Configure I2C Analog noise filter.
 * @param  hi2c: Pointer to a struct i2c_handle_s structure that contains
 *               the configuration information for the specified I2Cx peripheral
 * @param  analog_filter: New state of the Analog filter.
 * @retval 0 if OK, negative value else
 */
int stm32_i2c_config_analog_filter(struct i2c_handle_s *hi2c,
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
