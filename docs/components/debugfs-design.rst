========
Debug FS
========

.. contents::

Overview
--------

The *DebugFS* feature is primarily aimed at exposing firmware debug data to
higher SW layers such as a non-secure component. Such component can be the
TFTF test payload or a Linux kernel module.

Virtual filesystem
------------------

The core functionality lies in a virtual file system based on a 9p file server
interface (`Notes on the Plan 9 Kernel Source`_ and
`Linux 9p remote filesystem protocol`_).
The implementation permits exposing virtual files, firmware drivers, and file blobs.

Namespace
~~~~~~~~~

Two namespaces are exposed:

  - # is used as root for drivers (e.g. #t0 is the first uart)
  - / is used as root for virtual "files" (e.g. /fip, or /dev/uart)

9p interface
~~~~~~~~~~~~

The associated primitives are:

- Unix-like:

  - open(): create a file descriptor that acts as a handle to the file passed as
    an argument.
  - close(): close the file descriptor created by open().
  - read(): read from a file to a buffer.
  - write(): write from a buffer to a file.
  - seek(): set the file position indicator of a file descriptor either to a
    relative or an absolute offset.
  - stat(): get information about a file (type, mode, size, ...).

.. code:: c

    int open(const char *name, int flags);
    int close(int fd);
    int read(int fd, void *buf, int n);
    int write(int fd, void *buf, int n);
    int seek(int fd, long off, int whence);
    int stat(char *path, dir_t *dir);

- Specific primitives :

  - mount(): create a link between a driver and spec.
  - create(): create a file in a specific location.
  - bind(): expose the content of a directory to another directory.

.. code:: c

    int mount(char *srv, char *mnt, char *spec);
    int create(const char *name, int flags);
    int bind(char *path, char *where);

This interface is embedded into the BL31 run-time payload when selected by build
options. The interface multiplexes drivers or emulated "files":

- Debug data can be partitioned into different virtual files e.g. expose PMF
  measurements through a file, and internal firmware state counters through
  another file.
- This permits direct access to a firmware driver, mainly for test purposes
  (e.g. a hardware device that may not be accessible to non-privileged/
  non-secure layers, or for which no support exists in the NS side).

SMC interface
-------------

The communication with the 9p layer in BL31 is made through an SMC conduit
(`SMC Calling Convention PDD`_), using a specific SiP Function Id. An NS
shared buffer is used to pass path string parameters, or e.g. to exchange
data on a read operation. Refer to `ARM SiP Services`_ for a description
of the SMC interface.

Security considerations
-----------------------

- Due to the nature of the exposed data, the feature is considered experimental
  and importantly **shall only be used in debug builds**.
- Several primitive imply string manipulations and usage of string formats.
- Special care is taken with the shared buffer to avoid TOCTOU attacks.

Limitations
-----------

- In order to setup the shared buffer, the component consuming the interface
  needs to allocate a physical page frame and transmit its address.
- In order to map the shared buffer, BL31 requires enabling the dynamic xlat
  table option.
- Data exchange is limited by the shared buffer length. A large read operation
  might be split into multiple read operations of smaller chunks.
- On concurrent access, a spinlock is implemented in the BL31 service to protect
  the internal work buffer, and re-entrancy into the filesystem layers.
- Notice, a physical device driver if exposed by the firmware may conflict with
  the higher level OS if the latter implements its own driver for the same
  physical device.

Applications
------------

The SMC interface is accessible from an NS environment, that is:

- a test payload, bootloader or hypervisor running at NS-EL2
- a Linux kernel driver running at NS-EL1
- a Linux userspace application through the kernel driver

--------------

*Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.*

.. _SMC Calling Convention PDD: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/
.. _Notes on the Plan 9 Kernel Source: http://lsub.org/who/nemo/9.pdf
.. _Linux 9p remote filesystem protocol: https://www.kernel.org/doc/Documentation/filesystems/9p.txt
.. _ARM SiP Services: arm-sip-service.rst
