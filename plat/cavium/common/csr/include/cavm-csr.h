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
 * Functions and macros for accessing Cavium CSRs.
 *
 * <hr>$Revision: 49448 $<hr>
 *
 * @defgroup csr CSR support
 * @{
 */

/**
 * Possible CSR bus types
 */
typedef enum {
   CSR_TYPE_DAB,            /**< External debug 64bit CSR */
   CSR_TYPE_DAB32b,         /**< External debug 32bit CSR */
   CSR_TYPE_NCB,            /**< Fast 64bit CSR */
   CSR_TYPE_NCB32b,         /**< Fast 32bit CSR */
   CSR_TYPE_PCCBR,
   CSR_TYPE_PCCPF,
   CSR_TYPE_PCCVF,
   CSR_TYPE_PCICONFIGRC,    /**< PCIe config address (RC mode) */
   CSR_TYPE_PCICONFIGEP,    /**< PCIe config address (EP mode) */
   CSR_TYPE_PEXP,           /**< PCIe BAR 0 address only */
   CSR_TYPE_PEXP_NCB,       /**< NCB-direct and PCIe BAR0 address */
   CSR_TYPE_RSL,            /**< Slow 64bit CSR */
   CSR_TYPE_RSL32b,         /**< Slow 32bit CSR */
   CSR_TYPE_SYSREG,         /**< Core system register */
} cavm_csr_type_t;

#define CSR_DB_MAX_PARAM 4
typedef struct __attribute__ ((packed)) {
    uint32_t        name_index : 20;/**< Index into __cavm_csr_db_string where the name is */
    uint32_t        base_index : 13;/**< Index into __cavm_csr_db_number where the base address is */
    uint8_t         unused : 7;
    cavm_csr_type_t  type : 4;       /**< Enum type from above */
    uint8_t         width : 4;      /**< CSR width in bytes */
    uint16_t        field_index;    /**< Index into __cavm_csr_db_fieldList where the fields start */
    uint16_t        range[CSR_DB_MAX_PARAM]; /**< Index into __cavm_csr_db_range where the range is */
    uint16_t        param_inc[CSR_DB_MAX_PARAM]; /**< Index into __cavm_csr_db_number where the param multiplier is */
} __cavm_csr_db_type_t;

typedef struct __attribute__ ((packed)) {
    uint32_t        name_index : 20;/**< Index into __cavm_csr_db_string where the name is */
    uint32_t        start_bit : 6;  /**< LSB of the field */
    uint32_t        stop_bit : 6;   /**< MSB of the field */
} __cavm_csr_db_field_t;

typedef struct {
    uint32_t model;
    const int16_t *data;            /**< Array of integers indexing __cavm_csr_db_csr */
} __cavm_csr_db_map_t;

extern void __cavm_csr_fatal(const char *name, int num_args, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4) __attribute__ ((noreturn));
extern int cavm_csr_decode(const char *name, uint64_t value);
extern int cavm_csr_field(const char *csr_name, int field_start_bit, const char **field_name);
extern uint64_t cavm_csr_read_by_name(cavm_node_t node, const char *name);
extern int cavm_csr_write_by_name(cavm_node_t node, const char *name, uint64_t value);
extern int __cavm_csr_lookup_index(const char *name, int params[]);
extern int cavm_csr_get_name(const char *last_name, char *buffer);
struct cavm_readline_tab;
extern struct cavm_readline_tab *__cavm_csr_get_tab_complete(void) WEAK;
extern uint64_t cavm_sysreg_read(int node, int core, uint64_t regnum);
extern void cavm_sysreg_write(int node, int core, uint64_t regnum, uint64_t value);

#ifndef CAVM_BUILD_HOST

/**
 * Read a value from a CSR. Normally this function should not be
 * used directly. Instead use the macro CSR_READ that fills
 * in the parameters to this function for you.
 *
 * @param node    Node to use in a Numa setup. Can be an exact ID or a special value.
 * @param type    Bus type the CSR is on
 * @param busnum  Bus number the CSR is on
 * @param size    Width of the CSR in bytes
 * @param address The address of the CSR
 *
 * @return The value of the CSR
 */
static inline uint64_t cavm_csr_read(cavm_node_t node, cavm_csr_type_t type, int busnum, int size, uint64_t address) __attribute__ ((always_inline));
static inline uint64_t cavm_csr_read(cavm_node_t node, cavm_csr_type_t type, int busnum, int size, uint64_t address)
{
    switch (type)
    {
        case CSR_TYPE_DAB:
        case CSR_TYPE_DAB32b:
        case CSR_TYPE_NCB:
        case CSR_TYPE_NCB32b:
        case CSR_TYPE_PEXP_NCB:
        case CSR_TYPE_RSL:
        case CSR_TYPE_RSL32b:
            address |= (uint64_t)(node&3) << 44;
            /* Note: This code assume a 1:1 mapping of all of address space.
               It is designed to run with the MMU disabled */
            switch (size)
            {
                case 1:
                    return *(volatile uint8_t *)address;
                case 2:
                    return cavm_le16_to_cpu(*(volatile uint16_t *)address);
                case 4:
                    return cavm_le32_to_cpu(*(volatile uint32_t *)address);
                default:
                    return cavm_le64_to_cpu(*(volatile uint64_t *)address);
            }
        default:
            return 0;
    }
}


/**
 * Wrate a value to a CSR. Normally this function should not be
 * used directly. Instead use the macro CSR_WRITE that fills
 * in the parameters to this function for you.
 *
 * @param node    Node to use in a Numa setup. Can be an exact ID or a special value.
 * @param type    Bus type the CSR is on
 * @param busnum  Bus number the CSR is on
 * @param size    Width of the CSR in bytes
 * @param address The address of the CSR
 * @param value   Value to write to the CSR
 */
static inline void cavm_csr_write(cavm_node_t node, cavm_csr_type_t type, int busnum, int size, uint64_t address, uint64_t value) __attribute__ ((always_inline));
static inline void cavm_csr_write(cavm_node_t node, cavm_csr_type_t type, int busnum, int size, uint64_t address, uint64_t value)
{
    switch (type)
    {
        case CSR_TYPE_DAB:
        case CSR_TYPE_DAB32b:
        case CSR_TYPE_NCB:
        case CSR_TYPE_NCB32b:
        case CSR_TYPE_PEXP_NCB:
        case CSR_TYPE_RSL:
        case CSR_TYPE_RSL32b:
            address |= (uint64_t)(node&3) << 44;
            /* Note: This code assume a 1:1 mapping of all of address space.
               It is designed to run with the MMU disabled */
            switch (size)
            {
                case 1:
                    *(volatile uint8_t *)address = value;
                    break;
                case 2:
                    *(volatile uint16_t *)address = cavm_cpu_to_le16(value);
                    break;
                case 4:
                    *(volatile uint32_t *)address = cavm_cpu_to_le32(value);
                    break;
                default:
                    *(volatile uint64_t *)address = cavm_cpu_to_le64(value);
                    break;
            }
            break;

        default:
            break;
    }
}

#else
#define cavm_csr_read thunder_remote_read_csr
#define cavm_csr_write thunder_remote_write_csr
#endif

/**
 * This macro makes it easy to define a variable of the correct
 * type for a CSR.
 */
#define CSR_DEFINE(name, csr) typedef_##csr name

/**
 * This macro makes it easy to define a variable and initialize it
 * with a CSR.
 */
#define CSR_INIT(name, node, csr) typedef_##csr name = {.u = cavm_csr_read(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), csr)}

/**
 * Macro to read a CSR
 */
#define CSR_READ_PA(node, csr) cavm_csr_read(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), csr)
#define CSR_READ(base, csr) cavm_csr_read(0, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), base + offset_##csr)
//#define CSR_READ(base, csr) cavm_csr_read((((base) >> 44) & 0x3), bustype_##csr, busnum_##csr, sizeof(typedef_##csr), csr)

/**
 * Macro to write a CSR
 */
#define CSR_WRITE_PA(node, csr, value) cavm_csr_write(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), csr, value)
#define CSR_WRITE(base, csr, value) cavm_csr_write(0, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), base + offset_##csr, value)
//#define CSR_WRITE(base, csr, value) cavm_csr_write((((base) >> 44) & 0x3), bustype_##csr, busnum_##csr, sizeof(typedef_##csr), csr, value)

/**
 * Macro to make a read, modify, and write sequence easy. The "code_block"
 * should be replaced with a C code block or a comma separated list of
 * "name.s.field = value", without the quotes.
 */
#define CSR_MODIFY(name, node, csr, code_block) do { \
        uint64_t _tmp_address = csr; \
        typedef_##csr name = {.u = cavm_csr_read(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), _tmp_address)}; \
        code_block; \
        cavm_csr_write(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), _tmp_address, name.u); \
    } while (0)

