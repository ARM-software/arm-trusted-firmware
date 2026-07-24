CPU Monitor Service Calls
==========================

This document enumerates and describes the CPU Monitor Service Calls.

These are Service Calls defined by the implementation of the CPU Monitor.
They are accessed via ``SMC`` ("SMC calls") instruction executed from Exception
Levels below EL3. SMC calls for CPU Monitor Services:

-  Follow `SMC Calling Convention`_;
-  Use SMC function IDs that fall in the CPU range, which are

+---------------------------+---------------------------------------+
| SMC Function Identifier   | Service Type                          |
+===========================+=======================================+
| 0x81000000 - 0x8100FFFF   | SMC32: CPU Monitor Service Calls      |
+---------------------------+---------------------------------------+
| 0xC1000000 - 0xC100FFFF   | SMC64: CPU Monitor Service Calls      |
+---------------------------+---------------------------------------+

CPU monitor services are as follows:

+-----------------------------------+-----------------------+-----------------------------------+
| SMC Function Identifier           | Service Type          | FID's Usage                       |
+===================================+=======================+===================================+
| 0x81000010 - 0x8100001F (SMC32)   | CVE-2026-0995         | 0 is in use.                      |
+-----------------------------------+ mitigation for        | 1 - 15 are Reserved.              |
| 0xC1000010 - 0xC100001F (SMC64)   | C1-Pro CPU            |                                   |
+-----------------------------------+-----------------------+-----------------------------------+
| 0x81000020 - 0x8100FFFF (SMC32)   | Reserved              | Reserved                          |
+-----------------------------------+                       |                                   |
| 0xC1000020 - 0xC100FFFF (SMC64)   |                       |                                   |
+-----------------------------------+-----------------------+-----------------------------------+

Source definitions for CPU Monitor Service Calls used by TF-A are located in the
``cpu_svc.h`` header file.

+------------------------+------------------------+----------------------------------+
| CPU_SVC_VERSION_MAJOR  | CPU_SVC_VERSION_MINOR  | Changes                          |
+========================+========================+==================================+
|                      1 |                      0 | Added for mitigating for         |
|                        |                        | CVE-2026-0995 erratum            |
|                        |                        | for C1-Pro CPU .                 |
+------------------------+------------------------+----------------------------------+


*Table 1: Showing different versions of CPU service and changes done with each version*

Each sub service will have its own version, one FID allocated for sub service version.

Some ground rules when one should update top level version.
 - CPU_SVC_VERSION_MAJOR is incremented when any of the sub service version discovery
   FID changes or the FID that was allocated for discovery changes. So any breaking subfeature
   discovery changes will lead to major version update.
 - CPU_SVC_VERSION_MINOR is incremented when we add a new FID or a new sub service.
   For example adding an new monitor service at 0x20.

--------------

*Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
