#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME bl1_fwu
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/test_bl1_fwu.cpp
		${CMAKE_CURRENT_LIST_DIR}/bl1_fwu_add_loaded_id.yml
	COMPILE_DEFINITIONS
		-DFWU_MAX_SIMULTANEOUS_IMAGES=10
		-DINVALID_IMAGE_ID=0xffffffff
)
