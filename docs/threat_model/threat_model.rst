Generic Threat Model
********************

************
Introduction
************

This document provides a generic threat model for TF-A firmware.

.. _Target of Evaluation:

********************
Target of Evaluation
********************

In this threat model, the target of evaluation is the Trusted
Firmware for A-class Processors (TF-A). This includes the boot ROM (BL1),
the trusted boot firmware (BL2) and the runtime EL3 firmware (BL31) as
shown on Figure 1. Everything else on Figure 1 is outside of the scope of
the evaluation.

TF-A can be configured in various ways. In this threat model we consider
only the most basic configuration. To that end we make the following
assumptions:

- All TF-A images are run from either ROM or on-chip trusted SRAM. This means
  TF-A is not vulnerable to an attacker that can probe or tamper with off-chip
  memory.

- Trusted boot is enabled. This means an attacker can't boot arbitrary images
  that are not approved by platform providers.

- There is no Secure-EL2. We don't consider threats that may come with
  Secure-EL2 software.

- There are no Root and Realm worlds. These are introduced by :ref:`Realm
  Management Extension (RME)`.

  The :ref:`Threat Model for TF-A with Arm CCA support` covers these types of
  configurations.

- No experimental features are enabled. We do not consider threats that may come
  from them.


Data Flow Diagram
=================

Figure 1 shows a high-level data flow diagram for TF-A. The diagram
shows a model of the different components of a TF-A-based system and
their interactions with TF-A. A description of each diagram element
is given on Table 1. On the diagram, the red broken lines indicate
trust boundaries. Components outside of the broken lines
are considered untrusted by TF-A.

.. uml:: ../resources/diagrams/plantuml/tfa_dfd.puml
  :caption: Figure 1: TF-A Data Flow Diagram

.. table:: Table 1: TF-A Data Flow Diagram Description

  +-----------------+--------------------------------------------------------+
  | Diagram Element | Description                                            |
  +=================+========================================================+
  |       DF1       | | At boot time, images are loaded from non-volatile    |
  |                 |   memory and verified by TF-A boot firmware. These     |
  |                 |   images include TF-A BL2 and BL31 images, as well as  |
  |                 |   other secure and non-secure images.                  |
  +-----------------+--------------------------------------------------------+
  |       DF2       | | TF-A log system framework outputs debug or           |
  |                 |   informative messages over a UART interface.          |
  |                 |                                                        |
  |                 | | Also, characters can be read from a UART interface.  |
  +-----------------+--------------------------------------------------------+
  |       DF3       | | Debug and trace IP on a platform can allow access    |
  |                 |   to registers and memory of TF-A.                     |
  +-----------------+--------------------------------------------------------+
  |       DF4       | | Secure world software (e.g. trusted OS) interact     |
  |                 |   with TF-A through SMC call interface and/or shared   |
  |                 |   memory.                                              |
  +-----------------+--------------------------------------------------------+
  |       DF5       | | Non-secure world software (e.g. rich OS) interact    |
  |                 |   with TF-A through SMC call interface and/or shared   |
  |                 |   memory.                                              |
  +-----------------+--------------------------------------------------------+
  |       DF6       | | This path represents the interaction between TF-A and|
  |                 |   various hardware IPs such as TrustZone controller    |
  |                 |   and GIC. At boot time TF-A configures/initializes the|
  |                 |   IPs and interacts with them at runtime through       |
  |                 |   interrupts and registers.                            |
  +-----------------+--------------------------------------------------------+


.. _threat_analysis:

***************
Threat Analysis
***************

In this section we identify and provide assessment of potential threats to TF-A
firmware. The threats are identified for each diagram element on the
data flow diagram above.

For each threat, we identify the *asset* that is under threat, the
*threat agent* and the *threat type*. Each threat is given a *risk rating*
that represents the impact and likelihood of that threat. We also discuss
potential mitigations.

Assets
======

We have identified the following assets for TF-A:

.. table:: Table 2: TF-A Assets

  +--------------------+---------------------------------------------------+
  | Asset              | Description                                       |
  +====================+===================================================+
  | Sensitive Data     | | These include sensitive data that an attacker   |
  |                    |   must not be able to tamper with (e.g. the Root  |
  |                    |   of Trust Public Key) or see (e.g. secure logs,  |
  |                    |   debugging information such as crash reports).   |
  +--------------------+---------------------------------------------------+
  | Code Execution     | | This represents the requirement that the        |
  |                    |   platform should run only TF-A code approved by  |
  |                    |   the platform provider.                          |
  +--------------------+---------------------------------------------------+
  | Availability       | | This represents the requirement that TF-A       |
  |                    |   services should always be available for use.    |
  +--------------------+---------------------------------------------------+

Threat Agents
=============

To understand the attack surface, it is important to identify potential
attackers, i.e. attack entry points. The following threat agents are
in scope of this threat model.

.. table:: Table 3: Threat Agents

  +-------------------+-------------------------------------------------------+
  | Threat Agent      | Description                                           |
  +===================+=======================================================+
  |   NSCode          | | Malicious or faulty code running in the Non-secure  |
  |                   |   world, including NS-EL0 NS-EL1 and NS-EL2 levels    |
  +-------------------+-------------------------------------------------------+
  |   SecCode         | | Malicious or faulty code running in the secure      |
  |                   |   world, including S-EL0 and S-EL1 levels             |
  +-------------------+-------------------------------------------------------+
  |   AppDebug        | | Physical attacker using  debug signals to access    |
  |                   |   TF-A resources                                      |
  +-------------------+-------------------------------------------------------+
  |  PhysicalAccess   | | Physical attacker having access to external device  |
  |                   |   communication bus and to external flash             |
  |                   |   communication bus using common hardware             |
  +-------------------+-------------------------------------------------------+

.. note::

  In this threat model an advanced physical attacker that has the capability
  to tamper with a hardware (e.g. "rewiring" a chip using a focused
  ion beam (FIB) workstation or decapsulate the chip using chemicals) is
  considered out-of-scope.

  Certain non-invasive physical attacks that do not need modifications to the
  chip, notably those like Power Analysis Attacks, are out-of-scope. Power
  analysis side-channel attacks represent a category of security threats that
  capitalize on information leakage through a device's power consumption during
  its normal operation. These attacks leverage the correlation between a
  device's power usage and its internal data processing activities. This
  correlation provides attackers with the means to extract sensitive
  information, including cryptographic keys.

