Runtime Security Subsystem (RSS)
================================

This document focuses on the relationship between the Runtime Security Subsystem
(RSS) and the application processor (AP). According to the ARM reference design
the RSS is an independent core next to the AP and the SCP on the same die. It
provides fundamental security guarantees and runtime services for the rest of
the system (e.g.: trusted boot, measured boot, platform attestation,
key management, and key derivation).

At power up RSS boots first from its private ROM code. It validates and loads
its own images and the initial images of SCP and AP. When AP and SCP are
released from reset and their initial code is loaded then they continue their
own boot process, which is the same as on non-RSS systems. Please refer to the
``RSS documentation`` [1]_ for more details about the RSS boot flow.

The last stage of the RSS firmware is a persistent, runtime component. Much
like AP_BL31, this is a passive entity which has no periodical task to do and
just waits for external requests from other subsystems. RSS and other
subsystems can communicate with each other over message exchange. RSS waits
in idle for the incoming request, handles them, and sends a response then goes
back to idle.

RSS communication layer
-----------------------

The communication between RSS and other subsystems are primarily relying on the
Message Handling Unit (MHU) module. The number of MHU interfaces between RSS
and other cores is IMPDEF. Besides MHU other modules also could take part in
the communication. RSS is capable of mapping the AP memory to its address space.
Thereby either RSS core itself or a DMA engine if it is present, can move the
data between memory belonging to RSS or AP. In this way, a bigger amount of data
can be transferred in a short time.

The MHU comes in pairs. There is a sender and receiver side. They are connected
to each other. An MHU interface consists of two pairs of MHUs, one sender and
one receiver on both sides. Bidirectional communication is possible over an
interface. One pair provides message sending from AP to RSS and the other pair
from RSS to AP. The sender and receiver are connected via channels. There is an
IMPDEF number of channels (e.g: 4-16) between a sender and a receiver module.

The RSS communication layer provides two ways for message exchange:

- ``Embedded messaging``: The full message, including header and payload, are
  exchanged over the MHU channels. A channel is capable of delivering a single
  word. The sender writes the data to the channel register on its side and the
  receiver can read the data from the channel on the other side. One dedicated
  channel is used for signalling. It does not deliver any payload it is just
  meant for signalling that the sender loaded the data to the channel registers
  so the receiver can read them. The receiver uses the same channel to signal
  that data was read. Signalling happens via IRQ. If the message is longer than
  the data fit to the channel registers then the message is sent over in
  multiple rounds. Both, sender and receiver allocate a local buffer for the
  messages. Data is copied from/to these buffers to/from the channel registers.
- ``Pointer-access messaging``: The message header and the payload are
  separated and they are conveyed in different ways. The header is sent
  over the channels, similar to the embedded messaging but the payload is
  copied over by RSS core (or by DMA) between the sender and the receiver. This
  could be useful in the case of long messages because transaction time is less
  compared to the embedded messaging mode. Small payloads are copied by the RSS
  core because setting up DMA would require more CPU cycles. The payload is
  either copied into an internal buffer or directly read-written by RSS. Actual
  behavior depends on RSS setup, whether the partition supports memory-mapped
  ``iovec``. Therefore, the sender must handle both cases and prevent access to
  the memory, where payload data lives, while the RSS handles the request.

The RSS communication layer supports both ways of messaging in parallel. It is
decided at runtime based on the message size which way to transfer the message.

.. code-block:: bash

    +----------------------------------------------+       +-------------------+
    |                                              |       |                   |
    |                      AP                      |       |                   |
    |                                              |  +--->|       SRAM        |
    +----------------------------------------------|  |    |                   |
    |              BL1 / BL2 / BL31                |  |    |                   |
    +----------------------------------------------+  |    +-------------------+
             |                           ^            |        ^           ^
             |  send                 IRQ | receive    |direct  |           |
             V                           |            |access  |           |
    +--------------------+    +--------------------+  |        |           |
    |      MHU sender    |    |    MHU receiver    |  |        | Copy data |
    +--------------------+    +--------------------+  |        |           |
       | |           | |          | |           | |   |        |           |
       | | channels  | |          | | channels  | |   |        |           |
       | | e.g: 4-16 | |          | | e.g: 4-16 | |   |        V           |
    +--------------------+    +--------------------+  |    +-------+       |
    |     MHU receiver   |    |     MHU sender     |  | +->|  DMA  |       |
    +--------------------+    +--------------------+  | |  +-------+       |
             |                           ^            | |      ^           |
        IRQ  |  receive                  | send       | |      | Copy data |
             V                           |            | |      V           V
    +----------------------------------------------+  | |  +-------------------+
    |                                              |--+-+  |                   |
    |                  RSS                         |       |      SRAM         |
    |                                              |       |                   |
    +----------------------------------------------+       +-------------------+

