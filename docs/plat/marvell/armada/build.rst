TF-A Build Instructions for Marvell Platforms
=============================================

This section describes how to compile the Trusted Firmware-A (TF-A) project for Marvell's platforms.

Build Instructions
------------------
(1) Set the cross compiler

    .. code:: shell

        > export CROSS_COMPILE=/path/to/toolchain/aarch64-linux-gnu-

(2) Set path for FIP images:

Set U-Boot image path (relatively to TF-A root or absolute path)

    .. code:: shell

        > export BL33=path/to/u-boot.bin

For example: if U-Boot project (and its images) is located at ``~/project/u-boot``,
BL33 should be ``~/project/u-boot/u-boot.bin``

    .. note::

       *u-boot.bin* should be used and not *u-boot-spl.bin*

Set MSS/SCP image path (mandatory only for A7K/8K/CN913x when MSS_SUPPORT=1)

    .. code:: shell

        > export SCP_BL2=path/to/mrvl_scp_bl2*.img

(3) Armada-37x0 build requires WTP tools installation.

See below in the section "Tools and external components installation".
Install ARM 32-bit cross compiler, which is required for building WTMI image for CM3

    .. code:: shell

        > sudo apt-get install gcc-arm-linux-gnueabi

(4) Clean previous build residuals (if any)

    .. code:: shell

        > make distclean

(5) Build TF-A

There are several build options:

- PLAT

        Supported Marvell platforms are:

            - a3700        - A3720 DB, EspressoBin and Turris MOX
            - a70x0
            - a70x0_amc    - AMC board
            - a70x0_mochabin - Globalscale MOCHAbin
            - a80x0
            - a80x0_mcbin  - MacchiatoBin
            - a80x0_puzzle - IEI Puzzle-M801
            - t9130        - CN913x
            - t9130_cex7_eval - CN913x CEx7 Evaluation Board

- DEBUG

        Default is without debug information (=0). in order to enable it use ``DEBUG=1``.
        Must be disabled when building UART recovery images due to current console driver
        implementation that is not compatible with Xmodem protocol used for boot image download.

- LOG_LEVEL

        Defines the level of logging which will be purged to the default output port.

            -  0 - LOG_LEVEL_NONE
            - 10 - LOG_LEVEL_ERROR
            - 20 - LOG_LEVEL_NOTICE (default for DEBUG=0)
            - 30 - LOG_LEVEL_WARNING
            - 40 - LOG_LEVEL_INFO (default for DEBUG=1)
            - 50 - LOG_LEVEL_VERBOSE

- USE_COHERENT_MEM

        This flag determines whether to include the coherent memory region in the
        BL memory map or not. Enabled by default.

- LLC_ENABLE

        Flag defining the LLC (L3) cache state. The cache is enabled by default (``LLC_ENABLE=1``).

- LLC_SRAM

        Flag enabling the LLC (L3) cache SRAM support. The LLC SRAM is activated and used
        by Trusted OS (OP-TEE OS, BL32). The TF-A only prepares CCU address translation windows
        for SRAM address range at BL31 execution stage with window target set to DRAM-0.
        When Trusted OS activates LLC SRAM, the CCU window target is changed to SRAM.
        There is no reason to enable this feature if OP-TEE OS built with CFG_WITH_PAGER=n.
        Only set LLC_SRAM=1 if OP-TEE OS is built with CFG_WITH_PAGER=y.

- MARVELL_SECURE_BOOT

        Build trusted(=1)/non trusted(=0) image, default is non trusted.
        This parameter is used only for ``mrvl_flash`` and ``mrvl_uart`` targets.

- MV_DDR_PATH

        This parameter is required for ``mrvl_flash`` and ``mrvl_uart`` targets.
        For A7K/8K/CN913x it is used for BLE build and for Armada37x0 it used
        for ddr_tool build.

        Specify path to the full checkout of Marvell mv-ddr-marvell git
        repository. Checkout must contain also .git subdirectory because
        mv-ddr build process calls git commands.

        Do not remove any parts of git checkout becuase build process and other
        applications need them for correct building and version determination.


