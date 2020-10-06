Arm Fixed Virtual Platforms (FVP)
=================================

Fixed Virtual Platform (FVP) Support
------------------------------------

This section lists the supported Arm |FVP| platforms. Please refer to the FVP
documentation for a detailed description of the model parameter options.

The latest version of the AArch64 build of TF-A has been tested on the following
Arm FVPs without shifted affinities, and that do not support threaded CPU cores
(64-bit host machine only).

.. note::
   The FVP models used are Version 11.12 Build 38, unless otherwise stated.

-  ``FVP_Base_AEMvA``
-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_AEMv8A-AEMv8A-AEMv8A-AEMv8A-CCN502``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A53x4``
-  ``FVP_Base_Cortex-A55x4+Cortex-A75x4``
-  ``FVP_Base_Cortex-A55x4``
-  ``FVP_Base_Cortex-A57x1-A53x1``
-  ``FVP_Base_Cortex-A57x2-A53x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_Cortex-A57x4``
-  ``FVP_Base_Cortex-A65x4``
-  ``FVP_Base_Cortex-A65AEx8``
-  ``FVP_Base_Cortex-A72x4-A53x4``
-  ``FVP_Base_Cortex-A72x4``
-  ``FVP_Base_Cortex-A73x4-A53x4``
-  ``FVP_Base_Cortex-A73x4``
-  ``FVP_Base_Cortex-A75x4``
-  ``FVP_Base_Cortex-A76x4``
-  ``FVP_Base_Cortex-A76AEx4``
-  ``FVP_Base_Cortex-A76AEx8``
-  ``FVP_Base_Cortex-A77x4``
-  ``FVP_Base_Cortex-A78x4``
-  ``FVP_Base_Neoverse-E1x1``
-  ``FVP_Base_Neoverse-E1x2``
-  ``FVP_Base_Neoverse-E1x4``
-  ``FVP_Base_Neoverse-N1x4``
-  ``FVP_Base_Neoverse-V1x4``
-  ``FVP_CSS_SGI-575``     (Version 11.10 build 36)
-  ``FVP_CSS_SGM-775``
-  ``FVP_RD_E1_edge``      (Version 11.9 build 41)
-  ``FVP_RD_N1_edge``      (Version 11.10 build 36)
-  ``FVP_RD_N1_edge_dual`` (Version 11.10 build 36)
-  ``FVP_RD_Daniel``       (Version 11.10 build 36)
-  ``FVP_TC0``             (Version 0.0 build 6114)
-  ``Foundation_Platform``

The latest version of the AArch32 build of TF-A has been tested on the
following Arm FVPs without shifted affinities, and that do not support threaded
CPU cores (64-bit host machine only).

-  ``FVP_Base_AEMvA``
-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A32x4``

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` FVP only supports shifted affinities, which
   is not compatible with legacy GIC configurations. Therefore this FVP does not
   support these legacy GIC configurations.

The *Foundation* and *Base* FVPs can be downloaded free of charge. See the `Arm
FVP website`_. The Cortex-A models listed above are also available to download
from `Arm's website`_.

.. note::
   The build numbers quoted above are those reported by launching the FVP
   with the ``--version`` parameter.

.. note::
   Linaro provides a ramdisk image in prebuilt FVP configurations and full
   file systems that can be downloaded separately. To run an FVP with a virtio
   file system image an additional FVP configuration option
   ``-C bp.virtioblockdevice.image_path="<path-to>/<file-system-image>`` can be
   used.

.. note::
   The software will not work on Version 1.0 of the Foundation FVP.
   The commands below would report an ``unhandled argument`` error in this case.

.. note::
   FVPs can be launched with ``--cadi-server`` option such that a
   CADI-compliant debugger (for example, Arm DS-5) can connect to and control
   its execution.

.. warning::
   Since FVP model Version 11.0 Build 11.0.34 and Version 8.5 Build 0.8.5202
   the internal synchronisation timings changed compared to older versions of
   the models. The models can be launched with ``-Q 100`` option if they are
   required to match the run time characteristics of the older versions.

