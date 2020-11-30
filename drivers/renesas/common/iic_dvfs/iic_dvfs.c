/*
 * Copyright (c) 2015-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include "cpg_registers.h"
#include "iic_dvfs.h"
#include "rcar_def.h"
#include "rcar_private.h"

#define DVFS_RETRY_MAX				(2U)

#define IIC_DVFS_SET_ICCL_EXTAL_TYPE_0		(0x07U)
#define IIC_DVFS_SET_ICCL_EXTAL_TYPE_1		(0x09U)
#define IIC_DVFS_SET_ICCL_EXTAL_TYPE_2		(0x0BU)
#define IIC_DVFS_SET_ICCL_EXTAL_TYPE_3		(0x0EU)
#define IIC_DVFS_SET_ICCL_EXTAL_TYPE_E		(0x15U)

#define IIC_DVFS_SET_ICCH_EXTAL_TYPE_0		(0x01U)
#define IIC_DVFS_SET_ICCH_EXTAL_TYPE_1		(0x02U)
#define IIC_DVFS_SET_ICCH_EXTAL_TYPE_2		(0x03U)
#define IIC_DVFS_SET_ICCH_EXTAL_TYPE_3		(0x05U)
#define IIC_DVFS_SET_ICCH_EXTAL_TYPE_E		(0x07U)

#define CPG_BIT_SMSTPCR9_DVFS			(0x04000000U)

#define IIC_DVFS_REG_BASE			(0xE60B0000U)
#define IIC_DVFS_REG_ICDR			(IIC_DVFS_REG_BASE + 0x0000U)
#define IIC_DVFS_REG_ICCR			(IIC_DVFS_REG_BASE + 0x0004U)
#define IIC_DVFS_REG_ICSR			(IIC_DVFS_REG_BASE + 0x0008U)
#define IIC_DVFS_REG_ICIC			(IIC_DVFS_REG_BASE + 0x000CU)
#define IIC_DVFS_REG_ICCL			(IIC_DVFS_REG_BASE + 0x0010U)
#define IIC_DVFS_REG_ICCH			(IIC_DVFS_REG_BASE + 0x0014U)

#define IIC_DVFS_BIT_ICSR_BUSY			(0x10U)
#define IIC_DVFS_BIT_ICSR_AL			(0x08U)
#define IIC_DVFS_BIT_ICSR_TACK			(0x04U)
#define IIC_DVFS_BIT_ICSR_WAIT			(0x02U)
#define IIC_DVFS_BIT_ICSR_DTE			(0x01U)

#define IIC_DVFS_BIT_ICCR_ENABLE		(0x80U)
#define IIC_DVFS_SET_ICCR_START			(0x94U)
#define IIC_DVFS_SET_ICCR_STOP			(0x90U)
#define IIC_DVFS_SET_ICCR_RETRANSMISSION	(0x94U)
#define IIC_DVFS_SET_ICCR_CHANGE		(0x81U)
#define IIC_DVFS_SET_ICCR_STOP_READ		(0xC0U)

#define IIC_DVFS_BIT_ICIC_TACKE			(0x04U)
#define IIC_DVFS_BIT_ICIC_WAITE			(0x02U)
#define IIC_DVFS_BIT_ICIC_DTEE			(0x01U)

#define DVFS_READ_MODE				(0x01U)
#define DVFS_WRITE_MODE				(0x00U)

#define IIC_DVFS_SET_DUMMY			(0x52U)
#define IIC_DVFS_SET_BUSY_LOOP			(500000000U)

enum dvfs_state_t {
	DVFS_START = 0,
	DVFS_STOP,
	DVFS_RETRANSMIT,
	DVFS_READ,
	DVFS_STOP_READ,
	DVFS_SET_SLAVE_READ,
	DVFS_SET_SLAVE,
	DVFS_WRITE_ADDR,
	DVFS_WRITE_DATA,
	DVFS_CHANGE_SEND_TO_RECEIVE,
	DVFS_DONE,
};

#define DVFS_PROCESS			(1)
#define DVFS_COMPLETE			(0)
#define DVFS_ERROR			(-1)

#if IMAGE_BL31
#define IIC_DVFS_FUNC(__name, ...)					\
static int32_t	__attribute__ ((section(".system_ram")))		\
dvfs_ ##__name(__VA_ARGS__)

#define RCAR_DVFS_API(__name, ...)					\
int32_t __attribute__ ((section(".system_ram")))			\
rcar_iic_dvfs_ ##__name(__VA_ARGS__)

#else
#define IIC_DVFS_FUNC(__name, ...)					\
static int32_t dvfs_ ##__name(__VA_ARGS__)

#define RCAR_DVFS_API(__name, ...)					\
int32_t rcar_iic_dvfs_ ##__name(__VA_ARGS__)
#endif

IIC_DVFS_FUNC(check_error, enum dvfs_state_t *state, uint32_t *err, uint8_t mode)
{
	uint8_t icsr_al = 0U, icsr_tack = 0U;
	uint8_t reg, stop;
	uint32_t i = 0U;

	stop = mode == DVFS_READ_MODE ? IIC_DVFS_SET_ICCR_STOP_READ :
	    IIC_DVFS_SET_ICCR_STOP;

	reg = mmio_read_8(IIC_DVFS_REG_ICSR);
	icsr_al = (reg & IIC_DVFS_BIT_ICSR_AL) == IIC_DVFS_BIT_ICSR_AL;
	icsr_tack = (reg & IIC_DVFS_BIT_ICSR_TACK) == IIC_DVFS_BIT_ICSR_TACK;

	if (icsr_al == 0U && icsr_tack == 0U) {
		return DVFS_PROCESS;
	}

	if (icsr_al) {
		reg = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_AL;
		mmio_write_8(IIC_DVFS_REG_ICSR, reg);

		if (*state == DVFS_SET_SLAVE) {
			mmio_write_8(IIC_DVFS_REG_ICDR, IIC_DVFS_SET_DUMMY);
		}

		do {
			reg = mmio_read_8(IIC_DVFS_REG_ICSR) &
			    IIC_DVFS_BIT_ICSR_WAIT;
		} while (reg == 0U);

		mmio_write_8(IIC_DVFS_REG_ICCR, stop);

		reg = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
		mmio_write_8(IIC_DVFS_REG_ICSR, reg);

		i = 0U;
		do {
			reg = mmio_read_8(IIC_DVFS_REG_ICSR) &
			    IIC_DVFS_BIT_ICSR_BUSY;
			if (reg == 0U) {
				break;
			}

			if (i++ > IIC_DVFS_SET_BUSY_LOOP) {
				panic();
			}

		} while (true);

		mmio_write_8(IIC_DVFS_REG_ICCR, 0x00U);

		(*err)++;
		if (*err > DVFS_RETRY_MAX) {
			return DVFS_ERROR;
		}

		*state = DVFS_START;

		return DVFS_PROCESS;

	}

	/* icsr_tack */
	mmio_write_8(IIC_DVFS_REG_ICCR, stop);

	reg = mmio_read_8(IIC_DVFS_REG_ICIC);
	reg &= ~(IIC_DVFS_BIT_ICIC_WAITE | IIC_DVFS_BIT_ICIC_DTEE);
	mmio_write_8(IIC_DVFS_REG_ICIC, reg);

	reg = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_TACK;
	mmio_write_8(IIC_DVFS_REG_ICSR, reg);

	i = 0U;
	while ((mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_BUSY) != 0U) {
		if (i++ > IIC_DVFS_SET_BUSY_LOOP) {
			panic();
		}
	}

	mmio_write_8(IIC_DVFS_REG_ICCR, 0U);
	(*err)++;

	if (*err > DVFS_RETRY_MAX) {
		return DVFS_ERROR;
	}

	*state = DVFS_START;

	return DVFS_PROCESS;
}