CN913x specific build options:

- CP_NUM

        Total amount of CPs (South Bridge) connected to AP. When the parameter is omitted,
        the build uses the default number of CPs, which is a number of embedded CPs inside the
        package: 1 or 2 depending on the SoC used. The parameter is valid for OcteonTX2 CN913x SoC
        family (PLAT=t9130), which can have external CPs connected to the MCI ports. Valid
        values with CP_NUM are in a range of 1 to 3.


A7K/8K/CN913x specific build options:

- BLE_PATH

        Points to BLE (Binary ROM extension) sources folder.
        The parameter is optional, its default value is ``plat/marvell/armada/a8k/common/ble``
        which uses TF-A in-tree BLE implementation.

- MSS_SUPPORT

        When ``MSS_SUPPORT=1``, then TF-A includes support for Management SubSystem (MSS).
        When enabled it is required to specify path to the MSS firmware image via ``SCP_BL2``
        option.

        This option is by default enabled.

- SCP_BL2

        Specify path to the MSS fimware image binary which will run on Cortex-M3 coprocessor.
        It is available in Marvell binaries-marvell git repository. Required when ``MSS_SUPPORT=1``.

Globalscale MOCHAbin specific build options:

- DDR_TOPOLOGY

        The DDR topology map index/name, default is 0.

        Supported Options:
            -    0 - DDR4 1CS 2GB
            -    1 - DDR4 1CS 4GB
            -    2 - DDR4 2CS 8GB

Armada37x0 specific build options:

- HANDLE_EA_EL3_FIRST

        When ``HANDLE_EA_EL3_FIRST=1``, External Aborts and SError Interrupts will be always trapped
        in TF-A. TF-A in this case enables dirty hack / workaround for a bug found in U-Boot and
        Linux kernel PCIe controller driver pci-aardvark.c, traps and then masks SError interrupt
        caused by AXI SLVERR on external access (syndrome 0xbf000002).

        Otherwise when ``HANDLE_EA_EL3_FIRST=0``, these exceptions will be trapped in the current
        exception level (or in EL1 if the current exception level is EL0). So exceptions caused by
        U-Boot will be trapped in U-Boot, exceptions caused by Linux kernel (or user applications)
        will be trapped in Linux kernel.

        Mentioned bug in pci-aardvark.c driver is fixed in U-Boot version v2021.07 and Linux kernel
        version v5.13 (workarounded since Linux kernel version 5.9) and also backported in Linux
        kernel stable releases since versions v5.12.13, v5.10.46, v5.4.128, v4.19.198, v4.14.240.

        If target system has already patched version of U-Boot and Linux kernel then it is strongly
        recommended to not enable this workaround as it disallows propagating of all External Aborts
        to running Linux kernel and makes correctable errors as fatal aborts.

        This option is now disabled by default. In past this option was enabled by default in
        TF-A versions v2.2, v2.3, v2.4 and v2.5.

- CM3_SYSTEM_RESET

        When ``CM3_SYSTEM_RESET=1``, the Cortex-M3 secure coprocessor will be used for system reset.

        TF-A will send command 0x0009 with a magic value via the rWTM mailbox interface to the
        Cortex-M3 secure coprocessor.
        The firmware running in the coprocessor must either implement this functionality or
        ignore the 0x0009 command (which is true for the firmware from A3700-utils-marvell
        repository). If this option is enabled but the firmware does not support this command,
        an error message will be printed prior trying to reboot via the usual way.

        This option is needed on Turris MOX as a workaround to a HW bug which causes reset to
        sometime hang the board.

- A3720_DB_PM_WAKEUP_SRC

        For Armada 3720 Development Board only, when ``A3720_DB_PM_WAKEUP_SRC=1``,
        TF-A will setup PM wake up src configuration. This option is disabled by default.


