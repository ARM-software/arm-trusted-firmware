RD-Aspen (Zena CSS) Platform
============================

The RD-Aspen platform, as referenced in TF-A, includes the following features:

* Primary Compute with four processor clusters, each containing:

  * Four Cortex-A720AE cores (Armv9.2-A application processor, 64-bit mode)
  * A DynamIQ Shared Unit (DSU-120AE)

* A GIC-720AE, which is GICv4-compatible and supports GICv3 mode as well.

Further information on RD-Aspen is available at `Zena CSS`_

Boot Sequence
-------------

The boot process begins with the Runtime Security Engine (RSE), which loads the
Application Processor (AP) BL2 image into the Trusted SRAM at a fixed address.
Once loaded, the RSE signals the System Control Processor firmware (SCP-firmware)
running on Safety Island Cluster 0 (SI CL0) to initiate the AP power-up sequence.

The SCP-firmware then sets the reset vector base address (RVBAR) for the AP, ensuring
it starts executing BL2 from the designated address. Following this, the SCP-firmware
powers on AP Cluster 0, allowing the AP to run AP BL2.

The following tasks are executed for each AP BL stage:

1. AP BL2:

   * Performs the actions described in the `Trusted Board Boot (TBB)`_ document.
   * (Optional step) Find the FIP image in a GPT partition, incase the FIP lies
     within in a GPT image.
   * Copies the FW_CONFIG from Secure Flash to Trusted SRAM.
   * (Optional step) Computes FW_CONFIG image measurement and invokes extend
     operation of TF-M Measured Boot secure partition.
   * Completes its dynamic configuration from the FW_CONFIG loaded.
     This includes:

      * Parsing the configuration data.
      * Setting up the required system parameters.

   * Reads and loads AP BL31 image into the Trusted SRAM.
   * (Optional step) Computes AP BL31 image measurement and invokes extend operation of TF-M
     Measured Boot secure partition
   * Copies the HW_CONFIG from Secure Flash to DRAM.
   * (Optional step) Computes HW_CONFIG (DTB) image measurement and invokes
     extend operation of TF-M Measured Boot secure partition.
   * (If present) Reads and loads AP BL32 (Secure Payload) image into Secure DRAM
   * (Optional step, if present) Computes AP BL32 image measurement and invokes extend
      operation of TF-M Measured Boot secure partition
   * (If present) Reads and loads the SPMC manifest (for S-EL2 firmware configuration)
     into Trusted SRAM and passes its location to BL31.
   * Copies AP BL33 and Device tree blob from Secure Flash to Normal DRAM.
   * (Optional step) Computes AP BL33 image measurement and invokes extend operation of TF-M
     Measured Boot secure partition
   * Transfers the execution to AP BL31.

2. AP BL31:

   * Initializes Trusted Firmware-A Services.
   * Transfers the execution to AP BL32 and then transfers the execution to AP BL33

3. AP BL32:

   * Initializes Trusted OS (OP-TEE) environment
   * Initializes Secure Partitions
   * Transfers the execution back to AP BL31
   * During runtime, it facilitates secure communication between the
     normal world environment (e.g. Linux) and the Trusted Execution Environment.

Build Procedure (TF-A only)
---------------------------

-  Ensure all `Prerequisites`_  are met, and the ``CROSS_COMPILE`` environment
   variable is properly set.

-  Build TF-A:


   .. code:: shell

      make \
      PLAT=rdaspen \
      MBEDTLS_DIR=<mbedtls_dir> \
      CREATE_KEYS=1 \
      GENERATE_COT=1 \
      TRUSTED_BOARD_BOOT=1 \
      COT=tbbr \
      ARM_ROTPK_LOCATION=devel_rsa \
      ROT_KEY=plat/arm/board/common/rotpk/arm_rotprivk_rsa.pem \
      BL32=<path to optee binary> \
      ARM_GPT_SUPPORT=1 \
      BL33=<PATH-TO-BL33-BINARY> \
      MEASURED_BOOT=1 \

.. note::

   The ``BL32`` flag is optional and should be set only if a Trusted OS is required.
   If it is not set, then ``BL33`` will be loaded directly after ``BL31``.

   The ``ARM_GPT_SUPPORT`` flag is also optional. It must be enabled when the
   FIP image resides inside a GPT partition on Secure Flash.

   The ``MEASURED_BOOT`` flag is also optional. It must be enabled in order to
   enable measurement of FW images and data.

--------------

*Copyright (c) 2025, Arm Limited. All rights reserved.*

.. _Prerequisites:  https://trustedfirmware-a.readthedocs.io/en/latest/getting_started/prerequisites.html
.. _Trusted Board Boot (TBB): https://trustedfirmware-a.readthedocs.io/en/latest/design/trusted-board-boot.html
.. _Zena CSS: https://www.arm.com/products/automotive/compute-subsystems/zena
