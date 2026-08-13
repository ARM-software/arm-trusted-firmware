#
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

include(UnitTest)

unit_test_add_suite(
	NAME fdt
	SOURCES
		${CMAKE_CURRENT_LIST_DIR}/test_fdt.cpp
		${PROJECT_SOURCE_DIR}/mocks/common/debug.cpp
		${PROJECT_SOURCE_DIR}/mocks/common/uuid.c
		${UNIT_TEST_PROJECT_PATH}/common/fdt_wrappers.c
		${UNIT_TEST_PROJECT_PATH}/lib/libc/strlcpy.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_addresses.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_empty_tree.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_overlay.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_ro.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_rw.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_strerror.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_sw.c
		${UNIT_TEST_PROJECT_PATH}/lib/libfdt/fdt_wip.c
	INCLUDE_DIRECTORIES
		${PROJECT_SOURCE_DIR}/mocks/include/
		${PROJECT_SOURCE_DIR}/mocks/include/lib/libc/
		${UNIT_TEST_PROJECT_PATH}/include/
		${UNIT_TEST_PROJECT_PATH}/include/lib/libfdt/
	COMPILE_DEFINITIONS
		# Several additional DTB files from the Linux kernel and the TFA source
		# are needed to run these tests.  They cannot be included here due to
		# license restrictions but can be built from the sources using the
		# script device_trees/build_dtb.sh.  The script takes the paths to these
		# two source repositories as arguments, builds the DTB files, and places
		# them in a folder here called dtb.
		TFA_FVP_DTB_PATH="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/fvp-base-gicv3-psci.dtb"
		LINUX64_JUNO_R1_DTB="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/juno-r1.dtb"
		LINUX64_SUN50I_PINE64_PLUS_DTB="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/sun50i-a64-pine64-plus.dtb"
		LINUX32_ATLAS7_EVB_DTB="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/atlas7-evb.dtb"
		LINUX32_BCM2837_RPI_3_B_DTB="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/bcm2837-rpi-3-b-plus.dtb"
		TEST_DTB="${CMAKE_CURRENT_LIST_DIR}/device_trees/dtb/test.dtb"
)