Armada37x0 specific build options for ``mrvl_flash`` and ``mrvl_uart`` targets:

- DDR_TOPOLOGY

        The DDR topology map index/name, default is 0.

        Supported Options:
            -    0 - DDR3 1CS 512MB (DB-88F3720-DDR3-Modular, EspressoBin V3-V5)
            -    1 - DDR4 1CS 512MB (DB-88F3720-DDR4-Modular)
            -    2 - DDR3 2CS   1GB (EspressoBin V3-V5)
            -    3 - DDR4 2CS   4GB (DB-88F3720-DDR4-Modular)
            -    4 - DDR3 1CS   1GB (DB-88F3720-DDR3-Modular, EspressoBin V3-V5)
            -    5 - DDR4 1CS   1GB (EspressoBin V7, EspressoBin-Ultra)
            -    6 - DDR4 2CS   2GB (EspressoBin V7)
            -    7 - DDR3 2CS   2GB (EspressoBin V3-V5)
            - CUST - CUSTOMER BOARD (Customer board settings)

- CLOCKSPRESET

        The clock tree configuration preset including CPU and DDR frequency,
        default is CPU_800_DDR_800.

            - CPU_600_DDR_600  - CPU at 600 MHz, DDR at 600 MHz
            - CPU_800_DDR_800  - CPU at 800 MHz, DDR at 800 MHz
            - CPU_1000_DDR_800 - CPU at 1000 MHz, DDR at 800 MHz
            - CPU_1200_DDR_750 - CPU at 1200 MHz, DDR at 750 MHz

        Look at Armada37x0 chip package marking on board to identify correct CPU frequency.
        The last line on package marking (next line after the 88F37x0 line) should contain:

            - C080 or I080 - chip with  800 MHz CPU - use ``CLOCKSPRESET=CPU_800_DDR_800``
            - C100 or I100 - chip with 1000 MHz CPU - use ``CLOCKSPRESET=CPU_1000_DDR_800``
            - C120         - chip with 1200 MHz CPU - use ``CLOCKSPRESET=CPU_1200_DDR_750``

- BOOTDEV

        The flash boot device, default is ``SPINOR``.

        Currently, Armada37x0 only supports ``SPINOR``, ``SPINAND``, ``EMMCNORM`` and ``SATA``:

            - SPINOR - SPI NOR flash boot
            - SPINAND - SPI NAND flash boot
            - EMMCNORM - eMMC Download Mode

                Download boot loader or program code from eMMC flash into CM3 or CA53
                Requires full initialization and command sequence

            - SATA - SATA device boot

                Image needs to be stored at disk LBA 0 or at disk partition with
                MBR type 0x4d (ASCII 'M' as in Marvell) or at disk partition with
                GPT name ``MARVELL BOOT PARTITION``.

- PARTNUM

        The boot partition number, default is 0.

        To boot from eMMC, the value should be aligned with the parameter in
        U-Boot with name of ``CONFIG_SYS_MMC_ENV_PART``, whose value by default is
        1. For details about CONFIG_SYS_MMC_ENV_PART, please refer to the U-Boot
        build instructions.

- WTMI_IMG

        The path of the binary can point to an image which
        does nothing, an image which supports EFUSE or a customized CM3 firmware
        binary. The default image is ``fuse.bin`` that built from sources in WTP
        folder, which is the next option. If the default image is OK, then this
        option should be skipped.

        Please note that this is not a full WTMI image, just a main loop without
        hardware initialization code. Final WTMI image is built from this WTMI_IMG
        binary and sys-init code from the WTP directory which sets DDR and CPU
        clocks according to DDR_TOPOLOGY and CLOCKSPRESET options.

        CZ.NIC as part of Turris project released free and open source WTMI
        application firmware ``wtmi_app.bin`` for all Armada 3720 devices.
        This firmware includes additional features like access to Hardware
        Random Number Generator of Armada 3720 SoC which original Marvell's
        ``fuse.bin`` image does not have.

        CZ.NIC's Armada 3720 Secure Firmware is available at website:

            https://gitlab.nic.cz/turris/mox-boot-builder/

