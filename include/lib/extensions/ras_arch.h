/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RAS_ARCH_H
#define RAS_ARCH_H

/*
 * Size of nodes implementing Standard Error Records - currently only 4k is
 * supported.
 */
#define STD_ERR_NODE_SIZE_NUM_K		4U

/*
 * Individual register offsets within an error record in Standard Error Record
 * format when error records are accessed through memory-mapped registers.
 */
#define ERR_FR(n)	(0x0ULL + (64ULL * (n)))
#define ERR_CTLR(n)	(0x8ULL + (64ULL * (n)))
#define ERR_STATUS(n)	(0x10ULL + (64ULL * (n)))
#define ERR_ADDR(n)	(0x18ULL + (64ULL * (n)))
#define ERR_MISC0(n)	(0x20ULL + (64ULL * (n)))
#define ERR_MISC1(n)	(0x28ULL + (64ULL * (n)))

/* Group Status Register (ERR_STATUS) offset */
#define ERR_GSR(base, size_num_k, n) \
	((base) + (0x380ULL * (size_num_k)) + (8ULL * (n)))

/* Management register offsets */
#define ERR_DEVID(base, size_num_k) \
	((base) + ((0x400ULL * (size_num_k)) - 0x100ULL) + 0xc8ULL)

#define ERR_DEVID_MASK	0xffffUL

/* Standard Error Record status register fields */
#define ERR_STATUS_AV_SHIFT	31
#define ERR_STATUS_AV_MASK	U(0x1)

#define ERR_STATUS_V_SHIFT	30
#define ERR_STATUS_V_MASK	U(0x1)

#define ERR_STATUS_UE_SHIFT	29
#define ERR_STATUS_UE_MASK	U(0x1)

#define ERR_STATUS_ER_SHIFT	28
#define ERR_STATUS_ER_MASK	U(0x1)

#define ERR_STATUS_OF_SHIFT	27
#define ERR_STATUS_OF_MASK	U(0x1)

#define ERR_STATUS_MV_SHIFT	26
#define ERR_STATUS_MV_MASK	U(0x1)

#define ERR_STATUS_CE_SHIFT	24
#define ERR_STATUS_CE_MASK	U(0x3)

#define ERR_STATUS_DE_SHIFT	23
#define ERR_STATUS_DE_MASK	U(0x1)

#define ERR_STATUS_PN_SHIFT	22
#define ERR_STATUS_PN_MASK	U(0x1)

#define ERR_STATUS_UET_SHIFT	20
#define ERR_STATUS_UET_MASK	U(0x3)

#define ERR_STATUS_IERR_SHIFT	8
#define ERR_STATUS_IERR_MASK	U(0xff)

#define ERR_STATUS_SERR_SHIFT	0
#define ERR_STATUS_SERR_MASK	U(0xff)

