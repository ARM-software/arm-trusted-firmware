TF-A UART Booting Instructions for Marvell Platforms
====================================================

This section describes how to temporary boot the Trusted Firmware-A (TF-A) project over UART
without flashing it to non-volatile storage for Marvell's platforms.

See :ref:`TF-A Build Instructions for Marvell Platforms` how to build ``mrvl_uart`` and
``mrvl_flash`` targets used in this section.

Armada37x0 UART image downloading
---------------------------------

There are two options how to download UART image into any Armada37x0 board.

Marvell Wtpdownloader
~~~~~~~~~~~~~~~~~~~~~

Marvell Wtpdownloader works only with UART images stored in separate files and supports only upload
speed with 115200 bauds. Target ``mrvl_uart`` produces GZIPed TAR archive ``uart-images.tgz.bin``
with either three files ``TIM_ATF.bin``, ``wtmi_h.bin`` and ``boot-image_h.bin`` for non-secure
boot or with four files ``TIM_ATF_TRUSTED.bin``, ``TIMN_ATF_TRUSTED.bin``, ``wtmi_h.bin`` and
``boot-image_h.bin`` when secure boot is enabled.

Compilation:

.. code:: shell

    > git clone https://github.com/MarvellEmbeddedProcessors/A3700-utils-marvell.git
    > make -C A3700-utils-marvell/wtptp/src/Wtpdownloader_Linux -f makefile.mk

It produces executable binary ``A3700-utils-marvell/wtptp/src/Wtpdownloader_Linux/WtpDownload_linux``

To download images from ``uart-images.tgz.bin`` archive unpack it and for non-secure boot variant run:

.. code:: shell

    > stty -F /dev/ttyUSB<port#> clocal
    > WtpDownload_linux -P UART -C <port#> -E -B TIM_ATF.bin -I wtmi_h.bin -I boot-image_h.bin

After that immediately start terminal on ``/dev/ttyUSB<port#>`` to see boot output.

CZ.NIC mox-imager
~~~~~~~~~~~~~~~~~

CZ.NIC mox-imager supports all Armada37x0 boards (not only Turris MOX as name suggests). It works
with either with separate files from ``uart-images.tgz.bin`` archive (like Marvell Wtpdownloader)
produced by ``mrvl_uart`` target or also with ``flash-image.bin`` file produced by ``mrvl_flash``
target, which is the exactly same file as used for flashing. So when using CZ.NIC mox-imager there
is no need to build separate files for UART flashing like in case with Marvell Wtpdownloader.

CZ.NIC mox-imager moreover supports higher upload speeds up to the 6000000 bauds (which seems to
be limit of Armada37x0 SoC) which is much higher and faster than Marvell Wtpdownloader.

Compilation:

.. code:: shell

    > git clone https://gitlab.nic.cz/turris/mox-imager.git
    > make -C mox-imager

It produces executable binary ``mox-imager/mox-imager``

To download single file image built by ``mrvl_flash`` target at the highest speed, run:

.. code:: shell

    > mox-imager -D /dev/ttyUSB<port#> -E -b 6000000 -t flash-image.bin

To download images from ``uart-images.tgz.bin`` archive built by ``mrvl_uart`` target for
non-secure boot variant (like Wtpdownloader) but at the highest speed, first unpack
``uart-images.tgz.bin`` archive and then run:

.. code:: shell

    > mox-imager -D /dev/ttyUSB<port#> -E -b 6000000 -t TIM_ATF.bin wtmi_h.bin boot-image_h.bin

CZ.NIC mox-imager after successful download will start its own mini terminal (option ``-t``) to
not loose any boot output. It also prints boot output which is sent either by image files or by
bootrom during transferring of image files. This mini terminal can be quit by CTRL-\\ + C keypress.


A7K/A8K/CN913x UART image downloading
-------------------------------------

A7K/A8K/CN913x uses same image ``flash-image.bin`` for both flashing and booting over UART.
For downloading image over UART it is possible to use mvebu64boot tool.

Compilation:

.. code:: shell

    > git clone https://github.com/pali/mvebu64boot.git
    > make -C mvebu64boot

It produces executable binary ``mvebu64boot/mvebu64boot``

To download ``flash-image.bin`` image run:

.. code:: shell

    > mvebu64boot -t -b flash-image.bin /dev/ttyUSB0

After successful download it will start own mini terminal (option ``-t``) like CZ.NIC mox-imager.