/**
 * This macro spins on a field waiting for it to reach a value. It
 * is common in code to need to wait for a specific field in a CSR
 * to match a specific value. Conceptually this macro expands to:
 *
 * 1) read csr at "address" with a csr typedef of "type"
 * 2) Check if ("type".s."field" "op" "value")
 * 3) If #2 isn't true loop to #1 unless too much time has passed.
 */
#define CSR_WAIT_FOR_FIELD(node, csr, field, op, value, timeout_usec) \
    ({int result;                                                       \
    do {                                                                \
        uint64_t done = cavm_clock_get_count(CLOCK_TIME) + (uint64_t)timeout_usec * \
                        cavm_clock_get_rate(cavm_numa_local(), CLOCK_TIME) / 1000000;   \
        typedef_##csr c;                                                \
        uint64_t _tmp_address = csr;                                    \
        while (1)                                                       \
        {                                                               \
            c.u = cavm_csr_read(node, bustype_##csr, busnum_##csr, sizeof(typedef_##csr), _tmp_address); \
            if ((c.s.field) op (value)) {                               \
                result = 0;                                             \
                break;                                                  \
            } else if (cavm_clock_get_count(CLOCK_TIME) > done) {    \
                result = -1;                                            \
                break;                                                  \
            } else                                                      \
                cavm_thread_yield();                                     \
        }                                                               \
    } while (0);                                                        \
    result;})

/** @} */

#include "cavm-csr-atf.h"
