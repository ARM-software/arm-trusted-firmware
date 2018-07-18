/*
 * Copyright (c) 2003-2016  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 * Copyright (c) 2018 Facebook Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 *
 * Functions for determining which Cavium chip you are running
 * on.
 *
 * <hr>$Revision: 49448 $<hr>
 * @addtogroup chips
 * @{
 */


/* Flag bits in top byte. The top byte of MIDR_EL1 is defined
   as ox43, the Cavium implementer code. In this number, bits
   7,5,4 are defiend as zero. We use these bits to signal
   that revision numbers should be ignored. It isn't ideal
   that these are in the middle of an already defined field,
   but this keeps the model numbers as 32 bits */
#define __OM_IGNORE_REVISION        0x80000000
#define __OM_IGNORE_MINOR_REVISION  0x20000000
#define __OM_IGNORE_MODEL           0x10000000

#define CAVIUM_CN88XX_PASS1_0   0x430f0a10
#define CAVIUM_CN88XX_PASS1_1   0x430f0a11
#define CAVIUM_CN88XX_PASS2_0   0x431f0a10
#define CAVIUM_CN88XX_PASS2_1   0x431f0a11
#define CAVIUM_CN88XX_PASS2_2   0x431f0a12
#define CAVIUM_CN88XX           (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN88XX_PASS1_X   (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)
#define CAVIUM_CN88XX_PASS2_X   (CAVIUM_CN88XX_PASS2_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN83XX_PASS1_0   0x430f0a30
#define CAVIUM_CN83XX           (CAVIUM_CN83XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN83XX_PASS1_X   (CAVIUM_CN83XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN81XX_PASS1_0   0x430f0a20
#define CAVIUM_CN81XX           (CAVIUM_CN81XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN81XX_PASS1_X   (CAVIUM_CN81XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

#define CAVIUM_CN98XX_PASS1_0   0x430f0b10
#define CAVIUM_CN98XX           (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_REVISION)
#define CAVIUM_CN98XX_PASS1_X   (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_MINOR_REVISION)

/* These match entire families of chips */
#define CAVIUM_CN8XXX           (CAVIUM_CN88XX_PASS1_0 | __OM_IGNORE_MODEL)
#define CAVIUM_CN9XXX           (CAVIUM_CN98XX_PASS1_0 | __OM_IGNORE_MODEL)

static inline uint64_t cavium_get_model(void) __attribute__ ((pure, always_inline));
static inline uint64_t cavium_get_model(void)
{
#ifdef CAVM_BUILD_HOST
    extern uint32_t thunder_remote_get_model(void) __attribute__ ((pure));
    return thunder_remote_get_model();
#else
    uint64_t result;
    __asm ("mrs %[rd],MIDR_EL1" : [rd] "=r" (result));
    return result;
#endif
}

/**
 * Return non-zero if the chip matech the passed model.
 *
 * @param arg_model One of the CAVIUM_* constants for chip models and passes
 *
 * @return Non-zero if match
 */
static inline int CAVIUM_IS_MODEL(uint32_t arg_model) __attribute__ ((pure, always_inline));
static inline int CAVIUM_IS_MODEL(uint32_t arg_model)
{
    const uint32_t FAMILY = 0xff00;    /* Bits 15:8 */
    const uint32_t PARTNUM = 0xfff0;    /* Bits 15:4 */
    const uint32_t VARIANT = 0xf00000;  /* Bits 23:20 */
    const uint32_t REVISION = 0xf;      /* Bits 3:0 */

    uint32_t my_model = cavium_get_model();
    uint32_t mask;

    if (arg_model & __OM_IGNORE_MODEL)
        mask = FAMILY;
    else if (arg_model & __OM_IGNORE_REVISION)
        mask = PARTNUM;
    else if (arg_model & __OM_IGNORE_MINOR_REVISION)
        mask = PARTNUM | VARIANT;
    else
        mask = PARTNUM | VARIANT | REVISION;
    return ((arg_model & mask) == (my_model & mask));
}

/**
 * Return non-zero if the die is in an alternate package. The
 * normal is_model() checks will treat alternate package parts
 * as all the same, where this function can be used to detect
 * them.
 *
 * @param arg_model One of the CAVIUM_* constants for chip models and passes
 *
 * @return Non-zero if match
 */
extern int cavium_is_altpkg(uint32_t arg_model);

/**
 * Return the SKU string for a chip
 *
 * @param node   Node to get SKU for
 *
 * @return Chip's SKU
 */
extern const char* cavm_model_get_sku(int node);

/** @} */
