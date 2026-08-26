#!/bin/bash
# verify_build.sh - Verify a80x0_nbx TF-A build
#
# Usage: ./verify_build.sh [build_dir]
#   build_dir: Path to build output (default: build/a80x0_nbx/debug)
#              Also accepts BUILD_BASE; a80x0_nbx/{debug,release} is probed.
#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2025-2026 Free Mobile / Freebox

set -e

BUILD_DIR="${1:-build/a80x0_nbx/debug}"

# When called with BUILD_BASE (e.g. by CI), artifacts are in the
# a80x0_nbx/{debug,release} subdirectory.  Probe for it so that both
#   verify_build.sh build/a80x0_nbx/debug   (local, default)
#   verify_build.sh $BUILD_BASE              (CI: post_tf_build hook)
# work transparently.
if [ ! -f "${BUILD_DIR}/ble.bin" ]; then
    for _subdir in "${BUILD_DIR}"/a80x0_nbx/debug \
                   "${BUILD_DIR}"/a80x0_nbx/release; do
        if [ -d "${_subdir}" ]; then
            BUILD_DIR="${_subdir}"
            break
        fi
    done
    unset _subdir
fi

BLE_ELF="${BUILD_DIR}/ble/ble.elf"
BL31_ELF="${BUILD_DIR}/bl31/bl31.elf"
CROSS_PREFIX="${CROSS_COMPILE:-aarch64-linux-gnu-}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

pass() { printf '%b[OK]%b\n' "${GREEN}" "${NC}"; }
fail() { printf '%b[FAILED]%b\n' "${RED}" "${NC}"; }
warn() { printf '%b[WARN]%b\n' "${YELLOW}" "${NC}"; }
missing() { printf '%b[MISSING]%b\n' "${RED}" "${NC}"; }

echo "=============================================="
echo "  a80x0_nbx TF-A Build Verification"
echo "=============================================="
echo ""
echo "Build directory: ${BUILD_DIR}"
echo "Cross prefix:    ${CROSS_PREFIX}"
echo ""

ERRORS=0

# 1. Check build artifacts exist
echo "1. Checking build artifacts..."
echo "   ----------------------------------------"
ARTIFACTS="ble.bin bl1.bin bl2.bin bl31.bin fip.bin flash-image.bin"
for f in ${ARTIFACTS}; do
    if [ -f "${BUILD_DIR}/${f}" ]; then
        SIZE=$(stat -c%s "${BUILD_DIR}/${f}")
        printf "   %-20s %8d bytes  " "${f}" "${SIZE}"
        pass
    else
        printf "   %-20s %8s        " "${f}" "-"
        missing
        ERRORS=$((ERRORS + 1))
    fi
done
echo ""

# 2. Check ramoopsies linker wrapping
echo "2. Checking ramoopsies --wrap mechanism..."
echo "   ----------------------------------------"
if [ -f "${BLE_ELF}" ]; then
    # Check symbols exist
    printf "   %-35s " "__wrap_mv_ddr_mem_scrubbing"
    if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "__wrap_mv_ddr_mem_scrubbing"; then
        pass
    else
        fail
        ERRORS=$((ERRORS + 1))
    fi

    printf "   %-35s " "mv_ddr_mem_scrubbing (original)"
    if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "T mv_ddr_mem_scrubbing$"; then
        pass
    else
        fail
        ERRORS=$((ERRORS + 1))
    fi

    printf "   %-35s " "ramoopsies_backup buffer"
    if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "ramoopsies_backup"; then
        pass
    else
        fail
        ERRORS=$((ERRORS + 1))
    fi

    # Verify wrapper calls real function
    printf "   %-35s " "Wrapper calls original function"
    CALL_CHAIN=$("${CROSS_PREFIX}objdump" -d "${BLE_ELF}" 2>/dev/null | \
        grep -A50 "<__wrap_mv_ddr_mem_scrubbing>:" | \
        grep "bl.*<mv_ddr_mem_scrubbing>" || true)
    if [ -n "${CALL_CHAIN}" ]; then
        pass
    else
        fail
        ERRORS=$((ERRORS + 1))
    fi

    # Check SError vector handling
    printf "   %-35s " "SError exception vectors"
    if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "vectors"; then
        pass
    else
        warn
    fi
else
    echo "   BLE ELF not found: ${BLE_ELF}"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# 3. Check DDR driver integration
echo "3. Checking DDR driver integration..."
echo "   ----------------------------------------"
if [ -f "${BLE_ELF}" ]; then
    DDR_SYMS="ddr3_init mv_ddr_topology_map_get mv_ddr_spd_supported_cls_calc mv_ddr_is_ecc_ena"
    for sym in ${DDR_SYMS}; do
        printf "   %-35s " "${sym}"
        if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "T ${sym}"; then
            pass
        else
            fail
            ERRORS=$((ERRORS + 1))
        fi
    done
fi
echo ""