Threat Types
============

In this threat model we categorize threats using the `STRIDE threat
analysis technique`_. In this technique a threat is categorized as one
or more of these types: ``Spoofing``, ``Tampering``, ``Repudiation``,
``Information disclosure``, ``Denial of service`` or
``Elevation of privilege``.

Threat Risk Ratings
===================

For each threat identified, a risk rating that ranges
from *informational* to *critical* is given based on the likelihood of the
threat occurring if a mitigation is not in place, and the impact of the
threat (i.e. how severe the consequences could be). Table 4 explains each
rating in terms of score, impact and likelihood.

.. table:: Table 4: Rating and score as applied to impact and likelihood

  +-----------------------+-------------------------+---------------------------+
  | **Rating (Score)**    | **Impact**              | **Likelihood**            |
  +=======================+=========================+===========================+
  | Critical (5)          | | Extreme impact to     | | Threat is almost        |
  |                       |   entire organization   |   certain to be exploited.|
  |                       |   if exploited.         |                           |
  |                       |                         | | Knowledge of the threat |
  |                       |                         |   and how to exploit it   |
  |                       |                         |   are in the public       |
  |                       |                         |   domain.                 |
  +-----------------------+-------------------------+---------------------------+
  | High (4)              | | Major impact to entire| | Threat is relatively    |
  |                       |   organization or single|   easy to detect and      |
  |                       |   line of business if   |   exploit by an attacker  |
  |                       |   exploited             |   with little skill.      |
  +-----------------------+-------------------------+---------------------------+
  | Medium (3)            | | Noticeable impact to  | | A knowledgeable insider |
  |                       |   line of business if   |   or expert attacker could|
  |                       |   exploited.            |   exploit the threat      |
  |                       |                         |   without much difficulty.|
  +-----------------------+-------------------------+---------------------------+
  | Low (2)               | | Minor damage if       | | Exploiting the threat   |
  |                       |   exploited or could    |   would require           |
  |                       |   be used in conjunction|   considerable expertise  |
  |                       |   with other            |   and resources           |
  |                       |   vulnerabilities to    |                           |
  |                       |   perform a more serious|                           |
  |                       |   attack                |                           |
  +-----------------------+-------------------------+---------------------------+
  | Informational (1)     | | Poor programming      | | Threat is not likely    |
  |                       |   practice or poor      |   to be exploited on its  |
  |                       |   design decision that  |   own, but may be used to |
  |                       |   may not represent an  |   gain information for    |
  |                       |   immediate risk on its |   launching another       |
  |                       |   own, but may have     |   attack                  |
  |                       |   security implications |                           |
  |                       |   if multiplied and/or  |                           |
  |                       |   combined with other   |                           |
  |                       |   threats.              |                           |
  +-----------------------+-------------------------+---------------------------+

Aggregate risk scores are assigned to identified threats;
specifically, the impact score multiplied by the likelihood score.
For example, a threat with high likelihood and low impact would have an
aggregate risk score of eight (8); that is, four (4) for high likelihood
multiplied by two (2) for low impact. The aggregate risk score determines
the finding's overall risk level, as shown in the following table.

.. table:: Table 5: Overall risk levels and corresponding aggregate scores

  +---------------------+-----------------------------------+
  | Overall Risk Level  | Aggregate Risk Score              |
  |                     | (Impact multiplied by Likelihood) |
  +=====================+===================================+
  | Critical            | 20–25                             |
  +---------------------+-----------------------------------+
  | High                | 12–19                             |
  +---------------------+-----------------------------------+
  | Medium              | 6–11                              |
  +---------------------+-----------------------------------+
  | Low                 | 2–5                               |
  +---------------------+-----------------------------------+
  | Informational       | 1                                 |
  +---------------------+-----------------------------------+

The likelihood and impact of a threat depends on the
target environment in which TF-A is running. For example, attacks
that require physical access are unlikely in server environments while
they are more common in Internet of Things(IoT) environments.
In this threat model we consider three target environments:
``Internet of Things(IoT)``, ``Mobile`` and ``Server``.

Threat Assessment
=================

The following threats were identified by applying STRIDE analysis on
each diagram element of the data flow diagram.

For each threat, we strive to indicate whether the mitigations are currently
implemented or not. However, the answer to this question is not always straight
forward. Some mitigations are partially implemented in the generic code but also
rely on the platform code to implement some bits of it. This threat model aims
to be platform-independent and it is important to keep in mind that such threats
only get mitigated if the platform code properly fulfills its responsibilities.

Also, some mitigations require enabling specific features, which must be
explicitly turned on via a build flag.

When such conditions must be met, these are highlighted in the ``Mitigations
implemented?`` box.

As our :ref:`Target of Evaluation` is made of several, distinct firmware images,
some threats are confined in specific images, while others apply to each of
them. To help developers implement mitigations in the right place, threats below
are categorized based on the firmware image that should mitigate them.

.. _General Threats:

General Threats for All Firmware Images
---------------------------------------