- WTP

        Specify path to the full checkout of Marvell A3700-utils-marvell git
        repository. Checkout must contain also .git subdirectory because WTP
        build process calls git commands.

        WTP build process uses also Marvell mv-ddr-marvell git repository
        specified in MV_DDR_PATH option.

        Do not remove any parts of git checkout becuase build process and other
        applications need them for correct building and version determination.

- CRYPTOPP_PATH

        Use this parameter to point to Crypto++ source code
        directory. If this option is specified then Crypto++ source code in
        CRYPTOPP_PATH directory will be automatically compiled. Crypto++ library
        is required for building WTP image tool. Either CRYPTOPP_PATH or
        CRYPTOPP_LIBDIR with CRYPTOPP_INCDIR needs to be specified for Armada37x0.

- CRYPTOPP_LIBDIR

        Use this parameter to point to the directory with
        compiled Crypto++ library. By default it points to the CRYPTOPP_PATH.

- CRYPTOPP_INCDIR

        Use this parameter to point to the directory with
        header files of Crypto++ library. By default it points to the CRYPTOPP_PATH.


For example, in order to build the image in debug mode with log level up to 'notice' level run

.. code:: shell

    > make DEBUG=1 USE_COHERENT_MEM=0 LOG_LEVEL=20 PLAT=<MARVELL_PLATFORM> mrvl_flash

And if we want to build a Armada37x0 image in debug mode with log level up to 'notice' level,
the image has the preset CPU at 1000 MHz, preset DDR3 at 800 MHz, the DDR topology of DDR4 2CS,
the image boot from SPI NOR flash partition 0, and the image is non trusted in WTP, the command
line is as following

.. code:: shell

    > make DEBUG=1 USE_COHERENT_MEM=0 LOG_LEVEL=20 CLOCKSPRESET=CPU_1000_DDR_800 \
        MARVELL_SECURE_BOOT=0 DDR_TOPOLOGY=3 BOOTDEV=SPINOR PARTNUM=0 PLAT=a3700 \
        MV_DDR_PATH=/path/to/mv-ddr-marvell/ WTP=/path/to/A3700-utils-marvell/ \
        CRYPTOPP_PATH=/path/to/cryptopp/ BL33=/path/to/u-boot.bin \
        all fip mrvl_bootimage mrvl_flash mrvl_uart

To build just TF-A without WTMI image (useful for A3720 Turris MOX board), run following command:

.. code:: shell

    > make USE_COHERENT_MEM=0 PLAT=a3700 CM3_SYSTEM_RESET=1 BL33=/path/to/u-boot.bin \
        CROSS_COMPILE=aarch64-linux-gnu- mrvl_bootimage

Here is full example how to build production release of Marvell firmware image (concatenated
binary of Marvell's A3720 sys-init, CZ.NIC's Armada 3720 Secure Firmware, TF-A and U-Boot) for
EspressoBin board (PLAT=a3700) with 1GHz CPU (CLOCKSPRESET=CPU_1000_DDR_800) and
1GB DDR4 RAM (DDR_TOPOLOGY=5):

.. code:: shell

    > git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    > git clone https://source.denx.de/u-boot/u-boot.git
    > git clone https://github.com/weidai11/cryptopp.git
    > git clone https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git
    > git clone https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
    > git clone https://gitlab.nic.cz/turris/mox-boot-builder.git
    > make -C u-boot CROSS_COMPILE=aarch64-linux-gnu- mvebu_espressobin-88f3720_defconfig u-boot.bin
    > make -C mox-boot-builder CROSS_CM3=arm-linux-gnueabi- wtmi_app.bin
    > make -C trusted-firmware-a CROSS_COMPILE=aarch64-linux-gnu- CROSS_CM3=arm-linux-gnueabi- \
        USE_COHERENT_MEM=0 PLAT=a3700 CLOCKSPRESET=CPU_1000_DDR_800 DDR_TOPOLOGY=5 \
        MV_DDR_PATH=$PWD/mv-ddr-marvell/ WTP=$PWD/A3700-utils-marvell/ \
        CRYPTOPP_PATH=$PWD/cryptopp/ BL33=$PWD/u-boot/u-boot.bin \
        WTMI_IMG=$PWD/mox-boot-builder/wtmi_app.bin FIP_ALIGN=0x100 mrvl_flash