All the above platforms have been tested with `Linaro Release 19.06`_.

.. _build_options_arm_fvp_platform:

Arm FVP Platform Specific Build Options
---------------------------------------

-  ``FVP_CLUSTER_COUNT`` : Configures the cluster count to be used to
   build the topology tree within TF-A. By default TF-A is configured for dual
   cluster topology and this option can be used to override the default value.

-  ``FVP_INTERCONNECT_DRIVER``: Selects the interconnect driver to be built. The
   default interconnect driver depends on the value of ``FVP_CLUSTER_COUNT`` as
   explained in the options below:

   -  ``FVP_CCI`` : The CCI driver is selected. This is the default
      if 0 < ``FVP_CLUSTER_COUNT`` <= 2.
   -  ``FVP_CCN`` : The CCN driver is selected. This is the default
      if ``FVP_CLUSTER_COUNT`` > 2.

-  ``FVP_MAX_CPUS_PER_CLUSTER``: Sets the maximum number of CPUs implemented in
   a single cluster.  This option defaults to 4.

-  ``FVP_MAX_PE_PER_CPU``: Sets the maximum number of PEs implemented on any CPU
   in the system. This option defaults to 1. Note that the build option
   ``ARM_PLAT_MT`` doesn't have any effect on FVP platforms.

-  ``FVP_USE_GIC_DRIVER`` : Selects the GIC driver to be built. Options:

   -  ``FVP_GICV2`` : The GICv2 only driver is selected
   -  ``FVP_GICV3`` : The GICv3 only driver is selected (default option)

-  ``FVP_HW_CONFIG_DTS`` : Specify the path to the DTS file to be compiled
   to DTB and packaged in FIP as the HW_CONFIG. See :ref:`Firmware Design` for
   details on HW_CONFIG. By default, this is initialized to a sensible DTS
   file in ``fdts/`` folder depending on other build options. But some cases,
   like shifted affinity format for MPIDR, cannot be detected at build time
   and this option is needed to specify the appropriate DTS file.

-  ``FVP_HW_CONFIG`` : Specify the path to the HW_CONFIG blob to be packaged in
   FIP. See :ref:`Firmware Design` for details on HW_CONFIG. This option is
   similar to the ``FVP_HW_CONFIG_DTS`` option, but it directly specifies the
   HW_CONFIG blob instead of the DTS file. This option is useful to override
   the default HW_CONFIG selected by the build system.

Booting Firmware Update images
------------------------------

When Firmware Update (FWU) is enabled there are at least 2 new images
that have to be loaded, the Non-Secure FWU ROM (NS-BL1U), and the
FWU FIP.

The additional fip images must be loaded with:

::

    --data cluster0.cpu0="<path_to>/ns_bl1u.bin"@0x0beb8000	[ns_bl1u_base_address]
    --data cluster0.cpu0="<path_to>/fwu_fip.bin"@0x08400000	[ns_bl2u_base_address]

The address ns_bl1u_base_address is the value of NS_BL1U_BASE.
In the same way, the address ns_bl2u_base_address is the value of
NS_BL2U_BASE.

Booting an EL3 payload
----------------------

The EL3 payloads boot flow requires the CPU's mailbox to be cleared at reset for
the secondary CPUs holding pen to work properly. Unfortunately, its reset value
is undefined on the FVP platform and the FVP platform code doesn't clear it.
Therefore, one must modify the way the model is normally invoked in order to
clear the mailbox at start-up.

One way to do that is to create an 8-byte file containing all zero bytes using
the following command:

.. code:: shell

    dd if=/dev/zero of=mailbox.dat bs=1 count=8

and pre-load it into the FVP memory at the mailbox address (i.e. ``0x04000000``)
using the following model parameters:

::

    --data cluster0.cpu0=mailbox.dat@0x04000000   [Base FVPs]
    --data=mailbox.dat@0x04000000                 [Foundation FVP]

To provide the model with the EL3 payload image, the following methods may be
used:

#. If the EL3 payload is able to execute in place, it may be programmed into
   flash memory. On Base Cortex and AEM FVPs, the following model parameter
   loads it at the base address of the NOR FLASH1 (the NOR FLASH0 is already
   used for the FIP):

   ::

       -C bp.flashloader1.fname="<path-to>/<el3-payload>"

   On Foundation FVP, there is no flash loader component and the EL3 payload
   may be programmed anywhere in flash using method 3 below.

#. When using the ``SPIN_ON_BL1_EXIT=1`` loading method, the following DS-5
   command may be used to load the EL3 payload ELF image over JTAG:

   ::

       load <path-to>/el3-payload.elf

#. The EL3 payload may be pre-loaded in volatile memory using the following
   model parameters:

   ::

       --data cluster0.cpu0="<path-to>/el3-payload>"@address   [Base FVPs]
       --data="<path-to>/<el3-payload>"@address                [Foundation FVP]

   The address provided to the FVP must match the ``EL3_PAYLOAD_BASE`` address
   used when building TF-A.

Booting a preloaded kernel image (Base FVP)
-------------------------------------------

The following example uses a simplified boot flow by directly jumping from the
TF-A to the Linux kernel, which will use a ramdisk as filesystem. This can be
useful if both the kernel and the device tree blob (DTB) are already present in
memory (like in FVP).

For example, if the kernel is loaded at ``0x80080000`` and the DTB is loaded at
address ``0x82000000``, the firmware can be built like this:

.. code:: shell

    CROSS_COMPILE=aarch64-none-elf-  \
    make PLAT=fvp DEBUG=1             \
    RESET_TO_BL31=1                   \
    ARM_LINUX_KERNEL_AS_BL33=1        \
    PRELOADED_BL33_BASE=0x80080000    \
    ARM_PRELOADED_DTB_BASE=0x82000000 \
    all fip

Now, it is needed to modify the DTB so that the kernel knows the address of the
ramdisk. The following script generates a patched DTB from the provided one,
assuming that the ramdisk is loaded at address ``0x84000000``. Note that this
script assumes that the user is using a ramdisk image prepared for U-Boot, like
the ones provided by Linaro. If using a ramdisk without this header,the ``0x40``
offset in ``INITRD_START`` has to be removed.

.. code:: bash

    #!/bin/bash

    # Path to the input DTB
    KERNEL_DTB=<path-to>/<fdt>
    # Path to the output DTB
    PATCHED_KERNEL_DTB=<path-to>/<patched-fdt>
    # Base address of the ramdisk
    INITRD_BASE=0x84000000
    # Path to the ramdisk
    INITRD=<path-to>/<ramdisk.img>

    # Skip uboot header (64 bytes)
    INITRD_START=$(printf "0x%x" $((${INITRD_BASE} + 0x40)) )
    INITRD_SIZE=$(stat -Lc %s ${INITRD})
    INITRD_END=$(printf "0x%x" $((${INITRD_BASE} + ${INITRD_SIZE})) )

    CHOSEN_NODE=$(echo                                        \
    "/ {                                                      \
            chosen {                                          \
                    linux,initrd-start = <${INITRD_START}>;   \
                    linux,initrd-end = <${INITRD_END}>;       \
            };                                                \
    };")

    echo $(dtc -O dts -I dtb ${KERNEL_DTB}) ${CHOSEN_NODE} |  \
            dtc -O dtb -o ${PATCHED_KERNEL_DTB} -