IIC_DVFS_FUNC(start, enum dvfs_state_t *state)
{
	uint8_t iccl = IIC_DVFS_SET_ICCL_EXTAL_TYPE_E;
	uint8_t icch = IIC_DVFS_SET_ICCH_EXTAL_TYPE_E;
	int32_t result = DVFS_PROCESS;
	uint32_t reg, lsi_product;
	uint8_t mode;

	mode = mmio_read_8(IIC_DVFS_REG_ICCR) | IIC_DVFS_BIT_ICCR_ENABLE;
	mmio_write_8(IIC_DVFS_REG_ICCR, mode);

	lsi_product = mmio_read_32(RCAR_PRR) & PRR_PRODUCT_MASK;
	if (lsi_product == PRR_PRODUCT_E3) {
		goto start;
	}

	reg = mmio_read_32(RCAR_MODEMR) & CHECK_MD13_MD14;
	switch (reg) {
	case MD14_MD13_TYPE_0:
		iccl = IIC_DVFS_SET_ICCL_EXTAL_TYPE_0;
		icch = IIC_DVFS_SET_ICCH_EXTAL_TYPE_0;
		break;
	case MD14_MD13_TYPE_1:
		iccl = IIC_DVFS_SET_ICCL_EXTAL_TYPE_1;
		icch = IIC_DVFS_SET_ICCH_EXTAL_TYPE_1;
		break;
	case MD14_MD13_TYPE_2:
		iccl = IIC_DVFS_SET_ICCL_EXTAL_TYPE_2;
		icch = IIC_DVFS_SET_ICCH_EXTAL_TYPE_2;
		break;
	default:
		iccl = IIC_DVFS_SET_ICCL_EXTAL_TYPE_3;
		icch = IIC_DVFS_SET_ICCH_EXTAL_TYPE_3;
		break;
	}
start:
	mmio_write_8(IIC_DVFS_REG_ICCL, iccl);
	mmio_write_8(IIC_DVFS_REG_ICCH, icch);

	mode = mmio_read_8(IIC_DVFS_REG_ICIC)
	    | IIC_DVFS_BIT_ICIC_TACKE
	    | IIC_DVFS_BIT_ICIC_WAITE | IIC_DVFS_BIT_ICIC_DTEE;

	mmio_write_8(IIC_DVFS_REG_ICIC, mode);
	mmio_write_8(IIC_DVFS_REG_ICCR, IIC_DVFS_SET_ICCR_START);

	*state = DVFS_SET_SLAVE;

	return result;
}

