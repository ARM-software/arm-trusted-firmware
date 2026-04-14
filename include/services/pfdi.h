/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PFDI_H
#define PFDI_H

#include <stdbool.h>
#include <stdint.h>

#include <plat/common/platform.h>

/**
 * PFDI Return Codes
 */
typedef enum {
	PFDI_RET_TEST_COUNT_ZERO = -8,
	PFDI_RET_UNKNOWN = -7,
	PFDI_RET_NOT_RUN = -6,
	PFDI_RET_ERROR = -5,
	PFDI_RET_FAULT_FOUND = -4,
	PFDI_RET_INVALID_PARAMETERS = -3,
	PFDI_SMCC_RESERVED_ERROR_ID = -2,
	PFDI_SMCC_RET_NOT_SUPPORTED = -1,
	PFDI_SMCC_RET_SUCCESS = 0,
} pfdi_status_t;

/**
 * PFDI Execution Modes
 */
enum pfdi_execution_mode {
	PFDI_OOR_MODE = 1,
	PFDI_ONL_MODE = 2,
};

/**
 * PFDI Major and Minor version
 */
#define PFDI_VERSION_MAJOR		U(1)
#define PFDI_VERSION_MAJOR_SHIFT	16
#define PFDI_VERSION_MAJOR_MASK		U(0x7FFF)
#define PFDI_VERSION_MINOR		U(0)
#define PFDI_VERSION_MINOR_SHIFT	0
#define PFDI_VERSION_MINOR_MASK		U(0xFFFF)

#define PFDI_VENDOR_VERSION				\
		(((((uint64_t)PFDI_VERSION_MAJOR) &	\
		PFDI_VERSION_MAJOR_MASK)		\
		<< PFDI_VERSION_MAJOR_SHIFT) |		\
		((((uint64_t)PFDI_VERSION_MINOR) &	\
		PFDI_VERSION_MINOR_MASK)		\
		<< PFDI_VERSION_MINOR_SHIFT))

/* Allowed bits: [31:24]=vendor, [19:16]=impl, [15:8]=major, [7:0]=minor */
#define PFDI_VERSION_ALLOWED_MASK	UINT64_C(0x00000000FF0FFFFF)
#define PFDI_UPPER_RESERVED_MASK	UINT64_C(0xFFFFFFFF00000000)
#define PFDI_RESERVED_MASK		UINT64_C(0x0000000000F00000)

#define PFDI_VENDOR_SHIFT		24u
#define PFDI_IMPL_SHIFT			16u
#define PFDI_MAJOR_SHIFT		8u
#define PFDI_MINOR_SHIFT		0u

/********************************************
 * PFDI Vendor IDs
 * +-------------------+-------------------+
 * | Vendor ID         | Vendor Name       |
 * +-------------------+-------------------+
 * |   0               | Arm Limited       |
 * |   Others          | Reserved          |
 * +-------------------+-------------------+
 ********************************************/
#define PFDI_VENDOR_ARM			0u

/********************************************
 * PFDI Implementation IDs
 * +-------------------+--------------------+
 * | Implementation ID | Library Name       |
 * +-------------------+--------------------+
 * |   0               |  Reserved          |
 * |   1               |  Arm STL           |
 * |   Others          |  Reserved          |
 * +-------------------+--------------------+
 *********************************************/
#define PFDI_IMPL_RESERVED		0u
#define PFDI_IMPL_ARM_STL		1u

#ifndef PFDI_TEST_LIB_VERSION
#define PFDI_TEST_LIB_VERSION		UINT64_C(0x0000000000000000)
#endif

#define PFDI_CFG_VENDOR \
	(((PFDI_TEST_LIB_VERSION) >> PFDI_VENDOR_SHIFT) & UINT64_C(0xFF))

/* PFDI test library version sanity checks based on Platform
 * Fault Detection Interface Specification v1.0BET0.
 */

/* Reserved bits [63:32] must be zero */
#if ((PFDI_TEST_LIB_VERSION & PFDI_UPPER_RESERVED_MASK) != UINT64_C(0))
#error "PFDI_TEST_LIB_VERSION: reserved bits [63:32] must be zero"
#endif

/* Reserved bits [23:20] must be zero */
#if ((PFDI_TEST_LIB_VERSION & PFDI_RESERVED_MASK) != UINT64_C(0))
#error "PFDI_TEST_LIB_VERSION: reserved bits [23:20] must be zero"
#endif

/* Only Vendor ID 0 (Arm Limited) is currently valid */
#if (PFDI_CFG_VENDOR != PFDI_VENDOR_ARM)
#error "PFDI_TEST_LIB_VERSION: vendor ID must be 0 as per spec"
#endif