+------------------------+---------------------------------------------------+
| ID                     | 05                                                |
+========================+===================================================+
| Threat                 | | **Information leak via UART logs**              |
|                        |                                                   |
|                        | | During the development stages of software it is |
|                        |   common to print all sorts of information on the |
|                        |   console, including sensitive or confidential    |
|                        |   information such as crash reports with detailed |
|                        |   information of the CPU state, current registers |
|                        |   values, privilege level or stack dumps.         |
|                        |                                                   |
|                        | | This information is useful when debugging       |
|                        |   problems before releasing the production        |
|                        |   version but it could be used by an attacker     |
|                        |   to develop a working exploit if left enabled in |
|                        |   the production version.                         |
|                        |                                                   |
|                        | | This happens when directly logging sensitive    |
|                        |   information and more subtly when logging        |
|                        |   side-channel information that can be used by an |
|                        |   attacker to learn about sensitive information.  |
+------------------------+---------------------------------------------------+
| Diagram Elements       | DF2                                               |
+------------------------+---------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                    |
| Components             |                                                   |
+------------------------+---------------------------------------------------+
| Assets                 | Sensitive Data                                    |
+------------------------+---------------------------------------------------+
| Threat Agent           | AppDebug                                          |
+------------------------+---------------------------------------------------+
| Threat Type            | Information Disclosure                            |
+------------------------+------------------+----------------+---------------+
| Application            | Server           | IoT            | Mobile        |
+------------------------+------------------+----------------+---------------+
| Impact                 | N/A              | Low (2)        | Low (2)       |
+------------------------+------------------+----------------+---------------+
| Likelihood             | N/A              | High (4)       | High (4)      |
+------------------------+------------------+----------------+---------------+
| Total Risk Rating      | N/A              | Medium (8)     | Medium (8)    |
+------------------------+------------------+----------------+---------------+
| Mitigations            | | Remove sensitive information logging in         |
|                        |   production releases.                            |
|                        |                                                   |
|                        | | Do not conditionally log information depending  |
|                        |   on potentially sensitive data.                  |
|                        |                                                   |
|                        | | Do not log high precision timing information.   |
+------------------------+---------------------------------------------------+
| Mitigations            | | Yes / Platform Specific.                        |
| implemented?           |   Requires the right build options to be used.    |
|                        |                                                   |
|                        | | Crash reporting is only enabled for debug       |
|                        |   builds by default, see ``CRASH_REPORTING``      |
|                        |   build option.                                   |
|                        |                                                   |
|                        | | The log level can be tuned at build time, from  |
|                        |   very verbose to no output at all. See           |
|                        |   ``LOG_LEVEL`` build option. By default, release |
|                        |   builds are a lot less verbose than debug ones   |
|                        |   but still produce some output.                  |
|                        |                                                   |
|                        | | Messages produced by the platform code should   |
|                        |   use the appropriate level of verbosity so as    |
|                        |   not to leak sensitive information in production |
|                        |   builds.                                         |
+------------------------+---------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 06                                                 |
+========================+====================================================+
| Threat                 | | **An attacker can read sensitive data and        |
|                        |   execute arbitrary code through the external      |
|                        |   debug and trace interface**                      |
|                        |                                                    |
|                        | | Arm processors include hardware-assisted debug   |
|                        |   and trace features that can be controlled without|
|                        |   the need for software operating on the platform. |
|                        |   If left enabled without authentication, this     |
|                        |   feature can be used by an attacker to inspect and|
|                        |   modify TF-A registers and memory allowing the    |
|                        |   attacker to read sensitive data and execute      |
|                        |   arbitrary code.                                  |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF3                                                |
+------------------------+----------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                     |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Code Execution, Sensitive Data                     |
+------------------------+----------------------------------------------------+
| Threat Agent           | AppDebug                                           |
+------------------------+----------------------------------------------------+
| Threat Type            | Tampering, Information Disclosure,                 |
|                        | Elevation of privilege                             |
+------------------------+------------------+---------------+-----------------+
| Application            | Server           | IoT           | Mobile          |
+------------------------+------------------+---------------+-----------------+
| Impact                 | N/A              | High (4)      | High (4)        |
+------------------------+------------------+---------------+-----------------+
| Likelihood             | N/A              | Critical (5)  | Critical (5)    |
+------------------------+------------------+---------------+-----------------+
| Total Risk Rating      | N/A              | Critical (20) | Critical (20)   |
+------------------------+------------------+---------------+-----------------+
| Mitigations            | Disable the debug and trace capability for         |
|                        | production releases or enable proper debug         |
|                        | authentication as recommended by [`DEN0034`_].     |
+------------------------+----------------------------------------------------+
| Mitigations            | | Platform specific.                               |
| implemented?           |                                                    |
|                        | | Configuration of debug and trace capabilities is |
|                        |   entirely platform specific.                      |
+------------------------+----------------------------------------------------+

+------------------------+------------------------------------------------------+
| ID                     | 08                                                   |
+========================+======================================================+
| Threat                 | | **Memory corruption due to memory overflows and    |
|                        |   lack of boundary checking when accessing resources |
|                        |   could allow an attacker to execute arbitrary code, |
|                        |   modify some state variable to change the normal    |
|                        |   flow of the program, or leak sensitive             |
|                        |   information**                                      |
|                        |                                                      |
|                        | | Like in other software, TF-A has multiple points   |
|                        |   where memory corruption security errors can arise. |
|                        |                                                      |
|                        | | Some of the errors include integer overflow,       |
|                        |   buffer overflow, incorrect array boundary checks,  |
|                        |   and incorrect error management.                    |
|                        |   Improper use of asserts instead of proper input    |
|                        |   validations might also result in these kinds of    |
|                        |   errors in release builds.                          |
+------------------------+------------------------------------------------------+
| Diagram Elements       | DF4, DF5                                             |
+------------------------+------------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                       |
| Components             |                                                      |
+------------------------+------------------------------------------------------+
| Assets                 | Code Execution, Sensitive Data                       |
+------------------------+------------------------------------------------------+
| Threat Agent           | NSCode, SecCode                                      |
+------------------------+------------------------------------------------------+
| Threat Type            | Tampering, Information Disclosure,                   |
|                        | Elevation of Privilege                               |
+------------------------+-------------------+-----------------+----------------+
| Application            | Server            | IoT             | Mobile         |
+------------------------+-------------------+-----------------+----------------+
| Impact                 | Critical (5)      | Critical (5)    | Critical (5)   |
+------------------------+-------------------+-----------------+----------------+
| Likelihood             | Medium (3         | Medium (3)      | Medium (3)     |
+------------------------+-------------------+-----------------+----------------+
| Total Risk Rating      | High (15)         | High (15)       | High (15)      |
+------------------------+-------------------+-----------------+----------------+
| Mitigations            | | 1) Use proper input validation.                    |
|                        |                                                      |
|                        | | 2) Code reviews, testing.                          |
+------------------------+------------------------------------------------------+
| Mitigations            | | 1) Yes.                                            |
| implemented?           |   Data received from normal world, such as addresses |
|                        |   and sizes identifying memory regions, are          |
|                        |   sanitized before being used. These security checks |
|                        |   make sure that the normal world software does not  |
|                        |   access memory beyond its limit.                    |
|                        |                                                      |
|                        | | By default *asserts* are only used to check for    |
|                        |   programming errors in debug builds. Other types of |
|                        |   errors are handled through condition checks that   |
|                        |   remain enabled in release builds. See              |
|                        |   `TF-A error handling policy`_. TF-A provides an    |
|                        |   option to use *asserts* in release builds, however |
|                        |   we recommend using proper runtime checks instead   |
|                        |   of relying on asserts in release builds.           |
|                        |                                                      |
|                        | | 2) Yes.                                            |
|                        |   TF-A uses a combination of manual code reviews     |
|                        |   and automated program analysis and testing to      |
|                        |   detect and fix memory corruption bugs. All TF-A    |
|                        |   code including platform code go through manual     |
|                        |   code reviews. Additionally, static code analysis   |
|                        |   is performed using Coverity Scan on all TF-A code. |
|                        |   The code is also tested  with                      |
|                        |   `Trusted Firmware-A Tests`_ on Juno and FVP        |
|                        |   platforms.                                         |
+------------------------+------------------------------------------------------+