.. Note::

    The RSS communication layer is not prepared for concurrent execution. The
    current use case only requires message exchange during the boot phase. In
    the boot phase, only a single core is running and the rest of the cores are
    in reset.

Message structure
^^^^^^^^^^^^^^^^^
A description of the message format can be found in the ``RSS communication
design`` [2]_ document.

Source files
^^^^^^^^^^^^
- RSS comms:  ``drivers/arm/rss``
- MHU driver: ``drivers/arm/mhu``


API for communication over MHU
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The API is defined in these header files:

- ``include/drivers/arm/rss_comms.h``
- ``include/drivers/arm/mhu.h``

RSS provided runtime services
-----------------------------

RSS provides the following runtime services:

- ``Measured boot``: Securely store the firmware measurements which were
  computed during the boot process and the associated metadata (image
  description, measurement algorithm, etc.). More info on measured boot service
  in RSS can be found in the ``measured_boot_integration_guide`` [3]_ .
- ``Delegated attestation``: Query the platform attestation token and derive a
  delegated attestation key. More info on the delegated attestation service
  in RSS can be found in the ``delegated_attestation_integration_guide`` [4]_ .
- ``OTP assets management``: Public keys used by AP during the trusted boot
  process can be requested from RSS. Furthermore, AP can request RSS to
  increase a non-volatile counter. Please refer to the
  ``RSS key management`` [5]_ document for more details.

Runtime service API
^^^^^^^^^^^^^^^^^^^
The RSS provided runtime services implement a PSA aligned API. The parameter
encoding follows the PSA client protocol described in the
``Firmware Framework for M`` [6]_ document in chapter 4.4. The implementation is
restricted to the static handle use case therefore only the ``psa_call`` API is
implemented.


Software and API layers
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    +----------------+         +---------------------+
    |   BL1 / BL2    |         |       BL31          |
    +----------------+         +---------------------+
      |                         |
      | extend_measurement()    | get_delegated_key()
      |                         | get_platform_token()
      V                         V
    +----------------+         +---------------------+
    |  PSA protocol  |         |    PSA protocol     |
    +----------------+         +---------------------+
         |                               |
         | psa_call()                    | psa_call()
         |                               |
         V                               V
    +------------------------------------------------+
    |         RSS communication protocol             |
    +------------------------------------------------+
         |                     ^
         | mhu_send_data()     | mhu_receive_data()
         |                     |
         V                     |
    +------------------------------------------------+
    |                 MHU driver                     |
    +------------------------------------------------+
               |                      ^
               | Register access      | IRQ
               V                      |
    +------------------------------------------------+
    |             MHU HW on AP side                  |
    +------------------------------------------------+
                         ^
                         | Physical wires
                         |
                         V
    +------------------------------------------------+
    |             MHU HW on RSS side                 |
    +------------------------------------------------+
             |                        ^
             | IRQ                    | Register access
             V                        |
    +------------------------------------------------+
    |                 MHU driver                     |
    +------------------------------------------------+
             |                        |
             V                        V
    +---------------+       +------------------------+
    | Measured boot |       | Delegated attestation  |
    | service       |       | service                |
    +---------------+       +------------------------+


RSS based Measured Boot
-----------------------

Measured Boot is the process of cryptographically measuring (computing the hash
value of a binary) the code and critical data used at boot time. The
measurement must be stored in a tamper-resistant way, so the security state
of the device can be attested later to an external party. RSS provides a runtime
service which is meant to store measurements and associated metadata alongside.

Data is stored in internal SRAM which is only accessible by the secure runtime
firmware of RSS. Data is stored in so-called measurement slots. A platform has
IMPDEF number of measurement slots. The measurement storage follows extend
semantics. This means that measurements are not stored directly (as it was
taken) instead they contribute to the current value of the measurement slot.
The extension implements this logic, where ``||`` stands for concatenation:

