Poplar
======

Poplar is the first development board compliant with the 96Boards Enterprise
Edition TV Platform specification.

The board features the Hi3798C V200 with an integrated quad-core 64-bit
Arm Cortex A53 processor and high performance Mali T720 GPU, making it capable
of running any commercial set-top solution based on Linux or Android.

It supports a premium user experience with up to H.265 HEVC decoding of 4K
video at 60 frames per second.

::

    SOC Hisilicon Hi3798CV200
    CPU Quad-core Arm Cortex-A53 64 bit
    DRAM DDR3/3L/4 SDRAM interface, maximum 32-bit data width 2 GB
    USB Two USB 2.0 ports One USB 3.0 ports
    CONSOLE USB-micro port for console support
    ETHERNET 1 GBe Ethernet
    PCIE One PCIe 2.0 interfaces
    JTAG 8-Pin JTAG
    EXPANSION INTERFACE Linaro 96Boards Low Speed Expansion slot
    DIMENSION Standard 160×120 mm 96Boards Enterprice Edition form factor
    WIFI 802.11AC 2*2 with Bluetooth
    CONNECTORS One connector for Smart Card One connector for TSI

At the start of the boot sequence, the bootROM executes the so called l-loader
binary whose main role is to change the processor state to 64bit mode. This
must happen prior to invoking Trusted Firmware-A:

::

    l-loader --> Trusted Firmware-A --> u-boot

How to build
------------

Code Locations
~~~~~~~~~~~~~~

-  Trusted Firmware-A:
   `link <https://github.com/ARM-software/arm-trusted-firmware>`__

-  l-loader:
   `link <https://github.com/Linaro/poplar-l-loader.git>`__

-  u-boot:
   `link <http://git.denx.de/u-boot.git>`__

Build Procedure
~~~~~~~~~~~~~~~

-  Fetch all the above 3 repositories into local host.
   Make all the repositories in the same ${BUILD\_PATH}.

-  Prepare the AARCH64 toolchain.

-  Build u-boot using poplar_defconfig

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu- poplar_defconfig
       make CROSS_COMPILE=aarch64-linux-gnu-

-  Build atf providing the previously generated u-boot.bin as the BL33 image

.. code:: bash

       make CROSS_COMPILE=aarch64-linux-gnu-  all fip SPD=none PLAT=poplar
       BL33=u-boot.bin

-  Build l-loader (generated the final fastboot.bin)
       1. copy the atf generated files fip.bin and bl1.bin to l-loader/atf/
       2. export ARM_TRUSTED_FIRMWARE=${ATF_SOURCE_PATH)
       3. make

Install Procedure
-----------------

- Copy l-loader/fastboot.bin to a FAT partition on a USB pen drive.

- Plug the USB pen drive to any of the USB2 ports

- Power the board while keeping S3 pressed (usb_boot)

The system will boot into a u-boot shell which you can then use to write the
working firmware to eMMC.

Boot trace
----------

::

    Bootrom start
    Boot Media: eMMC
    Decrypt auxiliary code ...OK

    lsadc voltage min: 000000FE, max: 000000FF, aver: 000000FE, index: 00000000

    Entry boot auxiliary code

    Auxiliary code - v1.00
    DDR code - V1.1.2 20160205
    Build: Mar 24 2016 - 17:09:44
    Reg Version:  v134
    Reg Time:     2016/03/18 09:44:55
    Reg Name:     hi3798cv2dmb_hi3798cv200_ddr3_2gbyte_8bitx4_4layers.reg

    Boot auxiliary code success
    Bootrom success

    LOADER:  Switched to aarch64 mode
    LOADER:  Entering ARM TRUSTED FIRMWARE
    LOADER:  CPU0 executes at 0x000ce000

    INFO:    BL1: 0xe1000 - 0xe7000 [size = 24576]
    NOTICE:  Booting Trusted Firmware
    NOTICE:  BL1: v1.3(debug):v1.3-372-g1ba9c60
    NOTICE:  BL1: Built : 17:51:33, Apr 30 2017
    INFO:    BL1: RAM 0xe1000 - 0xe7000
    INFO:    BL1: Loading BL2
    INFO:    Loading image id=1 at address 0xe9000
    INFO:    Image id=1 loaded at address 0xe9000, size = 0x5008
    NOTICE:  BL1: Booting BL2
    INFO:    Entry point address = 0xe9000
    INFO:    SPSR = 0x3c5
    NOTICE:  BL2: v1.3(debug):v1.3-372-g1ba9c60
    NOTICE:  BL2: Built : 17:51:33, Apr 30 2017
    INFO:    BL2: Loading BL31
    INFO:    Loading image id=3 at address 0x129000
    INFO:    Image id=3 loaded at address 0x129000, size = 0x8038
    INFO:    BL2: Loading BL33
    INFO:    Loading image id=5 at address 0x37000000
    INFO:    Image id=5 loaded at address 0x37000000, size = 0x58f17
    NOTICE:  BL1: Booting BL31
    INFO:    Entry point address = 0x129000
    INFO:    SPSR = 0x3cd
    INFO:    Boot bl33 from 0x37000000 for 364311 Bytes
    NOTICE:  BL31: v1.3(debug):v1.3-372-g1ba9c60
    NOTICE:  BL31: Built : 17:51:33, Apr 30 2017
    INFO:    BL31: Initializing runtime services
    INFO:    BL31: Preparing for EL3 exit to normal world
    INFO:    Entry point address = 0x37000000
    INFO:    SPSR = 0x3c9


    U-Boot 2017.05-rc2-00130-gd2255b0 (Apr 30 2017 - 17:51:28 +0200)poplar

    Model: HiSilicon Poplar Development Board
    BOARD: Hisilicon HI3798cv200 Poplar
    DRAM:  1 GiB
    MMC:   Hisilicon DWMMC: 0
    In:    serial@f8b00000
    Out:   serial@f8b00000
    Err:   serial@f8b00000
    Net:   Net Initialization Skipped
    No ethernet found.

    Hit any key to stop autoboot:  0
    starting USB...
    USB0:   USB EHCI 1.00
    scanning bus 0 for devices... 1 USB Device(s) found
    USB1:   USB EHCI 1.00
    scanning bus 1 for devices... 4 USB Device(s) found
           scanning usb for storage devices... 1 Storage Device(s) found
           scanning usb for ethernet devices... 1 Ethernet Device(s) found

    USB device 0:
        Device 0: Vendor: SanDisk Rev: 1.00 Prod: Cruzer Blade
                Type: Removable Hard Disk
                Capacity: 7632.0 MB = 7.4 GB (15630336 x 512)
    ... is now current device
    Scanning usb 0:1...
    =>
