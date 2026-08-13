/*
 * Copyright (c) 2019-2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "CppUTest/TestHarness.h"

#define IMAGE_ID0	(0)
#define IMAGE_ID1	(1)

extern "C" {
extern unsigned int bl1_fwu_loaded_ids[FWU_MAX_SIMULTANEOUS_IMAGES];

extern int bl1_fwu_add_loaded_id(unsigned int image_id);
extern int bl1_fwu_remove_loaded_id(unsigned int image_id);
}

TEST_GROUP(bl1_fwu) {
	TEST_SETUP() {
		for (int i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {
			bl1_fwu_loaded_ids[i] = INVALID_IMAGE_ID;
		}
	}
};

TEST(bl1_fwu, add_one) {
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID0));
	UNSIGNED_LONGS_EQUAL(IMAGE_ID0, bl1_fwu_loaded_ids[0]);
}

TEST(bl1_fwu, add_two_different) {
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID0));
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID1));
	UNSIGNED_LONGS_EQUAL(IMAGE_ID0, bl1_fwu_loaded_ids[0]);
	UNSIGNED_LONGS_EQUAL(IMAGE_ID1, bl1_fwu_loaded_ids[1]);
}

TEST(bl1_fwu, add_two_identical) {
	for (int i = 0; i < 2; i++) {
		LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID0));
	}
	UNSIGNED_LONGS_EQUAL(IMAGE_ID0, bl1_fwu_loaded_ids[0]);
	UNSIGNED_LONGS_EQUAL(INVALID_IMAGE_ID, bl1_fwu_loaded_ids[1]);
}

TEST(bl1_fwu, add_until_overflow) {
	for (unsigned int i = 0; i < FWU_MAX_SIMULTANEOUS_IMAGES; i++) {
		LONGS_EQUAL(0, bl1_fwu_add_loaded_id(i));
		UNSIGNED_LONGS_EQUAL(i, bl1_fwu_loaded_ids[i]);
	}

	LONGS_EQUAL(1, bl1_fwu_add_loaded_id(FWU_MAX_SIMULTANEOUS_IMAGES));
}

TEST(bl1_fwu, remove_from_empty_array) {
	LONGS_EQUAL(1, bl1_fwu_remove_loaded_id(IMAGE_ID0));
}

TEST(bl1_fwu, add_one_and_remove_one) {
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID0));
	LONGS_EQUAL(0, bl1_fwu_remove_loaded_id(IMAGE_ID0));
	UNSIGNED_LONGS_EQUAL(INVALID_IMAGE_ID, bl1_fwu_loaded_ids[0]);
}

TEST(bl1_fwu, add_two_and_remove_first) {
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID0));
	LONGS_EQUAL(0, bl1_fwu_add_loaded_id(IMAGE_ID1));
	LONGS_EQUAL(0, bl1_fwu_remove_loaded_id(IMAGE_ID0));
	UNSIGNED_LONGS_EQUAL(INVALID_IMAGE_ID, bl1_fwu_loaded_ids[0]);
	UNSIGNED_LONGS_EQUAL(IMAGE_ID1, bl1_fwu_loaded_ids[1]);
}
