#
# Copyright (c) 2019-2025, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME memcmp
	SOURCES
		${UNIT_TEST_PROJECT_PATH}/lib/libc/memcmp.c
		${CMAKE_CURRENT_LIST_DIR}/test_memcmp.cpp
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
)

target_compile_options(memcmp PRIVATE -fno-builtin-memcmp)

unit_test_add_suite(
	NAME memcpy
	SOURCES
		${UNIT_TEST_PROJECT_PATH}/lib/libc/memcpy.c
		${CMAKE_CURRENT_LIST_DIR}/test_memcpy.cpp
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
)

target_compile_options(memcpy PRIVATE -fno-builtin-memcpy)

unit_test_add_suite(
	NAME memset
	SOURCES
		${UNIT_TEST_PROJECT_PATH}/lib/libc/memset.c
		${CMAKE_CURRENT_LIST_DIR}/test_memset.cpp
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
)

target_compile_options(memcpy PRIVATE -fno-builtin-memset)

unit_test_add_suite(
	NAME snprintf
	SOURCES
		${UNIT_TEST_PROJECT_PATH}/lib/libc/snprintf.c
		${CMAKE_CURRENT_LIST_DIR}/test_snprintf.cpp
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
		${PROJECT_SOURCE_DIR}/mocks/include/plat/host/include/
		${UNIT_TEST_PROJECT_PATH}/include/
		${UNIT_TEST_PROJECT_PATH}/include/lib/libc/aarch64/
)

target_compile_options(memcpy PRIVATE -fno-builtin-snprintf)