.. code-block:: bash

    new_value_of_measurement_slot = Hash(old_value_of_measurement_slot || measurement)

Supported hash algorithms: sha-256, sha-512

Measured Boot API
^^^^^^^^^^^^^^^^^

Defined here:

- ``include/lib/psa/measured_boot.h``

.. code-block:: c

    psa_status_t
    rss_measured_boot_extend_measurement(uint8_t        index,
                                         const uint8_t *signer_id,
                                         size_t         signer_id_size,
                                         const uint8_t *version,
                                         size_t         version_size,
                                         uint32_t       measurement_algo,
                                         const uint8_t *sw_type,
                                         size_t         sw_type_size,
                                         const uint8_t *measurement_value,
                                         size_t         measurement_value_size,
                                         bool           lock_measurement);

Measured Boot Metadata
^^^^^^^^^^^^^^^^^^^^^^

The following metadata can be stored alongside the measurement:

- ``Signer-id``: Mandatory. The hash of the firmware image signing public key.
- ``Measurement algorithm``: Optional. The hash algorithm which was used to
  compute the measurement (e.g.: sha-256, etc.).
- ``Version info``: Optional. The firmware version info (e.g.: 2.7).
- ``SW type``: Optional. Short text description (e.g.: BL1, BL2, BL31, etc.)

.. Note::
    Version info is not implemented in TF-A yet.


The caller must specify in which measurement slot to extend a certain
measurement and metadata. A measurement slot can be extended by multiple
measurements. The default value is IMPDEF. All measurement slot is cleared at
reset, there is no other way to clear them. In the reference implementation,
the measurement slots are initialized to 0. At the first call to extend the
measurement in a slot, the extend operation uses the default value of the
measurement slot. All upcoming extend operation on the same slot contributes
to the previous value of that measurement slot.

The following rules are kept when a slot is extended multiple times:

- ``Signer-id`` must be the same as the previous call(s), otherwise a
  PSA_ERROR_NOT_PERMITTED error code is returned.

- ``Measurement algorithm``: must be the same as the previous call(s),
  otherwise, a PSA_ERROR_NOT_PERMITTED error code is returned.

In case of error no further action is taken (slot is not locked). If there is
a valid data in a sub-sequent call then measurement slot will be extended. The
rest of the metadata is handled as follows when a measurement slot is extended
multiple times:

- ``SW type``: Cleared.
- ``Version info``: Cleared.

.. Note::

    Extending multiple measurements in the same slot leads to some metadata
    information loss. Since RSS is not constrained on special HW resources to
    store the measurements and metadata, therefore it is worth considering to
    store all of them one by one in distinct slots. However, they are one-by-one
    included in the platform attestation token. So, the number of distinct
    firmware image measurements has an impact on the size of the attestation
    token.

The allocation of the measurement slot among RSS, Root and Realm worlds is
platform dependent. The platform must provide an allocation of the measurement
slot at build time. An example can be found in
``tf-a/plat/arm/board/tc/tc_bl1_measured_boot.c``
Furthermore, the memory, which holds the metadata is also statically allocated
in RSS memory. Some of the fields have a static value (measurement algorithm),
and some of the values have a dynamic value (measurement value) which is updated
by the bootloaders when the firmware image is loaded and measured. The metadata
structure is defined in
``include/drivers/measured_boot/rss/rss_measured_boot.h``.

.. code-block:: c

    struct rss_mboot_metadata {
            unsigned int id;
            uint8_t slot;
            uint8_t signer_id[SIGNER_ID_MAX_SIZE];
            size_t  signer_id_size;
            uint8_t version[VERSION_MAX_SIZE];
            size_t  version_size;
            uint8_t sw_type[SW_TYPE_MAX_SIZE];
            size_t  sw_type_size;
            void    *pk_oid;
            bool    lock_measurement;
    };

Signer-ID API
^^^^^^^^^^^^^

This function calculates the hash of a public key (signer-ID) using the
``Measurement algorithm`` and stores it in the ``rss_mboot_metadata`` field
named ``signer_id``.
Prior to calling this function, the caller must ensure that the ``signer_id``
field points to the zero-filled buffer.

Defined here:

- ``include/drivers/measured_boot/rss/rss_measured_boot.h``