static inline void pack_vendor_id(uint64_t *version)
{
	uint64_t cfg    = (uint64_t)PFDI_TEST_LIB_VERSION;
	uint32_t vendor = (uint32_t)((cfg >> PFDI_VENDOR_SHIFT) & UINT64_C(0xFF));
	uint32_t impl   = (uint32_t)((cfg >> PFDI_IMPL_SHIFT)   & UINT64_C(0x0F));
	uint32_t major  = (uint32_t)((cfg >> PFDI_MAJOR_SHIFT)  & UINT64_C(0xFF));
	uint32_t minor  = (uint32_t)((cfg >> PFDI_MINOR_SHIFT)  & UINT64_C(0xFF));

	if (((cfg & PFDI_UPPER_RESERVED_MASK) != UINT64_C(0)) ||
		((cfg & PFDI_RESERVED_MASK) != UINT64_C(0)) ||
		(vendor != PFDI_VENDOR_ARM)) {
		*version = 0ull;    /* On error: Must be zero */
	} else {
		uint64_t v = 0ull;

		v |= ((uint64_t)vendor << 24);
		v |= ((uint64_t)impl   << 16);
		v |= ((uint64_t)major  << 8);
		v |= ((uint64_t)minor  << 0);
		*version = v & PFDI_VERSION_ALLOWED_MASK;
	}
}

static inline bool is_valid_mode(uint64_t mode)
{
	return ((mode == PFDI_ONL_MODE) || (mode == PFDI_OOR_MODE));
}

static inline bool is_valid_force_error_id(int64_t error_id)
{
	switch (error_id) {
	case PFDI_SMCC_RET_NOT_SUPPORTED:
	case PFDI_RET_INVALID_PARAMETERS:
	case PFDI_RET_FAULT_FOUND:
	case PFDI_RET_ERROR:
	case PFDI_RET_NOT_RUN:
	case PFDI_RET_UNKNOWN:
	case PFDI_RET_TEST_COUNT_ZERO:
		return true;
	default:
		return false;
	}
}

/**
 * Platform Fault Detection Interface Function descriptor.
 */
typedef struct pfdi_func_desc_s {
	/**
	 * Name of the PFDI function.
	 */
	const char *name;

	/**
	 * Run PFDI operations for a specific CPU.
	 *
	 * @param[in] start		The start test case number.
	 * @param[in] end		The end test case number.
	 * @param[in] mode		PFDI operation mode (online/out of reset)
	 * @param[out] ft_id		The failed test case id.
	 *
	 * @return			0 on success or an error code on failure.
	 */
	pfdi_status_t (*run)(uint64_t start, uint64_t end, uint64_t mode,
				uint64_t *ft_id);

	/**
	 * Get the PFDI test case size.
	 *
	 * @param[out] tc_size		Pointer to the test case size.
	 *
	 * @return			Total number of test cases on success or
	 *				an error code on failure.
	 */
	pfdi_status_t (*count)(uint64_t *tc_size);

	/**
	 * Get the PFDI test result.
	 *
	 * @param[in] cpu_num		Logical CPU number.
	 * @param[out] ft_id		The failed test case id.
	 *
	 * @return			0 on success or an error code on failure.
	 */
	pfdi_status_t (*result)(uint64_t cpu_num, uint64_t *ft_id);

} pfdi_func_desc_t;

/* PFDI shared functions */

/**
 * Initialize the PFDI.
 */
void pfdi_init(void);

/**
 * Run PFDI operations for a specific CPU.
 *
 * @param[in] start		The start test case number.
 * @param[in] end		The end test case number.
 * @param[in] mode		PFDI operation mode (online/out of reset).
 * @param[out] ft_id		Pointer to the failed test case id.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_test_run(uint64_t start, uint64_t end, uint64_t mode,
				 uint64_t *ft_id);

/**
 * Get the total number of PFDI test suites.
 *
 * @param[out] tc_size		Pointer to the test case size.
 *
 * @return			The total number of test suites on success or an error code on
 *				failure.
 */
pfdi_status_t pfdi_pe_test_part_count(uint64_t *tc_size);

/**
 * Get the PFDI vendor Id.
 *
 * @param[out] lib_version	Pointer to store the PFDI test library version.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_test_id(uint64_t *lib_version);

/**
 * Get the PFDI test result.
 *
 * @param[out] ft_id		Pointer to the failed test case id.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_test_result(uint64_t *ft_id);

/**
 * Get the PFDI version.
 *
 * @param[out] pfdi_version	Pointer to store the PFDI version.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_version(uint64_t *pfdi_version);

/**
 * Firmware check on request.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_fw_check(void);

/**
 * Validate PE test-run arguments without executing any test part.
 *
 * @param[in] start		The start test case number as ABI value.
 * @param[in] end		The end test case number as ABI value.
 * @param[in] mode		PFDI operation mode (online/out of reset).
 *
 * @return			0 when the request is valid, otherwise the
 *				an error code to report.
 */
