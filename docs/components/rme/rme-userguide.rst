RME Enabled TF-A userguide
=================================

Prerequisites
--------------

To evaluate |RME|, first follow the instructions as described in the |TF-A|
:ref:`Prerequisites` section to clone the repository and setup the build
environment. This should setup the required software libraries and aarch64
baremetal toolchain for the build. Specifically, ensure that the environment
variable ``CROSS_COMPILE`` is setup correctly as described in :ref:`Performing
an Initial Build`.

Next, similar to the |TF-A| approach, follow the instructions as described
in the `TF-A Tests Getting Started`_ for the pre-requisites.

At this point, we should be left with two directories, |TF-A| and |TF-A-Tests|.

.. code:: shell

    ls
    tf-a-tests  trusted-firmware-a

Getting the sources
--------------------------

The source code for |RME| is currently maintained as a branch in the |TF-A| git
repository hosted on `trustedfirmware.org`_. To checkout this branch, execute the
following commands:

.. code:: shell

    cd trusted-firmware-a
    git checkout topics/rme_prototype

Similarly, checkout the appropriate branch for |TF-A-Tests|:

.. code:: shell

    cd tf-a-tests
    git checkout topics/rme_prototype


Building the sources
---------------------

To build the |TF-A-Tests| source, follow the instructions to build for the
FVP as described in `Building TF-A Tests`_. An example command is shown below:

.. code:: shell

    cd tf-a-tests
    make PLAT=fvp DEBUG=1 USE_NVM=0 SHELL_COLOR=1 tftf

This should generate the tftf.bin binary which can be passed as BL33 while building |TF-A|.

Finally, to build |TF-A| execute the following commands:

.. code:: shell

    cd trusted-firmware-a
    make ARCH=aarch64 PLAT=fvp ARM_DISABLE_TRUSTED_WDOG=1 ENABLE_RME=1 DEBUG=1 BL33=../tf-a-tests/build/fvp/debug/tftf.bin FVP_HW_CONFIG_DTS=fdts/fvp-base-gicv3-psci-1t.dts all fip

This builds |TF-A| with |TF-A-Tests| as Normal world payload (BL33) and adds the
Test Realm payload (TRP) as BL32 image.


Executing on ARM |FVP|
-----------------------

In order to run on the FVP, please download an RME enabled FVP model through
`Arm FVP website`_ (expected availability by end of July 2021). Refer
:ref:`Arm Fixed Virtual Platforms (FVP)` for details about running the model.

To launch the Armv-A Base RevC AEM FVP, execute the following command:

.. code:: shell

    <path to fvp pkg>/models/Linux64_GCC-6.4/FVP_Base_RevC-2xAEMvA \
    -C bp.dram_size=2 \
    -C bp.pl011_uart0.uart_enable=1 \
    -C bp.pl011_uart1.uart_enable=1 \
    -C bp.pl011_uart2.uart_enable=1 \
    -C bp.refcounter.non_arch_start_at_default=1 \
    -C bp.refcounter.use_real_time=0 \
    -C bp.secure_memory=1 \
    -C cache_state_modelled=1 \
    -C cluster0.ecv_support_level=2 \
    -C cluster1.ecv_support_level=2 \
    -C cluster0.gicv3.cpuintf-mmap-access-level=2 \
    -C cluster1.gicv3.cpuintf-mmap-access-level=2 \
    -C cluster0.gicv4.mask-virtual-interrupt=1 \
    -C cluster1.gicv4.mask-virtual-interrupt=1 \
    -C cluster0.gicv3.without-DS-support=1 \
    -C cluster1.gicv3.without-DS-support=1 \
    -C cluster0.has_arm_v8-4=1 \
    -C cluster1.has_arm_v8-4=1 \
    -C cluster0.has_rme=1 \
    -C cluster1.has_rme=1 \
    -C cluster0.has_rndr=1 \
    -C cluster1.has_rndr=1 \
    -C cluster0.has_v8_7_pmu_extension=2 \
    -C cluster1.has_v8_7_pmu_extension=2 \
    -C cluster0.max_32bit_el=-1 \
    -C cluster1.max_32bit_el=-1 \
    -C cluster0.NUM_CORES=4 \
    -C cluster1.NUM_CORES=4 \
    -C cluster0.PA_SIZE=48 \
    -C cluster1.PA_SIZE=48 \
    -C bp.flashloader0.fname=<path to fip.bin> \
    -C bp.secureflashloader.fname=<path to bl1.bin>

where:

``<path to fvp pkg>``
  * The directory that the FVP was untared into
``<path to fip.bin>``
  * The fip.bin created during |TF-A| make
  * For example: ``trusted-firmware-a/build/fvp/debug/fip.bin``
``<path to bl1.bin>``
  * The bl1.bin created during |TF-A| make
  * For example: ``trusted-firmware-a/build/fvp/debug/bl1.bin``

Test results are output to the window titled "FVP terminal_0" and the Test
Realm Payload logs are output to the window titled "FVP terminal_3.


Details of the |TF-A-Tests| for Realm world
---------------------------------------------

The |TF-A-Tests| adds a new testsuite "Realm payload tests". This implements
a simple test payload which just initializes the memory and UART. The payload
also implements a handler which can respond to few RMI calls initiated
by the testsuite.

The log for the testsuite should appear as part of the results summary:

.. code-block:: shell

    ******************************* Summary *******************************

    > Test suite 'Realm payload tests'			   	     Passed


Known limitations of the prototype
-----------------------------------

This is a prototype to bootstrap TF-A on an RME enabled FVP. It replaces the
secure world payload with Realm world binary. The Granule protection table (GPT)
initialization sequence is a work in progress (although functional).

--------------

*Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.*

.. _Arm FVP website: https://developer.arm.com/tools-and-software/simulation-models/fixed-virtual-platforms/arm-ecosystem-models
.. _trustedfirmware.org: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
.. _TF-A Tests: https://trustedfirmware-a-tests.readthedocs.io
.. _TF-A Tests Getting Started: https://trustedfirmware-a-tests.readthedocs.io/en/latest/getting_started/index.html
.. _Building TF-A Tests: https://trustedfirmware-a-tests.readthedocs.io/en/latest/getting_started/build.html