.. code-block:: c

   int rss_mboot_set_signer_id(struct rss_mboot_metadata *metadata_ptr,
                               const void *pk_oid,
                               const void *pk_ptr,
                               size_t pk_len)


- First parameter is the pointer to the ``rss_mboot_metadata`` structure.
- Second parameter is the pointer to the key-OID of the public key.
- Third parameter is the pointer to the public key buffer.
- Fourth parameter is the size of public key buffer.
- This function returns 0 on success, a signed integer error code
  otherwise.

Build time config options
^^^^^^^^^^^^^^^^^^^^^^^^^

- ``MEASURED_BOOT``: Enable measured boot. It depends on the platform
  implementation whether RSS or TPM (or both) backend based measured boot is
  enabled.
- ``MBOOT_RSS_HASH_ALG``: Determine the hash algorithm to measure the images.
  The default value is sha-256.

Measured boot flow
^^^^^^^^^^^^^^^^^^

.. figure:: ../resources/diagrams/rss_measured_boot_flow.svg
  :align: center

Sample console log
^^^^^^^^^^^^^^^^^^

.. code-block:: bash

    INFO:    Measured boot extend measurement:
    INFO:     - slot        : 6
    INFO:     - signer_id   : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:                   : 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:     - version     :
    INFO:     - version_size: 0
    INFO:     - sw_type     : FW_CONFIG
    INFO:     - sw_type_size: 10
    INFO:     - algorithm   : 2000009
    INFO:     - measurement : aa ea d3 a7 a8 e2 ab 7d 13 a6 cb 34 99 10 b9 a1
    INFO:                   : 1b 9f a0 52 c5 a8 b1 d7 76 f2 c1 c1 ef ca 1a df
    INFO:     - locking     : true
    INFO:    FCONF: Config file with image ID:31 loaded at address = 0x4001010
    INFO:    Loading image id=24 at address 0x4001300
    INFO:    Image id=24 loaded: 0x4001300 - 0x400153a
    INFO:    Measured boot extend measurement:
    INFO:     - slot        : 7
    INFO:     - signer_id   : b0 f3 82 09 12 97 d8 3a 37 7a 72 47 1b ec 32 73
    INFO:                   : e9 92 32 e2 49 59 f6 5e 8b 4a 4a 46 d8 22 9a da
    INFO:     - version     :
    INFO:     - version_size: 0
    INFO:     - sw_type     : TB_FW_CONFIG
    INFO:     - sw_type_size: 13
    INFO:     - algorithm   : 2000009
    INFO:     - measurement : 05 b9 dc 98 62 26 a7 1c 2d e5 bb af f0 90 52 28
    INFO:                   : f2 24 15 8a 3a 56 60 95 d6 51 3a 7a 1a 50 9b b7
    INFO:     - locking     : true
    INFO:    FCONF: Config file with image ID:24 loaded at address = 0x4001300
    INFO:    BL1: Loading BL2
    INFO:    Loading image id=1 at address 0x404d000
    INFO:    Image id=1 loaded: 0x404d000 - 0x406412a
    INFO:    Measured boot extend measurement:
    INFO:     - slot        : 8
    INFO:     - signer_id   : b0 f3 82 09 12 97 d8 3a 37 7a 72 47 1b ec 32 73
    INFO:                   : e9 92 32 e2 49 59 f6 5e 8b 4a 4a 46 d8 22 9a da
    INFO:     - version     :
    INFO:     - version_size: 0
    INFO:     - sw_type     : BL_2
    INFO:     - sw_type_size: 5
    INFO:     - algorithm   : 2000009
    INFO:     - measurement : 53 a1 51 75 25 90 fb a1 d9 b8 c8 34 32 3a 01 16
    INFO:                   : c9 9e 74 91 7d 28 02 56 3f 5c 40 94 37 58 50 68
    INFO:     - locking     : true

Delegated Attestation
---------------------

Delegated Attestation Service was mainly developed to support the attestation
flow on the ``ARM Confidential Compute Architecture`` (ARM CCA) [7]_.
The detailed description of the delegated attestation service can be found in
the ``Delegated Attestation Service Integration Guide`` [4]_ document.