And the FVP binary can be run with the following command:

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.RVBAR=0x04001000                           \
    -C cluster0.cpu1.RVBAR=0x04001000                           \
    -C cluster0.cpu2.RVBAR=0x04001000                           \
    -C cluster0.cpu3.RVBAR=0x04001000                           \
    -C cluster1.cpu0.RVBAR=0x04001000                           \
    -C cluster1.cpu1.RVBAR=0x04001000                           \
    -C cluster1.cpu2.RVBAR=0x04001000                           \
    -C cluster1.cpu3.RVBAR=0x04001000                           \
    --data cluster0.cpu0="<path-to>/bl31.bin"@0x04001000        \
    --data cluster0.cpu0="<path-to>/<patched-fdt>"@0x82000000   \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk.img>"@0x84000000

Obtaining the Flattened Device Trees
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Depending on the FVP configuration and Linux configuration used, different
FDT files are required. FDT source files for the Foundation and Base FVPs can
be found in the TF-A source directory under ``fdts/``. The Foundation FVP has
a subset of the Base FVP components. For example, the Foundation FVP lacks
CLCD and MMC support, and has only one CPU cluster.

.. note::
   It is not recommended to use the FDTs built along the kernel because not
   all FDTs are available from there.

The dynamic configuration capability is enabled in the firmware for FVPs.
This means that the firmware can authenticate and load the FDT if present in
FIP. A default FDT is packaged into FIP during the build based on
the build configuration. This can be overridden by using the ``FVP_HW_CONFIG``
or ``FVP_HW_CONFIG_DTS`` build options (refer to
:ref:`build_options_arm_fvp_platform` for details on the options).

-  ``fvp-base-gicv2-psci.dts``

   For use with models such as the Cortex-A57-A53 Base FVPs without shifted
   affinities and with Base memory map configuration.

-  ``fvp-base-gicv2-psci-aarch32.dts``

   For use with models such as the Cortex-A32 Base FVPs without shifted
   affinities and running Linux in AArch32 state with Base memory map
   configuration.

-  ``fvp-base-gicv3-psci.dts``

   For use with models such as the Cortex-A57-A53 Base FVPs without shifted
   affinities and with Base memory map configuration and Linux GICv3 support.

-  ``fvp-base-gicv3-psci-1t.dts``

   For use with models such as the AEMv8-RevC Base FVP with shifted affinities,
   single threaded CPUs, Base memory map configuration and Linux GICv3 support.

-  ``fvp-base-gicv3-psci-dynamiq.dts``

   For use with models as the Cortex-A55-A75 Base FVPs with shifted affinities,
   single cluster, single threaded CPUs, Base memory map configuration and Linux
   GICv3 support.

-  ``fvp-base-gicv3-psci-aarch32.dts``

   For use with models such as the Cortex-A32 Base FVPs without shifted
   affinities and running Linux in AArch32 state with Base memory map
   configuration and Linux GICv3 support.

-  ``fvp-foundation-gicv2-psci.dts``

   For use with Foundation FVP with Base memory map configuration.

-  ``fvp-foundation-gicv3-psci.dts``

   (Default) For use with Foundation FVP with Base memory map configuration
   and Linux GICv3 support.


Running on the Foundation FVP with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``Foundation_Platform`` parameters should be used to boot Linux with
4 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/Foundation_Platform                   \
    --cores=4                                       \
    --arm-v8.0                                      \
    --secure-memory                                 \
    --visualization                                 \
    --gicv3                                         \
    --data="<path-to>/<bl1-binary>"@0x0             \
    --data="<path-to>/<FIP-binary>"@0x08000000      \
    --data="<path-to>/<kernel-binary>"@0x80080000   \
    --data="<path-to>/<ramdisk-binary>"@0x84000000

Notes:

-  BL1 is loaded at the start of the Trusted ROM.
-  The Firmware Image Package is loaded at the start of NOR FLASH0.
-  The firmware loads the FDT packaged in FIP to the DRAM. The FDT load address
   is specified via the ``hw_config_addr`` property in `TB_FW_CONFIG for FVP`_.
-  The default use-case for the Foundation FVP is to use the ``--gicv3`` option
   and enable the GICv3 device in the model. Note that without this option,
   the Foundation FVP defaults to legacy (Versatile Express) memory map which
   is not supported by TF-A.
