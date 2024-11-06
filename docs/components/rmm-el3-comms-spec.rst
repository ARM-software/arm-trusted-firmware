RMM-EL3 Communication interface
*******************************

This document defines the communication interface between RMM and EL3.
There are two parts in this interface: the boot interface and the runtime
interface.

The Boot Interface defines the ABI between EL3 and RMM when the CPU enters
R-EL2 for the first time after boot. The cold boot interface defines the ABI
for the cold boot path and the warm boot interface defines the same for the
warm path.

The RMM-EL3 runtime interface defines the ABI for EL3 services which can be
invoked by RMM as well as the register save-restore convention when handling an
SMC call from NS.

The below sections discuss these interfaces more in detail.

.. _rmm_el3_ifc_versioning:

RMM-EL3 Interface versioning
____________________________

The RMM Boot and Runtime Interface uses a version number to check
compatibility with the register arguments passed as part of Boot Interface and
RMM-EL3 runtime interface.

The Boot Manifest, discussed later in section :ref:`rmm_el3_boot_manifest`,
uses a separate version number but with the same scheme.

The version number is a 32-bit type with the following fields:

.. csv-table::
   :header: "Bits", "Value"

   [0:15],``VERSION_MINOR``
   [16:30],``VERSION_MAJOR``
   [31],RES0

The version numbers are sequentially increased and the rules for updating them
are explained below:

  - ``VERSION_MAJOR``: This value is increased when changes break
    compatibility with previous versions. If the changes
    on the ABI are compatible with the previous one, ``VERSION_MAJOR``
    remains unchanged.

  - ``VERSION_MINOR``: This value is increased on any change that is backwards
    compatible with the previous version. When ``VERSION_MAJOR`` is increased,
    ``VERSION_MINOR`` must be set to 0.

  - ``RES0``: Bit 31 of the version number is reserved 0 as to maintain
    consistency with the versioning schemes used in other parts of RMM.

This document specifies the 0.4 version of Boot Interface ABI and RMM-EL3
services specification and the 0.3 version of the Boot Manifest.

.. _rmm_el3_boot_interface:

RMM Boot Interface
__________________

This section deals with the Boot Interface part of the specification.

One of the goals of the Boot Interface is to allow EL3 firmware to pass
down into RMM certain platform specific information dynamically. This allows
RMM to be less platform dependent and be more generic across platform
variations. It also allows RMM to be decoupled from the other boot loader
images in the boot sequence and remain agnostic of any particular format used
for configuration files.

The Boot Interface ABI defines a set of register conventions and
also a memory based manifest file to pass information from EL3 to RMM. The
Boot Manifest and the associated platform data in it can be dynamically created
by EL3 and there is no restriction on how the data can be obtained (e.g by DTB,
hoblist or other).

The register convention and the manifest are versioned separately to manage
future enhancements and compatibility.

RMM completes the boot by issuing the ``RMM_BOOT_COMPLETE`` SMC (0xC40001CF)
back to EL3. After the RMM has finished the boot process, it can only be
entered from EL3 as part of RMI handling.

If RMM returns an error during boot (in any CPU), then RMM must not be entered
from any CPU.

.. _rmm_cold_boot_interface:

Cold Boot Interface
~~~~~~~~~~~~~~~~~~~

During cold boot RMM expects the following register values:

.. csv-table::
   :header: "Register", "Value"
   :widths: 1, 5

   x0,Linear index of this PE. This index starts from 0 and must be less than the maximum number of CPUs to be supported at runtime (see x2).
   x1,Version for this Boot Interface as defined in :ref:`rmm_el3_ifc_versioning`.
   x2,Maximum number of CPUs to be supported at runtime. RMM should ensure that it can support this maximum number.
   x3,Base address for the shared buffer used for communication between EL3 firmware and RMM. This buffer must be of 4KB size (1 page). The Boot Manifest must be present at the base of this shared buffer during cold boot.

