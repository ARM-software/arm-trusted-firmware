/*
 * Copyright (c) 2022-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>

#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#if STM32_SAES_CRYP2
#include <drivers/st/stm32_cryp2_saes_reg.h>
#else /* !STM32_SAES_CRYP2 */
#include <drivers/st/stm32_saes_reg.h>
#endif /* STM32_SAES_CRYP2 */
#include <drivers/st/stm32_saes.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <libfdt.h>

#include <platform_def.h>

#define UINT8_BIT			8U
#define AES_BLOCK_SIZE_BIT		128U
#define AES_BLOCK_SIZE			(AES_BLOCK_SIZE_BIT / UINT8_BIT)

#define AES_KEYSIZE_128			16U
#define AES_KEYSIZE_192			24U
#define AES_KEYSIZE_256			32U
#define AES_IVSIZE			16U

#define SAES_TIMEOUT_US			100000U
#define TIMEOUT_US_1MS			1000U
#define SAES_RESET_DELAY		20U

#define IS_CHAINING_MODE(mod, cr) \
	(((cr) & _SAES_CR_CHMOD_MASK) == (_SAES_CR_CHMOD_##mod << _SAES_CR_CHMOD_SHIFT))

#define SET_CHAINING_MODE(mod, cr) \
	mmio_clrsetbits_32((cr), _SAES_CR_CHMOD_MASK, _SAES_CR_CHMOD_##mod << _SAES_CR_CHMOD_SHIFT)

static struct stm32_saes_platdata saes_pdata;

static int stm32_saes_parse_fdt(struct stm32_saes_platdata *pdata)
{
	int node;
	struct dt_node_info info;
	void *fdt;

	if (fdt_get_address(&fdt) == 0) {
		return -FDT_ERR_NOTFOUND;
	}

	node = dt_get_node(&info, -1, DT_SAES_COMPAT);
	if (node < 0) {
		ERROR("No SAES entry in DT\n");
		return -FDT_ERR_NOTFOUND;
	}

	if (info.status == DT_DISABLED) {
		return -FDT_ERR_NOTFOUND;
	}

	if ((info.base == 0U) || (info.clock < 0) || (info.reset < 0)) {
		return -FDT_ERR_BADVALUE;
	}

	pdata->base = (uintptr_t)info.base;
	pdata->clock_id = (unsigned long)info.clock;
	pdata->reset_id = (unsigned int)info.reset;

	return 0;
}

static bool does_chaining_mode_need_iv(uint32_t cr)
{
	return !(IS_CHAINING_MODE(ECB, cr));
}

static bool is_encrypt(uint32_t cr)
{
	return (cr & _SAES_CR_MODE_MASK) == (_SAES_CR_MODE_ENC << _SAES_CR_MODE_SHIFT);
}

static bool is_decrypt(uint32_t cr)
{
	return (cr & _SAES_CR_MODE_MASK) == (_SAES_CR_MODE_DEC << _SAES_CR_MODE_SHIFT);
}

static int wait_computation_completed(uintptr_t base)
{
	uint64_t timeout = timeout_init_us(SAES_TIMEOUT_US);

	while ((mmio_read_32(base + _SAES_ISR) & _SAES_I_CCF) != _SAES_I_CCF) {
		if (timeout_elapsed(timeout)) {
			WARN("%s: timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void clear_computation_completed(uintptr_t base)
{
	mmio_setbits_32(base + _SAES_ICR, _SAES_I_CCF);
}

static int wait_key_valid(uintptr_t base)
{
	uint64_t timeout = timeout_init_us(SAES_TIMEOUT_US);

	while ((mmio_read_32(base + _SAES_SR) & _SAES_SR_KEYVALID) != _SAES_SR_KEYVALID) {
		if (timeout_elapsed(timeout)) {
			WARN("%s: timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int saes_start(struct stm32_saes_context *ctx)
{
	uint64_t timeout;

	/* Reset IP */
	if ((mmio_read_32(ctx->base + _SAES_SR) & _SAES_SR_BUSY) != _SAES_SR_BUSY) {
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_IPRST);
		udelay(SAES_RESET_DELAY);
		mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_IPRST);
	}

	timeout = timeout_init_us(SAES_TIMEOUT_US);
	while ((mmio_read_32(ctx->base + _SAES_SR) & _SAES_SR_BUSY) == _SAES_SR_BUSY) {
		if (timeout_elapsed(timeout)) {
			WARN("%s: timeout\n", __func__);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static void saes_end(struct stm32_saes_context *ctx, int prev_error)
{
	if (prev_error != 0) {
		/* Reset IP */
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_IPRST);
		udelay(SAES_RESET_DELAY);
		mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_IPRST);
	}

	/* Disable the SAES peripheral */
	mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);
}

static void saes_write_iv(struct stm32_saes_context *ctx)
{
	/* If chaining mode need to restore IV */
	if (does_chaining_mode_need_iv(ctx->cr)) {
		uint8_t i;

		/* Restore the _SAES_IVRx */
		for (i = 0U; i < AES_IVSIZE / sizeof(uint32_t); i++) {
			mmio_write_32(ctx->base + _SAES_IVR0 + i * sizeof(uint32_t), ctx->iv[i]);
		}
	}

}

static uint32_t saes_get_keysize(struct stm32_saes_context *ctx)
{
	int keysize;
	uint32_t ret = 0U;

#if STM32_SAES_CRYP2
	keysize = (ctx->cr & _SAES_CR_KEYSIZE_MASK) >> _SAES_CR_KEYSIZE_SHIFT;
#else /* !STM32_SAES_CRYP2 */
	keysize = (ctx->cr & _SAES_CR_KEYSIZE) >> _SAES_CR_KEYSIZE_SHIFT;
#endif /* STM32_SAES_CRYP2 */

	switch (keysize) {
	case _SAES_CR_KEYSIZE_256:
		ret = AES_KEYSIZE_256;
		break;
#if STM32_SAES_CRYP2
	case _SAES_CR_KEYSIZE_192:
		ret = AES_KEYSIZE_192;
		break;
#endif /* STM32_SAES_CRYP2 */
	case _SAES_CR_KEYSIZE_128:
		ret = AES_KEYSIZE_128;
		break;
	default:
		ERROR("Invalid keysize\n");
		break;
	}

	return ret;
}

static void saes_write_key(struct stm32_saes_context *ctx)
{
	/* Restore the _SAES_KEYRx if SOFTWARE key */
	if ((ctx->cr & _SAES_CR_KEYSEL_MASK) == (_SAES_CR_KEYSEL_SOFT << _SAES_CR_KEYSEL_SHIFT)) {
		uint8_t i;
		uint32_t ks;

		ks = saes_get_keysize(ctx);
		if (ks == 0U) {
			panic();
		}

		for (i = 0; i < (AES_KEYSIZE_128 / sizeof(uint32_t)); i++) {
			mmio_write_32(ctx->base + _SAES_KEYR0 + i * sizeof(uint32_t), ctx->key[i]);
		}

		if (ks > AES_KEYSIZE_128) {
			ks -= AES_KEYSIZE_128; /* writes only remaining bytes */
			for (i = 0; i < (ks / sizeof(uint32_t)); i++) {
				mmio_write_32(ctx->base + _SAES_KEYR4 + i * sizeof(uint32_t),
					      ctx->key[i + 4]);
			}
		}
	}
}

static int saes_prepare_key(struct stm32_saes_context *ctx)
{
	int ret;

	/* Disable the SAES peripheral */
	mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);

#if !STM32_SAES_CRYP2
	/* Set key size */
	if ((ctx->cr & _SAES_CR_KEYSIZE) != 0U) {
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_KEYSIZE);
	} else {
		mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_KEYSIZE);
	}
#endif /* !STM32_SAES_CRYP2 */

	saes_write_key(ctx);

	ret = wait_key_valid(ctx->base);
	if (ret != 0) {
		return ret;
	}

	/* For ECB/CBC decryption, key preparation mode must be selected to populate the key */
	if ((IS_CHAINING_MODE(ECB, ctx->cr) || IS_CHAINING_MODE(CBC, ctx->cr)) &&
	    is_decrypt(ctx->cr)) {

		/* Select Mode 2 */
		mmio_clrsetbits_32(ctx->base + _SAES_CR, _SAES_CR_MODE_MASK,
				   _SAES_CR_MODE_KEYPREP << _SAES_CR_MODE_SHIFT);

		/* Enable SAES */
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);

		/* Wait Computation completed */
		ret = wait_computation_completed(ctx->base);
		if (ret != 0) {
			return ret;
		}

		clear_computation_completed(ctx->base);

		/* Set Mode 3 */
		mmio_clrsetbits_32(ctx->base + _SAES_CR, _SAES_CR_MODE_MASK,
				   _SAES_CR_MODE_DEC << _SAES_CR_MODE_SHIFT);
	}

	return 0;
}

static int save_context(struct stm32_saes_context *ctx)
{
	if ((mmio_read_32(ctx->base + _SAES_ISR) & _SAES_I_CCF) != 0U) {
		/* Device should not be in a processing phase */
		return -EINVAL;
	}

	/* Save CR */
	ctx->cr = mmio_read_32(ctx->base + _SAES_CR);

	/* If chaining mode need to save current IV */
	if (does_chaining_mode_need_iv(ctx->cr)) {
		uint8_t i;

		/* Save IV */
		for (i = 0U; i < AES_IVSIZE / sizeof(uint32_t); i++) {
			ctx->iv[i] = mmio_read_32(ctx->base + _SAES_IVR0 + i * sizeof(uint32_t));
		}
	}

	/* Disable the SAES peripheral */
	mmio_clrbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);

	return 0;
}

/* To resume the processing of a message */
static int restore_context(struct stm32_saes_context *ctx)
{
	int ret;

	/* IP should be disabled */
	if ((mmio_read_32(ctx->base + _SAES_CR) & _SAES_CR_EN) != 0U) {
		VERBOSE("%s: Device is still enabled\n", __func__);
		return -EINVAL;
	}

	/* Reset internal state */
	mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_IPRST);

	/* Restore the _SAES_CR */
	mmio_write_32(ctx->base + _SAES_CR, ctx->cr);

	/* Preparation decrypt key */
	ret = saes_prepare_key(ctx);
	if (ret != 0) {
		return ret;
	}

	saes_write_iv(ctx);

	/* Enable the SAES peripheral */
	mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);

	return 0;
}

/**
 * @brief Initialize SAES driver.
 * @param None.
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_driver_init(void)
{
	int err;

	if (saes_pdata.base != 0U) {
		/* Driver is already initialized */
		return 0;
	}

	err = stm32_saes_parse_fdt(&saes_pdata);
	if (err != 0) {
		return err;
	}

	clk_enable(saes_pdata.clock_id);
	if (stm32mp_reset_assert(saes_pdata.reset_id, TIMEOUT_US_1MS) != 0) {
		panic();
	}

	udelay(SAES_RESET_DELAY);
	if (stm32mp_reset_deassert(saes_pdata.reset_id, TIMEOUT_US_1MS) != 0) {
		panic();
	}

	return 0;
}

