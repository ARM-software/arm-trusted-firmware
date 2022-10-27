Trusted Firmware-A Documentation
================================

.. toctree::
   :maxdepth: 1
   :numbered:

   Home<self>
   about/index
   getting_started/index
   process/index
   components/index
   design/index
   plat/index
   perf/index
   security_advisories/index
   design_documents/index
   threat_model/index
   change-log
   glossary
   license

Trusted Firmware-A (TF-A) provides a reference implementation of secure world
software for `Armv7-A and Armv8-A`_, including a `Secure Monitor`_ executing
at Exception Level 3 (EL3). It implements various Arm interface standards,
such as:

-  The `Power State Coordination Interface (PSCI)`_
-  `Trusted Board Boot Requirements CLIENT (TBBR-CLIENT)`_
-  `SMC Calling Convention`_
-  `System Control and Management Interface (SCMI)`_
-  `Software Delegated Exception Interface (SDEI)`_
-  `PSA FW update specification`_

Where possible, the code is designed for reuse or porting to other Armv7-A and
Armv8-A model and hardware platforms.

This release provides a suitable starting point for productization of secure
world boot and runtime firmware, in either the AArch32 or AArch64 execution
states.

Users are encouraged to do their own security validation, including penetration
testing, on any secure world code derived from TF-A.

In collaboration with interested parties, we will continue to enhance |TF-A|
with reference implementations of Arm standards to benefit developers working
with Armv7-A and Armv8-A TrustZone technology.

Getting Started
---------------

The |TF-A| documentation contains guidance for obtaining and building the
software for existing, supported platforms, as well as supporting information
for porting the software to a new platform.

The **About** chapter gives a high-level overview of |TF-A| features as well as
some information on the project and how it is organized.

Refer to the documents in the **Getting Started** chapter for information about
the prerequisites and requirements for building |TF-A|.

The **Processes & Policies** chapter explains the project's release schedule
and process, how security disclosures are handled, and the guidelines for
contributing to the project (including the coding style).

The **Components** chapter holds documents that explain specific components
that make up the |TF-A| software, the :ref:`Exception Handling Framework`, for
example.

In the **System Design** chapter you will find documents that explain the
design of portions of the software that involve more than one component, such
as the :ref:`Trusted Board Boot` process.

**Platform Ports** provides a list of the supported hardware and software-model
platforms that are supported upstream in |TF-A|. Most of these platforms also
have additional documentation that has been provided by the maintainers of the
platform.

The results of any performance evaluations are added to the
**Performance & Testing** chapter.

**Security Advisories** holds a list of documents relating to |CVE| entries that
have previously been raised against the software.

--------------

*Copyright (c) 2013-2021, Arm Limited and Contributors. All rights reserved.*

.. _Armv7-A and Armv8-A: https://developer.arm.com/products/architecture/a-profile
.. _Secure Monitor: http://www.arm.com/products/processors/technologies/trustzone/tee-smc.php
.. _Power State Coordination Interface (PSCI): http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _Trusted Board Boot Requirements CLIENT (TBBR-CLIENT): https://developer.arm.com/docs/den0006/latest/trusted-board-boot-requirements-client-tbbr-client-armv8-a
.. _System Control and Management Interface (SCMI): http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/DEN0056A_System_Control_and_Management_Interface.pdf
.. _Software Delegated Exception Interface (SDEI): http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _SMC Calling Convention: https://developer.arm.com/docs/den0028/latest
.. _PSA FW update specification: https://developer.arm.com/documentation/den0118/a/