During cold boot, EL3 firmware needs to allocate a 4KB page that will be
passed to RMM in x3. This memory will be used as shared buffer for communication
between EL3 and RMM. It must be assigned to Realm world and must be mapped with
Normal memory attributes (IWB-OWB-ISH) at EL3. At boot, this memory will be
used to populate the Boot Manifest. Since the Boot Manifest can be accessed by
RMM prior to enabling its MMU, EL3 must ensure that proper cache maintenance
operations are performed after the Boot Manifest is populated.

EL3 should also ensure that this shared buffer is always available for use by RMM
during the lifetime of the system and that it can be used for runtime
communication between RMM and EL3. For example, when RMM invokes attestation
service commands in EL3, this buffer can be used to exchange data between RMM
and EL3. It is also allowed for RMM to invoke runtime services provided by EL3
utilizing this buffer during the boot phase, prior to return back to EL3 via
RMM_BOOT_COMPLETE SMC.

RMM should map this memory page into its Stage 1 page-tables using Normal
memory attributes.

During runtime, it is the RMM which initiates any communication with EL3. If that
communication requires the use of the shared area, it is expected that RMM needs
to do the necessary concurrency protection to prevent the use of the same buffer
by other PEs.

The following sequence diagram shows how a generic EL3 Firmware would boot RMM.

.. image:: ../resources/diagrams/rmm_cold_boot_generic.png

Warm Boot Interface
~~~~~~~~~~~~~~~~~~~

At warm boot, RMM is already initialized and only some per-CPU initialization
is still pending. The only argument that is required by RMM at this stage is
the CPU Id, which will be passed through register x0 whilst x1 to x3 are RES0.
This is summarized in the following table:

.. csv-table::
   :header: "Register", "Value"
   :widths: 1, 5

   x0,Linear index of this PE. This index starts from 0 and must be less than the maximum number of CPUs to be supported at runtime (see x2).
   x1 - x3,RES0

Boot error handling and return values
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After boot up and initialization, RMM returns control back to EL3 through a
``RMM_BOOT_COMPLETE`` SMC call. The only argument of this SMC call will
be returned in x1 and it will encode a signed integer with the error reason
as per the following table:

.. csv-table::
   :header: "Error code", "Description", "ID"
   :widths: 2 4 1

   ``E_RMM_BOOT_SUCCESS``,Boot successful,0
   ``E_RMM_BOOT_ERR_UNKNOWN``,Unknown error,-1
   ``E_RMM_BOOT_VERSION_NOT_VALID``,Boot Interface version reported by EL3 is not supported by RMM,-2
   ``E_RMM_BOOT_CPUS_OUT_OF_RANGE``,Number of CPUs reported by EL3 larger than maximum supported by RMM,-3
   ``E_RMM_BOOT_CPU_ID_OUT_OF_RANGE``,Current CPU Id is higher or equal than the number of CPUs supported by RMM,-4
   ``E_RMM_BOOT_INVALID_SHARED_BUFFER``,Invalid pointer to shared memory area,-5
   ``E_RMM_BOOT_MANIFEST_VERSION_NOT_SUPPORTED``,Version reported by the Boot Manifest not supported by RMM,-6
   ``E_RMM_BOOT_MANIFEST_DATA_ERROR``,Error parsing core Boot Manifest,-7

For any error detected in RMM during cold or warm boot, RMM will return back to
EL3 using ``RMM_BOOT_COMPLETE`` SMC with an appropriate error code. It is
expected that EL3 will take necessary action to disable Realm world for further
entry from NS Host on receiving an error. This will be done across all the PEs
in the system so as to present a symmetric view to the NS Host. Any further
warm boot by any PE should not enter RMM using the warm boot interface.

.. _rmm_el3_boot_manifest:

Boot Manifest
~~~~~~~~~~~~~

During cold boot, EL3 Firmware passes a memory Boot Manifest to RMM containing
platform information.

This Boot Manifest is versioned independently of the Boot Interface, to help
evolve the former independent of the latter.
The current version for the Boot Manifest is ``v0.3`` and the rules explained
in :ref:`rmm_el3_ifc_versioning` apply on this version as well.