/**
 * @brief Start a AES computation.
 * @param ctx: SAES process context
 * @param is_dec: true if decryption, false if encryption
 * @param ch_mode: define the chaining mode
 * @param key_select: define where the key comes from.
 * @param key: pointer to key (if key_select is KEY_SOFT, else unused)
 * @param key_size: key size
 * @param iv: pointer to initialization vectore (unsed if ch_mode is ECB)
 * @param iv_size: iv size
 * @note this function doesn't access to hardware but store in ctx the values
 *
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_init(struct stm32_saes_context *ctx, bool is_dec,
		    enum stm32_saes_chaining_mode ch_mode, enum stm32_saes_key_selection key_select,
		    const void *key, size_t key_size, const void *iv, size_t iv_size)
{
	unsigned int i;
	const uint32_t *iv_u32;
	const uint32_t *key_u32;

	ctx->assoc_len = 0U;
	ctx->load_len = 0U;

	ctx->base = saes_pdata.base;
	ctx->cr = _SAES_CR_RESET_VALUE;

	/* We want buffer to be u32 aligned */
	assert((uintptr_t)key % __alignof__(uint32_t) == 0);
	assert((uintptr_t)iv % __alignof__(uint32_t) == 0);

	iv_u32 = iv;
	key_u32 = key;

	if (is_dec) {
		/* Save Mode 3 = decrypt */
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_MODE_MASK,
				   _SAES_CR_MODE_DEC << _SAES_CR_MODE_SHIFT);
	} else {
		/* Save Mode 1 = crypt */
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_MODE_MASK,
				   _SAES_CR_MODE_ENC << _SAES_CR_MODE_SHIFT);
	}

	/* Save chaining mode */
	switch (ch_mode) {
	case STM32_SAES_MODE_ECB:
		SET_CHAINING_MODE(ECB, (uintptr_t)&(ctx->cr));
		break;
	case STM32_SAES_MODE_CBC:
		SET_CHAINING_MODE(CBC, (uintptr_t)&(ctx->cr));
		break;
	case STM32_SAES_MODE_CTR:
		SET_CHAINING_MODE(CTR, (uintptr_t)&(ctx->cr));
		break;
	case STM32_SAES_MODE_GCM:
		SET_CHAINING_MODE(GCM, (uintptr_t)&(ctx->cr));
		break;
	case STM32_SAES_MODE_CCM:
		SET_CHAINING_MODE(CCM, (uintptr_t)&(ctx->cr));
		break;
	default:
		return -EINVAL;
	}

	/* We will use HW Byte swap (_SAES_CR_DATATYPE_BYTE) for data.
	 * so we won't need to
	 * htobe32(data) before write to DINR
	 * nor
	 * be32toh after reading from DOUTR
	 *
	 * But note that wrap key only accept _SAES_CR_DATATYPE_NONE
	 */
	mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_DATATYPE_MASK,
			   _SAES_CR_DATATYPE_BYTE << _SAES_CR_DATATYPE_SHIFT);

	/* Configure keysize */
