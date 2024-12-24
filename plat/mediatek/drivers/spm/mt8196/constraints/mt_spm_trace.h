/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_TRACE_H
#define MT_SPM_TRACE_H

#include <lib/mmio.h>
#include <platform_def.h>

enum mt_spm_sysram_type {
	MT_SPM_SYSRAM_COMMON,
	MT_SPM_SYSRAM_SUSPEND,
	MT_SPM_SYSRAM_LP,
};

/* SPM trace common type */
enum mt_spm_trace_common_type {
	MT_SPM_TRACE_COMM_HAED,
	MT_SPM_TRACE_COMM_FP,
	MT_SPM_TRACE_COMM_RC_LAST_TIME_H,
	MT_SPM_TRACE_COMM_RC_LAST_TIME_L,
	MT_SPM_TRACE_COMM_RC_INFO,
	MT_SPM_TRACE_COMM_RC_FP,
	MT_SPM_TRACE_COMM_RC_VALID,
};

/* SPM trace suspend type */
enum mt_spm_trace_suspend_type {
	MT_SPM_TRACE_SUSPEND_WAKE_SRC,
};

/*
 * SPM sram usage with mcdi sram
 * start offset : 0x500
 */
#define MT_SPM_SYSRAM_BASE		(MTK_LPM_SRAM_BASE + 0x500)
#define MT_SPM_SYSRAM_COMM_BASE		MT_SPM_SYSRAM_BASE
#define MT_SPM_SYSRAM_COMM_SZ		0x20

#define MT_SPM_SYSRAM_SUSPEND_BASE \
	(MT_SPM_SYSRAM_BASE + MT_SPM_SYSRAM_COMM_SZ)
#define MT_SPM_SYSRAM_SUSPEND_SZ	0xe0

#define MT_SPM_SYSRAM_LP_BASE \
	(MT_SPM_SYSRAM_SUSPEND_BASE + MT_SPM_SYSRAM_SUSPEND_SZ)

#define MT_SPM_SYSRAM_SLOT(slot)	((slot) << 2u)

#ifndef MTK_PLAT_SPM_TRACE_UNSUPPORT

#define MT_SPM_SYSRAM_W(_s, type, val, _sz) \
			mt_spm_sysram_write(_s, type, val, _sz)

#define MT_SPM_SYSRAM_R_U32(addr, val)	({ \
	unsigned int *r_val = (unsigned int *)val; \
	if (r_val) \
		*r_val = mmio_read_32(addr); })

#define MT_SPM_SYSRAM_R(_s, type, val) \
			mt_spm_sysram_read(_s, type, val)

/* SPM trace common */
#define MT_SPM_TRACE_INIT(_magic) ({ \
	mt_spm_sysram_init(_magic); })

#define MT_SPM_TRACE_COMMON_U32_WR(_type, _val) ({ \
	mmio_write_32((MT_SPM_SYSRAM_COMM_BASE + \
		      MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_COMMON_WR(_type, val, _sz) ({ \
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_COMMON, \
				  _type, val, _sz); ret; })

#define MT_SPM_TRACE_COMMON_U32_RD(_type, _val) ({ \
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_COMM_BASE + \
			    MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_COMMON_RD(_type, _val) ({ \
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_COMMON, \
				  _type, _val); ret; })

/* SPM trace suspend */
#define MT_SPM_TRACE_SUSPEND_U32_WR(_type, _val) ({ \
	mmio_write_32((MT_SPM_SYSRAM_SUSPEND_BASE + \
		      MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_SUSPEND_WR(_type, _val, _sz) ({ \
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_SUSPEND, \
				  _type, _val, _sz); ret; })

#define MT_SPM_TRACE_SUSPEND_U32_RD(_type, _val) ({\
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_SUSPEND_BASE + \
			    MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_SUSPEND_RD(_type, _val) ({ \
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_SUSPEND, \
				  _type, _val); ret; })

/* SPM trace low power */
#define MT_SPM_TRACE_LP_U32_WR(_type, _val) ({ \
	mmio_write_32((MT_SPM_SYSRAM_LP_BASE + \
		      MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_LP_WR(_type, _val, _sz) ({ \
	int ret = MT_SPM_SYSRAM_W(MT_SPM_SYSRAM_LP, \
				  _type, _val, _sz); ret; })

#define MT_SPM_TRACE_LP_U32_RD(_type, _val) ({ \
	MT_SPM_SYSRAM_R_U32((MT_SPM_SYSRAM_LP_BASE + \
			    MT_SPM_SYSRAM_SLOT(_type)), _val); })

#define MT_SPM_TRACE_LP_RD(_type, _val) ({ \
	int ret = MT_SPM_SYSRAM_R(MT_SPM_SYSRAM_LP, \
				  _type, _val); ret; })

#define MT_SPM_TRACE_LP_RINGBUF(_pval, _sz) ({ \
	int ret = mt_spm_sysram_lp_ringbuf_add(_pval, _sz); ret; })

int mt_spm_sysram_lp_ringbuf_add(const void *val, unsigned int sz);

int mt_spm_sysram_write(int section, int type, const void *val,
			unsigned int sz);
int mt_spm_sysram_read(int section, int type, void *val);

int mt_spm_sysram_init(unsigned int magic);
#else
/* SPM trace common */
#define MT_SPM_TRACE_INIT(_magic)
#define MT_SPM_TRACE_COMMON_U32_WR(type, val)
#define MT_SPM_TRACE_COMMON_WR(val)
#define MT_SPM_TRACE_COMMON_U32_RD(type, val)
#define MT_SPM_TRACE_COMMON_RD(val)

/* SPM trace suspend */
#define MT_SPM_TRACE_SUSPEND_U32_WR(type, val)
#define MT_SPM_TRACE_SUSPEND_WR(val)
#define MT_SPM_TRACE_SUSPEND_U32_RD(type, val)
#define MT_SPM_TRACE_SUSPEND_RD(val)

/* SPM trace low power */
#define MT_SPM_TRACE_LP_U32_WR(type, val)
#define MT_SPM_TRACE_LP_WR(val)
#define MT_SPM_TRACE_LP_U32_RD(type, val)
#define MT_SPM_TRACE_LP_RD(val)
#define MT_SPM_TRACE_LP_RINGBUF(pval, sz)

#define mt_spm_sysram_lp_ringbuf_add(_val, _sz)
#define mt_spm_sysram_write(_s, _type, _val, _sz)
#define mt_spm_sysram_read(_s, _type, _val)
#define mt_spm_sysram_init(_magic)
#endif

#endif /* MT_SPM_TRACE_H */

