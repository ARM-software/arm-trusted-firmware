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

To provide the model with the EL3 payload image, the following methods may be
used:

#. If the EL3 payload is able to execute in place, it may be programmed into
   flash memory. On Base Cortex and AEM FVPs, the following model parameter
   loads it at the base address of the NOR FLASH1 (the NOR FLASH0 is already
   used for the FIP):

   ::

       -C bp.flashloader1.fname="<path-to>/<el3-payload>"

#. When using the ``SPIN_ON_BL1_EXIT=1`` loading method, the following DS-5
   command may be used to load the EL3 payload ELF image over JTAG:

   ::

       load <path-to>/el3-payload.elf

#. The EL3 payload may be pre-loaded in volatile memory using the following
   model parameters:

   ::

       --data cluster0.cpu0="<path-to>/el3-payload>"@address   [Base FVPs]

   The address provided to the FVP must match the ``EL3_PAYLOAD_BASE`` address
   used when building TF-A.

Booting a kernel image in BL33
------------------------------

TF-A can boot a Linux kernel, which uses a ramdisk as a filesystem. The
required initrd properties are injected in to the device tree blob (DTB) at
build time.

Kernel image packaged in fip as a BL33 image
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A Linux kernel image can be packaged in the fip as a BL33 image and then
booted in TF-A.

For example, the firmware can be built as:

.. code:: shell

    make PLAT=fvp DEBUG=1             \
    ARM_LINUX_KERNEL_AS_BL33          \
    BL33=<path-to-kernel-binary>      \
    INITRD_SIZE=0x8000000             \
    all fip

The options ``INITRD_SIZE`` or ``INITRD_PATH`` triggers the insertion of initrd
properties in to the DTB. ``INITRD_BASE`` is also required but a default value
is set by the FVP platform.

The options available here are:

::

    INITRD_BASE: Set the initrd base address in memory. Defaults to 0x90000000 in FVP.
    INITRD_SIZE: Set the initrd size in dec or hex format. Hex format must precede with '0x'.
    INITRD_PATH: Provide an initrd path for the build time to determine its exact size.

Users can provide either ``INITRD_SIZE`` or ``INITRD_PATH`` to set the initrd
size value. ``INITRD_SIZE`` takes prioty over ``INITRD_PATH``.

Now the fvp binary can be run as:

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C bp.secureflashloader.fname=<path-to>/bl1.bin             \
    -C bp.flashloader0.fname=<path-to>/fip.bin                  \
    --data cluster0.cpu0="<path-to>/<initrd.bin>"@0x90000000

.. note::
    Providing a higher value for an initrd size than the actual size of the file
    is supported but it will trigger a non-breaking "Initramfs unpacking failed"
    error by the kernel at runtime. This error can be ignored because initrd's
    can be stacked one after another, when the kernel unpacks the first initrd it
    looks for another in the extra space which it won't find, hence the error.

Preloaded kernel image - Normal flow
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following example uses a simplified boot flow to boot a Linux kernel
using TF-A. This can be useful if the kernel is already present in memory
(like in FVP).

For example, if the kernel is loaded at ``0x80080000`` the firmware can be
built like this:

.. code:: shell

    make PLAT=fvp DEBUG=1             \
    ARM_LINUX_KERNEL_AS_BL33=1        \
    PRELOADED_BL33_BASE=0x80080000    \
    INITRD_SIZE=0x8000000             \
    all fip

Now the FVP binary can be run with the following command:

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                            \
    -C bp.secureflashloader.fname=<path-to>/bl1.bin             \
    -C bp.flashloader0.fname=<path-to>/fip.bin                  \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000 \
    --data cluster0.cpu0="<path-to>/<initrd.bin>"@0x90000000

Preloaded kernel image - Reset to BL31
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We can also boot a Linux kernel by jumping directly to BL31 ``RESET_TO_BL31=1``.
This requires preloading a DTB into memory. We can inject the initrd start and
end properties into the DTB (HW_CONFIG) at build time which is then stored by
TF-A in ``build/fvp/<build-type>/fdts/`` directory.

For example, we can build the firmware as:

.. code:: shell

    make PLAT=fvp DEBUG=1                   \
    RESET_TO_BL31=1                         \
    ARM_LINUX_KERNEL_AS_BL33=1              \
    PRELOADED_BL33_BASE=0x80080000          \
    ARM_PRELOADED_DTB_BASE=0x87F00000       \
    INITRD_BASE=0x88000000                  \
    INITRD_PATH=<path-to>/initrd.bin

Now we can run the binary as:

.. code:: shell

    <path-to>/FVP_Base_AEMv8A-AEMv8A                               \
    -C cluster0.NUM_CORES=4                                        \
    -C cluster0.cpu0.RVBAR=0x04001000                              \
    -C cluster0.cpu1.RVBAR=0x04001000                              \
    -C cluster0.cpu2.RVBAR=0x04001000                              \
    -C cluster0.cpu3.RVBAR=0x04001000                              \
    --data cluster0.cpu0="<path-to>/bl31.bin"@0x04001000           \
    --data cluster0.cpu0="<path-to>/<kernel-binary>"@0x80080000    \
    --data cluster0.cpu0="<path-to>/<initrd.bin>"@0x88000000       \
    --data cluster0.cpu0="<path-to>/fdts/fvp-base-gicv3-psci.dtb"@87F00000

Obtaining the Flattened Device Trees
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Depending on the FVP configuration and Linux configuration used, different
FDT files are required.

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

GICv5 Support
^^^^^^^^^^^^^

GICv5 support in TF-A is currently **experimental** and provided only for early
development and testing purposes. A simplified build configuration is available
to allow booting the Linux kernel as a BL33 payload on the FVP platform.

Key notes:

- The support is **not production-ready** and is intended to assist with
  upstream kernel development and validation.
- The device tree bindings are **not finalized**
- Use this configuration at your own discretion, understanding that the design
  and register usage may change in future revisions.

This configuration is **temporary** and may be removed once full GICv5 support
is integrated upstream.

.. code:: shell

    make PLAT=fvp DEBUG=1                       \
    CTX_INCLUDE_AARCH32_REGS=0                  \
    FVP_USE_GIC_DRIVER=FVP_GICV5                \
    ARM_LINUX_KERNEL_AS_BL33=1                  \
    PRELOADED_BL33_BASE=0x84000000              \
    FVP_HW_CONFIG_DTS=<PROVIDE_YOUR_OWN_DT>     \

--------------

*Copyright (c) 2019-2025, Arm Limited. All rights reserved.*