# 4. Check BL31 runtime services
echo "4. Checking BL31 runtime services..."
echo "   ----------------------------------------"
if [ -f "${BL31_ELF}" ]; then
    BL31_SYMS="mvebu_get_dram_size plat_marvell_get_pm_cfg a8k_pwr_domain_off a8k_system_reset"
    for sym in ${BL31_SYMS}; do
        printf "   %-35s " "${sym}"
        if "${CROSS_PREFIX}nm" "${BL31_ELF}" 2>/dev/null | grep -q " ${sym}"; then
            pass
        else
            echo -n "(inlined?) "
            warn
        fi
    done
else
    echo "   BL31 ELF not found: ${BL31_ELF}"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# 5. Check OP-TEE integration (optional)
echo "5. Checking OP-TEE integration..."
echo "   ----------------------------------------"
FIP_BIN="${BUILD_DIR}/fip.bin"
FIPTOOL="${BUILD_DIR}/tools/fiptool/fiptool"

# Check if opteed is compiled into BL31
OPTEE_ENABLED=false
if [ -f "${BL31_ELF}" ]; then
    if "${CROSS_PREFIX}nm" "${BL31_ELF}" 2>/dev/null | grep -q "opteed_setup"; then
        OPTEE_ENABLED=true
    fi
fi

if [ "${OPTEE_ENABLED}" = true ]; then
    printf "   %-35s " "OP-TEE dispatcher (opteed)"
    pass

    # Check opteed symbols in BL31
    OPTEE_SYMS="opteed_setup opteed_init optee_vector_table bl32_image_ep_info"
    for sym in ${OPTEE_SYMS}; do
        printf "   %-35s " "${sym}"
        if "${CROSS_PREFIX}nm" "${BL31_ELF}" 2>/dev/null | grep -q "${sym}"; then
            pass
        else
            fail
            ERRORS=$((ERRORS + 1))
        fi
    done

    # Check BL32 (header), BL32_EXTRA1 (pager), BL32_EXTRA2 (pageable) in FIP
    if [ -f "${FIP_BIN}" ] && [ -x "${FIPTOOL}" ]; then
        # Check BL32 (v2 header)
        printf "   %-35s " "BL32 (header) in FIP"
        BL32_INFO=$(${FIPTOOL} info "${FIP_BIN}" 2>/dev/null | \
            grep -E "Secure Payload BL32 \(Trusted OS\):" | \
            head -1 || true)
        if [ -n "${BL32_INFO}" ]; then
            BL32_SIZE=$(echo "${BL32_INFO}" | sed -n 's/.*size=\(0x[0-9a-fA-F]*\).*/\1/p')
            echo -n "(${BL32_SIZE}) "
            pass
        else
            fail
            echo "   ERROR: BL32 header not found in FIP!"
            ERRORS=$((ERRORS + 1))
        fi

        # Check BL32_EXTRA1 (pager)
        printf "   %-35s " "BL32_EXTRA1 (pager) in FIP"
        EXTRA1_INFO=$(${FIPTOOL} info "${FIP_BIN}" 2>/dev/null \
            | grep -E "Trusted OS Extra1" || true)
        if [ -n "${EXTRA1_INFO}" ]; then
            EXTRA1_SIZE=$(echo "${EXTRA1_INFO}" | sed -n 's/.*size=\(0x[0-9a-fA-F]*\).*/\1/p')
            echo -n "(${EXTRA1_SIZE}) "
            pass
        else
            fail
            echo "   ERROR: BL32_EXTRA1 (pager) not found in FIP - OP-TEE will fail!"
            ERRORS=$((ERRORS + 1))
        fi

        # Check BL32_EXTRA2 (pageable) - may be empty/missing for non-paged builds
        printf "   %-35s " "BL32_EXTRA2 (pageable) in FIP"
        EXTRA2_INFO=$(${FIPTOOL} info "${FIP_BIN}" 2>/dev/null \
            | grep -E "Trusted OS Extra2" || true)
        if [ -n "${EXTRA2_INFO}" ]; then
            EXTRA2_SIZE=$(echo "${EXTRA2_INFO}" | sed -n 's/.*size=\(0x[0-9a-fA-F]*\).*/\1/p')
            echo -n "(${EXTRA2_SIZE}) "
            pass
        else
            echo -n "(not present - OK for non-paged) "
            pass
        fi

        # Extract and verify OP-TEE header magic and version
        printf "   %-35s " "OP-TEE header magic"
        TMPBL32=$(mktemp)
        if ${FIPTOOL} unpack --tos-fw "${TMPBL32}" --force "${FIP_BIN}" 2>/dev/null; then
            MAGIC=$(hexdump -n 4 -e '4/1 "%c"' "${TMPBL32}" 2>/dev/null) || MAGIC=""
            if [ "${MAGIC}" = "OPTE" ]; then
                echo -n "(\"OPTE\") "
                pass

                # Check header version (byte 4, should be 2 for v2 format)
                printf "   %-35s " "OP-TEE header version"
                VERSION=$(hexdump -s 4 -n 1 -e '1/1 "%d"' "${TMPBL32}" 2>/dev/null) || VERSION="0"
                if [ "${VERSION}" = "2" ]; then
                    echo -n "(v${VERSION}) "
                    pass
                else
                    echo -n "(v${VERSION}) "
                    fail
                    echo "   ERROR: Expected version 2, got ${VERSION}!"
                    echo "   Use tee-header_v2.bin as BL32, not tee.bin"
                    ERRORS=$((ERRORS + 1))
                fi

                # Check load address (v2 header: load_addr at offset 16)
                printf "   %-35s " "OP-TEE load address"
                LOAD_ADDR=$(hexdump -s 16 -n 4 \
                    -e '1/4 "0x%08x"' "${TMPBL32}" \
                    2>/dev/null) || LOAD_ADDR=""
                if [ "${LOAD_ADDR}" = "0x04400000" ]; then
                    echo -n "(${LOAD_ADDR}) "
                    pass
                else
                    echo -n "(${LOAD_ADDR}) "
                    warn
                    echo "   WARNING: Expected 0x04400000"
                fi
            else
                echo -n "(\"${MAGIC}\") "
                fail
                echo "   ERROR: Invalid OP-TEE header - expected \"OPTE\" magic!"
                ERRORS=$((ERRORS + 1))
            fi
        else
            echo -n "(extract failed) "
            warn
        fi
        rm -f "${TMPBL32}"
    else
        printf "   %-35s " "BL32 in FIP"
        echo -n "(fiptool not available) "
        warn
    fi
