# turn_off_core.S
#
# Copyright (c) 2018, Andre Przywara <osp@andrep.de>
# SPDX-License-Identifier: BSD-3-Clause
#
# OpenRISC assembly to turn off an ARM core on an Allwinner SoC from
# the arisc management controller.
# Generate a binary representation with:
# $ or1k-elf-as -c -o turn_off_core.o turn_off_core.S
# $ or1k-elf-objcopy -O binary --reverse-bytes=4 turn_off_core.o \
#   turn_off_core.bin
# The encoded instructions go into an array defined in
# plat/allwinner/sun50i_*/include/core_off_arisc.h, to be handed off to
# the arisc processor.
#
# This routine is meant to be called directly from arisc reset (put the
# start address in the reset vector), to be actually triggered by that
# very ARM core to be turned off.
# It expects the core number presented as a mask in the upper half of
# r3, so to be patched in the lower 16 bits of the first instruction,
# overwriting the 0 in this code here.
# The code will do the following:
# - Read the C_CPU_STATUS register, which contains the status of the WFI
#   lines of each of the four A53 cores.
# - Loop until the core in question reaches WFI.
# - Using that mask, activate the core output clamps by setting the
#   respective core bit in CPUX_PWROFF_GATING_REG (0x1f01500).
#   Note that the clamp for core 0 covers more than just the core, activating
#   it hangs the whole system. So we skip this step for core 0.
# - Using the negated mask, assert the core's reset line by clearing the
#   respective bit in C_RST_CTRL (0x1f01c30).
# - Finally turn off the core's power switch by writing 0xff to the
#   respective CPUx_PWR_SWITCH_REG (0x1f01540 ff.)
# - Assert the arisc's own reset to end execution.
#   This also signals other arisc users that the chip is free again.
# So in C this would look like:
#	while (!(readl(0x1700030) & (1U << core_nr)))
#		;
#	if (core_nr != 0)
#		writel(readl(0x1f01500) | (1U << core_nr), 0x1f01500);
#	writel(readl(0x1f01c30) & ~(1U << core_nr), 0x1f01c30);
#	writel(0xff, 0x1f01540 + (core_nr * 4));
# (using A64/H5 addresses)

.text
_start:
	l.movhi	r3, 0				# FIXUP! with core mask
	l.movhi r0, 0				# clear r0
	l.movhi	r13, 0x170			# r13: CPU_CFG_BASE=0x01700000
wait_wfi:
	l.lwz	r5, 0x30(r13)			# load C_CPU_STATUS
	l.and	r5, r5, r3			# mask requested core
	l.sfeq	r5, r0				# is it not yet in WFI?
	l.bf	wait_wfi			# try again

	l.srli	r6, r3, 16			# move mask to lower 16 bits
	l.sfeqi	r6, 1				# core 0 is special
	l.bf	1f				# don't touch the bit for core 0
	l.movhi	r13, 0x1f0			# address of R_CPUCFG (delay)
	l.lwz	r5, 0x1500(r13)			# core output clamps
	l.or	r5, r5, r6			# set bit to ...
	l.sw	0x1500(r13), r5			# ... activate for our core

1:	l.lwz	r5, 0x1c30(r13)			# CPU power-on reset
	l.xori	r6, r6, -1			# negate core mask
	l.and	r5, r5, r6			# clear bit to ...
	l.sw	0x1c30(r13), r5			# ... assert for our core

	l.ff1	r6, r3				# get core number from high mask
	l.addi	r6, r6, -17			# convert to 0-3
	l.slli	r6, r6, 2			# r5: core number*4 (0-12)
	l.add	r6, r6, r13			# add to base address
	l.ori	r5, r0, 0xff			# 0xff means all switches off
	l.sw	0x1540(r6), r5			# core power switch registers

reset:	l.sw	0x1c00(r13),r0			# pull down our own reset line

	l.j	reset				# just in case ....
	l.nop	0x0				# (delay slot)

# same as above, but with the MMIO addresses matching the H6 SoC
_start_h6:
	l.movhi	r3, 0				# FIXUP! with core mask
	l.movhi r0, 0				# clear r0
	l.movhi	r13, 0x901			# r13: CPU_CFG_BASE=0x09010000
1:
	l.lwz	r5, 0x80(r13)			# load C_CPU_STATUS
	l.and	r5, r5, r3			# mask requested core
	l.sfeq	r5, r0				# is it not yet in WFI?
	l.bf	1b				# try again

	l.srli	r6, r3, 16			# move mask to lower 16 bits(ds)
	l.sfeqi	r6, 1				# core 0 is special
	l.bf	1f				# don't touch the bit for core 0
	l.movhi	r13, 0x700			# address of R_CPUCFG (ds)
	l.lwz	r5, 0x0444(r13)			# core output clamps
	l.or	r5, r5, r6			# set bit to ...
	l.sw	0x0444(r13), r5			# ... activate for our core

1:	l.lwz	r5, 0x0440(r13)			# CPU power-on reset
	l.xori	r6, r6, -1			# negate core mask
	l.and	r5, r5, r6			# clear bit to ...
	l.sw	0x0440(r13), r5			# ... assert for our core

	l.ff1	r6, r3				# get core number from high mask
	l.addi	r6, r6, -17			# convert to 0-3
	l.slli	r6, r6, 2			# r5: core number*4 (0-12)
	l.add	r6, r6, r13			# add to base address
	l.ori	r5, r0, 0xff			# 0xff means all switches off
	l.sw	0x0450(r6), r5			# core power switch registers

1:	l.sw	0x0400(r13),r0			# pull down our own reset line

	l.j	1b				# just in case ...
	l.nop	0x0				# (delay slot)
