EL3 Runtime Service Writer's Guide
=====================================================

Introduction
------------

This document describes how to add a runtime service to the EL3 Runtime
Firmware component of Trusted Firmware-A (TF-A), BL31.

Software executing in the normal world and in the trusted world at exception
levels lower than EL3 will request runtime services using the Secure Monitor
Call (SMC) instruction. These requests will follow the convention described in
the SMC Calling Convention PDD (`SMCCC`_). The `SMCCC`_ assigns function
identifiers to each SMC request and describes how arguments are passed and
results are returned.

SMC Functions are grouped together based on the implementor of the service, for
example a subset of the Function IDs are designated as "OEM Calls" (see `SMCCC`_
for full details). The EL3 runtime services framework in BL31 enables the
independent implementation of services for each group, which are then compiled
into the BL31 image. This simplifies the integration of common software from
Arm to support `PSCI`_, Secure Monitor for a Trusted OS and SoC specific
software. The common runtime services framework ensures that SMC Functions are
dispatched to their respective service implementation - the
:ref:`Firmware Design` document provides details of how this is achieved.

The interface and operation of the runtime services depends heavily on the
concepts and definitions described in the `SMCCC`_, in particular SMC Function
IDs, Owning Entity Numbers (OEN), Fast and Standard calls, and the SMC32 and
SMC64 calling conventions. Please refer to that document for a full explanation
of these terms.

Owning Entities, Call Types and Function IDs
--------------------------------------------

The SMC Function Identifier includes a OEN field. These values and their
meaning are described in `SMCCC`_ and summarized in table 1 below. Some entities
are allocated a range of of OENs. The OEN must be interpreted in conjunction
with the SMC call type, which is either *Fast* or *Yielding*. Fast calls are
uninterruptible whereas Yielding calls can be pre-empted. The majority of
Owning Entities only have allocated ranges for Fast calls: Yielding calls are
reserved exclusively for Trusted OS providers or for interoperability with
legacy 32-bit software that predates the `SMCCC`_.

::

    Type       OEN     Service
    Fast        0      Arm Architecture calls
    Fast        1      CPU Service calls
    Fast        2      SiP Service calls
    Fast        3      OEM Service calls
    Fast        4      Standard Service calls
    Fast       5-47    Reserved for future use
    Fast      48-49    Trusted Application calls
    Fast      50-63    Trusted OS calls

    Yielding   0- 1    Reserved for existing Armv7-A calls
    Yielding   2-63    Trusted OS Standard Calls

*Table 1: Service types and their corresponding Owning Entity Numbers*

Each individual entity can allocate the valid identifiers within the entity
range as they need - it is not necessary to coordinate with other entities of
the same type. For example, two SoC providers can use the same Function ID
within the SiP Service calls OEN range to mean different things - as these
calls should be specific to the SoC. The Standard Runtime Calls OEN is used for
services defined by Arm standards, such as `PSCI`_.

The SMC Function ID also indicates whether the call has followed the SMC32
calling convention, where all parameters are 32-bit, or the SMC64 calling
convention, where the parameters are 64-bit. The framework identifies and
rejects invalid calls that use the SMC64 calling convention but that originate
from an AArch32 caller.

The EL3 runtime services framework uses the call type and OEN to identify a
specific handler for each SMC call, but it is expected that an individual
handler will be responsible for all SMC Functions within a given service type.

Getting started
---------------

TF-A has a ``services`` directory in the source tree under which
each owning entity can place the implementation of its runtime service. The
`PSCI`_ implementation is located here in the ``lib/psci`` directory.

Runtime service sources will need to include the ``runtime_svc.h`` header file.

Registering a runtime service
-----------------------------

A runtime service is registered using the ``DECLARE_RT_SVC()`` macro, specifying
the name of the service, the range of OENs covered, the type of service and
initialization and call handler functions.

.. code:: c

    #define DECLARE_RT_SVC(_name, _start, _end, _type, _setup, _smch)

-  ``_name`` is used to identify the data structure declared by this macro, and
   is also used for diagnostic purposes

-  ``_start`` and ``_end`` values must be based on the ``OEN_*`` values defined in
   ``smccc.h``

-  ``_type`` must be one of ``SMC_TYPE_FAST`` or ``SMC_TYPE_YIELD``

-  ``_setup`` is the initialization function with the ``rt_svc_init`` signature:

   .. code:: c

       typedef int32_t (*rt_svc_init)(void);

-  ``_smch`` is the SMC handler function with the ``rt_svc_handle`` signature:

   .. code:: c

       typedef uintptr_t (*rt_svc_handle_t)(uint32_t smc_fid,
                                         u_register_t x1, u_register_t x2,
                                         u_register_t x3, u_register_t x4,
                                         void *cookie,
                                         void *handle,
                                         u_register_t flags);

Details of the requirements and behavior of the two callbacks is provided in
the following sections.

During initialization the services framework validates each declared service
to ensure that the following conditions are met:

#. The ``_start`` OEN is not greater than the ``_end`` OEN
#. The ``_end`` OEN does not exceed the maximum OEN value (63)
#. The ``_type`` is one of ``SMC_TYPE_FAST`` or ``SMC_TYPE_YIELD``
#. ``_setup`` and ``_smch`` routines have been specified

``std_svc_setup.c`` provides an example of registering a runtime service:

.. code:: c

    /* Register Standard Service Calls as runtime service */
    DECLARE_RT_SVC(
            std_svc,
            OEN_STD_START,
            OEN_STD_END,
            SMC_TYPE_FAST,
            std_svc_setup,
            std_svc_smc_handler
    );

Initializing a runtime service
------------------------------