+------------------------+----------------------------------------------------+
| ID                     | 11                                                 |
+========================+====================================================+
| Threat                 | | **Misconfiguration of the Memory Management Unit |
|                        |   (MMU) may allow a normal world software to       |
|                        |   access sensitive data, execute arbitrary         |
|                        |   code or access otherwise restricted HW           |
|                        |   interface**                                      |
|                        |                                                    |
|                        | | A misconfiguration of the MMU could              |
|                        |   lead to an open door for software running in the |
|                        |   normal world to access sensitive data or even    |
|                        |   execute code if the proper security mechanisms   |
|                        |   are not in place.                                |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF5, DF6                                           |
+------------------------+----------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                     |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Sensitive Data, Code execution                     |
+------------------------+----------------------------------------------------+
| Threat Agent           | NSCode                                             |
+------------------------+----------------------------------------------------+
| Threat Type            | Information Disclosure, Elevation of Privilege     |
+------------------------+-----------------+-----------------+----------------+
| Application            | Server          | IoT             | Mobile         |
+------------------------+-----------------+-----------------+----------------+
| Impact                 | Critical (5)    | Critical (5)    | Critical (5)   |
+------------------------+-----------------+-----------------+----------------+
| Likelihood             | High (4)        | High (4)        | High (4)       |
+------------------------+-----------------+-----------------+----------------+
| Total Risk Rating      | Critical (20)   | Critical (20)   | Critical (20)  |
+------------------------+-----------------+-----------------+----------------+
| Mitigations            | When configuring access permissions, the           |
|                        | principle of least privilege ought to be           |
|                        | enforced. This means we should not grant more      |
|                        | privileges than strictly needed, e.g. code         |
|                        | should be read-only executable, read-only data     |
|                        | should be read-only execute-never, and so on.      |
+------------------------+----------------------------------------------------+
| Mitigations            | | Platform specific.                               |
| implemented?           |                                                    |
|                        | | MMU configuration is platform specific,          |
|                        |   therefore platforms need to make sure that the   |
|                        |   correct attributes are assigned to memory        |
|                        |   regions.                                         |
|                        |                                                    |
|                        | | TF-A provides a library which abstracts the      |
|                        |   low-level details of MMU configuration. It       |
|                        |   provides well-defined and tested APIs.           |
|                        |   Platforms are encouraged to use it to limit the  |
|                        |   risk of misconfiguration.                        |
+------------------------+----------------------------------------------------+


+------------------------+-----------------------------------------------------+
| ID                     | 13                                                  |
+========================+=====================================================+
| Threat                 | | **Leaving sensitive information in the memory,    |
|                        |   can allow an attacker to retrieve them.**         |
|                        |                                                     |
|                        | | Accidentally leaving not-needed sensitive data in |
|                        |   internal buffers can leak them if an attacker     |
|                        |   gains access to memory due to a vulnerability.    |
+------------------------+-----------------------------------------------------+
| Diagram Elements       | DF4, DF5                                            |
+------------------------+-----------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                      |
| Components             |                                                     |
+------------------------+-----------------------------------------------------+
| Assets                 | Sensitive Data                                      |
+------------------------+-----------------------------------------------------+
| Threat Agent           | NSCode, SecCode                                     |
+------------------------+-----------------------------------------------------+
| Threat Type            | Information Disclosure                              |
+------------------------+-------------------+----------------+----------------+
| Application            | Server            | IoT            | Mobile         |
+------------------------+-------------------+----------------+----------------+
| Impact                 |  Critical (5)     | Critical (5)   | Critical (5)   |
+------------------------+-------------------+----------------+----------------+
| Likelihood             |  Medium (3)       | Medium (3)     | Medium (3)     |
+------------------------+-------------------+----------------+----------------+
| Total Risk Rating      |  High (15)        | High (15)      | High (15)      |
+------------------------+-------------------+----------------+----------------+
| Mitigations            |   Clear the sensitive data from internal buffers as |
|                        |   soon as they are not needed anymore.              |
+------------------------+-----------------------------------------------------+
| Mitigations            | | Yes / Platform specific                           |
| implemented?           |                                                     |
+------------------------+-----------------------------------------------------+


