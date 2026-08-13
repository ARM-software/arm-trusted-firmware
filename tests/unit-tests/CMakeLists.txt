#
# Copyright (c) 2019-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.11...3.15) # TODO: test with ubuntu
project(tf-a-unit-tests)

include(CTest)
include(FetchContent)

# Variables used to set up the Firmware Test Builder
set(FIRMWARE_TEST_BUILDER_URL "https://git.trustedfirmware.org/TS/trusted-services.git")
set(FIRMWARE_TEST_BUILDER_REFSPEC "topics/fwtb")

# Check whether path to TF-A has been set. Stop otherwise.
if (NOT UNIT_TEST_PROJECT_PATH)
	message(FATAL_ERROR "Variable UNIT_TEST_PROJECT_PATH must be set with TF-A location")
endif()

# Fetch the Firmware Test Builder
FetchContent_Declare(
  firmware_test_builder
  GIT_REPOSITORY ${FIRMWARE_TEST_BUILDER_URL}
  GIT_TAG ${FIRMWARE_TEST_BUILDER_REFSPEC}
  GIT_SHALLOW TRUE
)

FetchContent_GetProperties(firmware_test_builder)
if(NOT firmware_test_builder_POPULATED)
  message(STATUS "Fetching Firmware Test Builder")
  FetchContent_MakeAvailable(firmware_test_builder)
endif()

# Appending Firmware Test Builder's cmake directory to CMake module path
list(APPEND CMAKE_MODULE_PATH ${firmware_test_builder_SOURCE_DIR}/cmake)

# Initialize the Firmware Test Builder and its dependencies
include(UnitTest)
unit_test_init_cpputest()

# Coverage
if (COVERAGE)
	message("Using coverage")
	include(Coverage)

	set(COVERAGE_FILE "coverage.info")
	set(TF_A_COVERAGE_FILE "trusted-firmware-a-coverage.info")
	set(TF_A_UT_COVERAGE_FILE "tf-a-unit-tests-coverage.info")
	set(TF_A_COVERAGE_REPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/trusted-firmware-a-coverage")
	set(TF_A_UT_COVERAGE_REPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/tf-a-unit-tests-coverage")

	# Collecting coverage
	coverage_generate(
		NAME "Unit test"
		SOURCE_DIR ${UNIT_TEST_PROJECT_PATH}
		BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
		CPICKER_MAPPING_PATH ${CPICKER_CACHE_PATH}
		OUTPUT_FILE ${COVERAGE_FILE}
	)

	# Filtering TF-A and unit test coverage
	coverage_filter(
		INPUT_FILE ${COVERAGE_FILE}
		OUTPUT_FILE ${TF_A_COVERAGE_FILE}
		INCLUDE_DIRECTORY ${UNIT_TEST_PROJECT_PATH}
	)
	coverage_filter(
		INPUT_FILE ${COVERAGE_FILE}
		OUTPUT_FILE ${TF_A_UT_COVERAGE_FILE}
		INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}
	)

	# Coverage reports
	coverage_generate_report(
		INPUT_FILE trusted-firmware-a-coverage.info
		OUTPUT_DIRECTORY ${TF_A_COVERAGE_REPORT_DIR}
	)
	coverage_generate_report(
		INPUT_FILE tf-a-unit-tests-coverage.info
		OUTPUT_DIRECTORY ${TF_A_UT_COVERAGE_REPORT_DIR}
	)
endif()

# Project level include directory
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

# Include paths to each test cmake file
include(tests/bl1/test_bl1_fwu.cmake)
include(tests/lib/libc/test_libc.cmake)
include(tests/lib/object_pool/test_object_pool.cmake)
include(tests/lib/fdt/test_fdt.cmake)
include(tests/lib/gpt/test_gpt.cmake)
