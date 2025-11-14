
Realm Management Extension (RME)
====================================

FEAT_RME (or RME for short) is an Armv9-A extension and is one component of the
`Arm Confidential Compute Architecture (Arm CCA)`_. TF-A supports RME starting
from version 2.6. This chapter discusses the changes to TF-A to support RME and
provides instructions on how to build and run TF-A with RME.

RME support in TF-A
---------------------

The following diagram shows an Arm CCA software architecture with TF-A as the
EL3 firmware. In the Arm CCA architecture there are two additional security
states and address spaces: ``Root`` and ``Realm``. TF-A firmware runs in the
Root world. In the realm world, a Realm Management Monitor firmware (`RMM`_)
manages the execution of Realm VMs and their interaction with the hypervisor.

.. image:: ../resources/diagrams/arm-cca-software-arch.png

RME is the hardware extension to support Arm CCA. To support RME, various
changes have been introduced to TF-A. We discuss those changes below.

Changes to translation tables library
***************************************
RME adds Root and Realm Physical address spaces. To support this, two new
memory type macros, ``MT_ROOT`` and ``MT_REALM``, have been added to the
:ref:`Translation (XLAT) Tables Library`. These macros are used to configure
memory regions as Root or Realm respectively.

.. note::

 Only version 2 of the translation tables library supports the new memory
 types.

Changes to context management
*******************************
A new CPU context for the Realm world has been added. The existing can be used
to manage Realm context.

Boot flow changes
*******************
In a typical TF-A boot flow, BL2 runs at Secure-EL1. However when RME is
enabled, TF-A runs in the Root world at EL3. Therefore, the boot flow is
modified to run BL2 at EL3 when RME is enabled. In addition to this, a
Realm-world firmware (`RMM`_) is loaded by BL2 in the Realm physical address
space.

The boot flow when RME is enabled looks like the following:

1. BL1 loads and executes BL2 at EL3
2. BL2 loads images including RMM
3. BL2 transfers control to BL31
4. BL31 initializes SPM (if SPM is enabled)
5. BL31 initializes RMM
6. BL31 transfers control to Normal-world software

Granule Protection Tables (GPT) library
*****************************************
Isolation between the four physical address spaces is enforced by a process
called Granule Protection Check (GPC) performed by the MMU downstream any
address translation. GPC makes use of Granule Protection Table (GPT) in the
Root world that describes the physical address space assignment of every
page (granule). A GPT library that provides APIs to initialize GPTs and to
transition granules between different physical address spaces has been added.
More information about the GPT library can be found in the
:ref:`Granule Protection Tables Library` chapter.

RMM Dispatcher (RMMD)
************************
RMMD is a new standard runtime service that handles the switch to the Realm
world. It initializes the `RMM`_ and handles Realm Management Interface (RMI)
SMC calls from Non-secure.

There is a contract between `RMM`_ and RMMD that defines the arguments that the
former needs to take in order to initialize and also the possible return values.
This contract is defined in the `RMM`_ Boot Interface, which can be found at
:ref:`rmm_el3_boot_interface`.

There is also a specification of the runtime services provided by TF-A
to `RMM`_. This can be found at :ref:`runtime_services_and_interface`.

Test Realm Payload (TRP)
*************************
TRP is a small test payload that runs at R-EL2 and implements a subset of
the Realm Management Interface (RMI) commands to primarily test EL3 firmware
and the interface between R-EL2 and EL3. When building TF-A with RME enabled,
if the path to an RMM image is not provided, TF-A builds the TRP by default
and uses it as the R-EL2 payload.

Building and running TF-A with RME
----------------------------------

The recommended procedure for building and running an RME enabled reference
platform is by the use of `Shrinkwrap`_.

The tool offers examples of Three-worlds and Four-worlds configurations for
running on the *FVP_Base_RevC-2xAEMvA* model.

- Three-world execution: this is the configuration to use if Secure
  world functionality is not needed.

- Four-world execution: this is the configuration to use if both Secure
  and Realm world functionality is needed.

TF-A is tested with the following software entities in each world as listed below:

  - NS Host (RME capable Linux),
  - EL3 Root (TF-A)
  - R-EL2 (`RMM`_)
  - S-EL2 (`SPM`_ / Hafnium) in a 4-worlds configuration

Additionally, the TF-RMM project documentation has the specific `Shrinkwrap guide`_.

TF-A CI pipeline includes coverage for building TF-A along with TF-RMM and SPM/Hafnium
for running 3 worlds and 4 worlds configurations. In those cases, the normal world payload
and test suite originates from TF-a-tests project.

.. _Arm Confidential Compute Architecture (Arm CCA): https://www.arm.com/why-arm/architecture/security-features/arm-confidential-compute-architecture
.. _SPM: https://www.trustedfirmware.org/projects/hafnium/
.. _RMM: https://www.trustedfirmware.org/projects/tf-rmm/
.. _Shrinkwrap: https://shrinkwrap.docs.arm.com/en/latest/
.. _Shrinkwrap guide: https://tf-rmm.readthedocs.io/en/latest/getting_started/building-with-shrinkwrap.html