The Boot Manifest v0.3 has the following fields:

   - version : Version of the Manifest (v0.3)
   - plat_data : Pointer to the platform specific data and not specified by this
     document. These data are optional and can be NULL.
   - plat_dram : Structure encoding the NS DRAM information on the platform. This
     field is optional and platform can choose to zero out this structure if
     RMM does not need EL3 to send this information during the boot.
   - plat_console : Structure encoding the list of consoles for RMM use on the
     platform. This field is optional and platform can choose to not populate
     the console list if this is not needed by the RMM for this platform.

For the current version of the Boot Manifest, the core manifest contains a pointer
to the platform data. EL3 must ensure that the whole Boot Manifest, including
the platform data, if available, fits inside the RMM EL3 shared buffer.

For the data structure specification of Boot Manifest, refer to
:ref:`rmm_el3_manifest_struct`

.. _runtime_services_and_interface:

RMM-EL3 Runtime Interface
__________________________

This section defines the RMM-EL3 runtime interface which specifies the ABI for
EL3 services expected by RMM at runtime as well as the register save and
restore convention between EL3 and RMM as part of RMI call handling. It is
important to note that RMM is allowed to invoke EL3-RMM runtime interface
services during the boot phase as well. The EL3 runtime service handling must
not result in a world switch to another world unless specified. Both the RMM
and EL3 are allowed to make suitable optimizations based on this assumption.

If the interface requires the use of memory, then the memory references should
be within the shared buffer communicated as part of the boot interface. See
:ref:`rmm_cold_boot_interface` for properties of this shared buffer which both
EL3 and RMM must adhere to.

RMM-EL3 runtime service return codes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The return codes from EL3 to RMM is a 32 bit signed integer which encapsulates
error condition as described in the following table:

.. csv-table::
   :header: "Error code", "Description", "ID"
   :widths: 2 4 1

   ``E_RMM_OK``,No errors detected,0
   ``E_RMM_UNK``,Unknown/Generic error,-1
   ``E_RMM_BAD_ADDR``,The value of an address used as argument was invalid,-2
   ``E_RMM_BAD_PAS``,Incorrect PAS,-3
   ``E_RMM_NOMEM``,Not enough memory to perform an operation,-4
   ``E_RMM_INVAL``,The value of an argument was invalid,-5
   ``E_RMM_AGAIN``,The resource is busy. Try again.,-6

If multiple failure conditions are detected in an RMM to EL3 command, then EL3
is allowed to return an error code corresponding to any of the failure
conditions.

RMM-EL3 runtime services
~~~~~~~~~~~~~~~~~~~~~~~~

The following table summarizes the RMM runtime services that need to be
implemented by EL3 Firmware.

.. csv-table::
   :header: "FID", "Command"
   :widths: 2 5

   0xC400018F,``RMM_RMI_REQ_COMPLETE``
   0xC40001B0,``RMM_GTSI_DELEGATE``
   0xC40001B1,``RMM_GTSI_UNDELEGATE``
   0xC40001B2,``RMM_ATTEST_GET_REALM_KEY``
   0xC40001B3,``RMM_ATTEST_GET_PLAT_TOKEN``
   0xC40001B4,``RMM_EL3_FEATURES``
   0xC40001B5,``RMM_EL3_TOKEN_SIGN``

RMM_RMI_REQ_COMPLETE command
============================

Notifies the completion of an RMI call to the Non-Secure world.

This call is the only function currently in RMM-EL3 runtime interface which
results in a world switch to NS. This call is the reply to the original RMI
call and it is forwarded by EL3 to the NS world.

FID
---

``0xC400018F``

Input values
------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   err_code,x1,[63:0],RmiCommandReturnCode,Error code returned by the RMI service invoked by NS World. See Realm Management Monitor specification for more info

Output values
-------------

This call does not return.

Failure conditions
------------------

