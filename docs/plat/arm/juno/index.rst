Arm Juno Development Platform
=============================

Platform-specific build options
-------------------------------

-  ``JUNO_TZMP1`` : Boolean option to configure Juno to be used for TrustZone
   Media Protection (TZ-MP1). Default value of this flag is 0.

Running software on Juno
------------------------

This version of TF-A has been tested on variants r0, r1 and r2 of Juno.

To execute the software stack on Juno, the version of the Juno board recovery
image indicated in the `Linaro Release Notes`_ must be installed. If you have an
earlier version installed or are unsure which version is installed, please
re-install the recovery image by following the
`Instructions for using Linaro's deliverables on Juno`_.

Preparing TF-A images
---------------------

After building TF-A, the files ``bl1.bin`` and ``fip.bin`` need copying to the
``SOFTWARE/`` directory of the Juno SD card.

Creating a Firmware Image Package (FIP)
---------------------------------------

This section provides Juno and FVP specific instructions to build Trusted
Firmware, obtain the additional required firmware, and pack it all together in
a single FIP binary. It assumes that a Linaro release has been installed.

.. note::
   Pre-built binaries for AArch32 are available from Linaro Release 16.12
   onwards. Before that release, pre-built binaries are only available for
   AArch64.

.. warning::
   Follow the full instructions for one platform before switching to a
   different one. Mixing instructions for different platforms may result in
   corrupted binaries.

.. warning::
   The uboot image downloaded by the Linaro workspace script does not always
   match the uboot image packaged as BL33 in the corresponding fip file. It is
   recommended to use the version that is packaged in the fip file using the
   instructions below.

.. note::
   For the FVP, the kernel FDT is packaged in FIP during build and loaded
   by the firmware at runtime.

#. Clean the working directory

   .. code:: shell

       make realclean

#. Obtain SCP_BL2 (Juno) and BL33 (all platforms)

   Use the fiptool to extract the SCP_BL2 and BL33 images from the FIP
   package included in the Linaro release:

   .. code:: shell

       # Build the fiptool
       make [DEBUG=1] [V=1] fiptool

       # Unpack firmware images from Linaro FIP
       ./tools/fiptool/fiptool unpack <path-to-linaro-release>/[SOFTWARE]/fip.bin

   The unpack operation will result in a set of binary images extracted to the
   current working directory. The SCP_BL2 image corresponds to
   ``scp-fw.bin`` and BL33 corresponds to ``nt-fw.bin``.

   .. note::
      The fiptool will complain if the images to be unpacked already
      exist in the current directory. If that is the case, either delete those
      files or use the ``--force`` option to overwrite.

   .. note::
      For AArch32, the instructions below assume that nt-fw.bin is a
      normal world boot loader that supports AArch32.

#. Build TF-A images and create a new FIP for FVP

   .. code:: shell

       # AArch64
       make PLAT=fvp BL33=nt-fw.bin all fip

       # AArch32
       make PLAT=fvp ARCH=aarch32 AARCH32_SP=sp_min BL33=nt-fw.bin all fip

#. Build TF-A images and create a new FIP for Juno

   For AArch64:

   Building for AArch64 on Juno simply requires the addition of ``SCP_BL2``
   as a build parameter.

   .. code:: shell

       make PLAT=juno BL33=nt-fw.bin SCP_BL2=scp-fw.bin all fip

   For AArch32:

   Hardware restrictions on Juno prevent cold reset into AArch32 execution mode,
   therefore BL1 and BL2 must be compiled for AArch64, and BL32 is compiled
   separately for AArch32.

   -  Before building BL32, the environment variable ``CROSS_COMPILE`` must point
      to the AArch32 Linaro cross compiler.

      .. code:: shell

          export CROSS_COMPILE=<path-to-aarch32-gcc>/bin/arm-linux-gnueabihf-

   -  Build BL32 in AArch32.

      .. code:: shell

          make ARCH=aarch32 PLAT=juno AARCH32_SP=sp_min \
          RESET_TO_SP_MIN=1 JUNO_AARCH32_EL3_RUNTIME=1 bl32

   -  Save ``bl32.bin`` to a temporary location and clean the build products.

      ::

          cp <path-to-build>/bl32.bin <path-to-temporary>
          make realclean

   -  Before building BL1 and BL2, the environment variable ``CROSS_COMPILE``
      must point to the AArch64 Linaro cross compiler.

      .. code:: shell

          export CROSS_COMPILE=<path-to-aarch64-gcc>/bin/aarch64-none-elf-

   -  The following parameters should be used to build BL1 and BL2 in AArch64
      and point to the BL32 file.

      .. code:: shell

          make ARCH=aarch64 PLAT=juno JUNO_AARCH32_EL3_RUNTIME=1 \
          BL33=nt-fw.bin SCP_BL2=scp-fw.bin \
          BL32=<path-to-temporary>/bl32.bin all fip

