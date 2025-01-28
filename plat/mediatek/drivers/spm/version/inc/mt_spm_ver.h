/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_VER_H
#define MT_SPM_VER_H

#include <stdint.h>

enum mt_plat_dram_to_spm_type {
	SPMFW_DEFAULT_TYPE = 0,
	SPMFW_LP4_2CH_3200,
	SPMFW_LP4X_2CH_3600,
	SPMFW_LP3_1CH_1866,
	SPMFW_LP4X_2CH_3733,
	SPMFW_LP4X_2CH_4266,
	SPMFW_LP4X_2CH_3200,
	SPMFW_LP5_2CH_6400,
	SPMFW_LP5X_4CH_7500,
	SPMFW_TYPE_NOT_FOUND,
};

enum pwrctrl_selection {
	SPM_INIT_PWRCTRL = 0,
	SPM_VCOREDVFS_PWRCTRL,
	SPM_IDLE_PWRCTRL,
	SPM_SUSPEND_PWRCTRL,
	SPM_PWRCTRL_MAX,
};

struct pcm_desc {
	const char *version;	/* PCM code version */
	uint32_t *base;		/* Binary array base */
	uintptr_t base_dma;	/* DMA addr of base */
	uint32_t pmem_words;
	uint32_t total_words;
	uint32_t pmem_start;
	uint32_t dmem_start;
};

#define DYNA_LOAD_PCM_PATH_SIZE		128
#define PCM_FIRMWARE_VERSION_SIZE	128

struct dyna_load_pcm_t {
	char path[DYNA_LOAD_PCM_PATH_SIZE];
	char version[PCM_FIRMWARE_VERSION_SIZE];
	char *buf;
	struct pcm_desc desc;
	int ready;
};

struct load_pcm_fw_t {
	unsigned int fw_max_num;
	char **pcm_name_str;
	struct dyna_load_pcm_t *dyna_load_pcm;
	unsigned int (*is_fw_running)(void);
	unsigned int (*get_fw_index)(unsigned int fw_type);
	int (*fw_init)(struct pcm_desc *desc);
	int (*fw_run)(unsigned int first, void *pwrctrl);
};

/* SPM firmware restart definition */
#define SPM_FW_FORCE_RESET	BIT(0)
#define SPM_FW_FORCE_RESUME	BIT(1)
int spm_firmware_restart(unsigned int force, void *pwrctrl);
int spm_firmware_type_get(void);
void spm_firmware_type_probe(unsigned int type);
void spm_firmware_init(uint64_t addr, uint64_t size);
uint64_t spm_load_firmware_status(void);
void register_load_fw(struct load_pcm_fw_t *info);

#endif /* MT_SPM_VER_H */
