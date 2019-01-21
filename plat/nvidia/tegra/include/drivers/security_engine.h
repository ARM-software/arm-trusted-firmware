/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURITY_ENGINE_H
#define SECURITY_ENGINE_H

/*******************************************************************************
 * Structure definition
 ******************************************************************************/

/* Security Engine Linked List */
struct tegra_se_ll {
	/* DMA buffer address */
	uint32_t addr;
	/* Data length in DMA buffer */
	uint32_t data_len;
};

#define SE_LL_MAX_BUFFER_NUM			4
typedef struct tegra_se_io_lst {
	volatile uint32_t last_buff_num;
	volatile struct tegra_se_ll buffer[SE_LL_MAX_BUFFER_NUM];
} tegra_se_io_lst_t __attribute__((aligned(4)));

/* SE device structure */
typedef struct tegra_se_dev {
	/* Security Engine ID */
	const int se_num;
	/* SE base address */
	const uint64_t se_base;
	/* SE context size in AES blocks */
	const uint32_t ctx_size_blks;
	/* pointer to source linked list buffer */
	tegra_se_io_lst_t *src_ll_buf;
	/* pointer to destination linked list buffer */
	tegra_se_io_lst_t *dst_ll_buf;
	/* LP context buffer pointer */
	uint32_t *ctx_save_buf;
} tegra_se_dev_t;

/* PKA1 device structure */
typedef struct tegra_pka_dev {
	/* PKA1 base address */
	uint64_t pka_base;
} tegra_pka_dev_t;

/*******************************************************************************
 * Public interface
 ******************************************************************************/
void tegra_se_init(void);
int tegra_se_suspend(void);
void tegra_se_resume(void);
int tegra_se_save_tzram(void);

#endif /* SECURITY_ENGINE_H */
