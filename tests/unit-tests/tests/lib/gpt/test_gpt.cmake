#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME gpt
	SOURCES
		${UNIT_TEST_PROJECT_PATH}/lib/gpt_rme/gpt_rme.c
		${PROJECT_SOURCE_DIR}/mocks/lib/locks/exclusive/spinlock.c
		${PROJECT_SOURCE_DIR}/mocks/lib/xlat_tables_v2/aarch64/xlat_tables_arch.c
		${PROJECT_SOURCE_DIR}/mocks/lib/aarch64/cache_helpers.c
		${PROJECT_SOURCE_DIR}/mocks/lib/aarch64/misc_helpers.c
		${CMAKE_CURRENT_LIST_DIR}/test_tables.cpp
		${CMAKE_CURRENT_LIST_DIR}/test_gpt_utils.c
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/plat/arm/common
		${PROJECT_SOURCE_DIR}/mocks/include/
		${UNIT_TEST_PROJECT_PATH}/include
		${UNIT_TEST_PROJECT_PATH}/include/lib
		${PROJECT_SOURCE_DIR}/mocks/include/arch/aarch64/
		${UNIT_TEST_PROJECT_PATH}/include/arch/aarch64/
		${PROJECT_SOURCE_DIR}/mocks/include/common
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc
	COMPILE_DEFINITIONS
		ENABLE_FEAT_RME=1
)