+------------------------+-----------------------------------------------------+
| ID                     | 15                                                  |
+========================+=====================================================+
| Threat                 | | **Improper handling of input data received over   |
|                        |   a UART interface may allow an attacker to tamper  |
|                        |   with TF-A execution environment.**                |
|                        |                                                     |
|                        | | The consequences of the attack depend on the      |
|                        |   the exact usage of input data received over UART. |
|                        |   Examples are injection of arbitrary data,         |
|                        |   sensitive data tampering, influencing the         |
|                        |   execution path, denial of service (if using       |
|                        |   blocking I/O). This list may not be exhaustive.   |
+------------------------+-----------------------------------------------------+
| Diagram Elements       | DF2, DF4, DF5                                       |
+------------------------+-----------------------------------------------------+
| Affected TF-A          | BL1, BL2, BL31                                      |
| Components             |                                                     |
+------------------------+-----------------------------------------------------+
| Assets                 | Sensitive Data, Code Execution, Availability        |
+------------------------+-----------------------------------------------------+
| Threat Agent           | NSCode, SecCode                                     |
+------------------------+-----------------------------------------------------+
| Threat Type            | Tampering, Information Disclosure, Denial of        |
|                        | service, Elevation of privilege.                    |
+------------------------+-------------------+----------------+----------------+
| Application            | Server            | IoT            | Mobile         |
+------------------------+-------------------+----------------+----------------+
| Impact                 |  Critical (5)     | Critical (5)   | Critical (5)   |
+------------------------+-------------------+----------------+----------------+
| Likelihood             |  Critical (5)     | Critical (5)   | Critical (5)   |
+------------------------+-------------------+----------------+----------------+
| Total Risk Rating      |  Critical (25)    | Critical (25)  | Critical (25)  |
+------------------------+-------------------+----------------+----------------+
| Mitigations            | | By default, the code to read input data from UART |
|                        |   interfaces is disabled (see `ENABLE_CONSOLE_GETC` |
|                        |   build option). It should only be enabled on a     |
|                        |   need basis.                                       |
|                        |                                                     |
|                        | | Data received over UART interfaces should be      |
|                        |   treated as untrusted data. As such, it should be  |
|                        |   properly sanitized and handled with caution.      |
+------------------------+-----------------------------------------------------+
| Mitigations            | | Platform specific.                                |
| implemented?           |                                                     |
|                        | | Generic code does not read any input data from    |
|                        |   UART interface(s).                                |
+------------------------+-----------------------------------------------------+


.. _Boot Firmware Threats:

Threats to be Mitigated by the Boot Firmware
--------------------------------------------

The boot firmware here refers to the boot ROM (BL1) and the trusted boot
firmware (BL2). Typically it does not stay resident in memory and it is
dismissed once execution has reached the runtime EL3 firmware (BL31). Thus, past
that point in time, the threats below can no longer be exploited.

Note, however, that this is not necessarily true on all platforms. Platform
vendors should review these threats to make sure they cannot be exploited
nonetheless once execution has reached the runtime EL3 firmware.

+------------------------+----------------------------------------------------+
| ID                     | 01                                                 |
+========================+====================================================+
| Threat                 | | **An attacker can mangle firmware images to      |
|                        |   execute arbitrary code**                         |
|                        |                                                    |
|                        | | Some TF-A images are loaded from external        |
|                        |   storage. It is possible for an attacker to access|
|                        |   the external flash memory and change its contents|
|                        |   physically, through the Rich OS, or using the    |
|                        |   updating mechanism to modify the non-volatile    |
|                        |   images to execute arbitrary code.                |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF4, DF5                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | BL2, BL31                                          |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Code Execution                                     |
+------------------------+----------------------------------------------------+
| Threat Agent           | PhysicalAccess, NSCode, SecCode                    |
+------------------------+----------------------------------------------------+
| Threat Type            | Tampering, Elevation of Privilege                  |
+------------------------+------------------+-----------------+---------------+
| Application            | Server           | IoT             | Mobile        |
+------------------------+------------------+-----------------+---------------+
| Impact                 | Critical (5)     | Critical (5)    | Critical (5)  |
+------------------------+------------------+-----------------+---------------+
| Likelihood             | Critical (5)     | Critical (5)    | Critical (5)  |
+------------------------+------------------+-----------------+---------------+
| Total Risk Rating      | Critical (25)    | Critical (25)   | Critical (25) |
+------------------------+------------------+-----------------+---------------+
| Mitigations            | | 1) Implement the `Trusted Board Boot (TBB)`_     |
|                        |   feature which prevents malicious firmware from   |
|                        |   running on the platform by authenticating all    |
|                        |   firmware images.                                 |
|                        |                                                    |
|                        | | 2) Perform extra checks on unauthenticated data, |
|                        |   such as FIP metadata, prior to use.              |
+------------------------+----------------------------------------------------+
| Mitigations            | | 1) Yes, provided that the ``TRUSTED_BOARD_BOOT`` |
| implemented?           |   build option is set to 1.                        |
|                        |                                                    |
|                        | | 2) Yes.                                          |
+------------------------+----------------------------------------------------+

+------------------------+----------------------------------------------------+
| ID                     | 02                                                 |
+========================+====================================================+
| Threat                 | | **An attacker may attempt to boot outdated,      |
|                        |   potentially vulnerable firmware image**          |
|                        |                                                    |
|                        | | When updating firmware, an attacker may attempt  |
|                        |   to rollback to an older version that has unfixed |
|                        |   vulnerabilities.                                 |
+------------------------+----------------------------------------------------+
| Diagram Elements       | DF1, DF4, DF5                                      |
+------------------------+----------------------------------------------------+
| Affected TF-A          | BL2, BL31                                          |
| Components             |                                                    |
+------------------------+----------------------------------------------------+
| Assets                 | Code Execution                                     |
+------------------------+----------------------------------------------------+
| Threat Agent           | PhysicalAccess, NSCode, SecCode                    |
+------------------------+----------------------------------------------------+
| Threat Type            | Tampering                                          |
+------------------------+------------------+-----------------+---------------+
| Application            | Server           | IoT             | Mobile        |
+------------------------+------------------+-----------------+---------------+
| Impact                 | Critical (5)     | Critical (5)    | Critical (5)  |
+------------------------+------------------+-----------------+---------------+
| Likelihood             | Critical (5)     | Critical (5)    | Critical (5)  |
+------------------------+------------------+-----------------+---------------+
| Total Risk Rating      | Critical (25)    | Critical (25)   | Critical (25) |
+------------------------+------------------+-----------------+---------------+
| Mitigations            | Implement anti-rollback protection using           |
|                        | non-volatile counters (NV counters) as required    |
|                        | by `TBBR-Client specification`_.                   |
+------------------------+----------------------------------------------------+
| Mitigations            | | Yes / Platform specific.                         |
| implemented?           |                                                    |
|                        | | After a firmware image is validated, the image   |
|                        |   revision number taken from a certificate         |
|                        |   extension field is compared with the             |
|                        |   corresponding NV counter stored in hardware to   |
|                        |   make sure the new counter value is larger than   |
|                        |   the current counter value.                       |
|                        |                                                    |
|                        | | **Platforms must implement this protection using |
|                        |   platform specific hardware NV counters.**        |
+------------------------+----------------------------------------------------+


