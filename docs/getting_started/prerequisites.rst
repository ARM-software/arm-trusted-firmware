Prerequisites
=============

This document describes the software requirements for building |TF-A| for
AArch32 and AArch64 target platforms.

It may possible to build |TF-A| with combinations of software packages that are
different from those listed below, however only the software described in this
document can be officially supported.

Build Host
----------

|TF-A| can be built using either a Linux or a Windows machine as the build host.

A relatively recent Linux distribution is recommended for building |TF-A|. We
have performed tests using Ubuntu 16.04 LTS (64-bit) but other distributions
should also work fine as a base, provided that the necessary tools and libraries
can be installed.

.. _prerequisites_toolchain:

Toolchain
---------

|TF-A| can be built with any of the following *cross-compiler* toolchains that
target the Armv7-A or Armv8-A architectures:

- GCC >= 9.2-2019.12 (from the `Arm Developer website`_)
- Clang >= 4.0
- Arm Compiler >= 6.0

In addition, a native compiler is required to build the supporting tools.

.. note::
   The software has also been built on Windows 7 Enterprise SP1, using CMD.EXE,
   Cygwin, and Msys (MinGW) shells, using version 5.3.1 of the GNU toolchain.

.. note::
   For instructions on how to select the cross compiler refer to
   :ref:`Performing an Initial Build`.

.. _prerequisites_software_and_libraries:

Software and Libraries
----------------------

The following tools are required to obtain and build |TF-A|:

- An appropriate toolchain (see :ref:`prerequisites_toolchain`)
- GNU Make
- Git

The following libraries must be available to build one or more components or
supporting tools:

- OpenSSL >= 1.0.1

   Required to build the cert_create tool.

The following libraries are required for Trusted Board Boot support:

- mbed TLS == 2.24.0 (tag: ``mbedtls-2.24.0``)

These tools are optional:

- Device Tree Compiler (DTC) >= 1.4.6

   Needed if you want to rebuild the provided Flattened Device Tree (FDT)
   source files (``.dts`` files). DTC is available for Linux through the package
   repositories of most distributions.

- Arm `Development Studio 5 (DS-5)`_

   The standard software package used for debugging software on Arm development
   platforms and |FVP| models.

Package Installation (Linux)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are using the recommended Ubuntu distribution then you can install the
required packages with the following command:

.. code:: shell

    sudo apt install build-essential git libssl-dev

The optional packages can be installed using:

.. code:: shell

    sudo apt install device-tree-compiler

Supporting Files
----------------

TF-A has been tested with pre-built binaries and file systems from `Linaro
Release 19.06`_. Alternatively, you can build the binaries from source using
instructions in :ref:`Performing an Initial Build`.

.. _prerequisites_get_source:

Getting the TF-A Source
-----------------------

Source code for |TF-A| is maintained in a Git repository hosted on
TrustedFirmware.org. To clone this repository from the server, run the following
in your shell:

.. code:: shell

    git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a" && (cd "trusted-firmware-a" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg https://review.trustedfirmware.org/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg)

This will clone the Git repository also install a *commit hook* that
automatically inserts appropriate *Change-Id:* lines at the end of your
commit messages. These change IDs are required when committing changes that you
intend to push for review via our Gerrit system.

You can read more about Git hooks in the *githooks* page of the Git documentation,
available at: https://git-scm.com/docs/githooks

Alternatively, you can clone without the commit hook using:

.. code:: shell

    git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a"

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

.. _Arm Developer website: https://developer.arm.com/open-source/gnu-toolchain/gnu-a/downloads
.. _Linaro Release Notes: https://community.arm.com/dev-platforms/w/docs/226/old-release-notes
.. _Linaro instructions: https://community.arm.com/dev-platforms/w/docs/304/arm-reference-platforms-deliverables
.. _Development Studio 5 (DS-5): https://developer.arm.com/products/software-development-tools/ds-5-development-studio
.. _Linaro Release 19.06: http://releases.linaro.org/members/arm/platforms/19.06