#if STM32_SAES_CRYP2
	switch (key_size) {
	case AES_KEYSIZE_128:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSIZE_MASK,
				   _SAES_CR_KEYSIZE_128 << _SAES_CR_KEYSIZE_SHIFT);
		break;
	case AES_KEYSIZE_192:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSIZE_MASK,
				   _SAES_CR_KEYSIZE_192 << _SAES_CR_KEYSIZE_SHIFT);
		break;
	case AES_KEYSIZE_256:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSIZE_MASK,
				   _SAES_CR_KEYSIZE_256 << _SAES_CR_KEYSIZE_SHIFT);
		break;
	default:
		return -EINVAL;
	}
#else /* !STM32_SAES_CRYP2 */
	switch (key_size) {
	case AES_KEYSIZE_128:
		mmio_clrbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSIZE);
		break;
	case AES_KEYSIZE_256:
		mmio_setbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSIZE);
		break;
	default:
		return -EINVAL;
	}
#endif /* STM32_SAES_CRYP2 */

	/* Configure key */
	switch (key_select) {
	case STM32_SAES_KEY_SOFT:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSEL_MASK,
				   _SAES_CR_KEYSEL_SOFT << _SAES_CR_KEYSEL_SHIFT);
		/* Save key */
		switch (key_size) {
		case AES_KEYSIZE_128:
			/* First 16 bytes == 4 u32 */
			for (i = 0U; i < (AES_KEYSIZE_128 / sizeof(uint32_t)); i++) {
				mmio_write_32((uintptr_t)(ctx->key + i), htobe32(key_u32[3 - i]));
				/* /!\ we save the key in HW byte order
				 * and word order : key[i] is for _SAES_KEYRi
				 */
			}
			break;
#if STM32_SAES_CRYP2
		case AES_KEYSIZE_192:
			for (i = 0U; i < AES_KEYSIZE_192 / sizeof(uint32_t); i++) {
				mmio_write_32((uintptr_t)(ctx->key + i), htobe32(key_u32[5 - i]));
				/* /!\ we save the key in HW byte order
				 * and word order : key[i] is for _SAES_KEYRi
				 */
			}
			break;
#endif /* STM32_SAES_CRYP2 */
		case AES_KEYSIZE_256:
			for (i = 0U; i < AES_KEYSIZE_256 / sizeof(uint32_t); i++) {
				mmio_write_32((uintptr_t)(ctx->key + i), htobe32(key_u32[7 - i]));
				/* /!\ we save the key in HW byte order
				 * and word order : key[i] is for _SAES_KEYRi
				 */
			}
			break;
		default:
			return -EINVAL;
		}

		break;
	case STM32_SAES_KEY_DHU:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSEL_MASK,
				   _SAES_CR_KEYSEL_DHUK << _SAES_CR_KEYSEL_SHIFT);
		break;
	case STM32_SAES_KEY_BH:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSEL_MASK,
				   _SAES_CR_KEYSEL_BHK << _SAES_CR_KEYSEL_SHIFT);
		break;
	case STM32_SAES_KEY_BHU_XOR_BH:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSEL_MASK,
				   _SAES_CR_KEYSEL_BHU_XOR_BH_K << _SAES_CR_KEYSEL_SHIFT);
		break;
	case STM32_SAES_KEY_WRAPPED:
		mmio_clrsetbits_32((uintptr_t)&(ctx->cr), _SAES_CR_KEYSEL_MASK,
				   _SAES_CR_KEYSEL_SOFT << _SAES_CR_KEYSEL_SHIFT);
		break;

	default:
		return -EINVAL;
	}

	/* Save IV */
	if (ch_mode != STM32_SAES_MODE_ECB) {
		if ((iv == NULL) || (iv_size != AES_IVSIZE)) {
			return -EINVAL;
		}

		for (i = 0U; i < AES_IVSIZE / sizeof(uint32_t); i++) {
			mmio_write_32((uintptr_t)(ctx->iv + i), htobe32(iv_u32[3 - i]));
			/* /!\ We save the iv in HW byte order */
		}
	}

	return saes_start(ctx);
}