+------------------------+-------------------------------------------------------+
| ID                     | 03                                                    |
+========================+=======================================================+
| Threat                 | | **An attacker can use Time-of-Check-Time-of-Use     |
|                        |   (TOCTOU) attack to bypass image authentication      |
|                        |   during the boot process**                           |
|                        |                                                       |
|                        | | Time-of-Check-Time-of-Use (TOCTOU) threats occur    |
|                        |   when the security check is produced before the time |
|                        |   the resource is accessed. If an attacker is sitting |
|                        |   in the middle of the off-chip images, they could    |
|                        |   change the binary containing executable code right  |
|                        |   after the integrity and authentication check has    |
|                        |   been performed.                                     |
+------------------------+-------------------------------------------------------+
| Diagram Elements       | DF1                                                   |
+------------------------+-------------------------------------------------------+
| Affected TF-A          | BL1, BL2                                              |
| Components             |                                                       |
+------------------------+-------------------------------------------------------+
| Assets                 | Code Execution, Sensitive Data                        |
+------------------------+-------------------------------------------------------+
| Threat Agent           | PhysicalAccess                                        |
+------------------------+-------------------------------------------------------+
| Threat Type            | Elevation of Privilege                                |
+------------------------+---------------------+-----------------+---------------+
| Application            | Server              | IoT             | Mobile        |
+------------------------+---------------------+-----------------+---------------+
| Impact                 | N/A                 | Critical (5)    | Critical (5)  |
+------------------------+---------------------+-----------------+---------------+
| Likelihood             | N/A                 | Medium (3)      | Medium (3)    |
+------------------------+---------------------+-----------------+---------------+
| Total Risk Rating      | N/A                 | High (15)       | High (15)     |
+------------------------+---------------------+-----------------+---------------+
| Mitigations            | Copy image to on-chip memory before authenticating    |
|                        | it.                                                   |
+------------------------+-------------------------------------------------------+
| Mitigations            | | Platform specific.                                  |
| implemented?           |                                                       |
|                        | | The list of images to load and their location is    |
|                        |   platform specific. Platforms are responsible for    |
|                        |   arranging images to be loaded in on-chip memory.    |
+------------------------+-------------------------------------------------------+


+------------------------+-------------------------------------------------------+
| ID                     | 04                                                    |
+========================+=======================================================+
| Threat                 | | **An attacker with physical access can execute      |
|                        |   arbitrary image by bypassing the signature          |
|                        |   verification stage using glitching techniques**     |
|                        |                                                       |
|                        | | Glitching (Fault injection) attacks attempt to put  |
|                        |   a hardware into a undefined state by manipulating an|
|                        |   environmental variable such as power supply.        |
|                        |                                                       |
|                        | | TF-A relies on a chain of trust that starts with the|
|                        |   ROTPK, which is the key stored inside the chip and  |
|                        |   the root of all validation processes. If an attacker|
|                        |   can break this chain of trust, they could execute   |
|                        |   arbitrary code on the device. This could be         |
|                        |   achieved with physical access to the device by      |
|                        |   attacking the normal execution flow of the          |
|                        |   process using glitching techniques that target      |
|                        |   points where the image is validated against the     |
|                        |   signature.                                          |
+------------------------+-------------------------------------------------------+
| Diagram Elements       | DF1                                                   |
+------------------------+-------------------------------------------------------+
| Affected TF-A          | BL1, BL2                                              |
| Components             |                                                       |
+------------------------+-------------------------------------------------------+
| Assets                 | Code Execution                                        |
+------------------------+-------------------------------------------------------+
| Threat Agent           | PhysicalAccess                                        |
+------------------------+-------------------------------------------------------+
| Threat Type            | Tampering, Elevation of Privilege                     |
+------------------------+---------------------+-----------------+---------------+
| Application            | Server              | IoT             | Mobile        |
+------------------------+---------------------+-----------------+---------------+
| Impact                 | N/A                 | Critical (5)    | Critical (5)  |
+------------------------+---------------------+-----------------+---------------+
| Likelihood             | N/A                 | Medium (3)      | Medium (3)    |
+------------------------+---------------------+-----------------+---------------+
| Total Risk Rating      | N/A                 | High (15)       | High (15)     |
+------------------------+---------------------+-----------------+---------------+
| Mitigations            | Mechanisms to detect clock glitch and power           |
|                        | variations.                                           |
+------------------------+-------------------------------------------------------+
| Mitigations            | | No.                                                 |
| implemented?           |                                                       |
|                        | | The most effective mitigation is adding glitching   |
|                        |   detection and mitigation circuit at the hardware    |
|                        |   level.                                              |
|                        |                                                       |
|                        | | However, software techniques, such as adding        |
|                        |   redundant checks when performing conditional        |
|                        |   branches that are security sensitive, can be used   |
|                        |   to harden TF-A against such attacks.                |
|                        |   **At the moment TF-A doesn't implement such         |
|                        |   mitigations.**                                      |
+------------------------+-------------------------------------------------------+

.. topic:: Measured Boot Threats (or lack of)

 In the current Measured Boot design, BL1, BL2, and BL31, as well as the
 secure world components, form the |SRTM|. Measurement data is currently
 considered an asset to be protected against attack, and this is achieved
 by storing them in the Secure Memory.
 Beyond the measurements stored inside the TCG-compliant Event Log buffer,
 there are no other assets to protect or threats to defend against that
 could compromise |TF-A| execution environment's security.

 There are general security assets and threats associated with remote/delegated
 attestation. However, these are outside the |TF-A| security boundary and
 should be dealt with by the appropriate agent in the platform/system.
 Since current Measured Boot design does not use local attestation, there would
 be no further assets to protect(like unsealed keys).

 A limitation of the current Measured Boot design is that it is dependent upon
 Secure Boot as implementation of Measured Boot does not extend measurements
 into a discrete |TPM|, where they would be securely stored and protected
 against tampering. This implies that if Secure-Boot is compromised, Measured
 Boot may also be compromised.

 Platforms must carefully evaluate the security of the default implementation
 since the |SRTM| includes all secure world components.


.. _Runtime Firmware Threats:

Threats to be Mitigated by the Runtime EL3 Firmware
---------------------------------------------------