Since this call does not return to RMM, there is no failure condition which
can be notified back to RMM.

RMM_GTSI_DELEGATE command
=========================

Delegate a memory granule by changing its PAS from Non-Secure to Realm.

FID
---

``0xC40001B0``

Input values
------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   base_pa,x1,[63:0],Address,PA of the start of the granule to be delegated

Output values
-------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 2 4

   Result,x0,[63:0],Error Code,Command return status

Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table::
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_BAD_ADDR``,``PA`` does not correspond to a valid granule address
   ``E_RMM_BAD_PAS``,The granule pointed by ``PA`` does not belong to Non-Secure PAS
   ``E_RMM_OK``,No errors detected

RMM_GTSI_UNDELEGATE command
===========================

Undelegate a memory granule by changing its PAS from Realm to Non-Secure.

FID
---

``0xC40001B1``

Input values
------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   base_pa,x1,[63:0],Address,PA of the start of the granule to be undelegated

Output values
-------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 2 4

   Result,x0,[63:0],Error Code,Command return status

Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table::
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_BAD_ADDR``,``PA`` does not correspond to a valid granule address
   ``E_RMM_BAD_PAS``,The granule pointed by ``PA`` does not belong to Realm PAS
   ``E_RMM_OK``,No errors detected

RMM_ATTEST_GET_REALM_KEY command
================================

Retrieve the Realm Attestation Token Signing key from EL3.

FID
---

``0xC40001B2``

Input values
------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   buf_pa,x1,[63:0],Address,PA where the Realm Attestation Key must be stored by EL3. The PA must belong to the shared buffer
   buf_size,x2,[63:0],Size,Size in bytes of the Realm Attestation Key buffer. ``bufPa + bufSize`` must lie within the shared buffer
   ecc_curve,x3,[63:0],Enum,Type of the elliptic curve to which the requested attestation key belongs to. See :ref:`ecc_curves`

Output values
-------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   Result,x0,[63:0],Error Code,Command return status
   keySize,x1,[63:0],Size,Size of the Realm Attestation Key

Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table::
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_BAD_ADDR``,``PA`` is outside the shared buffer
   ``E_RMM_INVAL``,``PA + BSize`` is outside the shared buffer
   ``E_RMM_INVAL``,``Curve`` is not one of the listed in :ref:`ecc_curves`
   ``E_RMM_UNK``,An unknown error occurred whilst processing the command
   ``E_RMM_OK``,No errors detected

.. _ecc_curves:

Supported ECC Curves
--------------------

.. csv-table::
   :header: "ID", "Curve"
   :widths: 1 5

   0,ECC SECP384R1

RMM_ATTEST_GET_PLAT_TOKEN command
=================================

Retrieve the Platform Token from EL3. If the entire token does not fit in the
buffer, EL3 returns a hunk of the token (via ``tokenHunkSize`` parameter) and
indicates the remaining bytes that are pending retrieval (via ``remainingSize``
parameter). The challenge object for the platform token must be populated in
the buffer for the first call of this command and the size of the object is
indicated by ``c_size`` parameter. Subsequent calls to retrieve remaining hunks of
the token must be made with ``c_size`` as 0.

If ``c_size`` is not 0, this command could cause regeneration of platform token
and will return token hunk corresponding to beginning of the token.

It is valid for the calls of this command to return ``E_RMM_AGAIN`` error,
which is an indication to the caller to retry this command again. Depending on the
platform, this mechanism can be used to implement queuing to HES, if HES is
involved in platform token generation.

FID
---

``0xC40001B3``

Input values
------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   buf_pa,x1,[63:0],Address,"PA of the platform attestation token. The challenge object must be passed in this buffer for the first call of this command. Any subsequent calls, if required to retrieve the full token, should not have this object. The PA must belong to the shared buffer."
   buf_size,x2,[63:0],Size,Size in bytes of the platform attestation token buffer. ``bufPa + bufSize`` must lie within the shared buffer
   c_size,x3,[63:0],Size,"Size in bytes of the challenge object. It corresponds to the size of one of the defined SHA algorithms. Any subsequent calls, if required to retrieve the full token, should set this size to 0."