In the CCA use case, the Realm Management Monitor (RMM) relies on the delegated
attestation service of the RSS to get a realm attestation key and the CCA
platform token. BL31 does not use the service for its own purpose, only calls
it on behalf of RMM. The access to MHU interface and thereby to RSS is
restricted to BL31 only. Therefore, RMM does not have direct access, all calls
need to go through BL31. The RMM dispatcher module of the BL31 is responsible
for delivering the calls between the two parties.

.. Note::
     Currently the connection between the RMM dispatcher and the PSA/RSS layer
     is not yet implemented. RMM dispatcher just returns hard coded data.

Delegated Attestation API
^^^^^^^^^^^^^^^^^^^^^^^^^
Defined here:

- ``include/lib/psa/delegated_attestation.h``

.. code-block:: c

    psa_status_t
    rss_delegated_attest_get_delegated_key(uint8_t   ecc_curve,
                                           uint32_t  key_bits,
                                           uint8_t  *key_buf,
                                           size_t    key_buf_size,
                                           size_t   *key_size,
                                           uint32_t  hash_algo);

    psa_status_t
    rss_delegated_attest_get_token(const uint8_t *dak_pub_hash,
                                   size_t         dak_pub_hash_size,
                                   uint8_t       *token_buf,
                                   size_t         token_buf_size,
                                   size_t        *token_size);

Attestation flow
^^^^^^^^^^^^^^^^

.. figure:: ../resources/diagrams/rss_attestation_flow.svg
  :align: center

Sample attestation token
^^^^^^^^^^^^^^^^^^^^^^^^

Binary format:

