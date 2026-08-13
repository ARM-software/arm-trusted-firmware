#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME object_pool
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/test_object_pool.cpp
		${CMAKE_CURRENT_LIST_DIR}/object_pool_allocator_wrapper.c
		${PROJECT_SOURCE_DIR}/mocks/common/debug.cpp
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
		${UNIT_TEST_PROJECT_PATH}/include
	COMPILE_DEFINITIONS
		LOG_LEVEL=50
)
