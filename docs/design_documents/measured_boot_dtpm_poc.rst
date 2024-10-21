Measured Boot using a Discrete TPM (PoC)
========================================

Measured Boot is the process of cryptographically measuring the code and
critical data used at boot time, for example using a TPM, so that the
security state can be attested later.

The current implementation of the driver included in |TF-A| supports several
backends and each has a different means to store the measurements.
This section focuses on the Discrete TPM backend, which stores measurements
in a PCR within the TPM. This backend can be paired with the `TCG event log`_
to provide attestation of the measurements stored in the event log. See
details in :ref:`Measured Boot Design`.

This section provides instructions to setup and build a proof of concept (PoC)
that showcases the use of Measured Boot with a Discrete TPM interface.

.. note::
   The instructions given in this document are meant to build a PoC to
   show how Measured Boot on TF-A can interact with a Discrete TPM interface.
   This PoC is platform specific, and uses a SPI based Discrete TPM, the
   Raspberry Pi communicates with the TPM via a GPIO pin bit-banged SPI interface.
   For other platforms, different may be required to interface with the hardware
   (e.g., different hardware communication protocols) and different TPM interfaces
   (e.g., |FIFO| vs |CRB|).

Components
~~~~~~~~~~

   - **Platform**: The PoC is developed on the Raspberry Pi 3 (rpi3), due to quick
     driver development and the availability of GPIO pins to interface with a TPM
     expansion module. Measured boot capabilities using the TCG Event Log are
     ported to the Raspberry Pi 3 platform inside TF-A. This PoC specifically uses
     the Raspberry Pi 3 Model B V1.2, but this PoC is compatible with other
     Raspberry Pi 3 models.

   - **Discrete TPM**: The TPM chip selected is a breakout board compatible with
     the Raspberry Pi 3 GPIO pins. This PoC uses a |SPI| based LetsTrust TPM
     breakout board equipped with a Infineon Optiga™ SLB 9670 TPM 2.0 chip. Link
     to device: https://thepihut.com/products/letstrust-tpm-for-raspberry-pi

   .. note::
      If you have another TPM breakout board that uses the same
      Infineon Optiga™ SLB 9670 TPM 2.0 SPI based chip, it will also work.
      Ensure that the correct GPIO pins are utilized on the Raspberry Pi 3 to
      avoid communication issues, and possible hardware failures.

   - **TF-A TPM Drivers**: To interface with a physical (Discrete) TPM chip in
     TF-A, the PoC uses TF-A drivers that provide the command, interface, link,
     and platform layers required to send and receive data to and from the TPM.
     The drivers are located in TFA, and not in a |SP|, so that they may be used
     in early stages such as BL2, and in some cases, BL1. The design of the TPM
     Drivers is documented here: :ref:`Discrete TPM drivers`.

   - **U-boot BL33**: This PoC showcases measured boot up to BL33, and for
     simplicity uses a U-boot image for BL33, so that the image is measured and
     loaded. Currently U-boot does not have Discrete TPM support for the
     Raspberry Pi 3 platform so the boot flow ends here.


Building the PoC for the Raspberry Pi 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Build instructions for U-Boot.bin for Raspberry Pi 3.**

First, the build requires a BL33 firmware image that can be packaged and measured
by TF-A.

U-boot can be built for the Raspberry Pi 3, but there are some changes to be made
to allow the build to succeed. First Clone U-boot and enter the repo.

.. code:: shell

    git clone https://github.com/u-boot/u-boot.git
    cd u-boot

Now to switch to a specific tag ``v2024.04``  for testing purposes, and then build
the defconfig labelled ``rpi_3_b_plus_defconfig``.

.. code:: shell

    git checkout tags/v2024.04 -b tfa_dtpm_poc
    make CROSS_COMPILE=aarch64-linux-gnu- rpi_3_b_plus_defconfig

Lastly open the ``.config`` and change ``CONFIG_TEXT_BASE`` and
``CONFIG_SYS_UBOOT_START`` to ``0x11000000`` to match the BL33 starting point.

.. code:: shell

    vim .config
    CONFIG_TEXT_BASE=0x11000000
    CONFIG_SYS_UBOOT_START=0x11000000

To build the u-boot binary, use the following command.

.. code:: shell

    make CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc)

**Build TF-A for Raspberry Pi 3 with Discrete TPM and Measured Boot.**

