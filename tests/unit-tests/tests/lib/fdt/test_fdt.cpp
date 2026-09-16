/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"

extern "C" {
#include <common/fdt_wrappers.h>
#include <libfdt.h>
#include <stdio.h>
#include <string.h>
}

#define FDT_SUCCESS (0)

#define FILE_NOT_FOUND_HELP \
"          The DTB file used in this test must be generated manually before\n" \
"          running the test suite.  There is a script build_dtb.sh found in\n" \
"          tests/lib/fdt/device_trees which can be used to do this."

static int check_translation(const void *fdt, int node, uintptr_t *address)
{
	const char *prop;
	int len, ret;
	uintptr_t addr;

	prop = (const char *)fdt_getprop(fdt, node, "compatible", &len);
	if (!prop) {
		return 3;
	}

	ret = fdt_get_reg_props_by_index(fdt, node, 0, &addr, NULL);
	if (ret)
	{
		return 4;
	}
	addr = fdtw_translate_address(fdt, node, addr);

	*address = addr;
	return 0;
}

static uint64_t read_check_address (unsigned char *prop)
{
	uint64_t res = 0;
	/* Address field is stored as 4 big endian uint32_t values. */
	for (int i = 0; i < 8; i++)
	{
		res = res | ((uint64_t)prop[i] << (56-(i*8)));
	}
	return res;
}

static int run_test(const char *name, const char *dtb_path, const char *dev,
			uintptr_t expected)
{
	FILE *dtb;
	size_t size;
	int offs;
	int ret;
	int len;
	uintptr_t address;
	unsigned char *prop;
	void *fdt_buffer;

	printf("FDT Translation Test Case: %s\n", name);
	printf("  DTB file: %s\n", dtb_path);
	if (dev != NULL)
	{
		printf("  Device: %s\n", dev);
	}
	else
	{
		printf("  Device: stdout\n");
	}

	/* Read DTB file into buffer */
	dtb = fopen(dtb_path, "rb");
	if (dtb == NULL)
	{
		printf("  [ERROR] Could not open device tree binary \"%s\"\n",
			dtb_path);
		printf("%s", FILE_NOT_FOUND_HELP);
		return -1;
	}

	/* Get size and allocate buffer. */
	fseek(dtb, 0, SEEK_END);
	size = ftell(dtb);
	fdt_buffer = malloc(size);
	if (fdt_buffer == NULL)
	{
		printf("  [ERROR] Could not allocate file buffer.\n");
		return -1;
	}
	rewind(dtb);

	/* Read file into buffer and close it. */
	if(fread(fdt_buffer, 1, size, dtb) != size)
	{
		printf("  [ERROR] Could not read input file.\n");
		free(fdt_buffer);
		return -1;
	}
	fclose(dtb);

	ret = fdt_check_header(fdt_buffer);
	if (ret) {
		printf("  [ERROR] Invalid DTB header.\n");
		free(fdt_buffer);
		return ret;
	}

	if (dev != NULL)
	{
		/* If a specific device is given, look for it. */
		offs = fdt_node_offset_by_compatible(fdt_buffer, -1, dev);
	}
	else
	{
		/* If no device specified, look for stdout node. */
		offs = fdt_get_stdout_node_offset(fdt_buffer);
	}
	if (offs < 0) {
		printf("  [ERROR] Could not find requested node.\n");
		ret = offs;
		free(fdt_buffer);
		return ret;
	}

	/* Get address of device. */
	ret = check_translation(fdt_buffer, offs, &address);
	if (ret != 0)
	{
		free(fdt_buffer);
		return ret;
	}

	/* Verify address translation. */
	if (expected == 0)
	{
		/* Retrieve expected result from DTB "address" property. */
		prop = (unsigned char *)fdt_getprop(fdt_buffer, offs, "address",
							&len);
		if ((prop == NULL) || (len != 16))
		{
			printf("  [ERROR] Expected len %d, got %d\n", 16, len);
			free(fdt_buffer);
			return -1;
		}
		expected = read_check_address(prop);
	}

	printf("  Expected: 0x%0lX\n", expected);
	printf("  Found:    0x%0lX\n", address);
	if (address != expected)
	{
		printf("  TEST FAILED\n");
		free(fdt_buffer);
		return -1;
	}

	printf("  TEST SUCCEEDED\n");
	free(fdt_buffer);
	return 0;
}

TEST_GROUP(fdt)
{
	/* Nothing to declare here. */
};

TEST(fdt, test_fvp)
{
	int result = run_test("test_fvp", TFA_FVP_DTB_PATH, "arm,sp804",
				0x1C110000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_juno_r1_a)
{
	/* Search for stdout with NULL compatible string. */
	int result = run_test("test_juno_r1_a", LINUX64_JUNO_R1_DTB, NULL,
				0x7FF80000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_juno_r1_b)
{
	int result = run_test("test_juno_r1_b", LINUX64_JUNO_R1_DTB,
				"arm,sp804", 0x1C110000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_sun50i)
{
	int result = run_test("test_sun50i_pine64_plus",
				LINUX64_SUN50I_PINE64_PLUS_DTB,
				"allwinner,sun50i-a64-de2-rotate",
				0x01020000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_atlas)
{
	int result = run_test("test_atlas_7_evb", LINUX32_ATLAS7_EVB_DTB,
				"sirf,prima2-pwm", 0x18630000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_bcm2837)
{
	int result = run_test("test_bcm2837_rpi_3", LINUX32_BCM2837_RPI_3_B_DTB,
				"brcm,bcm2835-txp", 0x3F004000);
	CHECK_EQUAL(result, 0);
}

TEST(fdt, test_generic)
{
	int failcount = 0;
	int i = 0;
	char device[15] = {0};

	/* Count from 1 to 11 */
	for (int i = 1; i < 12; i++)
	{
		sprintf(device, "acme,device%d", i);
		if (run_test("test_generic", TEST_DTB, device, 0) != 0)
		{
			failcount++;
		}
	}

	CHECK_EQUAL(failcount, 0);
}
