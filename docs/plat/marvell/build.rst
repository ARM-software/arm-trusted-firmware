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

Set MSS/SCP image path (mandatory only for Armada80x0)

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

- DEBUG

        Default is without debug information (=0). in order to enable it use ``DEBUG=1``.
        Must be disabled when building UART recovery images due to current console driver
        implementation that is not compatible with Xmodem protocol used for boot image download.

- LOG_LEVEL

        Defines the level of logging which will be purged to the default output port.

        LOG_LEVEL_NONE		0
        LOG_LEVEL_ERROR		10
        LOG_LEVEL_NOTICE	20
        LOG_LEVEL_WARNING	30
        LOG_LEVEL_INFO		40
        LOG_LEVEL_VERBOSE	50

- USE_COHERENT_MEM

        This flag determines whether to include the coherent memory region in the
        BL memory map or not.

- LLC_ENABLE

        Flag defining the LLC (L3) cache state. The cache is enabled by default (``LLC_ENABLE=1``).

- MARVELL_SECURE_BOOT

        Build trusted(=1)/non trusted(=0) image, default is non trusted.

- BLE_PATH

        Points to BLE (Binary ROM extension) sources folder. Only required for A8K builds.
        The parameter is optional, its default value is ``plat/marvell/a8k/common/ble``.

- MV_DDR_PATH

        For A7/8K, use this parameter to point to mv_ddr driver sources to allow BLE build. For A37x0,
        it is used for ddr_tool build.

        Usage example: MV_DDR_PATH=path/to/mv_ddr

        The parameter is optional for A7/8K, when this parameter is not set, the mv_ddr
        sources are expected to be located at: drivers/marvell/mv_ddr. However, the parameter
        is necessary for A37x0.

        For the mv_ddr source location, check the section "Tools and external components installation"

- DDR_TOPOLOGY

        For Armada37x0 only, the DDR topology map index/name, default is 0.

        Supported Options:
            - DDR3 1CS (0): DB-88F3720-DDR3-Modular (512MB); EspressoBIN (512MB)
            - DDR4 1CS (1): DB-88F3720-DDR4-Modular (512MB)
            - DDR3 2CS (2): EspressoBIN V3-V5 (1GB)
            - DDR4 2CS (3): DB-88F3720-DDR4-Modular (4GB)
            - DDR3 1CS (4): DB-88F3720-DDR3-Modular (1GB)
            - DDR4 1CS (5): EspressoBin V7 (1GB)
            - DDR4 2CS (6): EspressoBin V7 (2GB)
            - CUSTOMER (CUST): Customer board, DDR3 1CS 512MB

- CLOCKSPRESET

        For Armada37x0 only, the clock tree configuration preset including CPU and DDR frequency,
        default is CPU_800_DDR_800.

            - CPU_600_DDR_600	-	CPU at 600 MHz, DDR at 600 MHz
            - CPU_800_DDR_800	-	CPU at 800 MHz, DDR at 800 MHz
            - CPU_1000_DDR_800	-	CPU at 1000 MHz, DDR at 800 MHz
            - CPU_1200_DDR_750	-	CPU at 1200 MHz, DDR at 750 MHz

- BOOTDEV

        For Armada37x0 only, the flash boot device, default is ``SPINOR``.

        Currently, Armada37x0 only supports ``SPINOR``, ``SPINAND``, ``EMMCNORM`` and ``SATA``:

            - SPINOR - SPI NOR flash boot
            - SPINAND - SPI NAND flash boot
            - EMMCNORM - eMMC Download Mode

                Download boot loader or program code from eMMC flash into CM3 or CA53
                Requires full initialization and command sequence

            - SATA - SATA device boot

- PARTNUM

        For Armada37x0 only, the boot partition number, default is 0.

        To boot from eMMC, the value should be aligned with the parameter in
        U-Boot with name of ``CONFIG_SYS_MMC_ENV_PART``, whose value by default is
        1. For details about CONFIG_SYS_MMC_ENV_PART, please refer to the U-Boot
        build instructions.

- WTMI_IMG

        For Armada37x0 only, the path of the WTMI image can point to an image which
        does nothing, an image which supports EFUSE or a customized CM3 firmware
        binary. The default image is wtmi.bin that built from sources in WTP
        folder, which is the next option. If the default image is OK, then this
        option should be skipped.

- WTP

    For Armada37x0 only, use this parameter to point to wtptools source code
    directory, which can be found as a3700_utils.zip in the release. Usage
    example: ``WTP=/path/to/a3700_utils``

    For example, in order to build the image in debug mode with log level up to 'notice' level run

    .. code:: shell

        > make DEBUG=1 USE_COHERENT_MEM=0 LOG_LEVEL=20 PLAT=<MARVELL_PLATFORM> all fip

    And if we want to build a Armada37x0 image in debug mode with log level up to 'notice' level,
    the image has the preset CPU at 1000 MHz, preset DDR3 at 800 MHz, the DDR topology of DDR4 2CS,
    the image boot from SPI NOR flash partition 0, and the image is non trusted in WTP, the command
    line is as following

    .. code:: shell

        > make DEBUG=1 USE_COHERENT_MEM=0 LOG_LEVEL=20 CLOCKSPRESET=CPU_1000_DDR_800 \
            MARVELL_SECURE_BOOT=0 DDR_TOPOLOGY=3 BOOTDEV=SPINOR PARTNUM=0 PLAT=a3700 all fip

    Supported MARVELL_PLATFORM are:
        - a3700 (for both A3720 DB and EspressoBin)
        - a70x0
        - a70x0_amc (for AMC board)
        - a80x0
        - a80x0_mcbin (for MacciatoBin)

Special Build Flags
--------------------

- PLAT_RECOVERY_IMAGE_ENABLE
    When set this option to enable secondary recovery function when build atf.
    In order to build UART recovery image this operation should be disabled for
    a70x0 and a80x0 because of hardware limitation (boot from secondary image
    can interrupt UART recovery process). This MACRO definition is set in
    ``plat/marvell/a8k/common/include/platform_def.h`` file.

For more information about build options, please refer to the
:ref:`Build Options` document.


Build output
------------
Marvell's TF-A compilation generates 7 files:

    - ble.bin		- BLe image
    - bl1.bin		- BL1 image
    - bl2.bin		- BL2 image
    - bl31.bin		- BL31 image
    - fip.bin		- FIP image (contains BL2, BL31 & BL33 (U-Boot) images)
    - boot-image.bin	- TF-A image (contains BL1 and FIP images)
    - flash-image.bin	- Image which contains boot-image.bin and SPL image.
      Should be placed on the boot flash/device.


Tools and external components installation
------------------------------------------

Armada37x0 Builds require installation of 3 components
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
    (use the "mv_ddr-armada-atf-mainline" branch):

    https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git

(3) Armada3700 tools available at the following repository (use the latest release branch):

    https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git

Armada70x0 and Armada80x0 Builds require installation of an additional component
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

(1) DDR initialization library sources (mv_ddr) available at the following repository
    (use the "mv_ddr-armada-atf-mainline" branch):

    https://github.com/MarvellEmbeddedProcessors/mv-ddr-marvell.git