else
    printf "   %-35s " "OP-TEE dispatcher (opteed)"
    echo -n "(not enabled) "
    warn
    echo "   INFO: Build without OP-TEE support - this is OK if not using --optee-*"
fi
echo ""

# 6. Check platform configuration
echo "6. Checking platform configuration..."
echo "   ----------------------------------------"
if [ -f "${BLE_ELF}" ]; then
    PLAT_SYMS=""
    PLAT_SYMS="${PLAT_SYMS} marvell_get_io_win_memory_map"
    PLAT_SYMS="${PLAT_SYMS} marvell_get_ccu_memory_map"
    PLAT_SYMS="${PLAT_SYMS} plat_marvell_dram_update_topology"
    for sym in ${PLAT_SYMS}; do
        printf "   %-35s " "${sym}"
        if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q " ${sym}"; then
            pass
        else
            echo -n "(inlined?) "
            warn
        fi
    done
fi
echo ""

# 7. Check I2C/SPD support
echo "7. Checking I2C/SPD support..."
echo "   ----------------------------------------"
if [ -f "${BLE_ELF}" ]; then
    I2C_SYMS="i2c_init i2c_read"
    for sym in ${I2C_SYMS}; do
        printf "   %-35s " "${sym}"
        if "${CROSS_PREFIX}nm" "${BLE_ELF}" 2>/dev/null | grep -q "T ${sym}"; then
            pass
        else
            warn
        fi
    done
fi
echo ""

# 8. Flash image sanity check
echo "8. Checking flash image format..."
echo "   ----------------------------------------"
FLASH_IMAGE="${BUILD_DIR}/flash-image.bin"
if [ -f "${FLASH_IMAGE}" ]; then
    # Check magic number (Marvell boot image starts with specific header)
    MAGIC=$(hexdump -n 4 -e '1/4 "%08x"' "${FLASH_IMAGE}" 2>/dev/null) || \
    MAGIC=$(od -A n -t x4 -N 4 "${FLASH_IMAGE}" 2>/dev/null | tr -d ' \n') || \
    MAGIC=""
    printf "   %-35s " "Flash image header"
    if [ -n "${MAGIC}" ]; then
        echo -n "(0x${MAGIC}) "
        pass
    else
        echo -n "(no hex tool) "
        warn
    fi

    # Check file size is reasonable
    SIZE=$(stat -c%s "${FLASH_IMAGE}")
    printf "   %-35s " "Flash image size"
    if [ "${SIZE}" -gt 100000 ] && [ "${SIZE}" -lt 2000000 ]; then
        echo -n "(${SIZE} bytes) "
        pass
    else
        echo -n "(${SIZE} bytes) "
        warn
    fi
else
    echo "   Flash image not found: ${FLASH_IMAGE}"
    ERRORS=$((ERRORS + 1))
fi
echo ""

# Summary
echo "=============================================="
if [ ${ERRORS} -eq 0 ]; then
    printf '  Result: %bALL CHECKS PASSED%b\n' "${GREEN}" "${NC}"
else
    printf '  Result: %b%d CHECK(S) FAILED%b\n' "${RED}" "${ERRORS}" "${NC}"
fi
echo "=============================================="
echo ""

exit ${ERRORS}