.. code-block:: bash

    INFO:    DELEGATED ATTEST TEST START
    INFO:    Get delegated attestation key start
    INFO:    Get delegated attest key succeeds, len: 48
    INFO:    Delegated attest key:
    INFO:            0d 2a 66 61 d4 89 17 e1 70 c6 73 56 df f4 11 fd
    INFO:            7d 1f 3b 8a a3 30 3d 70 4c d9 06 c3 c7 ef 29 43
    INFO:            0f ee b5 e7 56 e0 71 74 1b c4 39 39 fd 85 f6 7b
    INFO:    Get platform token start
    INFO:    Get platform token succeeds, len: 1086
    INFO:    Platform attestation token:
    INFO:            d2 84 44 a1 01 38 22 a0 59 03 d1 a9 0a 58 20 00
    INFO:            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    INFO:            00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 19
    INFO:            01 00 58 21 01 cb 8c 79 f7 a0 0a 6c ce 12 66 f8
    INFO:            64 45 48 42 0e c5 10 bf 84 ee 22 18 b9 8f 11 04
    INFO:            c7 22 31 9d fb 19 09 5c 58 20 aa aa aa aa aa aa
    INFO:            aa aa bb bb bb bb bb bb bb bb cc cc cc cc cc cc
    INFO:            cc cc dd dd dd dd dd dd dd dd 19 09 5b 19 30 00
    INFO:            19 09 5f 89 a4 05 58 20 bf e6 d8 6f 88 26 f4 ff
    INFO:            97 fb 96 c4 e6 fb c4 99 3e 46 19 fc 56 5d a2 6a
    INFO:            df 34 c3 29 48 9a dc 38 04 67 31 2e 36 2e 30 2b
    INFO:            30 01 64 52 54 5f 30 02 58 20 90 27 f2 46 ab 31
    INFO:            85 36 46 c4 d7 c6 60 ed 31 0d 3c f0 14 de f0 6c
    INFO:            24 0b de b6 7a 84 fc 3f 5b b7 a4 05 58 20 b3 60
    INFO:            ca f5 c9 8c 6b 94 2a 48 82 fa 9d 48 23 ef b1 66
    INFO:            a9 ef 6a 6e 4a a3 7c 19 19 ed 1f cc c0 49 04 67
    INFO:            30 2e 30 2e 30 2b 30 01 64 52 54 5f 31 02 58 20
    INFO:            52 13 15 d4 9d b2 cf 54 e4 99 37 44 40 68 f0 70
    INFO:            7d 73 64 ae f7 08 14 b0 f7 82 ad c6 17 db a3 91
    INFO:            a4 05 58 20 bf e6 d8 6f 88 26 f4 ff 97 fb 96 c4
    INFO:            e6 fb c4 99 3e 46 19 fc 56 5d a2 6a df 34 c3 29
    INFO:            48 9a dc 38 04 67 31 2e 35 2e 30 2b 30 01 64 52
    INFO:            54 5f 32 02 58 20 8e 5d 64 7e 6f 6c c6 6f d4 4f
    INFO:            54 b6 06 e5 47 9a cc 1b f3 7f ce 87 38 49 c5 92
    INFO:            d8 2f 85 2e 85 42 a4 05 58 20 bf e6 d8 6f 88 26
    INFO:            f4 ff 97 fb 96 c4 e6 fb c4 99 3e 46 19 fc 56 5d
    INFO:            a2 6a df 34 c3 29 48 9a dc 38 04 67 31 2e 35 2e
    INFO:            30 2b 30 01 60 02 58 20 b8 01 65 a7 78 8b c6 59
    INFO:            42 8d 33 10 85 d1 49 0a dc 9e c3 ee df 85 1b d2
    INFO:            f0 73 73 6a 0c 07 11 b8 a4 05 58 20 b0 f3 82 09
    INFO:            12 97 d8 3a 37 7a 72 47 1b ec 32 73 e9 92 32 e2
    INFO:            49 59 f6 5e 8b 4a 4a 46 d8 22 9a da 04 60 01 6a
    INFO:            46 57 5f 43 4f 4e 46 49 47 00 02 58 20 21 9e a0
    INFO:            13 82 e6 d7 97 5a 11 13 a3 5f 45 39 68 b1 d9 a3
    INFO:            ea 6a ab 84 23 3b 8c 06 16 98 20 ba b9 a4 05 58
    INFO:            20 b0 f3 82 09 12 97 d8 3a 37 7a 72 47 1b ec 32
    INFO:            73 e9 92 32 e2 49 59 f6 5e 8b 4a 4a 46 d8 22 9a
    INFO:            da 04 60 01 6d 54 42 5f 46 57 5f 43 4f 4e 46 49
    INFO:            47 00 02 58 20 41 39 f6 c2 10 84 53 c5 17 ae 9a
    INFO:            e5 be c1 20 7b cc 24 24 f3 9d 20 a8 fb c7 b3 10
    INFO:            e3 ee af 1b 05 a4 05 58 20 b0 f3 82 09 12 97 d8
    INFO:            3a 37 7a 72 47 1b ec 32 73 e9 92 32 e2 49 59 f6
    INFO:            5e 8b 4a 4a 46 d8 22 9a da 04 60 01 65 42 4c 5f
    INFO:            32 00 02 58 20 5c 96 20 e1 e3 3b 0f 2c eb c1 8e
    INFO:            1a 02 a6 65 86 dd 34 97 a7 4c 98 13 bf 74 14 45
    INFO:            2d 30 28 05 c3 a4 05 58 20 b0 f3 82 09 12 97 d8
    INFO:            3a 37 7a 72 47 1b ec 32 73 e9 92 32 e2 49 59 f6
    INFO:            5e 8b 4a 4a 46 d8 22 9a da 04 60 01 6e 53 45 43
    INFO:            55 52 45 5f 52 54 5f 45 4c 33 00 02 58 20 f6 fb
    INFO:            62 99 a5 0c df db 02 0b 72 5b 1c 0b 63 6e 94 ee
    INFO:            66 50 56 3a 29 9c cb 38 f0 ec 59 99 d4 2e a4 05
    INFO:            58 20 b0 f3 82 09 12 97 d8 3a 37 7a 72 47 1b ec
    INFO:            32 73 e9 92 32 e2 49 59 f6 5e 8b 4a 4a 46 d8 22
    INFO:            9a da 04 60 01 6a 48 57 5f 43 4f 4e 46 49 47 00
    INFO:            02 58 20 98 5d 87 21 84 06 33 9d c3 1f 91 f5 68
    INFO:            8d a0 5a f0 d7 7e 20 51 ce 3b f2 a5 c3 05 2e 3c
    INFO:            8b 52 31 19 01 09 78 1c 68 74 74 70 3a 2f 2f 61
    INFO:            72 6d 2e 63 6f 6d 2f 43 43 41 2d 53 53 44 2f 31
    INFO:            2e 30 2e 30 19 09 62 71 6e 6f 74 2d 68 61 73 68
    INFO:            2d 65 78 74 65 6e 64 65 64 19 09 61 44 ef be ad
    INFO:            de 19 09 60 77 77 77 77 2e 74 72 75 73 74 65 64
    INFO:            66 69 72 6d 77 61 72 65 2e 6f 72 67 58 60 29 4e
    INFO:            4a d3 98 1e 3b 70 9f b6 66 ed 47 33 0e 99 f0 b1
    INFO:            c3 f2 bc b2 1d b0 ae 90 0c c4 82 ff a2 6f ae 45
    INFO:            f6 87 09 4a 09 21 77 ec 36 1c 53 b8 a7 9b 8e f7
    INFO:            27 eb 7a 09 da 6f fb bf cb fd b3 e5 e9 36 91 b1
    INFO:            92 13 c1 30 16 b4 5c 49 5e c0 c1 b9 01 5c 88 2c
    INFO:            f8 2f 3e a4 a2 6d e4 9d 31 6a 06 f7 a7 73
    INFO:    DELEGATED ATTEST TEST END