Output values
-------------

.. csv-table::
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   Result,x0,[63:0],Error Code,Command return status
   tokenHunkSize,x1,[63:0],Size,Size of the platform token hunk retrieved
   remainingSize,x2,[63:0],Size,Remaining bytes of the token that are pending retrieval

Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table::
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_AGAIN``,Resource for Platform token retrieval is busy. Try again.
   ``E_RMM_BAD_ADDR``,``PA`` is outside the shared buffer
   ``E_RMM_INVAL``,``PA + BSize`` is outside the shared buffer
   ``E_RMM_INVAL``,``CSize`` does not represent the size of a supported SHA algorithm for the first call to this command
   ``E_RMM_INVAL``,``CSize`` is not 0 for subsequent calls to retrieve remaining hunks of the token
   ``E_RMM_UNK``,An unknown error occurred whilst processing the command
   ``E_RMM_OK``,No errors detected

RMM_EL3_FEATURES command
========================

This command provides a mechanism to discover features and ABIs supported by the
RMM-EL3 interface, for a given version. This command is helpful when there are
platform specific optional RMM-EL3 interfaces and features exposed by vendor
specific EL3 firmware, and a generic RMM that can modify its behavior based on
discovery of EL3 features.

The features can be discovered by specifying the feature register index that
has fields defined to indicate presence or absence of features and other
relevant information. The feature register index is specified in the
``feat_reg_idx`` parameter. Each feature register is a 64 bit register.

This command is available from v0.4 of the RMM-EL3 interface.

The following is the register definition for feature register index 0 for
v0.4 of the interface:

RMM-EL3 Feature Resister 0
--------------------------

.. code-block:: none

    63      32      31      16       15      8       7       1       0
    +-------+-------+-------+-------+-------+-------+-------+-------+
    |       |       |       |       |       |       |       |       |
    |       |       |       |       |       |       |       |       |
    +-------+-------+-------+-------+-------+-------+-------+-------+
                                                             ^
                                                             |
                                                 RMMD_EL3_TOKEN_SIGN

**Bit Fields:**

- **Bit 0**: `RMMD_EL3_TOKEN_SIGN`
    - When set to 1, the `RMMD_EL3_TOKEN_SIGN` feature is enabled.
    - When cleared (0), the feature is disabled.
- **Bits [1:63]**: Reserved (must be zero)

FID
---

``0xC40001B4``


Input values
------------

.. csv-table:: Input values for RMM_EL3_FEATURES
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   feat_reg_idx,x1,[63:0],UInt64, "Feature register index. For v0.4, a value of 0 is the only
   acceptable value"


Output values
-------------

.. csv-table:: Output values for RMM_EL3_FEATURES
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   Result,x0,[63:0],Error Code,Command return status
   feat_reg,x1,[63:0],Value,Value of the register as defined above

Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table:: Failure conditions for RMM_EL3_FEATURES
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_INVAL``,``feat_reg_idx`` is out of valid range
   ``E_RMM_UNK``,"if the SMC is not present, if interface version is <0.4"
   ``E_RMM_OK``,No errors detected

RMM_EL3_TOKEN_SIGN command
==========================

This command is an optional command that can be discovered using the RMM_EL3_FEATURES command.
This command is used to send requests related to realm attestation token signing requests to EL3.
The command supports 3 opcodes:

   - RMM_EL3_TOKEN_SIGN_PUSH_REQ_OP
   - RMM_EL3_TOKEN_SIGN_PULL_RESP_OP
   - RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP

The above opcodes can be used to send realm attestation token signing requests to EL3 and get their
response, so that the realm attestation token can be constructed.

This command is useful when the RMM may not have access to the private portion of the realm
attestation key and needs signing services from EL3 or CCA HES, or other platform specific
mechanisms to perform signing.

