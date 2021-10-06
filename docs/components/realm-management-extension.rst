
Realm Management Extension (RME)
====================================

FEAT_RME (or RME for short) is an Armv9-A extension and is one component of the
`Arm Confidential Compute Architecture (Arm CCA)`_. TF-A supports RME starting
from version 2.6. This document provides instructions on how to build and run
TF-A with RME.

Building and running TF-A with RME
------------------------------------

This section describes how you can build and run TF-A with RME enabled.
We assume you have all the :ref:`Prerequisites` to build TF-A.

To enable RME, you need to set the ENABLE_RME build flag when building
TF-A. Currently, this feature is only supported for the FVP platform.

The following instructions show you how to build and run TF-A with RME
for two scenarios: TF-A with TF-A Tests, and four-world execution with
Hafnium and TF-A Tests. The instructions assume you have already obtained
TF-A. You can use the following command to clone TF-A.

.. code:: shell

 git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git

To run the tests, you need an FVP model. You can download a model that supports
RME from the `Arm Architecture Models website`_. Please select the
*Base RevC AEM FVP* model. After extracting the downloaded file, you should be able to
find the *FVP_Base_RevC-2xAEMvA* binary. The instructions below have been tested
with model version 11.15 revision 18.

.. note::

 ENABLE_RME build option is currently experimental.

Building TF-A with TF-A Tests
********************************************
Use the following instructions to build TF-A with `TF-A Tests`_ as the
non-secure payload (BL33).

**1. Obtain and build TF-A Tests**

.. code:: shell

 git clone https://git.trustedfirmware.org/TF-A/tf-a-tests.git
 cd tf-a-tests
 make CROSS_COMPILE=aarch64-none-elf- PLAT=fvp DEBUG=1

This produces a TF-A Tests binary (*tftf.bin*) in the *build/fvp/debug* directory.

**2. Build TF-A**

.. code:: shell

 cd trusted-firmware-a
 make CROSS_COMPILE=aarch64-none-elf- \
 PLAT=fvp \
 ENABLE_RME=1 \
 FVP_HW_CONFIG_DTS=fdts/fvp-base-gicv3-psci-1t.dts \
 DEBUG=1 \
 BL33=<path/to/tftf.bin> \
 all fip

This produces *bl1.bin* and *fip.bin* binaries in the *build/fvp/debug* directory.
The above command also builds a Test Realm Payload (TRP), which is a small test
payload that implements Realm Monitor Management (RMM) functionalities and runs
in the realm world (R-EL2). The TRP binary is packaged in *fip.bin*.

Four-world execution with Hafnium and TF-A Tests
****************************************************
Four-world execution involves software components at each security state: root,
secure, realm and non-secure. This section describes how to build TF-A
with four-world support. We use TF-A as the root firmware, `Hafnium`_ as the
secure component, TRP as the realm-world firmware and TF-A Tests as the
non-secure payload.

Before building TF-A, you first need to build the other software components.
You can find instructions on how to get and build TF-A Tests above.

**1. Obtain and build Hafnium**

.. code:: shell

 git clone --recurse-submodules https://git.trustedfirmware.org/hafnium/hafnium.git
 cd hafnium
 make PROJECT=reference

The Hafnium binary should be located at
*out/reference/secure_aem_v8a_fvp_clang/hafnium.bin*

**2. Build TF-A**

Build TF-A with RME as well as SPM enabled.

.. code:: shell

 make CROSS_COMPILE=aarch64-none-elf- \
 PLAT=fvp \
 ENABLE_RME=1 \
 FVP_HW_CONFIG_DTS=fdts/fvp-base-gicv3-psci-1t.dts \
 SPD=spmd \
 SPMD_SPM_AT_SEL2=1 \
 BRANCH_PROTECTION=1 \
 CTX_INCLUDE_PAUTH_REGS=1 \
 DEBUG=1 \
 SP_LAYOUT_FILE=<path/to/tf-a-tests>/build/fvp/debug/sp_layout.json> \
 BL32=<path/to/hafnium.bin> \
 BL33=<path/to/tftf.bin> \
 all fip

