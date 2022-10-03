OP-TEE Dispatcher
=================

`OP-TEE OS`_ is a Trusted OS running as Secure EL1.

To build and execute OP-TEE follow the instructions at
`OP-TEE build.git`_

There are two different modes for loading the OP-TEE OS. The default mode will
load it as the BL32 payload during boot, and is the recommended technique for
platforms to use. There is also another technique that will load OP-TEE OS after
boot via an SMC call by enabling the option for OPTEE_ALLOW_SMC_LOAD that was
specifically added for ChromeOS. Loading OP-TEE via an SMC call may be insecure
depending upon the platform configuration. If using that option, be sure to
understand the risks involved with allowing the Trusted OS to be loaded this
way. ChromeOS uses a boot flow where it verifies the signature of the firmware
before executing it, and then only if the signature is valid will the 'secrets'
used by the TEE become accessible. The firmware then verifies the signature of
the kernel using depthcharge, and the kernel verifies the rootfs using
dm-verity.  The SMC call to load OP-TEE is then invoked immediately after the
kernel finishes loading and before any attack vectors can be opened up by
mounting writable filesystems or opening network/device connections. this
ensures the platform is 'closed' and running signed code through the point where
OP-TEE is loaded.

--------------

*Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.*

.. _OP-TEE OS: https://github.com/OP-TEE/build
.. _OP-TEE build.git: https://github.com/OP-TEE/build
