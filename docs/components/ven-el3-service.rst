Vendor Specific EL3 Monitor Service Calls
=========================================

This document enumerates and describes the Vendor Specific EL3 Monitor Service
Calls.

These are Service Calls defined by the vendor of the EL3 Monitor.
They are accessed via ``SMC`` ("SMC calls") instruction executed from Exception
Levels below EL3. SMC calls for Vendor Specific EL3 Monitor Services:

-  Follow `SMC Calling Convention`_;
-  Use SMC function IDs that fall in the vendor-specific EL3 range, which are

+---------------------------+--------------------------------------------------+
| SMC Function Identifier   | Service Type                                     |
+===========================+==================================================+
| 0x87000000 - 0x8700FFFF   | SMC32: Vendor Specific EL3 Monitor Service Calls |
+---------------------------+--------------------------------------------------+
| 0xC7000000 - 0xC700FFFF   | SMC64: Vendor Specific EL3 Monitor Service Calls |
+---------------------------+--------------------------------------------------+

Vendor-specific EL3 monitor services are as follows:

+-----------------------------------+-----------------------+---------------------------------------------+
| SMC Function Identifier           | Service Type          | FID's Usage                                 |
+===================================+=======================+=============================================+
| 0x87000010 - 0x8700001F (SMC32)   | DebugFS Interface     | | 0 - 11 are in use.                        |
+-----------------------------------+                       | | 12 - 15 are reserved for future expansion.|
| 0xC7000010 - 0xC700001F (SMC64)   |                       |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+
| 0x87000020 - 0x8700002F (SMC32)   | Performance           | | 0,1 is in use.                            |
+-----------------------------------+ Measurement Framework | | 2 - 15 are reserved for future expansion. |
| 0xC7000020 - 0xC700002F (SMC64)   | (PMF)                 |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+
| 0x87000030 - 0x8700FFFF (SMC32)   | Reserved              | | reserved for future expansion             |
+-----------------------------------+                       |                                             |
| 0xC7000030 - 0xC700FFFF (SMC64)   |                       |                                             |
+-----------------------------------+-----------------------+---------------------------------------------+

Source definitions for vendor-specific EL3 Monitor Service Calls used by TF-A are located in
the ``ven_el3_svc.h`` header file.

+----------------------------+----------------------------+--------------------------------+
| VEN_EL3_SVC_VERSION_MAJOR  | VEN_EL3_SVC_VERSION_MINOR  | Changes                        |
+============================+============================+================================+
|                          1 |                          0 | Added Debugfs and PMF services.|
+----------------------------+----------------------------+--------------------------------+

*Table 1: Showing different versions of Vendor-specific service and changes done with each version*

Each sub service will have its own version, one FID allocated for sub service version.

Some ground rules when one should update top level version.
 - VEN_EL3_SVC_VERSION_MAJOR is incremented when any of the sub service version discovery
   FID changes or the FID that was allocated for discovery changes. So any breaking subfeature
   discovery changes will lead to major version update.
 - VEN_EL3_SVC_VERSION_MINOR is incremented when we add a new FID or a new sub service.
   For example adding an new monitor service at 0x30, Debugfs starts at 0x10 and PMF
   starts at 0x20 next one will start at 0x30, this will need a update to minor version.

Performance Measurement Framework (PMF)
---------------------------------------

The :ref:`Performance Measurement Framework <firmware_design_pmf>`
allows callers to retrieve timestamps captured at various paths in TF-A
execution.

DebugFS interface
-----------------

The optional DebugFS interface is accessed through Vendor specific EL3 service. Refer
to :ref:`DebugFS interface` documentation for further details and usage.

--------------

*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