IIC_DVFS_FUNC(set_slave, enum dvfs_state_t *state, uint32_t *err, uint8_t slave)
{
	uint8_t mode;
	int32_t result;
	uint8_t address;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_DTE;
	if (mode != IIC_DVFS_BIT_ICSR_DTE) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICIC) & ~IIC_DVFS_BIT_ICIC_DTEE;
	mmio_write_8(IIC_DVFS_REG_ICIC, mode);

	address = slave << 1;
	mmio_write_8(IIC_DVFS_REG_ICDR, address);

	*state = DVFS_WRITE_ADDR;

	return result;
}

IIC_DVFS_FUNC(write_addr, enum dvfs_state_t *state, uint32_t *err, uint8_t reg_addr)
{
	uint8_t mode;
	int32_t result;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICDR, reg_addr);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	*state = DVFS_WRITE_DATA;

	return result;
}

IIC_DVFS_FUNC(write_data, enum dvfs_state_t *state, uint32_t *err,
	      uint8_t reg_data)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICDR, reg_data);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	*state = DVFS_STOP;

	return result;
}

IIC_DVFS_FUNC(stop, enum dvfs_state_t *state, uint32_t *err)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICCR, IIC_DVFS_SET_ICCR_STOP);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	*state = DVFS_DONE;

	return result;
}

IIC_DVFS_FUNC(done, void)
{
	uint32_t i;

	for (i = 0U; i < IIC_DVFS_SET_BUSY_LOOP; i++) {
		if ((mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_BUSY) != 0U) {
			continue;
		}
		goto done;
	}

	panic();
done:
	mmio_write_8(IIC_DVFS_REG_ICCR, 0U);

	return DVFS_COMPLETE;
}

IIC_DVFS_FUNC(write_reg_addr_read, enum dvfs_state_t *state, uint32_t *err,
	      uint8_t reg_addr)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICDR, reg_addr);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	*state = DVFS_RETRANSMIT;

	return result;
}

IIC_DVFS_FUNC(retransmit, enum dvfs_state_t *state, uint32_t *err)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICCR, IIC_DVFS_SET_ICCR_RETRANSMISSION);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	mode = mmio_read_8(IIC_DVFS_REG_ICIC) | IIC_DVFS_BIT_ICIC_DTEE;
	mmio_write_8(IIC_DVFS_REG_ICIC, mode);

	*state = DVFS_SET_SLAVE_READ;

	return result;
}

IIC_DVFS_FUNC(set_slave_read, enum dvfs_state_t *state, uint32_t *err,
	      uint8_t slave)
{
	uint8_t address;
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_DTE;
	if (mode != IIC_DVFS_BIT_ICSR_DTE) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICIC) & ~IIC_DVFS_BIT_ICIC_DTEE;
	mmio_write_8(IIC_DVFS_REG_ICIC, mode);

	address = ((uint8_t) (slave << 1) + DVFS_READ_MODE);
	mmio_write_8(IIC_DVFS_REG_ICDR, address);

	*state = DVFS_CHANGE_SEND_TO_RECEIVE;

	return result;
}

