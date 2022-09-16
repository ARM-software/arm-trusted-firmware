ProvenCore Dispatcher
=====================

ProvenCore dispatcher (PnC-D) adds support for ProvenRun's ProvenCore micro-kernel
to work with Trusted Firmware-A (TF-A).

ProvenCore is a secure OS developed by ProvenRun S.A.S. using deductive formal methods.

Once a BL32 is ready, PnC-D can be included in the image by adding "SPD=pncd"
to the build command.