The RMM-EL3 interface for this command is modeled as two separate queues, one for signing requests
and one for retrieving the signed responses. It is possible that the queue in EL3 is full or EL3 is busy and
unable to service the RMM requests, in which case the RMM is expected to retry the push operation
for requests and pop operation for responses.

FID
---

``0xC40001B5``

Input values
------------

.. csv-table:: Input values for RMM_EL3_TOKEN_SIGN
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   fid,x0,[63:0],UInt64,Command FID
   opcode,x1,[63:0],UInt64,"
   Opcode that is one of:

    - RMM_EL3_TOKEN_SIGN_PUSH_REQ_OP: 0x1 -
      Opcode to push a token signing request to EL3 using struct el3_token_sign_request as described above
    - RMM_EL3_TOKEN_SIGN_PULL_RESP_OP: 0x2 -
      Opcode to pull a token signing response from EL3 using struct el3_token_sign_response as described above
    - RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP: 0x3 -
      Opcode to get the realm attestation public key

   "
   buf_pa,x2,[63:0],Address,"PA where the request structure is stored for the opcode RMM_EL3_TOKEN_SIGN_PUSH_REQ_OP, the response structure needs to be populated for the opcode RMM_EL3_TOKEN_SIGN_PULL_RESP_OP, or where the public key must be populated for the opcode RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP. The PA must belong to the RMM-EL3 shared buffer"
   buf_size,x3,[63:0],Size,Size in bytes of the input buffer in ``buf_pa``. ``buf_pa + buf_size`` must lie within the shared buffer
   ecc_curve,x4,[63:0],Enum,Type of the elliptic curve to which the requested attestation key belongs to. See :ref:`ecc_curves`. This parameter is valid on for the opcode RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP

Output values
-------------

.. csv-table:: Output values for RMM_EL3_TOKEN_SIGN
   :header: "Name", "Register", "Field", "Type", "Description"
   :widths: 1 1 1 1 5

   Result,x0,[63:0],Error Code,Command return status. Valid for all opcodes listed in input values
   retval1,x1,[63:0],Value, "If opcode is RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP, then returns length of
   public key returned. Otherwise, reserved"


Failure conditions
------------------

The table below shows all the possible error codes returned in ``Result`` upon
a failure. The errors are ordered by condition check.

.. csv-table:: Failure conditions for RMM_EL3_TOKEN_SIGN
   :header: "ID", "Condition"
   :widths: 1 5

   ``E_RMM_INVAL``,"if opcode is invalid or buffer address and length passed to the EL3 are not in valid range
   corresponding to the RMM-EL3 shared buffer, or if the curve used for opcode
   RMM_EL3_TOKEN_SIGN_GET_RAK_PUB_OP is not the ECC P384 curve"
   ``E_RMM_UNK``,"if the SMC is not present, if interface version is <0.4"
   ``E_RMM_AGAIN``,"For opcode RMM_EL3_TOKEN_SIGN_PUSH_REQ_OP, if the request is not queued since
   the EL3 queue is full, or if the response is not ready yet, for other opcodes"
   ``E_RMM_OK``,No errors detected


RMM-EL3 world switch register save restore convention
_____________________________________________________

As part of NS world switch, EL3 is expected to maintain a register context
specific to each world and will save and restore the registers
appropriately. This section captures the contract between EL3 and RMM on the
register set to be saved and restored.

EL3 must maintain a separate register context for the following:

   #. General purpose registers (x0-x30) and ``sp_el0``, ``sp_el2`` stack pointers
   #. EL2 system register context for all enabled features by EL3. These include system registers with the ``_EL2`` prefix. The EL2 physical and virtual timer registers must not be included in this.

As part of SMC forwarding between the NS world and Realm world, EL3 allows x0-x7 to be passed
as arguments to Realm and x0-x4 to be used for return arguments back to Non Secure.
As per SMCCCv1.2, x4 must be preserved if not being used as return argument by the SMC function
and it is the responsibility of RMM to preserve this or use this as a return argument.
EL3 will always copy x0-x4 from Realm context to NS Context.