/**
 * @brief Update (or start) a AES authentificate process of associated data (CCM or GCM).
 * @param ctx: SAES process context
 * @param last_block: true if last assoc data block
 * @param data: pointer to associated data
 * @param data_size: data size
 *
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_update_assodata(struct stm32_saes_context *ctx, bool last_block,
			       uint8_t *data, size_t data_size)
{
	int ret;
	uint32_t *data_u32;
	unsigned int i = 0U;

	/* We want buffers to be u32 aligned */
	assert((uintptr_t)data % __alignof__(uint32_t) == 0);
	data_u32 = (uint32_t *)data;

	/* Init phase */
	ret = restore_context(ctx);
	if (ret != 0) {
		goto out;
	}

	ret = wait_computation_completed(ctx->base);
	if (ret != 0) {
		return ret;
	}

	clear_computation_completed(ctx->base);

	if ((data == NULL) || (data_size == 0U)) {
		/* No associated data */
		/* ret already = 0 */
		goto out;
	}

	/* There is an header/associated data phase */
	mmio_clrsetbits_32(ctx->base + _SAES_CR, _SAES_CR_GCMPH_MASK,
			   _SAES_CR_GCMPH_HEADER << _SAES_CR_GCMPH_SHIFT);

	/* Enable the SAES peripheral */
	mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);

	while (i < round_down(data_size, AES_BLOCK_SIZE)) {
		unsigned int w; /* Word index */

		w = i / sizeof(uint32_t);
		/* No need to htobe() as we configure the HW to swap bytes */
		mmio_write_32(ctx->base + _SAES_DINR, data_u32[w + 0U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_u32[w + 1U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_u32[w + 2U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_u32[w + 3U]);

		ret = wait_computation_completed(ctx->base);
		if (ret != 0) {
			goto out;
		}

		clear_computation_completed(ctx->base);

		/* Process next block */
		i += AES_BLOCK_SIZE;
		ctx->assoc_len += AES_BLOCK_SIZE_BIT;
	}

	/* Manage last block if not a block size multiple */
	if ((last_block) && (i < data_size)) {
		/* We don't manage unaligned last block yet */
		ret = -ENODEV;
		goto out;
	}

out:
	if (ret != 0) {
		saes_end(ctx, ret);
	}

	return ret;
}

/**
 * @brief Update (or start) a AES authenticate and de/encrypt with payload data (CCM or GCM).
 * @param ctx: SAES process context
 * @param last_block: true if last payload data block
 * @param data_in: pointer to payload
 * @param data_out: pointer where to save de/encrypted payload
 * @param data_size: payload size
 *
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_update_load(struct stm32_saes_context *ctx, bool last_block,
			   uint8_t *data_in, uint8_t *data_out, size_t data_size)
{
	int ret = 0;
	uint32_t *data_in_u32;
	uint32_t *data_out_u32;
	unsigned int i = 0U;
	uint32_t prev_cr;

	/* We want buffers to be u32 aligned */
	assert((uintptr_t)data_in % __alignof__(uint32_t) == 0);
	assert((uintptr_t)data_out % __alignof__(uint32_t) == 0);
	data_in_u32 = (uint32_t *)data_in;
	data_out_u32 = (uint32_t *)data_out;

	prev_cr = mmio_read_32(ctx->base + _SAES_CR);

	if ((data_in == NULL) || (data_size == 0U)) {
		/* there is no data */
		goto out;
	}

	/* There is a load phase */
	mmio_clrsetbits_32(ctx->base + _SAES_CR, _SAES_CR_GCMPH_MASK,
			   _SAES_CR_GCMPH_PAYLOAD << _SAES_CR_GCMPH_SHIFT);

	if ((prev_cr & _SAES_CR_GCMPH_MASK) ==
	    (_SAES_CR_GCMPH_INIT << _SAES_CR_GCMPH_SHIFT)) {
		/* Still in initialization phase, no header
		 * We need to enable the SAES peripheral
		 */
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);
	}

	while (i < round_down(data_size, AES_BLOCK_SIZE)) {
		unsigned int w; /* Word index */

		w = i / sizeof(uint32_t);
		/* No need to htobe() as we configure the HW to swap bytes */
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 0U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 1U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 2U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 3U]);

		ret = wait_computation_completed(ctx->base);
		if (ret != 0) {
			goto out;
		}

		/* No need to htobe() as we configure the HW to swap bytes */
		data_out_u32[w + 0U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 1U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 2U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 3U] = mmio_read_32(ctx->base + _SAES_DOUTR);

		clear_computation_completed(ctx->base);

		/* Process next block */
		i += AES_BLOCK_SIZE;
		ctx->load_len += AES_BLOCK_SIZE_BIT;
	}
	/* Manage last block if not a block size multiple */
	if ((last_block) && (i < data_size)) {
		uint32_t block_in[AES_BLOCK_SIZE / sizeof(uint32_t)] = {0};
		uint32_t block_out[AES_BLOCK_SIZE / sizeof(uint32_t)] = {0};

		memcpy(block_in, data_in + i, data_size - i);

		/* No need to htobe() as we configure the HW to swap bytes */
		mmio_write_32(ctx->base + _SAES_DINR, block_in[0U]);
		mmio_write_32(ctx->base + _SAES_DINR, block_in[1U]);
		mmio_write_32(ctx->base + _SAES_DINR, block_in[2U]);
		mmio_write_32(ctx->base + _SAES_DINR, block_in[3U]);

		ret = wait_computation_completed(ctx->base);
		if (ret != 0) {
			VERBOSE("%s %d\n", __func__, __LINE__);
			goto out;
		}

		/* No need to htobe() as we configure the HW to swap bytes */
		block_out[0U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		block_out[1U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		block_out[2U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		block_out[3U] = mmio_read_32(ctx->base + _SAES_DOUTR);

		clear_computation_completed(ctx->base);

		memcpy(data_out + i, block_out, data_size - i);

		ctx->load_len += (data_size - i) * UINT8_BIT;
	}

out:
	if (ret != 0) {
		saes_end(ctx, ret);
	}

	return ret;
}

/**
 * @brief Get authentication tag for AES authenticated algorithms (CCM or GCM).
 * @param ctx: SAES process context
 * @param tag: pointer where to save the tag
 * @param data_size: tag size
 *
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_final(struct stm32_saes_context *ctx, uint8_t *tag,
		     size_t tag_size)
{
	int ret;
	uint32_t tag_u32[4];
	uint32_t prev_cr;

	prev_cr = mmio_read_32(ctx->base + _SAES_CR);

	mmio_clrsetbits_32(ctx->base + _SAES_CR, _SAES_CR_GCMPH_MASK,
			   _SAES_CR_GCMPH_FINAL << _SAES_CR_GCMPH_SHIFT);

	if ((prev_cr & _SAES_CR_GCMPH_MASK) == (_SAES_CR_GCMPH_INIT << _SAES_CR_GCMPH_SHIFT)) {
		/* Still in initialization phase, no header
		 * We need to enable the SAES peripheral
		 */
		mmio_setbits_32(ctx->base + _SAES_CR, _SAES_CR_EN);
	}

	/* No need to htobe() as we configure the HW to swap bytes */
	mmio_write_32(ctx->base + _SAES_DINR, 0);
	mmio_write_32(ctx->base + _SAES_DINR, ctx->assoc_len);
	mmio_write_32(ctx->base + _SAES_DINR, 0);
	mmio_write_32(ctx->base + _SAES_DINR, ctx->load_len);

	ret = wait_computation_completed(ctx->base);
	if (ret != 0) {
		goto out;
	}

	/* No need to htobe() as we configure the HW to swap bytes */
	tag_u32[0] = mmio_read_32(ctx->base + _SAES_DOUTR);
	tag_u32[1] = mmio_read_32(ctx->base + _SAES_DOUTR);
	tag_u32[2] = mmio_read_32(ctx->base + _SAES_DOUTR);
	tag_u32[3] = mmio_read_32(ctx->base + _SAES_DOUTR);

	clear_computation_completed(ctx->base);

	memcpy(tag, tag_u32, MIN(sizeof(tag_u32), tag_size));

out:
	saes_end(ctx, ret);

	return ret;
}