IIC_DVFS_FUNC(change_send_to_receive, enum dvfs_state_t *state, uint32_t *err)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_WRITE_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICCR, IIC_DVFS_SET_ICCR_CHANGE);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	*state = DVFS_STOP_READ;

	return result;
}

IIC_DVFS_FUNC(stop_read, enum dvfs_state_t *state, uint32_t *err)
{
	int32_t result;
	uint8_t mode;

	result = dvfs_check_error(state, err, DVFS_READ_MODE);
	if (result == DVFS_ERROR) {
		return result;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_WAIT;
	if (mode != IIC_DVFS_BIT_ICSR_WAIT) {
		return result;
	}

	mmio_write_8(IIC_DVFS_REG_ICCR, IIC_DVFS_SET_ICCR_STOP_READ);

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & ~IIC_DVFS_BIT_ICSR_WAIT;
	mmio_write_8(IIC_DVFS_REG_ICSR, mode);

	mode = mmio_read_8(IIC_DVFS_REG_ICIC) | IIC_DVFS_BIT_ICIC_DTEE;
	mmio_write_8(IIC_DVFS_REG_ICIC, mode);

	*state = DVFS_READ;

	return result;
}

IIC_DVFS_FUNC(read, enum dvfs_state_t *state, uint8_t *reg_data)
{
	uint8_t mode;

	mode = mmio_read_8(IIC_DVFS_REG_ICSR) & IIC_DVFS_BIT_ICSR_DTE;
	if (mode != IIC_DVFS_BIT_ICSR_DTE) {
		return DVFS_PROCESS;
	}

	mode = mmio_read_8(IIC_DVFS_REG_ICIC) & ~IIC_DVFS_BIT_ICIC_DTEE;
	mmio_write_8(IIC_DVFS_REG_ICIC, mode);

	*reg_data = mmio_read_8(IIC_DVFS_REG_ICDR);
	*state = DVFS_DONE;

	return DVFS_PROCESS;
}

RCAR_DVFS_API(send, uint8_t slave, uint8_t reg_addr, uint8_t reg_data)
{
	enum dvfs_state_t state = DVFS_START;
	int32_t result = DVFS_PROCESS;
	uint32_t err = 0U;

	mstpcr_write(SCMSTPCR9, CPG_MSTPSR9, CPG_BIT_SMSTPCR9_DVFS);
	mmio_write_8(IIC_DVFS_REG_ICCR, 1U);
again:
	switch (state) {
	case DVFS_START:
		result = dvfs_start(&state);
		break;
	case DVFS_SET_SLAVE:
		result = dvfs_set_slave(&state, &err, slave);
		break;
	case DVFS_WRITE_ADDR:
		result = dvfs_write_addr(&state, &err, reg_addr);
		break;
	case DVFS_WRITE_DATA:
		result = dvfs_write_data(&state, &err, reg_data);
		break;
	case DVFS_STOP:
		result = dvfs_stop(&state, &err);
		break;
	case DVFS_DONE:
		result = dvfs_done();
		break;
	default:
		panic();
		break;
	}

	if (result == DVFS_PROCESS) {
		goto again;
	}

	return result;
}

RCAR_DVFS_API(receive, uint8_t slave, uint8_t reg, uint8_t *data)
{
	enum dvfs_state_t state = DVFS_START;
	int32_t result = DVFS_PROCESS;
	uint32_t err = 0U;

	mstpcr_write(SCMSTPCR9, CPG_MSTPSR9, CPG_BIT_SMSTPCR9_DVFS);
	mmio_write_8(IIC_DVFS_REG_ICCR, 1U);
again:
	switch (state) {
	case DVFS_START:
		result = dvfs_start(&state);
		break;
	case DVFS_SET_SLAVE:
		result = dvfs_set_slave(&state, &err, slave);
		break;
	case DVFS_WRITE_ADDR:
		result = dvfs_write_reg_addr_read(&state, &err, reg);
		break;
	case DVFS_RETRANSMIT:
		result = dvfs_retransmit(&state, &err);
		break;
	case DVFS_SET_SLAVE_READ:
		result = dvfs_set_slave_read(&state, &err, slave);
		break;
	case DVFS_CHANGE_SEND_TO_RECEIVE:
		result = dvfs_change_send_to_receive(&state, &err);
		break;
	case DVFS_STOP_READ:
		result = dvfs_stop_read(&state, &err);
		break;
	case DVFS_READ:
		result = dvfs_read(&state, data);
		break;
	case DVFS_DONE:
		result = dvfs_done();
		break;
	default:
		panic();
		break;
	}

	if (result == DVFS_PROCESS) {
		goto again;
	}

	return result;
}