EL3 must save and restore the following as part of world switch:
   #. EL2 system registers with the exception of ``zcr_el2`` register.
   #. PAuth key registers (APIA, APIB, APDA, APDB, APGA).

EL3 will not save some registers as mentioned in the below list. It is the
responsibility of RMM to ensure that these are appropriately saved if the
Realm World makes use of them:

   #. FP/SIMD registers
   #. SVE registers
   #. SME registers
   #. EL1/0 registers with the exception of PAuth key registers as mentioned above.
   #. zcr_el2 register.

It is essential that EL3 honors this contract to maintain the Confidentiality and integrity
of the Realm world.

SMCCC v1.3 allows NS world to specify whether SVE context is in use. In this
case, RMM could choose to not save the incoming SVE context but must ensure
to clear SVE registers if they have been used in Realm World. The same applies
to SME registers.

Types
_____

.. _rmm_el3_manifest_struct:

RMM-EL3 Boot Manifest structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The RMM-EL3 Boot Manifest v0.3 structure contains platform boot information passed
from EL3 to RMM. The size of the Boot Manifest is 64 bytes.

The members of the RMM-EL3 Boot Manifest structure are shown in the following
table:

+--------------+--------+----------------+----------------------------------------+
|   Name       | Offset |     Type       |               Description              |
+==============+========+================+========================================+
| version      |   0    |   uint32_t     | Boot Manifest version                  |
+--------------+--------+----------------+----------------------------------------+
| padding      |   4    |   uint32_t     | Reserved, set to 0                     |
+--------------+--------+----------------+----------------------------------------+
| plat_data    |   8    |   uintptr_t    | Pointer to Platform Data section       |
+--------------+--------+----------------+----------------------------------------+
| plat_dram    |   16   | ns_dram_info   | NS DRAM Layout Info structure          |
+--------------+--------+----------------+----------------------------------------+
| plat_console |   40   | console_list   | List of consoles available to RMM      |
+--------------+--------+----------------+----------------------------------------+

.. _ns_dram_info_struct:

NS DRAM Layout Info structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

NS DRAM Layout Info structure contains information about platform Non-secure
DRAM layout. The members of this structure are shown in the table below:

+-----------+--------+----------------+----------------------------------------+
|   Name    | Offset |     Type       |               Description              |
+===========+========+================+========================================+
| num_banks |   0    |   uint64_t     | Number of NS DRAM banks                |
+-----------+--------+----------------+----------------------------------------+
| banks     |   8    | ns_dram_bank * | Pointer to 'ns_dram_bank'[] array      |
+-----------+--------+----------------+----------------------------------------+
| checksum  |   16   |   uint64_t     | Checksum                               |
+-----------+--------+----------------+----------------------------------------+

Checksum is calculated as two's complement sum of 'num_banks', 'banks' pointer
and DRAM banks data array pointed by it.

.. _ns_dram_bank_struct:

NS DRAM Bank structure
~~~~~~~~~~~~~~~~~~~~~~

NS DRAM Bank structure contains information about each Non-secure DRAM bank:

+-----------+--------+----------------+----------------------------------------+
|   Name    | Offset |     Type       |               Description              |
+===========+========+================+========================================+
|   base    |   0    |   uintptr_t    | Base address                           |
+-----------+--------+----------------+----------------------------------------+
|   size    |   8    |   uint64_t     | Size of bank in bytes                  |
+-----------+--------+----------------+----------------------------------------+

.. _console_list_struct:

Console List structure
~~~~~~~~~~~~~~~~~~~~~~

Console List structure contains information about the available consoles for RMM.
The members of this structure are shown in the table below:

+--------------+--------+----------------+----------------------------------------+
|   Name       | Offset |     Type       |               Description              |
+==============+========+================+========================================+
| num_consoles |   0    |   uint64_t     | Number of consoles                     |
+--------------+--------+----------------+----------------------------------------+
| consoles     |   8    | console_info * | Pointer to 'console_info'[] array      |
+--------------+--------+----------------+----------------------------------------+
| checksum     |   16   |   uint64_t     | Checksum                               |
+--------------+--------+----------------+----------------------------------------+

