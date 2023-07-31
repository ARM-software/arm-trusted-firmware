Nuvoton NPCM845X
================

Nuvoton NPCM845X is the Nuvoton Arbel NPCM8XX Board Management controller (BMC) SoC.

The Nuvoton Arbel NPCM845X SoC is a fourth-generation BMC.
The NPCM845X computing subsystem comprises a quadcore Arm Cortex-A35 CPU.

This SoC includes secured components, i.e., bootblock stored in ROM,
u-boot, OPTEE-OS, trusted-firmware-a and Linux.
Every stage is measured and validated by the bootblock.
This SoC was tested on the Arbel NPCM845X evaluation board.


How to Build
------------

.. code:: shell

    make CROSS_COMPILE=aarch64-none-elf- PLAT=npcm845x all SPD=opteed

