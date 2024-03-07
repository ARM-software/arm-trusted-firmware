DebugFS interface
=================

The optional DebugFS interface is accessed through a Vendor specific EL3 service. Refer
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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
uint32_t FunctionID (0x87000010 / 0xC7000010)
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

*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*