Produced Marvell firmware flash image: ``trusted-firmware-a/build/a3700/release/flash-image.bin``

Special Build Flags
--------------------

- PLAT_RECOVERY_IMAGE_ENABLE
    When set this option to enable secondary recovery function when build atf.
    In order to build UART recovery image this operation should be disabled for
    A7K/8K/CN913x because of hardware limitation (boot from secondary image
    can interrupt UART recovery process). This MACRO definition is set in
    ``plat/marvell/armada/a8k/common/include/platform_def.h`` file.

- DDR32
    In order to work in 32bit DDR, instead of the default 64bit ECC DDR,
    this flag should be set to 1.

For more information about build options, please refer to the
:ref:`Build Options` document.


Build output
------------
Marvell's TF-A compilation generates 8 files:

    - ble.bin		- BLe image (not available for Armada37x0)
    - bl1.bin		- BL1 image
    - bl2.bin		- BL2 image
    - bl31.bin		- BL31 image
    - fip.bin		- FIP image (contains BL2, BL31 & BL33 (U-Boot) images)
    - boot-image.bin	- TF-A image (contains BL1 and FIP images)
    - flash-image.bin	- Flashable Marvell firmware image. For Armada37x0 it
      contains TIM, WTMI and boot-image.bin images. For other platforms it contains
      BLe and boot-image.bin images. Should be placed on the boot flash/device.
    - uart-images.tgz.bin - GZIPed TAR archive which contains Armada37x0 images
      for booting via UART. Could be loaded via Marvell's WtpDownload tool from
      A3700-utils-marvell repository.

Additional make target ``mrvl_bootimage`` produce ``boot-image.bin`` file. Target
``mrvl_flash`` produce final ``flash-image.bin`` file and target ``mrvl_uart``
produce ``uart-images.tgz.bin`` file.


Tools and external components installation
------------------------------------------

Armada37x0 Builds require installation of additional components
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(1) ARM cross compiler capable of building images for the service CPU (CM3).
    This component is usually included in the Linux host packages.
    On Debian/Ubuntu hosts the default GNU ARM tool chain can be installed
    using the following command

    .. code:: shell

        > sudo apt-get install gcc-arm-linux-gnueabi

    Only if required, the default tool chain prefix ``arm-linux-gnueabi-`` can be
    overwritten using the environment variable ``CROSS_CM3``.
    Example for BASH shell

    .. code:: shell

        > export CROSS_CM3=/opt/arm-cross/bin/arm-linux-gnueabi

(2) DDR initialization library sources (mv_ddr) available at the following repository
    (use the "master" branch):

    https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git

(3) Armada3700 tools available at the following repository
    (use the "master" branch):

    https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git

(4) Crypto++ library available at the following repository:

    https://github.com/weidai11/cryptopp.git

(5) Optional CZ.NIC's Armada 3720 Secure Firmware:

    https://gitlab.nic.cz/turris/mox-boot-builder.git

Armada70x0, Armada80x0 and CN913x Builds require installation of additional components
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(1) DDR initialization library sources (mv_ddr) available at the following repository
    (use the "master" branch):

    https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git

(2) MSS Management SubSystem Firmware available at the following repository
    (use the "binaries-marvell-armada-SDK10.0.1.0" branch):

    https://github.com/MarvellEmbeddedProcessors/binaries-marvell.git