+------------------------+------------------------------------------------------+
| ID                     | 07                                                   |
+========================+======================================================+
| Threat                 | | **An attacker can perform a denial-of-service      |
|                        |   attack by using a broken SMC call that causes the  |
|                        |   system to reboot or enter into unknown state.**    |
|                        |                                                      |
|                        | | Secure and non-secure clients access TF-A services |
|                        |   through SMC calls. Malicious code can attempt to   |
|                        |   place the TF-A runtime into an inconsistent state  |
|                        |   by calling unimplemented SMC call or by passing    |
|                        |   invalid arguments.                                 |
+------------------------+------------------------------------------------------+
| Diagram Elements       | DF4, DF5                                             |
+------------------------+------------------------------------------------------+
| Affected TF-A          | BL31                                                 |
| Components             |                                                      |
+------------------------+------------------------------------------------------+
| Assets                 | Availability                                         |
+------------------------+------------------------------------------------------+
| Threat Agent           | NSCode, SecCode                                      |
+------------------------+------------------------------------------------------+
| Threat Type            | Denial of Service                                    |
+------------------------+-------------------+----------------+-----------------+
| Application            | Server            | IoT            | Mobile          |
+------------------------+-------------------+----------------+-----------------+
| Impact                 | Medium (3)        | Medium (3)     | Medium (3)      |
+------------------------+-------------------+----------------+-----------------+
| Likelihood             | High (4)          | High (4)       | High (4)        |
+------------------------+-------------------+----------------+-----------------+
| Total Risk Rating      | High (12)         | High (12)      | High (12)       |
+------------------------+-------------------+----------------+-----------------+
| Mitigations            | Validate SMC function ids and arguments before using |
|                        | them.                                                |
+------------------------+------------------------------------------------------+
| Mitigations            | | Yes / Platform specific.                           |
| implemented?           |                                                      |
|                        | | For standard services, all input is validated.     |
|                        |                                                      |
|                        | | Platforms that implement SiP services must also    |
|                        |   validate SMC call arguments.                       |
+------------------------+------------------------------------------------------+


+------------------------+------------------------------------------------------+
| ID                     | 09                                                   |
+========================+======================================================+
| Threat                 | | **Improperly handled SMC calls can leak register   |
|                        |   contents**                                         |
|                        |                                                      |
|                        | | When switching between worlds, TF-A register state |
|                        |   can leak to software in different security         |
|                        |   contexts.                                          |
+------------------------+------------------------------------------------------+
| Diagram Elements       | DF4, DF5                                             |
+------------------------+------------------------------------------------------+
| Affected TF-A          | BL31                                                 |
| Components             |                                                      |
+------------------------+------------------------------------------------------+
| Assets                 | Sensitive Data                                       |
+------------------------+------------------------------------------------------+
| Threat Agent           | NSCode, SecCode                                      |
+------------------------+------------------------------------------------------+
| Threat Type            | Information Disclosure                               |
+------------------------+-------------------+----------------+-----------------+
| Application            | Server            | IoT            | Mobile          |
+------------------------+-------------------+----------------+-----------------+
| Impact                 | Medium (3)        | Medium (3)     | Medium (3)      |
+------------------------+-------------------+----------------+-----------------+
| Likelihood             | High (4)          | High (4)       | High (4)        |
+------------------------+-------------------+----------------+-----------------+
| Total Risk Rating      | High (12)         | High (12)      | High (12)       |
+------------------------+-------------------+----------------+-----------------+
| Mitigations            | Save and restore registers when switching contexts.  |
+------------------------+------------------------------------------------------+
| Mitigations            | | Yes.                                               |
| implemented?           |                                                      |
|                        | | This is the default behaviour in TF-A.             |
|                        |   Build options are also provided to save/restore    |
|                        |   additional registers such as floating-point        |
|                        |   registers. These should be enabled if required.    |
+------------------------+------------------------------------------------------+

+------------------------+-----------------------------------------------------+
| ID                     | 10                                                  |
+========================+=====================================================+
| Threat                 | | **SMC calls can leak sensitive information from   |
|                        |   TF-A memory via microarchitectural side channels**|
|                        |                                                     |
|                        | | Microarchitectural side-channel attacks such as   |
|                        |   `Spectre`_ can be used to leak data across        |
|                        |   security boundaries. An attacker might attempt to |
|                        |   use this kind of attack to leak sensitive         |
|                        |   data from TF-A memory.                            |
+------------------------+-----------------------------------------------------+
| Diagram Elements       | DF4, DF5                                            |
+------------------------+-----------------------------------------------------+
| Affected TF-A          | BL31                                                |
| Components             |                                                     |
+------------------------+-----------------------------------------------------+
| Assets                 | Sensitive Data                                      |
+------------------------+-----------------------------------------------------+
| Threat Agent           | SecCode, NSCode                                     |
+------------------------+-----------------------------------------------------+
| Threat Type            | Information Disclosure                              |
+------------------------+-------------------+----------------+----------------+
| Application            | Server            | IoT            | Mobile         |
+------------------------+-------------------+----------------+----------------+
| Impact                 | Medium (3)        | Medium (3)     | Medium (3)     |
+------------------------+-------------------+----------------+----------------+
| Likelihood             | Medium (3)        | Medium (3)     | Medium (3)     |
+------------------------+-------------------+----------------+----------------+
| Total Risk Rating      | Medium (9)        | Medium (9)     | Medium (9)     |
+------------------------+-------------------+----------------+----------------+
| Mitigations            | Enable appropriate side-channel protections.        |
+------------------------+-----------------------------------------------------+
| Mitigations            | | Yes / Platform specific.                          |
| implemented?           |                                                     |
|                        | | TF-A implements software mitigations for Spectre  |
|                        |   type attacks as recommended by `Cache Speculation |
|                        |   Side-channels`_ for the generic code.             |
|                        |                                                     |
|                        | | SiPs should implement similar mitigations for     |
|                        |   code that is deemed to be vulnerable to such      |
|                        |   attacks.                                          |
+------------------------+-----------------------------------------------------+


