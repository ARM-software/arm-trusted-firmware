Change Log for Upcoming Release
===============================

This document contains a summary of the new features, changes, fixes and known
issues to be included in the upcoming release of Trusted Firmware-A. The contents
of this file will be moved to the collective change-log.rst file at the time of
release code freeze.


Upcoming Release Version 2.3
----------------------------

**Trusted Firmware-A Contributors,
Please log all relevant new features, changes, fixes, and known issues for the
upcoming release.  For the CPU support, drivers, and tools sections please preface
the log description with the relevant key word, example: "<CPU>: <CPU Support
addition>".  Use the RST format convention already used in the Change Log.**

New Features
^^^^^^^^^^^^

- Arm Architecture
   - Add support for Armv8.4-SecEL2 extension through the SPCI defined SPMD/SPMC
     components.
   - Build option to support EL2 context save and restore in the secure world
     (CTX_INCLUDE_EL2_REGS).
   - Example: "Add support for Branch Target Identification (BTI)"

- BL-specific
   - Enhanced BL2 bootloader flow to load secure partitions based on firmware
     configuration data (fconf).

- Build System
   - Add support for documentation build as a target in Makefile
   - Add ``COT`` build option to select the chain of trust to use when the
     Trusted Boot feature is enabled (default: ``tbbr``).
   - Added creation and injection of secure partition packages into the FIP.
   - Build option to support SPMC component loading and run at S-EL1
     or S-EL2 (SPMD_SPM_AT_SEL2).

- CPU Support
   - Example: "cortex-a55: Workaround for erratum 1221012"
   - Use Speculation Barrier instruction for v8.5+ cores

- Drivers
   - Example: "console: Allow the console to register multiple times"

- Libraries
   - Example: "Introduce BTI support in Library at ROM (romlib)"
   - Add Firmware Configuration Framework (fconf).
   - Add DebugFS functionality

- New Platforms Support
   - Example: "qemu/qemu_sbsa: New platform support added for QEMU SBSA platform"

- Platforms
   - Example: "arm/common: Introduce wrapper functions to setup secure watchdog"
   - plat/arm: Add support for the new `dualroot` chain of trust.
   - plat/arm/fvp: Add support for fconf in BL31 and SP_MIN. Populate power
     domain desciptor dynamically by leveraging fconf APIs.

- PSCI
   - Example: "Adding new optional PSCI hook ``pwr_domain_on_finish_late``"

- Security
   - Example: "UBSAN support and handlers"
   - Add support for optional firmware encryption feature (experimental).
   - Introduce a new `dualroot` chain of trust.
   - aarch32: stop speculative execution past exception returns.

- SPCI
   - Introduced the SPM Dispatcher (SPMD) component as a new standard service.

- Tools
   - Example: "fiptool: Add support to build fiptool on Windows."


Changed
^^^^^^^

- Arm Architecture
   - Example: "Refactor ARMv8.3 Pointer Authentication support code"

- BL-Specific
   - Example: "BL2: Invalidate dcache build option for BL2 entry at EL3"

- Boot Flow
   - Example: "Add helper to parse BL31 parameters (both versions)"

- Drivers
   - Example: "gicv3: Prevent pending G1S interrupt from becoming G0 interrupt"

- Platforms
   - Example: "arm/common: Shorten the Firmware Update (FWU) process"

- PSCI
   - Example: "PSCI: Lookup list of parent nodes to lock only once"

- Secure Partition Manager (SPM)
   - Example: "Move shim layer to TTBR1_EL1"

- Security
   - Example: "Refactor SPSR initialisation code"

- Tools
   - sptool updated to accomodate building secure partition packages.
   - Example: "cert_create: Remove RSA PKCS#1 v1.5 support"


Resolved Issues
^^^^^^^^^^^^^^^

- Arm Architecture
   - Example: "Fix restoration of PAuth context"

- BL-Specific
   - Example: "Fix BL31 crash reporting on AArch64 only platforms"

- Build System
   - Example: "Remove several warnings reported with W=2 and W=1"

- Code Quality
   - Example: "Unify type of "cpu_idx" across PSCI module"

- CPU Support
   - Example: "cortex-a12: Fix MIDR mask"

- Drivers
   - Example: "scmi: Fix wrong payload length"

- Library Code
   - Example: "libc: Fix memchr implementation"

- Platforms
   - Example: "rpi: rpi3: Fix compilation error when stack protector is enabled"

- Security
   - Example: "AArch32: Disable Secure Cycle Counter"

Deprecations
^^^^^^^^^^^^

- Common Code
   - Example: "Remove MULTI_CONSOLE_API flag and references to it"

- Drivers
   - Example: "console: Remove deprecated finish_console_register"

- Secure Partition Manager (SPM):
   - Example: "Prototype SPCI-based SPM (services/std_svc/spm) will be replaced
     with alternative methods of secure partitioning support."

Known Issues
^^^^^^^^^^^^

- Build System
   - dtb: DTB creation not supported when building on a Windows host.

     This step in the build process is skipped when running on a Windows host. A
     known issue from the 1.6 release.

- Platforms
   - arm/juno: System suspend from Linux does not function as documented in the
     user guide

     Following the instructions provided in the user guide document does not
     result in the platform entering system suspend state as expected. A message
     relating to the hdlcd driver failing to suspend will be emitted on the
     Linux terminal.

   - mediatek/mt6795: This platform does not build in this release
