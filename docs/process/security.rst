Security Handling
=================

Security Disclosures
--------------------

We disclose all security vulnerabilities we find, or are advised about, that are
relevant to Trusted Firmware-A. We encourage responsible disclosure of
vulnerabilities and inform users as best we can about all possible issues.

We disclose TF-A vulnerabilities as Security Advisories, all of which are listed
at the bottom of this page. Any new ones will, additionally, be announced on the
TF-A project's `mailing list`_.

Found a Security Issue?
-----------------------

Although we try to keep TF-A secure, we can only do so with the help of the
community of developers and security researchers.

.. warning::
   If you think you have found a security vulnerability, please **do not**
   report it in the `issue tracker`_ or on the `mailing list`_. Instead, please
   follow the `TrustedFirmware.org security incident process`_.

One of the goals of this process is to ensure providers of products that use
TF-A have a chance to consider the implications of the vulnerability and its
remedy before it is made public. As such, please follow the disclosure plan
outlined in the process. We do our best to respond and fix any issues quickly.

Afterwards, we encourage you to write-up your findings about the TF-A source
code.

Attribution
-----------

We will name and thank you in the :ref:`Change Log & Release Notes` distributed
with the source code and in any published security advisory.

Security Advisories
-------------------

+-----------+------------------------------------------------------------------+
| ID        | Title                                                            |
+===========+==================================================================+
|  |TFV-1|  | Malformed Firmware Update SMC can result in copy of unexpectedly |
|           | large data into secure memory                                    |
+-----------+------------------------------------------------------------------+
|  |TFV-2|  | Enabled secure self-hosted invasive debug interface can allow    |
|           | normal world to panic secure world                               |
+-----------+------------------------------------------------------------------+
|  |TFV-3|  | RO memory is always executable at AArch64 Secure EL1             |
+-----------+------------------------------------------------------------------+
|  |TFV-4|  | Malformed Firmware Update SMC can result in copy or              |
|           | authentication of unexpected data in secure memory in AArch32    |
|           | state                                                            |
+-----------+------------------------------------------------------------------+
|  |TFV-5|  | Not initializing or saving/restoring PMCR_EL0 can leak secure    |
|           | world timing information                                         |
+-----------+------------------------------------------------------------------+
|  |TFV-6|  | Trusted Firmware-A exposure to speculative processor             |
|           | vulnerabilities using cache timing side-channels                 |
+-----------+------------------------------------------------------------------+
|  |TFV-7|  | Trusted Firmware-A exposure to cache speculation vulnerability   |
|           | Variant 4                                                        |
+-----------+------------------------------------------------------------------+
|  |TFV-8|  | Not saving x0 to x3 registers can leak information from one      |
|           | Normal World SMC client to another                               |
+-----------+------------------------------------------------------------------+
|  |TFV-9|  | Trusted Firmware-A exposure to speculative processor             |
|           | vulnerabilities with branch prediction target reuse              |
+-----------+------------------------------------------------------------------+
|  |TFV-10| | Incorrect validation of X.509 certificate extensions can result  |
|           | in an out-of-bounds read                                         |
+-----------+------------------------------------------------------------------+
|  |TFV-11| |  A Malformed SDEI SMC can cause out of bound memory read         |
+-----------+------------------------------------------------------------------+
|  |TFV-12| | When Hardware Page Aggregation (HPA) is enabled memory           |
|           | accesses may be translated incorrectly.                          |
+-----------+------------------------------------------------------------------+
|  |TFV-13| | An unprivileged context can trigger a data memory-dependent      |
|           | prefetch engine to fetch the contents of a privileged location   |
|           | and consume those contents as an address that is also            |
|           | dereferenced.                                                    |
+-----------+------------------------------------------------------------------+
|  |TFV-14| | BL1 FWU range-check mismatch in FWU_SMC_IMAGE_COPY can cause     |
|           | out-of-range memory access.                                      |
+-----------+------------------------------------------------------------------+
|  |TFV-15| | Insufficient validation of FIP ToC offsets in BL1/BL2 can        |
|           | cause unintended reads and secure memory disclosure at boot.     |
+-----------+------------------------------------------------------------------+
|  |TFV-16| | SME erratum in C1-Pro means memory accesses from the SME unit    |
|           | can remain outstanding after another CPU issues TLBI+DSB         |
+-----------+------------------------------------------------------------------+
|  |TFV-17| | TLBI+DSB might complete too early                                |
+-----------+------------------------------------------------------------------+

.. _issue tracker: https://github.com/TrustedFirmware-A/trusted-firmware-a/issues
.. _mailing list: https://lists.trustedfirmware.org/mailman3/lists/tf-a.lists.trustedfirmware.org/

.. |TFV-1| replace:: :ref:`Advisory TFV-1 (CVE-2016-10319)`
.. |TFV-2| replace:: :ref:`Advisory TFV-2 (CVE-2017-7564)`
.. |TFV-3| replace:: :ref:`Advisory TFV-3 (CVE-2017-7563)`
.. |TFV-4| replace:: :ref:`Advisory TFV-4 (CVE-2017-9607)`
.. |TFV-5| replace:: :ref:`Advisory TFV-5 (CVE-2017-15031)`
.. |TFV-6| replace:: :ref:`Advisory TFV-6 (CVE-2017-5753, CVE-2017-5715, CVE-2017-5754)`
.. |TFV-7| replace:: :ref:`Advisory TFV-7 (CVE-2018-3639)`
.. |TFV-8| replace:: :ref:`Advisory TFV-8 (CVE-2018-19440)`
.. |TFV-9| replace:: :ref:`Advisory TFV-9 (CVE-2022-23960)`
.. |TFV-10| replace:: :ref:`Advisory TFV-10 (CVE-2022-47630)`
.. |TFV-11| replace:: :ref:`Advisory TFV-11 (CVE-2023-49100)`
.. |TFV-12| replace:: :ref:`Advisory TFV-12 (CVE-2024-5660)`
.. |TFV-13| replace:: :ref:`Advisory TFV-13 (CVE-2024-7881)`
.. |TFV-14| replace:: :ref:`Advisory TFV-14 (CVE-2026-34879)`
.. |TFV-15| replace:: :ref:`Advisory TFV-15 (CVE-2026-34878)`
.. |TFV-16| replace:: :ref:`Advisory TFV-16 (CVE-2026-0995)`
.. |TFV-17| replace:: :ref:`Advisory TFV-17 (CVE-2025-10263)`

.. _TrustedFirmware.org security incident process: https://trusted-firmware-docs.readthedocs.io/en/latest/security_center/

--------------

*Copyright (c) 2019-2026, Arm Limited. All rights reserved.*
