Truste Secure Payload Dispatcher for AARCH32 application
========================================================
TSPD AARCH32 dispatcher adds support for Trusted kernel 32-bits to work with the
Trusted Firmware. TSP AARCH32 is primarily meant to execute a
FreeRTOS 32-bits port as Secure application (More information about FreeRTOS
port, please refer to the next section.

FreeRTOS port for ARM Trusted Firmware
======================================
FreeRTOS is a Real-Time OS 32-bits from Real Time Engineers Ltd. It is a Free
Open Source Software licensed under a modfied version of the GNU GPL, version 2.
(For more information see http://www.freertos.org/license.txt). You can download
the last realease of FreeRTOS source code here "http://sourceforge.net/projects/
freertos/files/latest/download?source=files".

Some FreeRTOS features:

• Free RTOS kernel - preemptive, cooperative and hybrid configuration options
• Direct to task notifications, queues, binary semaphores, counting semaphores,
  recursive semaphores and mutexes for communication and synchronization between
  tasks, or between real time tasks and interrupts.
• Stack overflow detection options
• No restrictions imposed on task priority assignment - more than one real time
  task can be assigned the same priority
• Efficient software timers

In order to integrate FreeRTOS in the BL3-2 layer of ARM Trusted Firmware,
Virtual Open Systems SAS has developed a port to run FreeRTOS as a 32-bit
Secure Payload.
This port is available on both ARM Fast Model and ARM JUNO Development Platform.


This FreeRTOS port has been sharing with the FreeRTOS contribution community.
You can download it on "http://interactive.freertos.org/forums/135282-Any-other-
files-Any-other-manufacturers-Any-other-business" in the subject "FreeRTOS
v8.2.2 port (AARCH32) for ARMv8 platform (ARM FastModel virtual platform and ARM
JUNO Development Platform) using the GCC ARM compiler (arm-none-eabi-)".

Build TSP
=========
To build the TSP AARCH32 dispatcher, define the SPD argument in the compilation
makefile command as follow "SPD=tspd_aarch32".
Before to launch your test you have to add the FreeRTOS binary in the BL3-2
layer.
