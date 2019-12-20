Arm SiP Services
================

This document enumerates and describes the Arm SiP (Silicon Provider) services.

SiP services are non-standard, platform-specific services offered by the silicon
implementer or platform provider. They are accessed via ``SMC`` ("SMC calls")
instruction executed from Exception Levels below EL3. SMC calls for SiP
services:

-  Follow `SMC Calling Convention`_;
-  Use SMC function IDs that fall in the SiP range, which are ``0xc2000000`` -
   ``0xc200ffff`` for 64-bit calls, and ``0x82000000`` - ``0x8200ffff`` for 32-bit
   calls.

The Arm SiP implementation offers the following services:

-  Performance Measurement Framework (PMF)
-  Execution State Switching service
-  DebugFS interface

Source definitions for Arm SiP service are located in the ``arm_sip_svc.h`` header
file.

Performance Measurement Framework (PMF)
---------------------------------------

The :ref:`Performance Measurement Framework <firmware_design_pmf>`
allows callers to retrieve timestamps captured at various paths in TF-A
execution.

Execution State Switching service
---------------------------------

Execution State Switching service provides a mechanism for a non-secure lower
Exception Level (either EL2, or NS EL1 if EL2 isn't implemented) to request to
switch its execution state (a.k.a. Register Width), either from AArch64 to
AArch32, or from AArch32 to AArch64, for the calling CPU. This service is only
available when Trusted Firmware-A (TF-A) is built for AArch64 (i.e. when build
option ``ARCH`` is set to ``aarch64``).

``ARM_SIP_SVC_EXE_STATE_SWITCH``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    Arguments:
        uint32_t Function ID
        uint32_t PC hi
        uint32_t PC lo
        uint32_t Cookie hi
        uint32_t Cookie lo

    Return:
        uint32_t

The function ID parameter must be ``0x82000020``. It uniquely identifies the
Execution State Switching service being requested.

The parameters *PC hi* and *PC lo* defines upper and lower words, respectively,
of the entry point (physical address) at which execution should start, after
Execution State has been switched. When calling from AArch64, *PC hi* must be 0.

When execution starts at the supplied entry point after Execution State has been
switched, the parameters *Cookie hi* and *Cookie lo* are passed in CPU registers
0 and 1, respectively. When calling from AArch64, *Cookie hi* must be 0.

This call can only be made on the primary CPU, before any secondaries were
brought up with ``CPU_ON`` PSCI call. Otherwise, the call will always fail.

The effect of switching execution state is as if the Exception Level were
entered for the first time, following power on. This means CPU registers that
have a defined reset value by the Architecture will assume that value. Other
registers should not be expected to hold their values before the call was made.
CPU endianness, however, is preserved from the previous execution state. Note
that this switches the execution state of the calling CPU only. This is not a
substitute for PSCI ``SYSTEM_RESET``.

The service may return the following error codes:

-  ``STATE_SW_E_PARAM``: If any of the parameters were deemed invalid for
   a specific request.
-  ``STATE_SW_E_DENIED``: If the call is not successful, or when TF-A is
   built for AArch32.

If the call is successful, the caller wouldn't observe the SMC returning.
Instead, execution starts at the supplied entry point, with the CPU registers 0
and 1 populated with the supplied *Cookie hi* and *Cookie lo* values,
respectively.

DebugFS interface
-----------------

The optional DebugFS interface is accessed through an SMC SiP service. Refer
to the component documentation for details.

String parameters are passed through a shared buffer using a specific union:

.. code:: c

    union debugfs_parms {
        struct {
            char fname[MAX_PATH_LEN];
        } open;

        struct mount {
            char srv[MAX_PATH_LEN];
            char where[MAX_PATH_LEN];
            char spec[MAX_PATH_LEN];
        } mount;

        struct {
            char path[MAX_PATH_LEN];
            dir_t dir;
        } stat;

        struct {
            char oldpath[MAX_PATH_LEN];
            char newpath[MAX_PATH_LEN];
        } bind;
    };

Format of the dir_t structure as such:

.. code:: c

    typedef struct {
        char		name[NAMELEN];
        long		length;
        unsigned char	mode;
        unsigned char	index;
        unsigned char	dev;
        qid_t		qid;
    } dir_t;


* Identifiers

======================== =============================================
SMC_OK                   0
SMC_UNK                  -1
DEBUGFS_E_INVALID_PARAMS -2
======================== =============================================

======================== =============================================
MOUNT                    0
CREATE                   1
OPEN                     2
CLOSE                    3
READ                     4
WRITE                    5
SEEK                     6
BIND                     7
STAT                     8
INIT                     10
VERSION                  11
======================== =============================================

MOUNT
~~~~~

Description
^^^^^^^^^^^
This operation mounts a blob of data pointed to by path stored in `src`, at
filesystem location pointed to by path stored in `where`, using driver pointed
to by path in `spec`.

Parameters
^^^^^^^^^^
======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``MOUNT``
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if mount operation failed
=============== ==========================================================

OPEN
~~~~

Description
^^^^^^^^^^^
This operation opens the file path pointed to by `fname`.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``OPEN``
uint32_t mode
======== ============================================================

mode can be one of:

.. code:: c

    enum mode {
        O_READ   = 1 << 0,
        O_WRITE  = 1 << 1,
        O_RDWR   = 1 << 2,
        O_BIND   = 1 << 3,
        O_DIR    = 1 << 4,
        O_STAT   = 1 << 5
    };

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if open operation failed

uint32_t        w1: file descriptor id on success.
=============== ==========================================================

CLOSE
~~~~~

Description
^^^^^^^^^^^

This operation closes a file described by a file descriptor obtained by a
previous call to OPEN.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``CLOSE``
uint32_t File descriptor id returned by OPEN
======== ============================================================

Return values
^^^^^^^^^^^^^
=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if close operation failed
=============== ==========================================================

READ
~~~~

Description
^^^^^^^^^^^

This operation reads a number of bytes from a file descriptor obtained by
a previous call to OPEN.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``READ``
uint32_t File descriptor id returned by OPEN
uint32_t Number of bytes to read
======== ============================================================

Return values
^^^^^^^^^^^^^

On success, the read data is retrieved from the shared buffer after the
operation.

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if read operation failed

uint32_t        w1: number of bytes read on success.
=============== ==========================================================

SEEK
~~~~

Description
^^^^^^^^^^^

Move file pointer for file described by given `file descriptor` of given
`offset` related to `whence`.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``SEEK``
uint32_t File descriptor id returned by OPEN
sint32_t offset in the file relative to whence
uint32_t whence
======== ============================================================

whence can be one of:

========= ============================================================
KSEEK_SET 0
KSEEK_CUR 1
KSEEK_END 2
========= ============================================================

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if seek operation failed
=============== ==========================================================

BIND
~~~~

Description
^^^^^^^^^^^

Create a link from `oldpath` to `newpath`.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``BIND``
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if bind operation failed
=============== ==========================================================

STAT
~~~~

Description
^^^^^^^^^^^

Perform a stat operation on provided file `name` and returns the directory
entry statistics into `dir`.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``STAT``
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ==========================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if stat operation failed
=============== ==========================================================

INIT
~~~~

Description
^^^^^^^^^^^
Initial call to setup the shared exchange buffer. Notice if successful once,
subsequent calls fail after a first initialization. The caller maps the same
page frame in its virtual space and uses this buffer to exchange string
parameters with filesystem primitives.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``INIT``
uint64_t Physical address of the shared buffer.
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ======================================================
int32_t         w0 == SMC_OK on success

                w0 == DEBUGFS_E_INVALID_PARAMS if already initialized,
                or internal error occurred.
=============== ======================================================

VERSION
~~~~~~~

Description
^^^^^^^^^^^
Returns the debugfs interface version if implemented in TF-A.

Parameters
^^^^^^^^^^

======== ============================================================
uint32_t FunctionID (0x82000030 / 0xC2000030)
uint32_t ``VERSION``
======== ============================================================

Return values
^^^^^^^^^^^^^

=============== ======================================================
int32_t         w0 == SMC_OK on success

                w0 == SMC_UNK if interface is not implemented

uint32_t        w1: On success, debugfs interface version, 32 bits
                value with major version number in upper 16 bits and
                minor version in lower 16 bits.
=============== ======================================================

* CREATE(1) and WRITE (5) command identifiers are unimplemented and
  return `SMC_UNK`.

--------------

*Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028a/index.html