/**
 * @brief Update (or start) a AES de/encrypt process (ECB, CBC or CTR).
 * @param ctx: SAES process context
 * @param last_block: true if last payload data block
 * @param data_in: pointer to payload
 * @param data_out: pointer where to save de/encrypted payload
 * @param data_size: payload size
 *
 * @retval 0 if OK; negative value else.
 */
int stm32_saes_update(struct stm32_saes_context *ctx, bool last_block,
		      uint8_t *data_in, uint8_t *data_out, size_t data_size)
{
	int ret;
	uint32_t *data_in_u32;
	uint32_t *data_out_u32;
	unsigned int i = 0U;

	/* We want buffers to be u32 aligned */
	assert((uintptr_t)data_in % __alignof__(uint32_t) == 0);
	assert((uintptr_t)data_out % __alignof__(uint32_t) == 0);
	data_in_u32 = (uint32_t *)data_in;
	data_out_u32 = (uint32_t *)data_out;

	if ((!last_block) &&
	    (round_down(data_size, AES_BLOCK_SIZE) != data_size)) {
		ERROR("%s: non last block must be multiple of 128 bits\n",
		      __func__);
		ret = -EINVAL;
		goto out;
	}

	/* In CBC encryption we need to manage specifically last 2 128bits
	 * blocks if total size in not a block size aligned
	 * work TODO. Currently return ENODEV.
	 * Morevoer as we need to know last 2 block, if unaligned and
	 * call with less than two block, return -EINVAL.
	 */
	if (last_block && IS_CHAINING_MODE(CBC, ctx->cr) && is_encrypt(ctx->cr) &&
	    (round_down(data_size, AES_BLOCK_SIZE) != data_size)) {
		if (data_size < AES_BLOCK_SIZE * 2U) {
			ERROR("if CBC, last part size should be at least 2 * AES_BLOCK_SIZE\n");
			ret = -EINVAL;
			goto out;
		}
		/* Moreover the CBC specific padding for encrypt is not yet implemented */
		ret = -ENODEV;
		goto out;
	}

	ret = restore_context(ctx);
	if (ret != 0) {
		goto out;
	}

	while (i < round_down(data_size, AES_BLOCK_SIZE)) {
		unsigned int w; /* Word index */

		w = i / sizeof(uint32_t);
		/* No need to htobe() as we configure the HW to swap bytes */
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 0U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 1U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 2U]);
		mmio_write_32(ctx->base + _SAES_DINR, data_in_u32[w + 3U]);

		ret = wait_computation_completed(ctx->base);
		if (ret != 0) {
			goto out;
		}

		/* No need to htobe() as we configure the HW to swap bytes */
		data_out_u32[w + 0U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 1U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 2U] = mmio_read_32(ctx->base + _SAES_DOUTR);
		data_out_u32[w + 3U] = mmio_read_32(ctx->base + _SAES_DOUTR);

		clear_computation_completed(ctx->base);

		/* Process next block */
		i += AES_BLOCK_SIZE;
	}
	/* Manage last block if not a block size multiple */

	if ((last_block) && (i < data_size)) {
		/* In and out buffer have same size so should be AES_BLOCK_SIZE multiple */
		ret = -ENODEV;
		goto out;
	}

	if (!last_block) {
		ret = save_context(ctx);
	}

out:
	/* If last block or error, end of SAES process */
	if (last_block || (ret != 0)) {
		saes_end(ctx, ret);
	}

	return ret;
}
