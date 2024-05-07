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

   For use with models such as the Cortex-A57-A53 or Cortex-A32 Base FVPs
   without shifted affinities and with Base memory map configuration.

-  ``fvp-base-gicv3-psci.dts``

   For use with models such as the Cortex-A57-A53 or Cortex-A32 Base FVPs
   without shifted affinities and with Base memory map configuration and
   Linux GICv3 support.

-  ``fvp-base-gicv3-psci-1t.dts``

   For use with models such as the AEMv8-RevC Base FVP with shifted affinities,
   single threaded CPUs, Base memory map configuration and Linux GICv3 support.

-  ``fvp-base-gicv3-psci-dynamiq.dts``

   For use with models as the Cortex-A55-A75 Base FVPs with shifted affinities,
   single cluster, single threaded CPUs, Base memory map configuration and Linux
   GICv3 support.

-  ``fvp-foundation-gicv2-psci.dts``

   For use with Foundation FVP with Base memory map configuration.

-  ``fvp-foundation-gicv3-psci.dts``

   (Default) For use with Foundation FVP with Base memory map configuration
   and Linux GICv3 support.

--------------

*Copyright (c) 2019-2024, Arm Limited. All rights reserved.*
