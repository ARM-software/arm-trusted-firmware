FIRME
=====

This page summarizes the current status of the FIRME implementation in TF-A.

Background
----------

FIRME defines standard EL3 firmware interfaces used by software above EL3 to
manage capabilities associated with Arm Confidential Compute Architecture
(Arm CCA) and, in particular, the Realm Management Extension (FEAT_RME).
These interfaces cover the EL3 dependencies needed for Realm-related flows such
as resource assignment and isolation, Realm construction, and attestation.

The specification groups these interfaces into FIRME services. The service
areas defined by the specification are:

- granule protection table management
- IDE key management for PCIe and CXL root ports
- memory encryption context management
- attestation token management
- integrated device management

In addition to those service-specific interfaces, FIRME defines a base service
that provides common concepts plus version and feature discovery for the other
services.

FIRME sits on the EL3 boundary and is intended to be callable from the
Non-secure, Realm, and Secure security states. A FIRME implementation is also
permitted on systems that do not implement FEAT_RME, for example when parts of
the interface are still useful independently of Realm support.

TF-A currently implements the Arm FIRME specification version ``1.0 alp2`` as
defined by `DEN0149`_.

FIRME is exposed as a 64-bit fast SMC standard service. The currently allocated
TF-A function ID range is ``0xC4000400`` to ``0xC4000412``.

Build and integration
---------------------

FIRME support in TF-A is experimental:

- ``FIRME_SUPPORT`` enables support in builds.
- When build with ``FIRME_SUPPORT``, RME supported system will make use of the
  FIRME granule management services.
- When FIRME support is enabled in a RMM enabled build, TRP uses
  ``FIRME_GM_GPI_SET`` for granule transitions instead of the legacy
  delegate/undelegate interface.

Current status
--------------

- Only a single FIRME version is supported per service.
- The FIRME base service implements version and feature discovery through
  ``FIRME_SERVICE_VERSION`` and ``FIRME_SERVICE_FEATURES``.
- ``FIRME_GM_GPI_SET`` is currently the only implemented granule management
  operation.
- The IDE key management, MECID management, attestation, and integrated device
  management services are not implemented yet.
- ``FIRME_GM_GPI_OP_CONTINUE_FID`` is mandatory in the ``1.0 alp2`` specification,
  but it is not implemented yet.

As a result, the current implementation tracks FIRME ``1.0 alp2`` for
versioning purposes, but it does not yet provide all mandatory functionality
defined by that specification revision.

Implementation details
----------------------

- TF-A currently advertises FIRME support for Secure, Non-secure, and Realm
  callers.
- The base service is the entry point used to discover per-service versions and
  feature registers.
- The granule management service publishes two feature registers:

  - register 0 advertises ``FIRME_GM_GPI_SET`` support only
  - register 1 is populated at runtime from ``GPCCR_EL3`` and reports platform
    granule protection geometry such as page size, ``L0GPTSZ``, and ``PPS``

- ``FIRME_GM_GPI_SET`` is implemented on top of TF-A's generic GPT transition
  helper and returns the number of successfully transitioned granules.

--------------

*Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.*

.. _DEN0149: https://developer.arm.com/documentation/den0149/1-0alp2/?lang=en