The resulting BL1 and FIP images may be found in:

::

    # Juno
    ./build/juno/release/bl1.bin
    ./build/juno/release/fip.bin

    # FVP
    ./build/fvp/release/bl1.bin
    ./build/fvp/release/fip.bin


Booting Firmware Update images
------------------------------

The new images must be programmed in flash memory by adding
an entry in the ``SITE1/HBI0262x/images.txt`` configuration file
on the Juno SD card (where ``x`` depends on the revision of the Juno board).
Refer to the `Juno Getting Started Guide`_, section 2.3 "Flash memory
programming" for more information. User should ensure these do not
overlap with any other entries in the file.

::

        NOR10UPDATE: AUTO                       ;Image Update:NONE/AUTO/FORCE
        NOR10ADDRESS: 0x00400000                ;Image Flash Address [ns_bl2u_base_address]
        NOR10FILE: \SOFTWARE\fwu_fip.bin        ;Image File Name
        NOR10LOAD: 00000000                     ;Image Load Address
        NOR10ENTRY: 00000000                    ;Image Entry Point

        NOR11UPDATE: AUTO                       ;Image Update:NONE/AUTO/FORCE
        NOR11ADDRESS: 0x03EB8000                ;Image Flash Address [ns_bl1u_base_address]
        NOR11FILE: \SOFTWARE\ns_bl1u.bin        ;Image File Name
        NOR11LOAD: 00000000                     ;Image Load Address

The address ns_bl1u_base_address is the value of NS_BL1U_BASE - 0x8000000.
In the same way, the address ns_bl2u_base_address is the value of
NS_BL2U_BASE - 0x8000000.

.. _plat_juno_booting_el3_payload:

Booting an EL3 payload
----------------------

If the EL3 payload is able to execute in place, it may be programmed in flash
memory by adding an entry in the ``SITE1/HBI0262x/images.txt`` configuration file
on the Juno SD card (where ``x`` depends on the revision of the Juno board).
Refer to the `Juno Getting Started Guide`_, section 2.3 "Flash memory
programming" for more information.

Alternatively, the same DS-5 command mentioned in the FVP section above can
be used to load the EL3 payload's ELF file over JTAG on Juno.

For more information on EL3 payloads in general, see
:ref:`alt_boot_flows_el3_payload`.

Booting a preloaded kernel image
--------------------------------

The Trusted Firmware must be compiled in a similar way as for FVP explained
above. The process to load binaries to memory is the one explained in
`plat_juno_booting_el3_payload`_.

Testing System Suspend
----------------------

The SYSTEM SUSPEND is a PSCI API which can be used to implement system suspend
to RAM. For more details refer to section 5.16 of `PSCI`_. To test system suspend
on Juno, at the linux shell prompt, issue the following command:

.. code:: shell

    echo +10 > /sys/class/rtc/rtc0/wakealarm
    echo -n mem > /sys/power/state

The Juno board should suspend to RAM and then wakeup after 10 seconds due to
wakeup interrupt from RTC.

Additional Resources
--------------------

Please visit the `Arm Platforms Portal`_ to get support and obtain any other Juno
software information. Please also refer to the `Juno Getting Started Guide`_ to
get more detailed information about the Juno Arm development platform and how to
configure it.

--------------

*Copyright (c) 2019, Arm Limited. All rights reserved.*

.. _Linaro Release Notes: https://community.arm.com/dev-platforms/w/docs/226/old-release-notes
.. _Instructions for using Linaro's deliverables on Juno: https://community.arm.com/dev-platforms/w/docs/303/juno
.. _Arm Platforms Portal: https://community.arm.com/dev-platforms/
.. _Juno Getting Started Guide: http://infocenter.arm.com/help/topic/com.arm.doc.dui0928e/DUI0928E_juno_arm_development_platform_gsg.pdf
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _Juno Arm Development Platform: http://www.arm.com/products/tools/development-boards/versatile-express/juno-arm-development-platform.php
