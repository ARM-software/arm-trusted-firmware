Trusted Boot Firmware Configuration bindings
============================================

This document defines the nodes and properties used to define the Trusted-Boot
firmware configuration. Platform owners are advised to define shared bindings
here. If a binding does not generalize, they should be documented
alongside platform documentation. There is no guarantee of backward
compatibility with the nodes and properties outlined in this context.

Trusted Boot Firmware Configuration
-----------------------------------

- compatible [mandatory]
   - value type: <string>
   - Should be the string ``"<plat>,tb_fw"``, where ``<plat>`` is the name of the
     platform (i.e. ``"arm,tb_fw"``).

- disable_auth [mandatory]
   - value type: <u32>
   - Flag used to dynamically disable authentication for development purposes.
     Has two possible values: 0 or 1. Setting the flag to 1 disables
     authentication.

- mbedtls_heap_addr [mandatory]
   - value type: <u64>
   - Base address of the dynamically allocated Mbed TLS heap. This is given as a placeholder.

- mbedtls_heap_size [mandatory]
   - value type: <u32>
   - Size of the Mbed TLS heap.

IO FIP Handles
--------------

- compatible [mandatory]
   - value type: <string>
   - Should be the string ``"<plat>,io-fip-handle"``, where ``<plat>`` is the name of the
     platform (i.e. ``"arm,io-fip-handle"``).

- scp_bl2_uuid [mandatory]
   - value type: <string>
   - SCP Firmware SCP_BL2 UUID

- bl31_uuid [mandatory]
   - value type: <string>
   - EL3 Runtime Firmware BL31 UUID

- bl32_uuid [mandatory]
   - value type: <string>
   - Secure Payload BL32 (Trusted OS) UUID

- bl32_extra1_uuid [mandatory]
   - value type: <string>
   - Secure Payload BL32_EXTRA1 (Trusted OS Extra1) UUID

- bl32_extra2_uuid [mandatory]
   - value type: <string>
   - Secure Payload BL32_EXTRA2 (Trusted OS Extra2) UUID

- bl33_uuid [mandatory]
   - value type: <string>
   - Non-Trusted Firmware BL33 UUID

- hw_cfg_uuid [mandatory]
   - value type: <string>
   - HW_CONFIG (e.g. Kernel DT) UUID

- soc_fw_cfg_uuid [mandatory]
   - value type: <string>
   - SOC Firmware Configuration SOC_FW_CONFIG UUID

- tos_fw_cfg_uuid [mandatory]
   - value type: <string>
   - Trusted OS Firmware Configuration TOS_FW_CONFIG UUID

- nt_fw_cfg_uuid [mandatory]
   - value type: <string>
   - Non-Trusted Firmware Configuration NT_FW_CONFIG UUID

- cca_cert_uuid [optional]
   - value type: <string>
   - CCA Content Certificate UUID

- core_swd_cert_uuid [optional]
   - value type: <string>
   - Core SWD Key Certificate UUID

- plat_cert_uuid [optional]
   - value type: <string>
   - Core SWD Key Certificate UUID

- t_key_cert_uuid [optional]
   - value type: <string>
   - Trusted Key Certificate UUID

- scp_fw_key_uuid [optional]
   - value type: <string>
   - SCP Firmware Key UUID

- soc_fw_key_uuid [optional]
   - value type: <string>
   - SOC Firmware Key UUID

- tos_fw_key_cert_uuid [optional]
   - value type: <string>
   - TOS Firmware Key UUID

- nt_fw_key_cert_uuid [optional]
   - value type: <string>
   - Non-Trusted Firmware Key UUID

- scp_fw_content_cert_uuid [optional]
   - value type: <string>
   - SCP Firmware Content Certificate UUID

- soc_fw_content_cert_uuid [optional]
   - value type: <string>
   - SOC Firmware Content Certificate UUID

- tos_fw_content_cert_uuid [optional]
   - value type: <string>
   - TOS Firmware Content Certificate UUID

- nt_fw_content_cert_uuid [optional]
   - value type: <string>
   - Non-Trusted Firmware Content Certificate UUID

- plat_sp_content_cert_uuid [optional]
   - value type: <string>
   - Platform Secure Partition Content Certificate UUID


Secure Partitions
-----------------

- compatible [mandatory]
   - value type: <string>
   - Should be the string ``"<plat>,sp"``, where ``<plat>`` is the name of the
     platform (i.e. ``"arm,sp"``).

- uuid [mandatory]
   - value type: <string>
   - A string identifying the UUID of the service implemented by this partition.
     The UUID format is described in RFC 4122.

- load-address [mandatory]
   - value type: <u32>
   - Physical base address of the partition in memory. Absence of this field
     indicates that the partition is position independent and can be loaded at
     any address chosen at boot time.

- owner [optional]
   - value type: <string>
   - A string property representing the name of the owner of the secure
     partition, which may be the silicon or platform provider.

--------------

*Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.*
