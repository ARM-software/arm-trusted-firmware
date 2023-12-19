/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "mhu_v3_x.h"

#include "mhu_v3_x_private.h"

/*
 * Get the device base from the device struct. Return an error if the dev is
 * invalid.
 */
static enum mhu_v3_x_error_t get_dev_base(const struct mhu_v3_x_dev_t *dev,
	 union _mhu_v3_x_frame_t **base)
{
	if (dev == NULL) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	/* Ensure driver has been initialized */
	if (dev->is_initialized == false) {
		return MHU_V_3_X_ERR_NOT_INIT;
	}

	*base = (union _mhu_v3_x_frame_t *)dev->base;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_driver_init(struct mhu_v3_x_dev_t *dev)
{
	uint32_t aidr = 0;
	uint8_t mhu_major_rev;
	union _mhu_v3_x_frame_t *p_mhu;

	if (dev == NULL) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	/* Return if already initialized */
	if (dev->is_initialized == true) {
		return MHU_V_3_X_ERR_NONE;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	/* Read revision from MHU hardware */
	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		aidr = p_mhu->pbx_frame.pbx_ctrl_page.pbx_aidr;
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		aidr = p_mhu->mbx_frame.mbx_ctrl_page.mbx_aidr;
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	/* Read the MHU Architecture Major Revision */
	mhu_major_rev =
		((aidr & MHU_ARCH_MAJOR_REV_MASK) >> MHU_ARCH_MAJOR_REV_OFF);

	/* Return error if the MHU major revision is not 3 */
	if (mhu_major_rev != MHU_MAJOR_REV_V3) {
		/* Unsupported MHU version */
		return MHU_V_3_X_ERR_UNSUPPORTED_VERSION;
	}

	/* Read the MHU Architecture Minor Revision */
	dev->subversion =
		((aidr & MHU_ARCH_MINOR_REV_MASK) >> MHU_ARCH_MINOR_REV_MASK);

	/* Return error if the MHU minor revision is not 0 */
	if (dev->subversion != MHU_MINOR_REV_3_0) {
		/* Unsupported subversion */
		return MHU_V_3_X_ERR_UNSUPPORTED_VERSION;
	}

	/* Initialize the Postbox/Mailbox to remain in operational state */
	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		p_mhu->pbx_frame.pbx_ctrl_page.pbx_ctrl |= MHU_V3_OP_REQ;
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		p_mhu->mbx_frame.mbx_ctrl_page.mbx_ctrl |= MHU_V3_OP_REQ;
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	dev->is_initialized = true;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_get_num_channel_implemented(
	 const struct mhu_v3_x_dev_t *dev,
	 enum mhu_v3_x_channel_type_t ch_type, uint8_t *num_ch)
{
	enum mhu_v3_x_error_t status;
	union _mhu_v3_x_frame_t *p_mhu;

	if (num_ch == NULL) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only doorbell channel is supported */
	if (ch_type != MHU_V3_X_CHANNEL_TYPE_DBCH) {
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	/* Read the number of channels implemented in the MHU */
	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		*num_ch = (p_mhu->pbx_frame.pbx_ctrl_page.pbx_dbch_cfg0 + 1);
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		*num_ch = (p_mhu->mbx_frame.mbx_ctrl_page.mbx_dbch_cfg0 + 1);
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_clear(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;
	enum mhu_v3_x_error_t status;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only MBX can clear the Doorbell channel */
	if (dev->frame != MHU_V3_X_MBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;
	mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
		&(p_mhu->mbx_frame.mdbcw_page);

	/* Clear the bits in the doorbell channel */
	mdbcw_reg[channel].mdbcw_clr |= flags;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_write(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_pdbcw_reg_t *pdbcw_reg;
	enum mhu_v3_x_error_t status;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only PBX can set the Doorbell channel value */
	if (dev->frame != MHU_V3_X_PBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	pdbcw_reg = (struct _mhu_v3_x_pdbcw_reg_t *)
		&(p_mhu->pbx_frame.pdbcw_page);

	/* Write the value to the doorbell channel */
	pdbcw_reg[channel].pdbcw_set |= flags;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_read(const struct mhu_v3_x_dev_t *dev,
	 const uint32_t channel, uint32_t *flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	enum mhu_v3_x_error_t status;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;
	struct _mhu_v3_x_pdbcw_reg_t *pdbcw_reg;

	if (flags == NULL) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		pdbcw_reg = (struct _mhu_v3_x_pdbcw_reg_t *)
			&(p_mhu->pbx_frame.pdbcw_page);

		/* Read the value from Postbox Doorbell status register */
		*flags = pdbcw_reg[channel].pdbcw_st;
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
			&(p_mhu->mbx_frame.mdbcw_page);

		/* Read the value from Mailbox Doorbell status register */
		*flags = mdbcw_reg[channel].mdbcw_st;
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_set(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 uint32_t flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;
	enum mhu_v3_x_error_t status;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Doorbell channel mask is not applicable for PBX */
	if (dev->frame != MHU_V3_X_MBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
		&(p_mhu->mbx_frame.mdbcw_page);

	/* Set the Doorbell channel mask */
	mdbcw_reg[channel].mdbcw_msk_set |= flags;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_clear(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 uint32_t flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;
	enum mhu_v3_x_error_t status;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Doorbell channel mask is not applicable for PBX */
	if (dev->frame != MHU_V3_X_MBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
		&(p_mhu->mbx_frame.mdbcw_page);

	/* Clear the Doorbell channel mask */
	mdbcw_reg[channel].mdbcw_msk_clr = flags;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_doorbell_mask_get(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 uint32_t *flags)
{
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;
	enum mhu_v3_x_error_t status;

	if (flags == NULL) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Doorbell channel mask is not applicable for PBX */
	if (dev->frame != MHU_V3_X_MBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
		&(p_mhu->mbx_frame.mdbcw_page);

	/* Save the Doorbell channel mask status */
	*flags = mdbcw_reg[channel].mdbcw_msk_st;

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_enable(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type)
{
	enum mhu_v3_x_error_t status;

	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_pdbcw_reg_t *pdbcw_reg;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only doorbell channel is supported */
	if (ch_type != MHU_V3_X_CHANNEL_TYPE_DBCH) {
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		pdbcw_reg = (struct _mhu_v3_x_pdbcw_reg_t *)
			&(p_mhu->pbx_frame.pdbcw_page);

		/*
		 * Enable this doorbell channel to generate interrupts for
		 * transfer acknowledge events.
		 */
		pdbcw_reg[channel].pdbcw_int_en = MHU_V3_X_PDBCW_INT_X_TFR_ACK;

		/*
		 * Enable this doorbell channel to contribute to the PBX
		 * combined interrupt.
		 */
		pdbcw_reg[channel].pdbcw_ctrl = MHU_V3_X_PDBCW_CTRL_PBX_COMB_EN;
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
			&(p_mhu->mbx_frame.mdbcw_page);

		/*
		 * Enable this doorbell channel to contribute to the MBX
		 * combined interrupt.
		 */
		mdbcw_reg[channel].mdbcw_ctrl = MHU_V3_X_MDBCW_CTRL_MBX_COMB_EN;
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_disable(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type)
{
	enum mhu_v3_x_error_t status;

	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_pdbcw_reg_t *pdbcw_reg;
	struct _mhu_v3_x_mdbcw_reg_t *mdbcw_reg;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only doorbell channel is supported */
	if (ch_type != MHU_V3_X_CHANNEL_TYPE_DBCH) {
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;

	if (dev->frame == MHU_V3_X_PBX_FRAME) {
		pdbcw_reg = (struct _mhu_v3_x_pdbcw_reg_t *)
			&(p_mhu->pbx_frame.pdbcw_page);

		/* Clear channel transfer acknowledge event interrupt */
		pdbcw_reg[channel].pdbcw_int_clr = MHU_V3_X_PDBCW_INT_X_TFR_ACK;

		/* Disable channel transfer acknowledge event interrupt */
		pdbcw_reg[channel].pdbcw_int_en &=
			~(MHU_V3_X_PDBCW_INT_X_TFR_ACK);

		/*
		 * Disable this doorbell channel from contributing to the PBX
		 * combined interrupt.
		 */
		pdbcw_reg[channel].pdbcw_ctrl &=
			~(MHU_V3_X_PDBCW_CTRL_PBX_COMB_EN);
	} else if (dev->frame == MHU_V3_X_MBX_FRAME) {
		mdbcw_reg = (struct _mhu_v3_x_mdbcw_reg_t *)
			&(p_mhu->mbx_frame.mdbcw_page);

		/*
		 * Disable this doorbell channel from contributing to the MBX
		 * combined interrupt.
		 */
		mdbcw_reg[channel].mdbcw_ctrl &=
			~(MHU_V3_X_MDBCW_CTRL_MBX_COMB_EN);
	} else {
		/* Only PBX and MBX frames are supported. */
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	return MHU_V_3_X_ERR_NONE;
}

enum mhu_v3_x_error_t mhu_v3_x_channel_interrupt_clear(
	 const struct mhu_v3_x_dev_t *dev, const uint32_t channel,
	 enum mhu_v3_x_channel_type_t ch_type)
{
	enum mhu_v3_x_error_t status;
	union _mhu_v3_x_frame_t *p_mhu;
	struct _mhu_v3_x_pdbcw_reg_t *pdbcw_reg;

	/* Get dev->base if it is valid or return an error if dev is not */
	status = get_dev_base(dev, &p_mhu);
	if (status != MHU_V_3_X_ERR_NONE) {
		return status;
	}

	/* Only doorbell channel is supported */
	if (ch_type != MHU_V3_X_CHANNEL_TYPE_DBCH) {
		return MHU_V_3_X_ERR_UNSUPPORTED;
	}

	/*
	 * Only postbox doorbell channel transfer acknowledge interrupt can be
	 * cleared manually.
	 *
	 * To clear MBX interrupt the unmasked status must be cleared using
	 * mhu_v3_x_doorbell_clear.
	 */
	if (dev->frame != MHU_V3_X_PBX_FRAME) {
		return MHU_V_3_X_ERR_INVALID_PARAM;
	}

	p_mhu = (union _mhu_v3_x_frame_t *)dev->base;
	pdbcw_reg = (struct _mhu_v3_x_pdbcw_reg_t *)&(
			p_mhu->pbx_frame.pdbcw_page);

	/* Clear channel transfer acknowledge event interrupt */
	pdbcw_reg[channel].pdbcw_int_clr |= 0x1;

	return MHU_V_3_X_ERR_NONE;
}