pfdi_status_t pfdi_pe_test_run_validate(int64_t start, int64_t end,
	uint64_t mode);

/**
 * Check supported features.
 *
 * @param[in] fid		Function identifier to check.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_features(uint32_t fid);

/**
 * Force error on request.
 *
 * @param[in] fid		Targeted smc function id.
 * @param[in] error_id		Targeted PFDI error id. Must be negative.
 *
 * @return			0 on success or an error code on failure.
 */
pfdi_status_t pfdi_pe_force_error(uint32_t fid, int64_t error_id);

/**
 * Consume a pending forced error for a PFDI ABI function.
 *
 * @param[in] fid		Targeted SMC function id.
 *
 * @return			The injected error if one is pending for this
 *				PE/function, otherwise
 *				`PFDI_SMCC_RESERVED_ERROR_ID`.
 */
int64_t pfdi_consume_force_error(uint32_t fid);

/**
 * Validate whether a FORCE_ERROR request is well-formed.
 *
 * @param[in] fid		Targeted smc function id.
 * @param[in] error_id		Targeted PFDI error id. Must be negative.
 *
 * @return			0 when the request is valid, otherwise the
 *				an error code to report.
 */
pfdi_status_t pfdi_pe_force_error_validate(uint32_t fid, int64_t error_id);

/**
 * Macro to register a callback with pfdi library.
 *
 * This macro defines and registers a PFDI function descriptor.
 *
 * @param _name		The name of the callback.
 * @param _run		The function pointer for running PFDI operations.
 * @param _count	The function pointer for retrieving test count.
 * @param _result	The function pointer for retrieving test result.
 */
#define REGISTER_PFDI_FUNC(_name, _run, _count, _result)	\
		const pfdi_func_desc_t pfdi_func_desc = {	\
			.name = _name,				\
			.run = _run,				\
			.count = _count,			\
			.result = _result			\
		}

/**
 * Declaration for a registered PFDI handlers
 */
extern const pfdi_func_desc_t pfdi_func_desc;

/**
 * Platform PFDI function descriptor (optional).
 */
struct plat_pfdi_func_desc {
	/**
	 * Name of the Platform PFDI function.
	 */
	const char *name;

	/**
	 * Optional function handler to force platform specific error.
	 *
	 * @param fid           Function Id to inject error.
	 * @param error_id      ERROR Id to force error.
	 *
	 * @return              0 on success or an error code on failure.
	 */
	int64_t (*force_plat_err)(uint32_t fid, int64_t error_id);

	/**
	 * Optional function handler to check platform specific errors.
	 *
	 * @param fid           Function Id to check error status.
	 * @param error_id      ERROR Id to be expected.
	 *
	 * @return              return the validated error id. The PFDI service
	 *			always reports the original injected ABI error in
	 *			x0, so platform hooks must not rely on rewriting it.
	 */
	int64_t (*check_plat_err)(uint32_t fid, int64_t error_id);

	/**
	 * Post-run PFDI operations for a specific CPU.
	 *
	 * @param[in] status            Whether the last run succeeded or failed.
	 * @param[in] start             The start test case number.
	 * @param[in] end               The end test case number.
	 * @param[in] mode              PFDI operation mode (online/out of reset)
	 * @param[out] ft_id            The failed test case id.
	 *
	 * @return                      Void
	 */
	void (*post_run)(pfdi_status_t status, uint64_t start, uint64_t end,
			 uint64_t mode, uint64_t *ft_id);
};

/**
 * Register the optional platform PFDI callback descriptor.
 *
 * Platforms that implement platform-specific hooks must call this during
 * platform setup before `pfdi_init()` is reached.
 *
 * @param desc	Pointer to the platform callback descriptor.
 */
void pfdi_register_plat_func_desc(const struct plat_pfdi_func_desc *desc);

/**
 * Get the registered platform PFDI callback descriptor.
 *
 * The PFDI service always returns a valid descriptor. When the platform does
 * not register any callbacks, all function pointers in the returned
 * descriptor are `NULL`.
 *
 * @return			Pointer to the active platform callback descriptor.
 */
const struct plat_pfdi_func_desc *pfdi_get_plat_func_desc(void);

#endif /* PFDI_H */
