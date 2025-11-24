Trusted Firmware-A
==================

Trusted Firmware-A (TF-A) is a reference implementation of secure world software
for `Arm A-Profile architectures`_ (Armv8-A and Armv7-A), including an Exception
Level 3 (EL3) `Secure Monitor`_. It provides a suitable starting point for
productization of secure world boot and runtime firmware, in either the AArch32
or AArch64 execution states.

TF-A implements Arm interface standards, including:

-  `Power State Coordination Interface (PSCI)`_
-  `Trusted Board Boot Requirements CLIENT (TBBR-CLIENT)`_
-  `SMC Calling Convention`_
-  `System Control and Management Interface (SCMI)`_
-  `Software Delegated Exception Interface (SDEI)`_

The code is designed to be portable and reusable across hardware platforms and
software models that are based on the Armv8-A and Armv7-A architectures.

In collaboration with interested parties, we will continue to enhance TF-A
with reference implementations of Arm standards to benefit developers working
with Armv7-A and Armv8-A TrustZone technology.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from TF-A.

More Info and Documentation
---------------------------

To find out more about Trusted Firmware-A, please `view the full documentation`_
that is available through `trustedfirmware.org`_.

--------------

*Copyright (c) 2013-2025, Arm Limited and Contributors. All rights reserved.*

.. _Armv7-A and Armv8-A: https://developer.arm.com/products/architecture/a-profile
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): PSCI_
.. _PSCI: https://developer.arm.com/documentation/den0022/latest
.. _Trusted Board Boot Requirements CLIENT (TBBR-CLIENT): https://developer.arm.com/docs/den0006/latest
.. _SMC Calling Convention: https://developer.arm.com/documentation/den0028/latest
.. _System Control and Management Interface (SCMI): SCMI_
.. _SCMI: https://developer.arm.com/documentation/den0056/latest
.. _Software Delegated Exception Interface (SDEI): SDEI_
.. _SDEI: https://developer.arm.com/documentation/den0054/latest
.. _Arm A-Profile architectures: https://developer.arm.com/architectures/cpu-architecture/a-profile
.. _view the full documentation: https://www.trustedfirmware.org/docs/tf-a
.. _trustedfirmware.org: http://www.trustedfirmware.org