Runtime services are initialized once, during cold boot, by the primary CPU
after platform and architectural initialization is complete. The framework
performs basic validation of the declared service before calling
the service initialization function (``_setup`` in the declaration). This
function must carry out any essential EL3 initialization prior to receiving a
SMC Function call via the handler function.

On success, the initialization function must return ``0``. Any other return value
will cause the framework to issue a diagnostic:

::

    Error initializing runtime service <name of the service>

and then ignore the service - the system will continue to boot but SMC calls
will not be passed to the service handler and instead return the *Unknown SMC
Function ID* result ``0xFFFFFFFF``.

If the system must not be allowed to proceed without the service, the
initialization function must itself cause the firmware boot to be halted.

If the service uses per-CPU data this must either be initialized for all CPUs
during this call, or be done lazily when a CPU first issues an SMC call to that
service.

Handling runtime service requests
---------------------------------

SMC calls for a service are forwarded by the framework to the service's SMC
handler function (``_smch`` in the service declaration). This function must have
the following signature:

.. code:: c

    typedef uintptr_t (*rt_svc_handle_t)(uint32_t smc_fid,
                                       u_register_t x1, u_register_t x2,
                                       u_register_t x3, u_register_t x4,
                                       void *cookie,
                                       void *handle,
                                       u_register_t flags);

The handler is responsible for:

#. Determining that ``smc_fid`` is a valid and supported SMC Function ID,
   otherwise completing the request with the *Unknown SMC Function ID*:

   .. code:: c

       SMC_RET1(handle, SMC_UNK);

#. Determining if the requested function is valid for the calling security
   state. SMC Calls can be made from both the normal and trusted worlds and
   the framework will forward all calls to the service handler.

   The ``flags`` parameter to this function indicates the caller security state
   in bit[0], where a value of ``1`` indicates a non-secure caller. The
   ``is_caller_secure(flags)`` and ``is_caller_non_secure(flags)`` can be used to
   test this condition.

   If invalid, the request should be completed with:

   .. code:: c

       SMC_RET1(handle, SMC_UNK);

#. Truncating parameters for calls made using the SMC32 calling convention.
   Such calls can be determined by checking the CC field in bit[30] of the
   ``smc_fid`` parameter, for example by using:

   ::

       if (GET_SMC_CC(smc_fid) == SMC_32) ...

   For such calls, the upper bits of the parameters x1-x4 and the saved
   parameters X5-X7 are UNDEFINED and must be explicitly ignored by the
   handler. This can be done by truncating the values to a suitable 32-bit
   integer type before use, for example by ensuring that functions defined
   to handle individual SMC Functions use appropriate 32-bit parameters.

#. Providing the service requested by the SMC Function, utilizing the
   immediate parameters x1-x4 and/or the additional saved parameters X5-X7.
   The latter can be retrieved using the ``SMC_GET_GP(handle, ref)`` function,
   supplying the appropriate ``CTX_GPREG_Xn`` reference, e.g.

   .. code:: c

       uint64_t x6 = SMC_GET_GP(handle, CTX_GPREG_X6);

#. Implementing the standard SMC32 Functions that provide information about
   the implementation of the service. These are the Call Count, Implementor
   UID and Revision Details for each service documented in section 6 of the
   `SMCCC`_.

   TF-A expects owning entities to follow this recommendation.

#. Returning the result to the caller. The `SMCCC`_ allows for up to 256 bits
   of return value in SMC64 using X0-X3 and 128 bits in SMC32 using W0-W3. The
   framework provides a family of macros to set the multi-register return
   value and complete the handler:

   .. code:: c

       SMC_RET1(handle, x0);
       SMC_RET2(handle, x0, x1);
       SMC_RET3(handle, x0, x1, x2);
       SMC_RET4(handle, x0, x1, x2, x3);

The ``cookie`` parameter to the handler is reserved for future use and can be
ignored. The ``handle`` is returned by the SMC handler - completion of the
handler function must always be via one of the ``SMC_RETn()`` macros.

.. note::
   The PSCI and Test Secure-EL1 Payload Dispatcher services do not follow
   all of the above requirements yet.

Services that contain multiple sub-services
-------------------------------------------

It is possible that a single owning entity implements multiple sub-services. For
example, the Standard calls service handles ``0x84000000``-``0x8400FFFF`` and
``0xC4000000``-``0xC400FFFF`` functions. Within that range, the `PSCI`_ service
handles the ``0x84000000``-``0x8400001F`` and ``0xC4000000``-``0xC400001F`` functions.
In that respect, `PSCI`_ is a 'sub-service' of the Standard calls service. In
future, there could be additional such sub-services in the Standard calls
service which perform independent functions.

In this situation it may be valuable to introduce a second level framework to
enable independent implementation of sub-services. Such a framework might look
very similar to the current runtime services framework, but using a different
part of the SMC Function ID to identify the sub-service. TF-A does not provide
such a framework at present.

Secure-EL1 Payload Dispatcher service (SPD)
-------------------------------------------

Services that handle SMC Functions targeting a Trusted OS, Trusted Application,
or other Secure-EL1 Payload are special. These services need to manage the
Secure-EL1 context, provide the *Secure Monitor* functionality of switching
between the normal and secure worlds, deliver SMC Calls through to Secure-EL1
and generally manage the Secure-EL1 Payload through CPU power-state transitions.

TODO: Provide details of the additional work required to implement a SPD and
the BL31 support for these services. Or a reference to the document that will
provide this information....

--------------

*Copyright (c) 2014-2019, Arm Limited and Contributors. All rights reserved.*

.. _SMCCC: http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022c/DEN0022C_Power_State_Coordination_Interface.pdf
