Platform Fault Detection Interface (PFDI)
*****************************************

This document describes the Trusted Firmware-A (TF-A) support for the Platform
Fault Detection Interface (PFDI).

PFDI provides a standard EL3 interface for running platform or PE fault-detection
tests, reporting their result to the non-secure world, and coordinating
out-of-reset (OoR) self-tests during CPU bring-up.

PFDI is intended for platforms that integrate a processor or platform
self-test library and need a standard firmware interface for diagnostics.
Non-secure software can use it to discover the available test library, request
online tests, and read the last recorded test result, while TF-A can use it to
run OoR tests before a PE is released for normal execution.

For the architectural definition of PFDI, refer to the `PFDI Supplement`_.
This document focuses on the TF-A implementation and on the platform code that
must be provided to make it usable.

SMC Interface
=============

The following SMC function IDs are implemented:

- ``PFDI_VERSION``
- ``PFDI_FEATURES``
- ``PFDI_PE_TEST_ID``
- ``PFDI_PE_TEST_PART_COUNT``
- ``PFDI_PE_TEST_RUN``
- ``PFDI_PE_TEST_RESULT``
- ``PFDI_FW_CHECK``
- ``PFDI_FORCE_ERROR``

Execution Model
===============

- PFDI operations are **PE-local**. ``PFDI_PE_TEST_RUN`` and
  ``PFDI_PE_TEST_RESULT`` operate on the calling PE.
- ``PFDI_FORCE_ERROR`` is also PE-local and applies to the next invocation of
  the targeted PFDI ABI from that same PE, even if that invocation would
  otherwise fail normal parameter validation.
- ``PFDI_PE_TEST_RUN`` is executed in one of two modes:

  - **OoR mode** (``PFDI_OOR_MODE``): used by TF-A during PE bring-up.
  - **Online mode** (``PFDI_ONL_MODE``): used by non-secure callers after boot.

- For OoR execution, TF-A stores the per-PE result and uses it later to decide
  whether a PE is allowed to continue boot.
- ``start`` and ``end`` may be passed as ``-1`` together to request the
  full test range. Any other mixed use of ``-1`` is rejected.
- Pending injected errors, stored results, and any other PFDI state are not
  persistent across reset or firmware update.

Required Platform Functions
===========================

The core PFDI service requires exactly one registered test library descriptor:

.. code-block:: c

   REGISTER_PFDI_FUNC(name, run, count, result);

All three callbacks must be implemented.

``run(start, end, mode, ft_id)``
--------------------------------

This callback executes the requested test range on the current PE.

- ``start`` and ``end`` are the test-case indices to execute.
- ``mode`` is either ``PFDI_OOR_MODE`` or ``PFDI_ONL_MODE``.
- ``ft_id`` must be updated with the failing test identifier when returning
  ``PFDI_RET_FAULT_FOUND``.
- Return ``PFDI_SMCC_RET_SUCCESS`` on success, or a ``pfdi_status_t`` error
  code on failure.

``count(tc_size)``
------------------

This callback returns the total number of available test cases.

- ``tc_size`` must be populated with the number of test parts.
- Return ``PFDI_RET_TEST_COUNT_ZERO`` if no tests are available.

``result(cpu_num, ft_id)``
--------------------------

This callback returns the stored result for the requested PE.

- ``cpu_num`` is the logical CPU number supplied by TF-A.
- ``ft_id`` should be updated with the failing test identifier when the stored
  result is ``PFDI_RET_FAULT_FOUND``.
- Return ``PFDI_RET_NOT_RUN`` until OoR testing has completed for that PE.

Optional Platform-Specific Functions
====================================

Platforms may also register an optional descriptor:

.. code-block:: c

   static const struct plat_pfdi_func_desc plat_pfdi_func_desc = {
     .name = "my-platform",
     .force_plat_err = force_plat_err,
     .check_plat_err = check_plat_err,
     .post_run = post_run,
   };

   pfdi_register_plat_func_desc(&plat_pfdi_func_desc);

All callbacks are optional and may be ``NULL``. The platform must register the
descriptor during BL31 platform setup, before ``pfdi_init()`` runs.
Registrations are merged by callback slot, which allows separate components to
install disjoint hooks such as ``check_plat_err()`` and ``post_run()``.

``force_plat_err(fid, error_id)``
---------------------------------

This hook runs when ``PFDI_FORCE_ERROR`` is used to inject a failure for a given
SMC function.

Use it when error injection must also modify platform state, for example:

- programming a hardware fault-status register,
- forcing an SBIST/BIST failure indication,
- priming a platform monitor so that the next request observes the injected
  error.

