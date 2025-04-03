Prerequisites
=============

This document describes the software requirements for building |TF-A| for
AArch32 and AArch64 target platforms.

It may possible to build |TF-A| with combinations of software packages that are
different from those listed below, however only the software described in this
document can be officially supported.

Getting the TF-A Source
-----------------------

Source code for |TF-A| is maintained in a Git repository hosted on
`TrustedFirmware.org`_. To clone this repository from the server, run the following
in your shell:

.. code:: shell

    git clone "https://review.trustedfirmware.org/TF-A/trusted-firmware-a"


Requirements
------------

======================== =====================
        Program          Min supported version
======================== =====================
Arm Compiler             6.18
Arm GNU Compiler         13.3
Clang/LLVM               18.1.8
Device Tree Compiler     1.6.1
GNU make                 3.81
mbed TLS\ [#f1]_         3.6.3
Node.js [#f2]_           16
OpenSSL                  1.0.0
Poetry                   1.3.2
QCBOR\ [#f3]_            1.2
Sphinx\ [#f2]_           5.3.0
======================== =====================

.. [#f1] Required for Trusted Board Boot and Measured Boot.
.. [#f2] Required only for building TF-A documentation.
.. [#f3] Required only when enabling DICE Protection Environment support.

Toolchain
^^^^^^^^^

|TF-A| can be compiled using any cross-compiler toolchain specified in the
preceding table that target Armv7-A or Armv8-A. For AArch32 and
AArch64 builds, the respective targets required are ``arm-none-eabi`` and
``aarch64-none-elf``.

Testing has been performed with version 13.3.Rel1 (gcc 13.3) of the Arm
GNU compiler, which can be installed from the `Arm Developer website`_.

In addition, a native compiler is required to build supporting tools.

.. note::
   Versions greater than the ones specified are likely but not guaranteed to
   work. This is predominantly because TF-A carries its own copy of compiler-rt,
   which may be older than the version expected by the compiler. Fixes and bug
   reports are always welcome.

.. note::
   For instructions on how to select the cross compiler refer to
   :ref:`Performing an Initial Build`.

OpenSSL
^^^^^^^

OpenSSL is required to build the cert_create, encrypt_fw, and fiptool tools.

If using OpenSSL 3, older Linux versions may require it to be built from
source code, as it may not be available in the default package repositories.
Please refer to the OpenSSL project documentation for more information.

.. warning::
    Versions 1.0.x and from v3.0.0 up to v3.0.6 are strongly advised against due
    to concerns regarding security vulnerabilities!

Device Tree Compiler (DTC)
^^^^^^^^^^^^^^^^^^^^^^^^^^

Needed if you want to rebuild the provided Flattened Device Tree (FDT)
source files (``.dts`` files). DTC is available for Linux through the package
repositories of most distributions.

Arm Development Studio (`Arm-DS`_)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The standard software package used for debugging software on Arm development
platforms and |FVP| models.

Node.js
^^^^^^^

Highly recommended, and necessary in order to install and use the packaged
Git hooks and helper tools. Without these tools you will need to rely on the
CI for feedback on commit message conformance.

Poetry
^^^^^^

Required for managing Python dependencies, this will allow you to reliably
reproduce a Python environment to build documentation and run some of the
integrated Python tools. Most importantly, it ensures your system environment
will not be affected by dependencies in the Python scripts.

For installation instructions, see the `official Poetry documentation`_.

.. _prerequisites_software_and_libraries:

Package Installation (Linux)
----------------------------

|TF-A| can be compiled on both Linux and Windows-based machines.
However, we strongly recommend using a UNIX-compatible build environment.

Testing is performed using Ubuntu 22.04 LTS (64-bit), but other distributions
should also work, provided the necessary tools and libraries are installed.

The following are steps to install the required packages:

.. code:: shell

    sudo apt install build-essential

The optional packages can be installed using:

.. code:: shell

    sudo apt install device-tree-compiler

Additionally, to install a version of Node.js compatible with TF-A's repository
scripts, you can use the `Node Version Manager`_. To install both NVM and an
appropriate version of Node.js, run the following **from the root directory of
the repository**:

.. code:: shell

    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.1/install.sh | bash
    exec "$SHELL" -ic "nvm install; exec $SHELL"

.. _Node Version Manager: https://github.com/nvm-sh/nvm#install--update-script

Supporting Files
----------------

TF-A has been tested with pre-built binaries and file systems from `Linaro
Release 20.01`_. Alternatively, you can build the binaries from source using
instructions in :ref:`Performing an Initial Build`.

.. _prerequisites_get_source:

Additional Steps for Contributors
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you are planning on contributing back to TF-A, there are some things you'll
want to know.

TF-A is hosted by a `Gerrit Code Review`_ server. Gerrit requires that all
commits include a ``Change-Id`` footer, and this footer is typically
automatically generated by a Git hook installed by you, the developer.

If you have Node.js installed already, you can automatically install this hook,
along with any additional hooks and Javascript-based tooling that we use, by
running from within your newly-cloned repository:

.. code:: shell

    npm install --no-save

If you have opted **not** to install Node.js, you can install the Gerrit hook
manually by running:

.. code:: shell

    curl -Lo $(git rev-parse --git-dir)/hooks/commit-msg https://review.trustedfirmware.org/tools/hooks/commit-msg
    chmod +x $(git rev-parse --git-dir)/hooks/commit-msg

You can read more about Git hooks in the *githooks* page of the Git
documentation, available `here <https://git-scm.com/docs/githooks>`_.

--------------

*Copyright (c) 2021-2024, Arm Limited. All rights reserved.*

.. _Arm Developer website: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads
.. _Gerrit Code Review: https://www.gerritcodereview.com/
.. _Linaro Release Notes: https://community.arm.com/dev-platforms/w/docs/226/old-release-notes
.. _Linaro instructions: https://community.arm.com/dev-platforms/w/docs/304/arm-reference-platforms-deliverables
.. _Arm-DS: https://developer.arm.com/Tools%20and%20Software/Arm%20Development%20Studio
.. _Linaro Release 20.01: http://releases.linaro.org/members/arm/platforms/20.01
.. _TrustedFirmware.org: https://www.trustedfirmware.org/
.. _official Poetry documentation: https://python-poetry.org/docs/#installation