Checksum is calculated as two's complement sum of 'num_consoles', 'consoles'
pointer and the consoles array pointed by it.

.. _console_info_struct:

Console Info structure
~~~~~~~~~~~~~~~~~~~~~~

Console Info structure contains information about each Console available to RMM.

+-----------+--------+---------------+----------------------------------------+
|   Name    | Offset |     Type      |               Description              |
+===========+========+===============+========================================+
| base      |   0    |   uintptr_t   | Console Base address                   |
+-----------+--------+---------------+----------------------------------------+
| map_pages |   8    |   uint64_t    | Num of pages to map for console MMIO   |
+-----------+--------+---------------+----------------------------------------+
| name      |   16   |   char[]      | Name of console                        |
+-----------+--------+---------------+----------------------------------------+
| clk_in_hz |   24   |   uint64_t    | UART clock (in hz) for console         |
+-----------+--------+---------------+----------------------------------------+
| baud_rate |   32   |   uint64_t    | Baud rate                              |
+-----------+--------+---------------+----------------------------------------+
| flags     |   40   |   uint64_t    | Additional flags (RES0)                |
+-----------+--------+---------------+----------------------------------------+

.. _el3_token_sign_request_struct:

EL3 Token Sign Request structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This structure represents a realm attestation toekn signing request.

+-------------+--------+---------------+-----------------------------------------+
|   Name      | Offset |     Type      |               Description               |
+=============+========+===============+=========================================+
| sig_alg_id  |   0    |   uint32_t    | Algorithm idenfier for the sign request.|
|             |        |               | - 0x0: ECC SECP384R1 (ECDSA)            |
|             |        |               | - Other values reserved                 |
+-------------+--------+---------------+-----------------------------------------+
| rec_granule |   8    |   uint64_t    | Identifier used by RMM to associate     |
|             |        |               | a signing request to a realm. Must not  |
|             |        |               | be interpreted or modified.             |
+-------------+--------+---------------+-----------------------------------------+
| req_ticket  |   16   |   uint64_t    | Value used by RMM to associate request  |
|             |        |               | and responses. Must not be interpreted  |
|             |        |               | or modified.                            |
+-------------+--------+---------------+-----------------------------------------+
| hash_alg_id |   24   |   uint32_t    | Hash algorithm for data in `hash_buf`   |
|             |        |               | - 0x1: SHA2-384                         |
|             |        |               | - All other values reserved.            |
+-------------+--------+---------------+-----------------------------------------+
| hash_buf    |   32   |   uint8_t[]   | TBS (to-be-signed) Hash of length       |
|             |        |               | defined by hash algorithm `hash_alg_id` |
+-------------+--------+---------------+-----------------------------------------+

.. _el3_token_sign_response_struct:

EL3 Token Sign Response structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This structure represents a realm attestation token signing response.

+---------------+--------+---------------+-----------------------------------------+
|   Name        | Offset |     Type      |               Description               |
+===============+========+===============+=========================================+
| rec_granule   |   0    |   uint64_t    | Identifier used by RMM to associate     |
|               |        |               | a signing request to a realm. Must not  |
|               |        |               | be interpreted or modified.             |
+---------------+--------+---------------+-----------------------------------------+
| req_ticket    |   8    |   uint64_t    | Value used by RMM to associate request  |
|               |        |               | and responses. Must not be interpreted  |
|               |        |               | or modified.                            |
+---------------+--------+---------------+-----------------------------------------+
| sig_len       |   16   |   uint16_t    | Length of the `signature_buf` field     |
+---------------+--------+---------------+-----------------------------------------+
| signature_buf |   18   |   uint8_t[]   | Signature                               |
+---------------+--------+---------------+-----------------------------------------+