#define ERR_STATUS_GET_FIELD(_status, _field) \
	(((_status) >> ERR_STATUS_ ##_field ##_SHIFT) & ERR_STATUS_ ##_field ##_MASK)

#define ERR_STATUS_CLR_FIELD(_status, _field) \
	(_status) &= ~(ERR_STATUS_ ##_field ##_MASK << ERR_STATUS_ ##_field ##_SHIFT)

#define ERR_STATUS_SET_FIELD(_status, _field, _value) \
	(_status) |= (((_value) & ERR_STATUS_ ##_field ##_MASK) << ERR_STATUS_ ##_field ##_SHIFT)

#define ERR_STATUS_WRITE_FIELD(_status, _field, _value) do { \
		ERR_STATUS_CLR_FIELD(_status, _field, _value); \
		ERR_STATUS_SET_FIELD(_status, _field, _value); \
	} while (0)


/* Standard Error Record control register fields */
#define ERR_CTLR_WDUI_SHIFT	11
#define ERR_CTLR_WDUI_MASK	0x1

#define ERR_CTLR_RDUI_SHIFT	10
#define ERR_CTLR_RDUI_MASK	0x1
#define ERR_CTLR_DUI_SHIFT	ERR_CTLR_RDUI_SHIFT
#define ERR_CTLR_DUI_MASK	ERR_CTLR_RDUI_MASK

#define ERR_CTLR_WCFI_SHIFT	9
#define ERR_CTLR_WCFI_MASK	0x1

#define ERR_CTLR_RCFI_SHIFT	8
#define ERR_CTLR_RCFI_MASK	0x1
#define ERR_CTLR_CFI_SHIFT	ERR_CTLR_RCFI_SHIFT
#define ERR_CTLR_CFI_MASK	ERR_CTLR_RCFI_MASK

#define ERR_CTLR_WUE_SHIFT	7
#define ERR_CTLR_WUE_MASK	0x1

#define ERR_CTLR_WFI_SHIFT	6
#define ERR_CTLR_WFI_MASK	0x1

#define ERR_CTLR_WUI_SHIFT	5
#define ERR_CTLR_WUI_MASK	0x1

#define ERR_CTLR_RUE_SHIFT	4
#define ERR_CTLR_RUE_MASK	0x1
#define ERR_CTLR_UE_SHIFT	ERR_CTLR_RUE_SHIFT
#define ERR_CTLR_UE_MASK	ERR_CTLR_RUE_MASK

#define ERR_CTLR_RFI_SHIFT	3
#define ERR_CTLR_RFI_MASK	0x1
#define ERR_CTLR_FI_SHIFT	ERR_CTLR_RFI_SHIFT
#define ERR_CTLR_FI_MASK	ERR_CTLR_RFI_MASK

#define ERR_CTLR_RUI_SHIFT	2
#define ERR_CTLR_RUI_MASK	0x1
#define ERR_CTLR_UI_SHIFT	ERR_CTLR_RUI_SHIFT
#define ERR_CTLR_UI_MASK	ERR_CTLR_RUI_MASK

#define ERR_CTLR_ED_SHIFT	0
#define ERR_CTLR_ED_MASK	0x1

#define ERR_CTLR_CLR_FIELD(_ctlr, _field) \
	(_ctlr) &= ~(ERR_CTLR_ ##_field _MASK << ERR_CTLR_ ##_field ##_SHIFT)

#define ERR_CTLR_SET_FIELD(_ctlr, _field, _value) \
	(_ctlr) |= (((_value) & ERR_CTLR_ ##_field ##_MASK) << ERR_CTLR_ ##_field ##_SHIFT)

#define ERR_CTLR_ENABLE_FIELD(_ctlr, _field) \
	ERR_CTLR_SET_FIELD(_ctlr, _field, ERR_CTLR_ ##_field ##_MASK)

/* Uncorrected error types for Asynchronous exceptions */
#define ERROR_STATUS_UET_UC	0x0	/* Uncontainable */
#define ERROR_STATUS_UET_UEU	0x1	/* Unrecoverable */
#define ERROR_STATUS_UET_UEO	0x2	/* Restable */
#define ERROR_STATUS_UET_UER	0x3	/* Recoverable */

/* Error types for Synchronous exceptions */
#define ERROR_STATUS_SET_UER	0x0	/* Recoverable */
#define ERROR_STATUS_SET_UEO	0x1	/* Restable */
#define ERROR_STATUS_SET_UC	0x2     /* Uncontainable */
#define ERROR_STATUS_SET_CE	0x3     /* Corrected */

/* Implementation Defined Syndrome bit in ESR */
#define SERROR_IDS_BIT		U(24)

/*
 * Asynchronous Error Type in exception syndrome. The field has same values in
 * both DISR_EL1 and ESR_EL3 for SError.
 */
#define EABORT_AET_SHIFT	U(10)
#define EABORT_AET_WIDTH	U(3)
#define EABORT_AET_MASK		U(0x7)

/* DFSC field in Asynchronous exception syndrome */
#define EABORT_DFSC_SHIFT	U(0)
#define EABORT_DFSC_WIDTH	U(6)
#define EABORT_DFSC_MASK	U(0x3f)

/* Synchronous Error Type in exception syndrome. */
#define EABORT_SET_SHIFT	U(11)
#define EABORT_SET_WIDTH	U(2)
#define EABORT_SET_MASK		U(0x3)

/* DFSC code for SErrors */
#define DFSC_SERROR		0x11

/* I/DFSC code for synchronous external abort */
#define SYNC_EA_FSC		0x10

#ifndef __ASSEMBLY__

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <context.h>
#include <lib/mmio.h>
#include <stdint.h>

/*
 * Standard Error Record accessors for memory-mapped registers.
 */

static inline uint64_t ser_get_feature(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_FR(idx));
}

static inline uint64_t ser_get_control(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_CTLR(idx));
}

static inline uint64_t ser_get_status(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_STATUS(idx));
}

/*
 * Error handling agent would write to the status register to clear an
 * identified/handled error. Most fields in the status register are
 * conditional write-one-to-clear.
 *
 * Typically, to clear the status, it suffices to write back the same value
 * previously read. However, if there were new, higher-priority errors recorded
 * on the node since status was last read, writing read value won't clear the
 * status. Therefore, an error handling agent must wait on and verify the status
 * has indeed been cleared.
 */
static inline void ser_set_status(uintptr_t base, unsigned int idx,
		uint64_t status)
{
	mmio_write_64(base + ERR_STATUS(idx), status);
}

static inline uint64_t ser_get_addr(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_ADDR(idx));
}

static inline uint64_t ser_get_misc0(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_MISC0(idx));
}

static inline uint64_t ser_get_misc1(uintptr_t base, unsigned int idx)
{
	return mmio_read_64(base + ERR_MISC1(idx));
}


/*
 * Standard Error Record helpers for System registers.
 */
static inline void ser_sys_select_record(unsigned int idx)
{
	unsigned int max_idx __unused =
		(unsigned int) read_erridr_el1() & ERRIDR_MASK;

	assert(idx < max_idx);

	write_errselr_el1(idx);
	isb();
}

/* Library functions to probe Standard Error Record */
int ser_probe_memmap(uintptr_t base, unsigned int size_num_k, int *probe_data);
int ser_probe_sysreg(unsigned int idx_start, unsigned int num_idx, int *probe_data);
#endif /* __ASSEMBLY__ */

#endif /* RAS_ARCH_H */