JSON format:

.. code-block:: JSON

    {
        "CCA_PLATFORM_CHALLENGE": "b'0000000000000000000000000000000000000000000000000000000000000000'",
        "CCA_PLATFORM_INSTANCE_ID": "b'01CB8C79F7A00A6CCE1266F8644548420EC510BF84EE2218B98F1104C722319DFB'",
        "CCA_PLATFORM_IMPLEMENTATION_ID": "b'AAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDD'",
        "CCA_PLATFORM_LIFECYCLE": "secured_3000",
        "CCA_PLATFORM_SW_COMPONENTS": [
            {
                "SIGNER_ID": "b'BFE6D86F8826F4FF97FB96C4E6FBC4993E4619FC565DA26ADF34C329489ADC38'",
                "SW_COMPONENT_VERSION": "1.6.0+0",
                "SW_COMPONENT_TYPE": "RT_0",
                "MEASUREMENT_VALUE": "b'9027F246AB31853646C4D7C660ED310D3CF014DEF06C240BDEB67A84FC3F5BB7'"
            },
            {
                "SIGNER_ID": "b'B360CAF5C98C6B942A4882FA9D4823EFB166A9EF6A6E4AA37C1919ED1FCCC049'",
                "SW_COMPONENT_VERSION": "0.0.0+0",
                "SW_COMPONENT_TYPE": "RT_1",
                "MEASUREMENT_VALUE": "b'521315D49DB2CF54E49937444068F0707D7364AEF70814B0F782ADC617DBA391'"
            },
            {
                "SIGNER_ID": "b'BFE6D86F8826F4FF97FB96C4E6FBC4993E4619FC565DA26ADF34C329489ADC38'",
                "SW_COMPONENT_VERSION": "1.5.0+0",
                "SW_COMPONENT_TYPE": "RT_2",
                "MEASUREMENT_VALUE": "b'8E5D647E6F6CC66FD44F54B606E5479ACC1BF37FCE873849C592D82F852E8542'"
            },
            {
                "SIGNER_ID": "b'BFE6D86F8826F4FF97FB96C4E6FBC4993E4619FC565DA26ADF34C329489ADC38'",
                "SW_COMPONENT_VERSION": "1.5.0+0",
                "SW_COMPONENT_TYPE": "",
                "MEASUREMENT_VALUE": "b'B80165A7788BC659428D331085D1490ADC9EC3EEDF851BD2F073736A0C0711B8'"
            },
            {
                "SIGNER_ID": "b'b0f382091297d83a377a72471bec3273e99232e24959f65e8b4a4a46d8229ada'",
                "SW_COMPONENT_VERSION": "",
                "SW_COMPONENT_TYPE": "FW_CONFIG\u0000",
                "MEASUREMENT_VALUE": "b'219EA01382E6D7975A1113A35F453968B1D9A3EA6AAB84233B8C06169820BAB9'"
            },
            {
                "SIGNER_ID": "b'b0f382091297d83a377a72471bec3273e99232e24959f65e8b4a4a46d8229ada'",
                "SW_COMPONENT_VERSION": "",
                "SW_COMPONENT_TYPE": "TB_FW_CONFIG\u0000",
                "MEASUREMENT_VALUE": "b'4139F6C2108453C517AE9AE5BEC1207BCC2424F39D20A8FBC7B310E3EEAF1B05'"
            },
            {
                "SIGNER_ID": "b'b0f382091297d83a377a72471bec3273e99232e24959f65e8b4a4a46d8229ada'",
                "SW_COMPONENT_VERSION": "",
                "SW_COMPONENT_TYPE": "BL_2\u0000",
                "MEASUREMENT_VALUE": "b'5C9620E1E33B0F2CEBC18E1A02A66586DD3497A74C9813BF7414452D302805C3'"
            },
            {
                "SIGNER_ID": "b'b0f382091297d83a377a72471bec3273e99232e24959f65e8b4a4a46d8229ada'",
                "SW_COMPONENT_VERSION": "",
                "SW_COMPONENT_TYPE": "SECURE_RT_EL3\u0000",
                "MEASUREMENT_VALUE": "b'F6FB6299A50CDFDB020B725B1C0B636E94EE6650563A299CCB38F0EC5999D42E'"
            },
            {
                "SIGNER_ID": "b'b0f382091297d83a377a72471bec3273e99232e24959f65e8b4a4a46d8229ada'",
                "SW_COMPONENT_VERSION": "",
                "SW_COMPONENT_TYPE": "HW_CONFIG\u0000",
                "MEASUREMENT_VALUE": "b'985D87218406339DC31F91F5688DA05AF0D77E2051CE3BF2A5C3052E3C8B5231'"
            }
        ],
        "CCA_ATTESTATION_PROFILE": "http://arm.com/CCA-SSD/1.0.0",
        "CCA_PLATFORM_HASH_ALGO_ID": "not-hash-extended",
        "CCA_PLATFORM_CONFIG": "b'EFBEADDE'",
        "CCA_PLATFORM_VERIFICATION_SERVICE": "www.trustedfirmware.org"
    }