Copy over the ``u-boot.bin`` file over to your TF-A working directory.

.. code:: shell

    cp /path/to/u-boot/build/u-boot.bin /path/to/tfa/u-boot.bin

TF-A build command:

.. code:: shell

    CROSS_COMPILE=aarch64-linux-gnu-      \
    make PLAT=rpi3                        \
    RPI3_PRELOADED_DTB_BASE=0x200000      \
    BL33=u-boot.bin                       \
    SUPPORT_VFP=1                         \
    DEBUG=0                               \
    MEASURED_BOOT=1                       \
    DISCRETE_TPM=1                        \
    MBOOT_TPM_HASH_ALG=sha256             \
    TPM_INTERFACE=FIFO_SPI                \
    MBEDTLS_DIR=/path/to/mbedtls/repo     \
    LOG_LEVEL=40                          \
    fip all

This build command is similar to the one provided in the TF-A Raspberry Pi 3
platform port, To learn more about the platform and its build options, visit
:ref:`Raspberry Pi 3`.

   - ``RPI3_PRELOADED_DTB_BASE`` is given a different address to accommodate the
     larger BL1 and BL2 firmware sizes, this is to accommodate the TPM drivers
     that are packaged in BL1 and BL2 for this PoC.
   - ``BL33`` is the non trusted firmware, in this case the U-Boot binary built
     earlier.
   - ``SUPPORT_VFP`` is enabled, allows Vector Floating Point operations in EL3.
   - ``MEASURED_BOOT`` is enabled to allow the Measured Boot flow.
   - ``DISCRETE_TPM=1`` enables the build of Discrete TPM drivers.
   - ``MBOOT_TPM_HASH_ALG=sha256`` sets the hash algorithm to sha256, this is
     the only algorithm supported by both TF-A Measured Boot and the SLB 9670
     TPM 2.0.
   - ``TPM_INTERFACE=FIFO_SPI`` specifies the use of the FIFO SPI interface.
   - ``MBEDTLS_DIR`` is the path to your local mbedtls repo.
   - ``LOG_LEVEL=40`` ensures that eventlog is printed at the end of BL1 and BL2.


**Hardware Setup:**

   - **TPM Connection**: Connect the LetsTrust TPM board to GPIO pins 17 - 26 on
     the 40-pin GPIO header on the Raspberry Pi board. The 2x5 header of the TPM
     module must be aligned to the pins in a specific orientation, match the 3v3
     and RST pins from the TPM board to pins 17 and 18 respectively on the
     Raspberry Pi 3 header. See `rpi3 pinout`_.

   - **Serial Console**: Establish a serial connection to the Raspberry Pi 3 to
     view serial output during the boot sequence. The GND, TXD, and RXD pins,
     which are labelled 6, 8, and 10 on the Raspberry Pi 3 header respectively,
     are the required pins to establish a serial connection. The recommended way
     to connect to the board from another system is to use a USB to serial TTL
     cable to output the serial console in a easy manner.

   - **SD Card Setup**: Format a SD Card as ``FAT32`` with a default Raspbian
     installation that is similar to the default Raspberry Pi 3 boot partition,
     this partition will utilize the default files installed in the root
     directory with Rasbian such as:

    ::

        bcm2710-rpi3-b.dtb
        bootcode.bin
        config.txt
        fixup.dat
        start.elf

    Open ``config.txt`` and overwrite the file with the following lines:

    ::

        arm_64bit=1
        disable_commandline_tags=2
        enable_uart=1
        armstub=armstub8.bin
        device_tree_address=0x200000
        device_tree_end=0x210000

    These configurations are required to enable uart, enable 64bit mode,
    use the build TF binary, and the modified rpi3 device tree address
    and size.

    Copy ``armstub8.bin`` from the TF-A build path to the root folder of the
    SD card.

    The SD Card is now ready to be booted.

Running the PoC for the Raspberry Pi 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Insert the SD Card into the Raspberry Pi 3 SD card port and boot the system.

To access the serial console output from the Raspberry Pi 3 you can either:

   - Follow `instructions`_ to use PuTTY to connect to Raspberry Pi 3 serial console.

   - Use the linux ``screen`` command:

      .. code:: shell

        screen /dev/ttyUSB0 115200

Once booted the output from the serial console will look like this:

.. code:: shell

    Raspberry Pi Bootcode

    Read File: config.txt, 153

    Read File: start.elf, 2975040 (bytes)

    Read File: fixup.dat, 7265 (bytes)

    MESS:00:00:01.170422:0: brfs: File read: /mfs/sd/config.txt
    MESS:00:00:01.174630:0: brfs: File read: 153 bytes
    MESS:00:00:01.211473:0: HDMI0:EDID error reading EDID block 0 attempt 0
    MESS:00:00:01.217639:0: HDMI0:EDID error reading EDID block 0 attempt 1
    MESS:00:00:01.223977:0: HDMI0:EDID error reading EDID block 0 attempt 2
    MESS:00:00:01.230313:0: HDMI0:EDID error reading EDID block 0 attempt 3
    MESS:00:00:01.236650:0: HDMI0:EDID error reading EDID block 0 attempt 4
    MESS:00:00:01.242987:0: HDMI0:EDID error reading EDID block 0 attempt 5
    MESS:00:00:01.249324:0: HDMI0:EDID error reading EDID block 0 attempt 6
    MESS:00:00:01.255660:0: HDMI0:EDID error reading EDID block 0 attempt 7
    MESS:00:00:01.261997:0: HDMI0:EDID error reading EDID block 0 attempt 8
    MESS:00:00:01.268334:0: HDMI0:EDID error reading EDID block 0 attempt 9
    MESS:00:00:01.274429:0: HDMI0:EDID giving up on reading EDID block 0
    MESS:00:00:01.282647:0: brfs: File read: /mfs/sd/config.txt
    MESS:00:00:01.286929:0: gpioman: gpioman_get_pin_num: pin LEDS_PWR_OK not defined
    MESS:00:00:01.487295:0: gpioman: gpioman_get_pin_num: pin DISPLAY_DSI_PORT not defined
    MESS:00:00:01.494853:0: gpioman: gpioman_get_pin_num: pin LEDS_PWR_OK not defined
    MESS:00:00:01.500763:0: *** Restart logging
    MESS:00:00:01.504638:0: brfs: File read: 153 bytes
    MESS:00:00:01.510139:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 0
    MESS:00:00:01.517254:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 1
    MESS:00:00:01.524112:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 2
    MESS:00:00:01.530970:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 3
    MESS:00:00:01.537826:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 4
    MESS:00:00:01.544685:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 5
    MESS:00:00:01.551543:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 6
    MESS:00:00:01.558399:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 7
    MESS:00:00:01.565258:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 8
    MESS:00:00:01.572116:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 9
    MESS:00:00:01.578730:0: hdmi: HDMI0:EDID giving up on reading EDID block 0
    MESS:00:00:01.584634:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 0
    MESS:00:00:01.592427:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 1
    MESS:00:00:01.599286:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 2
    MESS:00:00:01.606142:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 3
    MESS:00:00:01.613001:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 4
    MESS:00:00:01.619858:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 5
    MESS:00:00:01.626717:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 6
    MESS:00:00:01.633575:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 7
    MESS:00:00:01.640431:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 8
    MESS:00:00:01.647288:0: hdmi: HDMI0:EDID error reading EDID block 0 attempt 9
    MESS:00:00:01.653905:0: hdmi: HDMI0:EDID giving up on reading EDID block 0
    MESS:00:00:01.659769:0: hdmi: HDMI:hdmi_get_state is deprecated, use hdmi_get_display_state instead
    MESS:00:00:01.668264:0: HDMI0: hdmi_pixel_encoding: 162000000
    MESS:00:00:01.673988:0: vec: vec_middleware_power_on: vec_base: 0x7e806000 rev-id 0x00002708 @ vec: 0x7e806100 @ 0x00000420 enc: 0x7e806060 @ 0x00000220 cgmsae: 0x7e80605c @ 0x00000000
    MESS:00:00:01.880234:0: dtb_file 'bcm2710-rpi-3-b.dtb'
    MESS:00:00:01.889713:0: brfs: File read: /mfs/sd/bcm2710-rpi-3-b.dtb
    MESS:00:00:01.894375:0: Loaded 'bcm2710-rpi-3-b.dtb' to 0x200000 size 0x7cb2
    MESS:00:00:01.915761:0: brfs: File read: 31922 bytes
    MESS:00:00:02.007202:0: brfs: File read: /mfs/sd/config.txt
    MESS:00:00:02.017277:0: brfs: File read: 153 bytes
    MESS:00:00:02.020772:0: Failed to open command line file 'cmdline.txt'
    MESS:00:00:02.042302:0: gpioman: gpioman_get_pin_num: pin EMMC_ENABLE not defined
    MESS:00:00:02.398066:0: kernel=
    MESS:00:00:02.455255:0: brfs: File read: /mfs/sd/armstub8.bin
    MESS:00:00:02.459284:0: Loaded 'armstub8.bin' to 0x0 size 0xdbe74
    MESS:00:00:02.465109:0: No compatible kernel found
    MESS:00:00:02.469610:0: Device tree loaded to 0x200000 (size 0x823f)
    MESS:00:00:02.476805:0: uart: Set PL011 baud rate to 103448.300000 Hz
    MESS:00:00:02.483381:0: uart: Baud rate change done...
    MESS:00:00:02.486793:0: uart: Baud rateNOTICE:  Booting Trusted Firmware
    NOTICE:  BL1: v2.11.0(release):v2.11.0-187-g0cb1ddc9c-dirty
    NOTICE:  BL1: Built : 10:57:10, Jul  9 2024
    INFO:    BL1: RAM 0x100ee000 - 0x100f9000
    INFO:    Using crypto library 'mbed TLS'
    NOTICE:  TPM Chip: vendor-id 0xd1, device-id 0x0, revision-id: 0x16
    NOTICE:  rpi3: Detected: Raspberry Pi 3 Model B (1GB, Sony, UK) [0x00a02082]
    INFO:    BL1: Loading BL2
    INFO:    Loading image id=1 at address 0x100b4000
    INFO:    Image id=1 loaded: 0x100b4000 - 0x100c0281
    INFO:    TCG_EfiSpecIDEvent:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 3
    INFO:      Digest             : 00
    INFO:          : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:          : 00 00 00
    INFO:      EventSize          : 33
    INFO:      Signature          : Spec ID Event03
    INFO:      PlatformClass      : 0
    INFO:      SpecVersion        : 2.0.2
    INFO:      UintnSize          : 1
    INFO:      NumberOfAlgorithms : 1
    INFO:      DigestSizes        :
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           DigestSize    : 32
    INFO:      VendorInfoSize     : 0
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 3
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:          : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:      EventSize          : 17
    INFO:      Signature          : StartupLocality
    INFO:      StartupLocality    : 0
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 1
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : 55 11 51 d8 8b 7f 41 d3 18 16 f2 e8 80 bf 80 fa
    INFO:          : b4 03 6d 96 4c a0 0a 98 45 cf 25 2f 1e a9 09 3e
    INFO:      EventSize          : 5
    INFO:      Event              : BL_2
    NOTICE:  BL1: Booting BL2
    INFO:    Entry point address = 0x100b4000
    INFO:    SPSR = 0x3c5
    NOTICE:  BL2: v2.11.0(release):v2.11.0-187-g0cb1ddc9c-dirty
    NOTICE:  BL2: Built : 10:56:39, Jul  9 2024
    INFO:    Using crypto library 'mbed TLS'
    NOTICE:  TPM Chip: vendor-id 0xd1, device-id 0x0, revision-id: 0x16
    INFO:    BL2: Doing platform setup
    INFO:    BL2: Loading image id 3
    INFO:    Loading image id=3 at address 0x100e0000
    INFO:    Image id=3 loaded: 0x100e0000 - 0x100e706b
    INFO:    BL2: Loading image id 5
    INFO:    Loading image id=5 at address 0x11000000
    INFO:    Image id=5 loaded: 0x11000000 - 0x110a8ad8
    INFO:    TCG_EfiSpecIDEvent:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 3
    INFO:      Digest             : 00
    INFO:          : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:          : 00 00 00
    INFO:      EventSize          : 33
    INFO:      Signature          : Spec ID Event03
    INFO:      PlatformClass      : 0
    INFO:      SpecVersion        : 2.0.2
    INFO:      UintnSize          : 1
    INFO:      NumberOfAlgorithms : 1
    INFO:      DigestSizes        :
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           DigestSize    : 32
    INFO:      VendorInfoSize     : 0
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 3
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:          : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:      EventSize          : 17
    INFO:      Signature          : StartupLocality
    INFO:      StartupLocality    : 0
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 1
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : 55 11 51 d8 8b 7f 41 d3 18 16 f2 e8 80 bf 80 fa
    INFO:          : b4 03 6d 96 4c a0 0a 98 45 cf 25 2f 1e a9 09 3e
    INFO:      EventSize          : 5
    INFO:      Event              : BL_2
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 1
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : f3 00 5c ed a2 12 8b 76 b7 82 da c5 28 c3 02 52
    INFO:          : 19 e4 3a 82 f2 3c ab 1e 0d 78 84 9c b5 fe e2 4f
    INFO:      EventSize          : 14
    INFO:      Event              : SECURE_RT_EL3
    INFO:    PCR_Event2:
    INFO:      PCRIndex           : 0
    INFO:      EventType          : 1
    INFO:      Digests Count      : 1
    INFO:        #0 AlgorithmId   : SHA256
    INFO:           Digest        : 90 28 81 42 12 b7 9b ca aa 0c 40 76 33 5a 69 71
    INFO:          : b6 19 2b 90 f2 d2 69 b8 de 8e 6d 05 4d c2 73 f9
    INFO:      EventSize          : 6
    INFO:      Event              : BL_33
    NOTICE:  BL1: Booting BL31
    INFO:    Entry point address = 0x100e0000
    INFO:    SPSR = 0x3cd
    NOTICE:  BL31: v2.11.0(release):v2.11.0-187-g0cb1ddc9c-dirty
    NOTICE:  BL31: Built : 10:56:58, Jul  9 2024
    INFO:    rpi3: Checking DTB...
    INFO:    rpi3: Reserved 0x10000000 - 0x10100000 in DTB
    INFO:    BL31: Initializing runtime services
    INFO:    BL31: Preparing for EL3 exit to normal world
    INFO:    Entry point address = 0x11000000
    INFO:    SPSR = 0x3c9


    U-Boot 2024.04-g84314330-dirty (Apr 23 2024 - 15:41:54 -0500)

    DRAM:  948 MiB
    RPI 3 Model B (0xa02082)
    Core:  68 devices, 14 uclasses, devicetree: embed
    MMC:   mmc@7e202000: 0, mmc@7e300000: 1
    Loading Environment from FAT... OK
    In:    serial,usbkbd
    Out:   serial,vidconsole
    Err:   serial,vidconsole
    Net:   No ethernet found.
    starting USB...
    Bus usb@7e980000: USB DWC2
    scanning bus usb@7e980000 for devices...
    Error: smsc95xx_eth No valid MAC address found.
    2 USB Device(s) found
          scanning usb for storage devices... 0 Storage Device(s) found
    Hit any key to stop autoboot:  2  1  0
    Card did not respond to voltage select! : -110
    No EFI system partition
    No EFI system partition
    Failed to persist EFI variables
    No EFI system partition
    Failed to persist EFI variables
    No EFI system partition
    Failed to persist EFI variables
    Missing TPMv2 device for EFI_TCG_PROTOCOL
    ** Booting bootflow '<NULL>' with efi_mgr
    Loading Boot0000 'mmc 0' failed
    EFI boot manager: Cannot load any image
    Boot failed (err=-14)
    Card did not respond to voltage select! : -110
    No ethernet found.
    No ethernet found.
    U-Boot>


Next steps for Discrete TPM and Measured Boot development
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to automatically validate the workings of the Discrete TPM, the creation
of test cases that compare the eventlog image hashes with what is stored in PCR0
are a great way to test the core functionality of the Discrete TPM in Measured Boot.

Development of Discrete TPM drivers such as a reference FIFO |I2C|, MMIO, and CRB
drivers has not started, these drivers will allow a larger number of platform
to use a Discrete TPM in TF-A.

*Copyright (c) 2025, Arm Limited. All rights reserved.*

.. _TCG event log: https://trustedcomputinggroup.org/resource/tcg-efi-platform-specification/
.. _rpi3 pinout: https://www.raspberrypi.com/documentation/computers/raspberry-pi.html#gpio
.. _instructions: https://www.circuitbasics.com/use-putty-to-access-the-raspberry-pi-terminal-from-a-computer/
.. _workaround:  https://github.com/mhomran/u-boot-rpi3-b-plus
