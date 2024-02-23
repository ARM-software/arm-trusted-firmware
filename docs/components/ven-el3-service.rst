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

Source definitions for vendor-specific EL3 Monitor Service Calls are located in
the ``ven_el3_svc.h`` header file.

--------------

*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