If this hook returns non-zero, the PFDI service rejects the injection request.

``check_plat_err(fid, error_id)``
---------------------------------

This hook runs when an injected error is consumed by a later PFDI operation.

Use it to validate a generic injected error against platform state.
Typical uses are:

- checking whether the expected hardware condition was actually latched before
  returning the error to the caller.

If no platform hook is registered, the PFDI service returns the injected
``error_id`` directly. When a hook is present, TF-A still returns the original
injected ABI error to the caller, so the hook must not depend on rewriting the
reported ``x0`` value.

``post_run(status, start, end, mode, ft_id)``
---------------------------------------------

This hook runs after every ``pfdi_pe_test_run()`` attempt, regardless of whether
the test run succeeded or failed.

Use it for follow-up actions such as:

- clearing temporary hardware state,
- recording telemetry,
- updating platform result storage,
- triggering additional housekeeping after an OoR or online run.

Platform Integration Steps
==========================

To enable PFDI on a platform:

1. Select the ``PFDI_SUPPORT`` option in the platform build.
2. Define ``PFDI_TEST_LIB_VERSION`` to the implemented test library version,
   either in ``platform_def.h`` or on the build command line.
3. Compile the PFDI service code and the platform test implementation into BL31.
4. Provide one ``REGISTER_PFDI_FUNC()`` instance with working ``run()``,
   ``count()`` and ``result()`` callbacks.
5. If OoR tests must run when a secondary PE is powered on through a
   platform-specific path, implement ``pfdi_enable()``.
6. Optionally provide ``struct plat_pfdi_func_desc`` when platform-specific
   error injection, validation, or post-run processing is needed.

Configuration
=============

``PFDI_SUPPORT``
----------------

Enables the EL3 PFDI service.

``PFDI_TEST_LIB_VERSION``
-------------------------

This value is optional. If it is not defined, TF-A uses zero and
``PFDI_PE_TEST_ID`` returns ``PFDI_RET_UNKNOWN``. Platforms can define it in
``platform_def.h`` or pass it on the build command line.

PFDI_TEST_LIB_VERSION encoding (uint64_t)
As per Platform Fault Detection Interface Specification v1.0BET0, the test
library version returned by ``PFDI_PE_TEST_ID`` in the following format:

::

   Bits [63:32] : Reserved, must be 0
   Bits [31:24] : Vendor ID
   Bits [23:20] : Reserved, must be 0
   Bits [19:16] : Implementation ID
   Bits [15:8]  : Major version
   Bits [7:0]   : Minor version

   # Vendor IDs
   _____________________________________________________________________________
   |     Vendor ID                      |     Vendor name                      |
   |____________________________________|______________________________________|
   |     0                              |     Arm Limited                      |
   |____________________________________|______________________________________|
   |     Others                         |     Reserved                         |
   |____________________________________|______________________________________|

   PFDI Test Library Implementation IDs
   _____________________________________________________________________________
   |     Implementation ID              |     Library name                     |
   |____________________________________|______________________________________|
   |     0                              |     Reserved                         |
   |____________________________________|______________________________________|
   |     1                              |     Arm STL                          |
   |____________________________________|______________________________________|
   |     Others                         |     Reserved                         |
   |____________________________________|______________________________________|

  Notes:
    - A value of 0 means no test engine metadata is available, and
     ``PFDI_PE_TEST_ID`` returns ``PFDI_RET_UNKNOWN``.
    - Currently only Vendor ID 0 is valid.
    - For Arm vendor, Implementation ID 1 identifies Arm STL.
    - On error, the encoded value must be zero.

Example::

   PFDI_TEST_LIB_VERSION := 0x0000000000010100

   Vendor ID         = 0x00 (Arm Limited)
   Implementation ID = 0x01 (Arm STL)
   Major version     = 0x01
   Minor version     = 0x00

Limitations
===========

- ``PFDI_FW_CHECK`` currently has no firmware-specific checks behind it and
  returns success unless an injected error is configured.
- ``PFDI_FORCE_ERROR`` is one-shot per PE and per ABI function. The next
  invocation of the targeted function consumes the pending injected error and
  returns it in ``x0`` with ``x1``-``x4`` cleared.
- The platform must provide result storage semantics that match the TF-A flow,
  especially for OoR execution on secondary PEs.
- The current implementation assumes that PFDI tests are safe to run on the
  target PE in the requested execution mode.

--------------

*Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.*

.. _PFDI Supplement: https://support.arm.com/documentation/110468/latest/