+------------------------+-----------------------------------------------------+
| ID                     | 12                                                  |
+========================+=====================================================+
| Threat                 | | **Incorrect configuration of Performance Monitor  |
|                        |   Unit (PMU) counters can allow an attacker to      |
|                        |   mount side-channel attacks using information      |
|                        |   exposed by the counters**                         |
|                        |                                                     |
|                        | | Non-secure software can configure PMU registers   |
|                        |   to count events at any exception level and in     |
|                        |   both Secure and Non-secure states. This allows    |
|                        |   a Non-secure software (or a lower-level Secure    |
|                        |   software) to potentially carry out                |
|                        |   side-channel timing attacks against TF-A.         |
+------------------------+-----------------------------------------------------+
| Diagram Elements       | DF5, DF6                                            |
+------------------------+-----------------------------------------------------+
| Affected TF-A          | BL31                                                |
| Components             |                                                     |
+------------------------+-----------------------------------------------------+
| Assets                 | Sensitive Data                                      |
+------------------------+-----------------------------------------------------+
| Threat Agent           | NSCode                                              |
+------------------------+-----------------------------------------------------+
| Threat Type            | Information Disclosure                              |
+------------------------+-------------------+----------------+----------------+
| Application            | Server            | IoT            | Mobile         |
+------------------------+-------------------+----------------+----------------+
| Impact                 | Medium (3)        | Medium (3)     | Medium (3)     |
+------------------------+-------------------+----------------+----------------+
| Likelihood             | Low (2)           | Low (2)        | Low (2)        |
+------------------------+-------------------+----------------+----------------+
| Total Risk Rating      | Medium (6)        | Medium (6)     | Medium (6)     |
+------------------------+-------------------+----------------+----------------+
| Mitigations            | Follow mitigation strategies as described in        |
|                        | `Secure Development Guidelines`_.                   |
+------------------------+-----------------------------------------------------+
| Mitigations            | | Yes / platform specific.                          |
| implemented?           |                                                     |
|                        | | General events and cycle counting in the Secure   |
|                        |   world is prohibited by default when applicable.   |
|                        |                                                     |
|                        | | However, on some implementations (e.g. PMUv3)     |
|                        |   Secure world event counting depends on external   |
|                        |   debug interface signals, i.e. Secure world event  |
|                        |   counting is enabled if external debug is enabled. |
|                        |                                                     |
|                        | | Configuration of debug signals is platform        |
|                        |   specific, therefore platforms need to make sure   |
|                        |   that external debug is disabled in production or  |
|                        |   proper debug authentication is in place. This     |
|                        |   should be the case if threat #06 is properly      |
|                        |   mitigated.                                        |
+------------------------+-----------------------------------------------------+


Threats to be Mitigated by an External Agent Outside of TF-A
------------------------------------------------------------

+------------------------+-----------------------------------------------------+
| ID                     | 14                                                  |
+========================+=====================================================+
| Threat                 | | **Attacker wants to execute an arbitrary or       |
|                        |   untrusted binary as the secure OS.**              |
|                        |                                                     |
|                        | | When the option OPTEE_ALLOW_SMC_LOAD is enabled,  |
|                        |   this trusts the non-secure world up until the     |
|                        |   point it issues the SMC call to load the Secure   |
|                        |   BL32 payload. If a compromise occurs before the   |
|                        |   SMC call is invoked, then arbitrary code execution|
|                        |   in S-EL1 can occur or arbitrary memory in EL3 can |
|                        |   be overwritten.                                   |
+------------------------+-----------------------------------------------------+
| Diagram Elements       | DF5                                                 |
+------------------------+-----------------------------------------------------+
| Affected TF-A          | BL31, BL32                                          |
| Components             |                                                     |
+------------------------+-----------------------------------------------------+
| Assets                 | Code Execution, Sensitive Data                      |
+------------------------+-----------------------------------------------------+
| Threat Agent           | NSCode                                              |
+------------------------+-----------------------------------------------------+
| Threat Type            | Tampering, Information Disclosure,                  |
|                        | Elevation of privilege                              |
+------------------------+-----------------+-----------------+-----------------+
| Application            | Server          | IoT             | Mobile          |
+------------------------+-----------------+-----------------+-----------------+
| Impact                 | Critical (5)    | Critical (5)    | Critical (5)    |
+------------------------+-----------------+-----------------+-----------------+
| Likelihood             | High (4)        | High (4)        | High (4)        |
+------------------------+-----------------+-----------------+-----------------+
| Total Risk Rating      | Critical (20)   | Critical (20)   | Critical (20)   |
+------------------------+-----------------+-----------------+-----------------+
| Mitigations            | When enabling the option OPTEE_ALLOW_SMC_LOAD,      |
|                        | the non-secure OS must be considered a closed       |
|                        | platform up until the point the SMC can be invoked  |
|                        | to load OP-TEE.                                     |
+------------------------+-----------------------------------------------------+
| Mitigations            | | None in TF-A itself. This option is only used by  |
| implemented?           |   ChromeOS currently which has other mechanisms to  |
|                        |   to mitigate this threat which are described in    |
|                        |   `OP-TEE Dispatcher`_.                             |
+------------------------+-----------------------------------------------------+

--------------

*Copyright (c) 2021-2023, Arm Limited. All rights reserved.*


.. _STRIDE threat analysis technique: https://docs.microsoft.com/en-us/azure/security/develop/threat-modeling-tool-threats#stride-model
.. _DEN0034: https://developer.arm.com/documentation/den0034/latest
.. _Cache Speculation Side-channels: https://developer.arm.com/support/arm-security-updates/speculative-processor-vulnerability
.. _Spectre: https://developer.arm.com/support/arm-security-updates/speculative-processor-vulnerability
.. _TBBR-Client specification: https://developer.arm.com/documentation/den0006/d/
.. _Trusted Board Boot (TBB): https://trustedfirmware-a.readthedocs.io/en/latest/design/trusted-board-boot.html
.. _TF-A error handling policy: https://trustedfirmware-a.readthedocs.io/en/latest/process/coding-guidelines.html#error-handling-and-robustness
.. _Secure Development Guidelines: https://trustedfirmware-a.readthedocs.io/en/latest/process/security-hardening.html#secure-development-guidelines
.. _Trusted Firmware-A Tests: https://git.trustedfirmware.org/TF-A/tf-a-tests.git/about/
.. _OP-TEE Dispatcher: https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/components/spd/optee-dispatcher.rst
