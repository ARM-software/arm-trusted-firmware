Socionext Synquacer
===================

Socionext's Synquacer SC2A11 is a multi-core processor with 24 cores of Arm
Cortex-A53. The Developerbox, of 96boards, is a platform that contains this
processor. This port of the Trusted Firmware only supports this platform at
the moment.

More information are listed in `link`_.

How to build
------------

Code Locations
~~~~~~~~~~~~~~

-  Trusted Firmware-A:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  edk2:
   `link <https://github.com/tianocore/edk2>`__

-  edk2-platforms:
   `link <https://github.com/tianocore/edk2-platforms>`__

-  edk2-non-osi:
   `link <https://github.com/tianocore/edk2-non-osi>`__

Boot Flow
~~~~~~~~~

SCP firmware --> TF-A BL31 --> UEFI(edk2)

Build Procedure
~~~~~~~~~~~~~~~

-  Firstly, in addition to the “normal” build tools you will also need a
   few specialist tools. On a Debian or Ubuntu operating system try:

   .. code:: shell

       sudo apt install acpica-tools device-tree-compiler uuid-dev

-  Secondly, create a new working directory and store the absolute path to this
   directory in an environment variable, WORKSPACE. It does not matter where
   this directory is created but as an example:

   .. code:: shell

       export WORKSPACE=$HOME/build/developerbox-firmware
       mkdir -p $WORKSPACE

-  Run the following commands to clone the source code:

   .. code:: shell

       cd $WORKSPACE
       git clone https://github.com/ARM-software/arm-trusted-firmware -b master
       git clone https://github.com/tianocore/edk2.git -b master
       git clone https://github.com/tianocore/edk2-platforms.git -b master
       git clone https://github.com/tianocore/edk2-non-osi.git -b master

-  Build ATF:

   .. code:: shell

       cd $WORKSPACE/arm-trusted-firmware
       make -j`nproc` PLAT=synquacer PRELOADED_BL33_BASE=0x8200000 bl31 fiptool
       tools/fiptool/fiptool create \
             --tb-fw ./build/synquacer/release/bl31.bin \
             --soc-fw ./build/synquacer/release/bl31.bin \
             --scp-fw ./build/synquacer/release/bl31.bin \
             ../edk2-non-osi/Platform/Socionext/DeveloperBox/fip_all_arm_tf.bin

-  Build EDK2:

   .. code:: shell

       cd $WORKSPACE
       export PACKAGES_PATH=$WORKSPACE/edk2:$WORKSPACE/edk2-platforms:$WORKSPACE/edk2-non-osi
       export ACTIVE_PLATFORM="Platform/Socionext/DeveloperBox/DeveloperBox.dsc"
       export GCC5_AARCH64_PREFIX=aarch64-linux-gnu-
       unset ARCH

       . edk2/edksetup.sh
       make -C edk2/BaseTools

       build -p $ACTIVE_PLATFORM -b RELEASE -a AARCH64 -t GCC5 -n `nproc` -D DO_X86EMU=TRUE

-  The firmware image, which comprises the option ROM, ARM trusted firmware and
   EDK2 itself, can be found $WORKSPACE/../Build/DeveloperBox/RELEASE_GCC5/FV/.
   Use SYNQUACERFIRMWAREUPDATECAPSULEFMPPKCS7.Cap for UEFI capsule update and
   SPI_NOR_IMAGE.fd for the serial flasher.

   Note #1: -t GCC5 can be loosely translated as “enable link-time-optimization”;
   any version of gcc >= 5 will support this feature and may be used to build EDK2.

   Note #2: Replace -b RELEASE with -b DEBUG to build a debug.

Install the System Firmware
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Providing your Developerbox is fully working and has on operating system
   installed then you can adopt your the newly compiled system firmware using
   the capsule update method:.

   .. code:: shell

       sudo apt install fwupdate
       sudo fwupdate --apply {50b94ce5-8b63-4849-8af4-ea479356f0e3} \
                     SYNQUACERFIRMWAREUPDATECAPSULEFMPPKCS7.Cap
       sudo reboot

-  Alternatively you can install SPI_NOR_IMAGE.fd using the `board recovery method`_.

.. _link: https://www.96boards.org/product/developerbox/
.. _board recovery method: https://www.96boards.org/documentation/enterprise/developerbox/installation/board-recovery.md.html