RSS OTP Assets Management
-------------------------

RSS provides access for AP to assets in OTP, which include keys for image
signature verification and non-volatile counters for anti-rollback protection.

Non-Volatile Counter API
^^^^^^^^^^^^^^^^^^^^^^^^

AP/RSS interface for retrieving and incrementing non-volatile counters API is
as follows.

Defined here:

- ``include/lib/psa/rss_platform_api.h``

.. code-block:: c

    psa_status_t rss_platform_nv_counter_increment(uint32_t counter_id)

    psa_status_t rss_platform_nv_counter_read(uint32_t counter_id,
            uint32_t size, uint8_t *val)

Through this service, we can read/increment any of the 3 non-volatile
counters used on an Arm CCA platform:

- ``Non-volatile counter for CCA firmware (BL2, BL31, RMM).``
- ``Non-volatile counter for secure firmware.``
- ``Non-volatile counter for non-secure firmware.``

Public Key API
^^^^^^^^^^^^^^

AP/RSS interface for reading the ROTPK is as follows.

Defined here:

- ``include/lib/psa/rss_platform_api.h``

.. code-block:: c

    psa_status_t rss_platform_key_read(enum rss_key_id_builtin_t key,
            uint8_t *data, size_t data_size, size_t *data_length)

Through this service, we can read any of the 3 ROTPKs used on an
Arm CCA platform:

- ``ROTPK for CCA firmware (BL2, BL31, RMM).``
- ``ROTPK for secure firmware.``
- ``ROTPK for non-secure firmware.``

References
----------

.. [1] https://tf-m-user-guide.trustedfirmware.org/platform/arm/rss/readme.html
.. [2] https://tf-m-user-guide.trustedfirmware.org/platform/arm/rss/rss_comms.html
.. [3] https://git.trustedfirmware.org/TF-M/tf-m-extras.git/tree/partitions/measured_boot/measured_boot_integration_guide.rst
.. [4] https://git.trustedfirmware.org/TF-M/tf-m-extras.git/tree/partitions/delegated_attestation/delegated_attest_integration_guide.rst
.. [5] https://tf-m-user-guide.trustedfirmware.org/platform/arm/rss/rss_key_management.html
.. [6] https://developer.arm.com/-/media/Files/pdf/PlatformSecurityArchitecture/Architect/DEN0063-PSA_Firmware_Framework-1.0.0-2.pdf?revision=2d1429fa-4b5b-461a-a60e-4ef3d8f7f4b4&hash=3BFD6F3E687F324672F18E5BE9F08EDC48087C93
.. [7] https://developer.arm.com/documentation/DEN0096/A_a/?lang=en

--------------

*Copyright (c) 2023, Arm Limited. All rights reserved.*