Running the tests
*********************
Use the following command to run the tests on FVP. TF-A Tests should boot
and run the default tests including RME tests.

.. code:: shell

 FVP_Base_RevC-2xAEMvA \
 -C bp.flashloader0.fname=<path/to/fip.bin> \
 -C bp.secureflashloader.fname=<path/to/bl1.bin> \
 -C bp.refcounter.non_arch_start_at_default=1 \
 -C bp.refcounter.use_real_time=0 \
 -C bp.ve_sysregs.exit_on_shutdown=1 \
 -C cache_state_modelled=1 \
 -C cluster0.NUM_CORES=4 \
 -C cluster0.PA_SIZE=48 \
 -C cluster0.ecv_support_level=2 \
 -C cluster0.gicv3.cpuintf-mmap-access-level=2 \
 -C cluster0.gicv3.without-DS-support=1 \
 -C cluster0.gicv4.mask-virtual-interrupt=1 \
 -C cluster0.has_arm_v8-6=1 \
 -C cluster0.has_branch_target_exception=1 \
 -C cluster0.has_rme=1 \
 -C cluster0.has_rndr=1 \
 -C cluster0.has_amu=1 \
 -C cluster0.has_v8_7_pmu_extension=2 \
 -C cluster0.max_32bit_el=-1 \
 -C cluster0.restriction_on_speculative_execution=2 \
 -C cluster0.restriction_on_speculative_execution_aarch32=2 \
 -C cluster1.NUM_CORES=4 \
 -C cluster1.PA_SIZE=48 \
 -C cluster1.ecv_support_level=2 \
 -C cluster1.gicv3.cpuintf-mmap-access-level=2 \
 -C cluster1.gicv3.without-DS-support=1 \
 -C cluster1.gicv4.mask-virtual-interrupt=1 \
 -C cluster1.has_arm_v8-6=1 \
 -C cluster1.has_branch_target_exception=1 \
 -C cluster1.has_rme=1 \
 -C cluster1.has_rndr=1 \
 -C cluster1.has_amu=1 \
 -C cluster1.has_v8_7_pmu_extension=2 \
 -C cluster1.max_32bit_el=-1 \
 -C cluster1.restriction_on_speculative_execution=2 \
 -C cluster1.restriction_on_speculative_execution_aarch32=2 \
 -C pci.pci_smmuv3.mmu.SMMU_AIDR=2 \
 -C pci.pci_smmuv3.mmu.SMMU_IDR0=0x0046123B \
 -C pci.pci_smmuv3.mmu.SMMU_IDR1=0x00600002 \
 -C pci.pci_smmuv3.mmu.SMMU_IDR3=0x1714 \
 -C pci.pci_smmuv3.mmu.SMMU_IDR5=0xFFFF0475 \
 -C pci.pci_smmuv3.mmu.SMMU_S_IDR1=0xA0000002 \
 -C pci.pci_smmuv3.mmu.SMMU_S_IDR2=0 \
 -C pci.pci_smmuv3.mmu.SMMU_S_IDR3=0 \
 -C bp.pl011_uart0.out_file=uart0.log \
 -C bp.pl011_uart1.out_file=uart1.log \
 -C bp.pl011_uart2.out_file=uart2.log \
 -C pctl.startup=0.0.0.0 \
 -Q 1000 \
 "$@"

The bottom of the output from *uart0* should look something like the following.

.. code-block:: shell

 ...

 > Test suite 'FF-A Interrupt'
                                                                Passed
 > Test suite 'SMMUv3 tests'
                                                                Passed
 > Test suite 'PMU Leakage'
                                                                Passed
 > Test suite 'DebugFS'
                                                                Passed
 > Test suite 'Realm payload tests'
                                                                Passed
 ...


.. _Arm Confidential Compute Architecture (Arm CCA): https://www.arm.com/why-arm/architecture/security-features/arm-confidential-compute-architecture
.. _Arm Architecture Models website: https://developer.arm.com/tools-and-software/simulation-models/fixed-virtual-platforms/arm-ecosystem-models
.. _TF-A Tests: https://trustedfirmware-a-tests.readthedocs.io/en/latest
.. _Hafnium: https://www.trustedfirmware.org/projects/hafnium
