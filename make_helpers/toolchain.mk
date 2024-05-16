#
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# TF-A uses three toolchains:
#
#   - The host toolchain (`host`) for building native tools
#   - The AArch32 toolchain (`aarch32`) for building Arm AArch32 images
#   - The AArch64 toolchain (`aarch64`) for building Arm AArch64 images
#
# In the main Makefile only one of the two Arm toolchains is enabled in any
# given build, but individual tools and libraries may need access to both.
#

ifndef toolchain-mk
        toolchain-mk := $(lastword $(MAKEFILE_LIST))

        toolchains ?= host $(ARCH)

        include $(dir $(lastword $(MAKEFILE_LIST)))build_env.mk
        include $(dir $(lastword $(MAKEFILE_LIST)))utilities.mk

        include $(addprefix $(dir $(lastword $(MAKEFILE_LIST)))toolchains/, \
                $(addsuffix .mk,$(toolchains)))

        #
        # Configure tool classes that we recognize.
        #
        # In the context of this build system, a tool class identifies a
        # specific role or type of tool in the toolchain.
        #

        tool-classes := cc
        tool-class-name-cc := C compiler

        tool-classes += cpp
        tool-class-name-cpp := C preprocessor

        tool-classes += as
        tool-class-name-as := assembler

        tool-classes += ld
        tool-class-name-ld := linker

        tool-classes += oc
        tool-class-name-oc := object copier

        tool-classes += od
        tool-class-name-od := object dumper

        tool-classes += ar
        tool-class-name-ar := archiver

        tool-classes += dtc
        tool-class-name-dtc := device tree compiler

        #
        # Configure tools that we recognize.
        #
        # Here we declare the list of specific toolchain tools that we know how
        # to interact with. We don't organize these into tool classes yet - that
        # happens further down.
        #

        # Arm® Compiler for Embedded
        tools := arm-clang
        tool-name-arm-clang := Arm® Compiler for Embedded `armclang`

        tools += arm-link
        tool-name-arm-link := Arm® Compiler for Embedded `armlink`

        tools += arm-ar
        tool-name-arm-ar := Arm® Compiler for Embedded `armar`

        tools += arm-fromelf
        tool-name-arm-fromelf := Arm® Compiler for Embedded `fromelf`

        # LLVM Project
        tools += llvm-clang
        tool-name-llvm-clang := LLVM Clang (`clang`)

        tools += llvm-lld
        tool-name-llvm-lld := LLVM LLD (`lld`)

        tools += llvm-objcopy
        tool-name-llvm-objcopy := LLVM `llvm-objcopy`

        tools += llvm-objdump
        tool-name-llvm-objdump := LLVM `llvm-objdump`

        tools += llvm-ar
        tool-name-llvm-ar := LLVM `llvm-ar`

        # GNU Compiler Collection & GNU Binary Utilities
        tools += gnu-gcc
        tool-name-gnu-gcc := GNU GCC (`gcc`)

        tools += gnu-ld
        tool-name-gnu-ld := GNU LD (`ld.bfd`)

        tools += gnu-objcopy
        tool-name-gnu-objcopy := GNU `objcopy`

        tools += gnu-objdump
        tool-name-gnu-objdump := GNU `objdump`

        tools += gnu-ar
        tool-name-gnu-ar := GNU `ar`

        # Other tools
        tools += generic-dtc
        tool-name-generic-dtc := Device Tree Compiler (`dtc`)

        #
        # Assign tools to tool classes.
        #
        # Multifunctional tools, i.e. tools which can perform multiple roles in
        # a toolchain, may be specified in multiple tool class lists. For
        # example, a C compiler which can also perform the role of a linker may
        # be placed in both `tools-cc` and `tools-ld`.
        #

        # C-related tools
        tools-cc := arm-clang llvm-clang gnu-gcc # C compilers
        tools-cpp := arm-clang llvm-clang gnu-gcc # C preprocessors

        # Assembly-related tools
        tools-as := arm-clang llvm-clang gnu-gcc # Assemblers

        # Linking and object-handling tools
        tools-ld := arm-clang arm-link llvm-clang llvm-lld gnu-gcc gnu-ld # Linkers
        tools-oc := arm-fromelf llvm-objcopy gnu-objcopy # Object copiers
        tools-od := arm-fromelf llvm-objdump gnu-objdump # Object dumpers
        tools-ar := arm-ar llvm-ar gnu-ar # Archivers

        # Other tools
        tools-dtc := generic-dtc # Device tree compilers

        define check-tool-class-tools
                $(eval tool-class := $(1))

                ifndef tools-$(tool-class)
                        $$(error no tools registered to handle tool class `$(tool-class)`)
                endif
        endef

        $(foreach tool-class,$(tool-classes), \
                $(eval $(call check-tool-class-tools,$(tool-class))))

        #
        # Default tools for each toolchain.
        #
        # Toolchains can specify a default path to any given tool with a tool
        # class. These values are used in the absence of user-specified values,
        # and are configured by the makefile for each toolchain using variables
        # of the form:
        #
        #   - $(toolchain)-$(tool-class)-default
        #
        # For example, the default C compiler for the AArch32 and AArch64
        # toolchains could be configured with:
        #
        #   - aarch32-cc-default
        #   - aarch64-cc-default
        #

        define check-toolchain-tool-class-default
                $(eval toolchain := $(1))
                $(eval tool-class := $(2))

                ifndef $(toolchain)-$(tool-class)-default
                        $$(error no default value specified for tool class `$(tool-class)` of toolchain `$(toolchain)`)
                endif
        endef

        define check-toolchain-tool-class-defaults
                $(eval toolchain := $(1))

                $(foreach tool-class,$(tool-classes), \
                        $(eval $(call check-toolchain-tool-class-default,$(toolchain),$(tool-class))))
        endef

        $(foreach toolchain,$(toolchains), \
                $(eval $(call check-toolchain-tool-class-defaults,$(toolchain))))

        #
        # Helper functions to identify toolchain tools.
        #
        # The functions defined in this section return a tool identifier when
        # given a path to a binary. We generally check a help or version string
        # to more reliably identify tools than by looking at the path alone
        # (e.g. `gcc` on macOS is actually Apple Clang).
        #
        # Each tool-guessing function (`guess-tool-$(tool)`) takes a single
        # argument giving the path to the tool to guess, and returns a non-empty
        # value if the tool corresponds to the tool identifier `$(tool)`:
        #
        #     $(call guess-tool-llvm-clang,aarch64-none-elf-gcc) # <empty>
        #     $(call guess-tool-gnu-gcc,aarch64-none-elf-gcc) # <non-empty>
        #
        # The `guess-tool` function tries to find the corresponding tool
        # identifier for a tool given its path. It takes two arguments:
        #
        #   - $(1): a list of candidate tool identifiers to check
        #   - $(2): the path to the tool to identify
        #
        # If any of the guess functions corresponding to candidate tool
        # identifiers return a non-empty value then the tool identifier of the
        # first function to do so is returned:
        #
        #     $(call guess-tool,gnu-gcc llvm-clang,armclang) # <empty>
        #     $(call guess-tool,gnu-gcc llvm-clang,clang-14) # llvm-clang
        #     $(call guess-tool,gnu-gcc llvm-clang,aarch64-none-elf-gcc-12) # gnu-gcc
        #
        # Tools are checked in the order that they appear in
        # `tools-$(tool-class)`, and the first match is returned.
        #

        # Arm Compiler for Embedded
        guess-tool-arm-clang = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Tool: armclang")
        guess-tool-arm-link = $(shell $(1) --help 2>&1 <$(nul) | grep -o "Tool: armlink")
        guess-tool-arm-fromelf = $(shell $(1) --help 2>&1 <$(nul) | grep -o "Tool: fromelf")
        guess-tool-arm-ar = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Tool: armar")

        # LLVM Project
        guess-tool-llvm-clang = $(shell $(1) -v 2>&1 <$(nul) | grep -o "clang version")
        guess-tool-llvm-lld = $(shell $(1) --help 2>&1 <$(nul) | grep -o "OVERVIEW: lld")
        guess-tool-llvm-objcopy = $(shell $(1) --help 2>&1 <$(nul) | grep -o "llvm-objcopy tool")
        guess-tool-llvm-objdump = $(shell $(1) --help 2>&1 <$(nul) | grep -o "llvm object file dumper")
        guess-tool-llvm-ar = $(shell $(1) --help 2>&1 <$(nul) | grep -o "LLVM Archiver")

        # GNU Compiler Collection & GNU Binary Utilities
        guess-tool-gnu-gcc = $(shell $(1) -v 2>&1 <$(nul) | grep -o "gcc version")
        guess-tool-gnu-ld = $(shell $(1) -v 2>&1 <$(nul) | grep -o "GNU ld")
        guess-tool-gnu-objcopy = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU objcopy")
        guess-tool-gnu-objdump = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU objdump")
        guess-tool-gnu-ar = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU ar")

        # Other tools
        guess-tool-generic-dtc = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Version: DTC")

        guess-tool = $(firstword $(foreach candidate,$(1), \
                $(if $(call guess-tool-$(candidate),$(2)),$(candidate))))

        #
        # Locate and identify tools belonging to each toolchain.
        #
        # Each tool class in each toolchain receives a variable of the form
        # `$(toolchain)-$(tool)` giving the associated path to the program. For
        # example:
        #
        #   - `aarch64-ld` gives the linker for the AArch64 toolchain,
        #   - `aarch32-oc` gives the object copier for the AArch32 toolchain, and
        #   - `host-cc` gives the C compiler for the host toolchain.
        #
        # For each of these variables, if no program path is explicitly provided
        # by the parent Makefile then the C compiler is queried (if supported)
        # for its location. This is done via the `guess-$(tool)-$(tool-class)`
        # set of functions. For example:
        #
        #   - `guess-arm-clang-ld` guesses the linker via Arm Clang,
        #   - `guess-llvm-clang-as` guesses the assembler via LLVM Clang, and
        #   - `guess-gnu-gcc-od` guesses the object dumper via GNU GCC.
        #
        # If the C compiler cannot provide the location (or the tool class is
        # the C compiler), then it is assigned the value of the
        # `$(toolchain)-$(tool)-default` variable.
        #

        guess-arm-clang-cpp = $(1)
        guess-arm-clang-as = $(1)
        guess-arm-clang-ld = # Fall back to `$(toolchain)-ld-default`
        guess-arm-clang-oc = # Fall back to `$(toolchain)-oc-default`
        guess-arm-clang-od = # Fall back to `$(toolchain)-od-default`
        guess-arm-clang-ar = # Fall back to `$(toolchain)-ar-default`

        guess-llvm-clang-cpp = $(1)
        guess-llvm-clang-as = $(1)
        guess-llvm-clang-ld = $(shell $(1) --print-prog-name ld.lld 2>$(nul))
        guess-llvm-clang-oc = $(shell $(1) --print-prog-name llvm-objcopy 2>$(nul))
        guess-llvm-clang-od = $(shell $(1) --print-prog-name llvm-objdump 2>$(nul))
        guess-llvm-clang-ar = $(shell $(1) --print-prog-name llvm-ar 2>$(nul))

        guess-gnu-gcc-cpp = $(1)
        guess-gnu-gcc-as = $(1)
        guess-gnu-gcc-ld = $(1)
        guess-gnu-gcc-oc = $(shell $(1) --print-prog-name objcopy 2>$(nul))
        guess-gnu-gcc-od = $(shell $(1) --print-prog-name objdump 2>$(nul))
        guess-gnu-gcc-ar = $(call which,$(call decompat-path,$(patsubst %$(call file-name,$(1)),%$(subst gcc,gcc-ar,$(call file-name,$(1))),$(call compat-path,$(1)))))

        define toolchain-warn-unrecognized
                $$(warning )
                $$(warning The configured $$($(1)-name) $$(tool-class-name-$(2)) could not be identified and may not be supported:)
                $$(warning )
                $$(warning $$(space)   $$($(1)-$(2)))
                $$(warning )
                $$(warning The default $$($(1)-name) $$(tool-class-name-$(2)) is:)
                $$(warning )
                $$(warning $$(space)   $$($(1)-$(2)-default))
                $$(warning )
                $$(warning The following tools are supported:)
                $$(warning )

                $$(foreach tool,$$(tools-$(2)), \
                        $$(warning $$(space) - $$(tool-name-$$(tool))))

                $$(warning )
                $$(warning The build system will treat this $$(tool-class-name-$(2)) as $$(tool-name-$$($(1)-$(2)-id-default)).)
                $$(warning )
        endef

        define toolchain-determine-tool
                $(1)-$(2)-guess = $$(if $$(filter-out cc,$(2)),$\
                        $$(call guess-$$($(1)-cc-id)-$(2),$$($(1)-cc)))

                $(1)-$(2) := $$(or $$($(1)-$(2)),$$($(1)-$(2)-guess))
                $(1)-$(2) := $$(or $$($(1)-$(2)),$$($(1)-$(2)-default))

                ifneq ($$(call which,$$($(1)-$(2))),)
                        # If we can resolve this tool to a program on the `PATH`
                        # then escape it for use in a shell, which allows us to
                        # preserve spaces.

                        $(1)-$(2) := $$(call escape-shell,$$($(1)-$(2)))
                endif

                $(1)-$(2)-id := $$(call guess-tool,$$(tools-$(2)),$$($(1)-$(2)))

                ifndef $(1)-$(2)-id
                        $(1)-$(2)-id := $$($(1)-$(2)-id-default)

                        $$(eval $$(call toolchain-warn-unrecognized,$(1),$(2)))
                endif
        endef

        define toolchain-determine
                $$(foreach tool-class,$$(tool-classes), \
                        $$(eval $$(call toolchain-determine-tool,$(1),$$(tool-class))))
        endef

        $(foreach toolchain,$(toolchains), \
                $(eval $(call toolchain-determine,$(toolchain))))
endif