-  In order for TF-A to run correctly on the Foundation FVP, the architecture
   versions must match. The Foundation FVP defaults to the highest v8.x
   version it supports but the default build for TF-A is for v8.0. To avoid
   issues either start the Foundation FVP to use v8.0 architecture using the
   ``--arm-v8.0`` option, or build TF-A with an appropriate value for
   ``ARM_ARCH_MINOR``.

Running on the AEMv8 Base FVP with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_RevC-2xAEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_RevC-2xAEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

.. note::
   The ``FVP_Base_RevC-2xAEMv8A`` has shifted affinities and requires
   a specific DTS for all the CPUs to be loaded.

Running on the AEMv8 Base FVP (AArch32) with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cluster0.NUM_CORES=4                                     \
    -C cluster1.NUM_CORES=4                                     \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.CONFIG64=0                                 \
    -C cluster0.cpu1.CONFIG64=0                                 \
    -C cluster0.cpu2.CONFIG64=0                                 \
    -C cluster0.cpu3.CONFIG64=0                                 \
    -C cluster1.cpu0.CONFIG64=0                                 \
    -C cluster1.cpu1.CONFIG64=0                                 \
    -C cluster1.cpu2.CONFIG64=0                                 \
    -C cluster1.cpu3.CONFIG64=0                                 \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A57-A53 Base FVP with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A57x4-A53x4`` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                       \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A32 Base FVP (AArch32) with reset to BL1 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C bp.secureflashloader.fname="<path-to>/<bl1-binary>"      \
    -C bp.flashloader0.fname="<path-to>/<FIP-binary>"           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000


Running on the AEMv8 Base FVP with reset to BL31 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_RevC-2xAEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_RevC-2xAEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBAR=0x04010000                            \
    -C cluster0.cpu1.RVBAR=0x04010000                            \
    -C cluster0.cpu2.RVBAR=0x04010000                            \
    -C cluster0.cpu3.RVBAR=0x04010000                            \
    -C cluster1.cpu0.RVBAR=0x04010000                            \
    -C cluster1.cpu1.RVBAR=0x04010000                            \
    -C cluster1.cpu2.RVBAR=0x04010000                            \
    -C cluster1.cpu3.RVBAR=0x04010000                            \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04010000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0xff000000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Notes:

-  If Position Independent Executable (PIE) support is enabled for BL31
   in this config, it can be loaded at any valid address for execution.

-  Since a FIP is not loaded when using BL31 as reset entrypoint, the
   ``--data="<path-to><bl31|bl32|bl33-binary>"@<base-address-of-binary>``
   parameter is needed to load the individual bootloader images in memory.
   BL32 image is only needed if BL31 has been built to expect a Secure-EL1
   Payload. For the same reason, the FDT needs to be compiled from the DT source
   and loaded via the ``--data cluster0.cpu0="<path-to>/<fdt>"@0x82000000``
   parameter.

-  The ``FVP_Base_RevC-2xAEMv8A`` has shifted affinities and requires a
   specific DTS for all the CPUs to be loaded.

-  The ``-C cluster<X>.cpu<Y>.RVBAR=@<base-address-of-bl31>`` parameter, where
   X and Y are the cluster and CPU numbers respectively, is used to set the
   reset vector for each core.

-  Changing the default value of ``ARM_TSP_RAM_LOCATION`` will also require
   changing the value of
   ``--data="<path-to><bl32-binary>"@<base-address-of-bl32>`` to the new value of
   ``BL32_BASE``.


Running on the AEMv8 Base FVP (AArch32) with reset to SP_MIN entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_AEMv8A-AEMv8A`` parameters should be used to boot Linux
with 8 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                             \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cluster0.NUM_CORES=4                                      \
    -C cluster1.NUM_CORES=4                                      \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.CONFIG64=0                                  \
    -C cluster0.cpu1.CONFIG64=0                                  \
    -C cluster0.cpu2.CONFIG64=0                                  \
    -C cluster0.cpu3.CONFIG64=0                                  \
    -C cluster1.cpu0.CONFIG64=0                                  \
    -C cluster1.cpu1.CONFIG64=0                                  \
    -C cluster1.cpu2.CONFIG64=0                                  \
    -C cluster1.cpu3.CONFIG64=0                                  \
    -C cluster0.cpu0.RVBAR=0x04002000                            \
    -C cluster0.cpu1.RVBAR=0x04002000                            \
    -C cluster0.cpu2.RVBAR=0x04002000                            \
    -C cluster0.cpu3.RVBAR=0x04002000                            \
    -C cluster1.cpu0.RVBAR=0x04002000                            \
    -C cluster1.cpu1.RVBAR=0x04002000                            \
    -C cluster1.cpu2.RVBAR=0x04002000                            \
    -C cluster1.cpu3.RVBAR=0x04002000                            \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04002000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

.. note::
   The load address of ``<bl32-binary>`` depends on the value ``BL32_BASE``.
   It should match the address programmed into the RVBAR register as well.

Running on the Cortex-A57-A53 Base FVP with reset to BL31 entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A57x4-A53x4`` model parameters should be used to
boot Linux with 8 CPUs using the AArch64 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A57x4-A53x4                        \
    -C pctl.startup=0.0.0.0                                      \
    -C bp.secure_memory=1                                        \
    -C bp.tzc_400.diagnostics=1                                  \
    -C cache_state_modelled=1                                    \
    -C cluster0.cpu0.RVBARADDR=0x04010000                        \
    -C cluster0.cpu1.RVBARADDR=0x04010000                        \
    -C cluster0.cpu2.RVBARADDR=0x04010000                        \
    -C cluster0.cpu3.RVBARADDR=0x04010000                        \
    -C cluster1.cpu0.RVBARADDR=0x04010000                        \
    -C cluster1.cpu1.RVBARADDR=0x04010000                        \
    -C cluster1.cpu2.RVBARADDR=0x04010000                        \
    -C cluster1.cpu3.RVBARADDR=0x04010000                        \
    --data cluster0.cpu0="<path-to>/<bl31-binary>"@0x04010000    \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0xff000000    \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000    \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000            \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000  \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

Running on the Cortex-A32 Base FVP (AArch32) with reset to SP_MIN entrypoint
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following ``FVP_Base_Cortex-A32x4`` model parameters should be used to
boot Linux with 4 CPUs using the AArch32 build of TF-A.

.. code:: shell

    <path-to>/FVP_Base_Cortex-A32x4                             \
    -C pctl.startup=0.0.0.0                                     \
    -C bp.secure_memory=1                                       \
    -C bp.tzc_400.diagnostics=1                                 \
    -C cache_state_modelled=1                                   \
    -C cluster0.cpu0.RVBARADDR=0x04002000                       \
    -C cluster0.cpu1.RVBARADDR=0x04002000                       \
    -C cluster0.cpu2.RVBARADDR=0x04002000                       \
    -C cluster0.cpu3.RVBARADDR=0x04002000                       \
    --data cluster0.cpu0="<path-to>/<bl32-binary>"@0x04002000   \
    --data cluster0.cpu0="<path-to>/<bl33-binary>"@0x88000000   \
    --data cluster0.cpu0="<path-to>/<fdt>"@0x82000000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<ramdisk>"@0x84000000

--------------

*Copyright (c) 2019-2020, Arm Limited. All rights reserved.*

.. _TB_FW_CONFIG for FVP: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/tree/plat/arm/board/fvp/fdts/fvp_tb_fw_config.dts
.. _Arm's website: `FVP models`_
.. _FVP models: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Linaro Release 19.06: http://releases.linaro.org/members/arm/platforms/19.06
.. _Arm FVP website: https://developer.arm.com/products/system-design/fixed-virtual-platforms
