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

        include $(dir $(toolchain-mk))build_env.mk
        include $(dir $(toolchain-mk))utilities.mk

        #
        # Make assigns generic default values to `CC`, `CPP`, `AS`, etc. if they
        # are not explicitly assigned values by the user. These are usually okay
        # for very simple programs when building for the host system, but we
        # need greater control over the toolchain flow.
        #
        # Therefore, we undefine these built-in variables if they have default
        # values, so that we can provide our own default values later instead.
        #

        ifeq ($(origin CC),default)
                undefine CC
        endif

        ifeq ($(origin CPP),default)
                undefine CPP
        endif

        ifeq ($(origin AS),default)
                undefine AS
        endif

        ifeq ($(origin AR),default)
                undefine AR
        endif

        ifeq ($(origin LD),default)
                undefine LD
        endif

        #
        # The full list of toolchains supported by TF-A.
        #
        # Each of these toolchains defines a file of the same name in the
        # `toolchains` directory, which must configure the following variables:
        #
        #   - <toolchain>-name
        #
        #     A human-readable name for the toolchain,
        #
        # Additionally, for every tool class, it must also define:
        #
        #   - <toolchain>-<tool-class>-parameter
        #
        #     The command line or environment variable used to set the tool for
        #     for the given tool class.
        #
        #   - <toolchain>-<tool-class>-default-id
        #
        #     The default tool identifier used if the tool for the given tool
        #     class cannot be identified.
        #
        #   - <toolchain>-<tool-class>-default
        #
        #     The default commands to try, in the order defined, for the given
        #     tool class if the user does not explicitly provide one, and if the
        #     command could not be derived from the C compiler.
        #

        toolchains := host # Used for host targets
        toolchains += aarch32 # Used for AArch32 targets
        toolchains += aarch64 # Used for AArch64 targets
        toolchains += rk3399-m0 # Used for RK3399 Cortex-M0 targets

        include $(toolchains:%=$(dir $(toolchain-mk))toolchains/%.mk)

        #
        # Configure tool classes that we recognize.
        #
        # In the context of this build system, a tool class identifies a
        # specific role or type of tool in the toolchain.
        #

        toolchain-tool-classes := cc
        toolchain-tool-class-name-cc := C compiler

        toolchain-tool-classes += cpp
        toolchain-tool-class-name-cpp := C preprocessor

        toolchain-tool-classes += as
        toolchain-tool-class-name-as := assembler

        toolchain-tool-classes += ld
        toolchain-tool-class-name-ld := linker

        toolchain-tool-classes += oc
        toolchain-tool-class-name-oc := object copier

        toolchain-tool-classes += od
        toolchain-tool-class-name-od := object dumper

        toolchain-tool-classes += ar
        toolchain-tool-class-name-ar := archiver

        toolchain-tool-classes += dtc
        toolchain-tool-class-name-dtc := device tree compiler

        toolchain-tool-classes += poetry
        toolchain-tool-class-name-poetry := Python Poetry package manager

        #
        # Configure tools that we recognize.
        #
        # Here we declare the list of specific toolchain tools that we know how
        # to interact with. We don't organize these into tool classes yet - that
        # happens further down.
        #

        # Arm® Compiler for Embedded
        toolchain-tools := arm-clang
        toolchain-tool-name-arm-clang := Arm® Compiler for Embedded `armclang`

        toolchain-tools += arm-link
        toolchain-tool-name-arm-link := Arm® Compiler for Embedded `armlink`

        toolchain-tools += arm-ar
        toolchain-tool-name-arm-ar := Arm® Compiler for Embedded `armar`

        toolchain-tools += arm-fromelf
        toolchain-tool-name-arm-fromelf := Arm® Compiler for Embedded `fromelf`

        # LLVM Project
        toolchain-tools += llvm-clang
        toolchain-tool-name-llvm-clang := LLVM Clang (`clang`)

        toolchain-tools += llvm-lld
        toolchain-tool-name-llvm-lld := LLVM LLD (`lld`)

        toolchain-tools += llvm-objcopy
        toolchain-tool-name-llvm-objcopy := LLVM `llvm-objcopy`

        toolchain-tools += llvm-objdump
        toolchain-tool-name-llvm-objdump := LLVM `llvm-objdump`

        toolchain-tools += llvm-ar
        toolchain-tool-name-llvm-ar := LLVM `llvm-ar`

        # GNU Compiler Collection & GNU Binary Utilities
        toolchain-tools += gnu-gcc
        toolchain-tool-name-gnu-gcc := GNU GCC (`gcc`)

        toolchain-tools += gnu-ld
        toolchain-tool-name-gnu-ld := GNU LD (`ld.bfd`)

        toolchain-tools += gnu-objcopy
        toolchain-tool-name-gnu-objcopy := GNU `objcopy`

        toolchain-tools += gnu-objdump
        toolchain-tool-name-gnu-objdump := GNU `objdump`

        toolchain-tools += gnu-ar
        toolchain-tool-name-gnu-ar := GNU `ar`

        # Other tools
        toolchain-tools += generic-dtc
        toolchain-tool-name-generic-dtc := Device Tree Compiler (`dtc`)

        toolchain-tools += generic-poetry
        toolchain-tool-name-generic-poetry := Poetry (`poetry`)

        #
        # Assign tools to tool classes.
        #
        # Multifunctional tools, i.e. tools which can perform multiple roles in
        # a toolchain, may be specified in multiple tool class lists. For
        # example, a C compiler which can also perform the role of a linker may
        # be placed in both `toolchain-tools-cc` and `toolchain-tools-ld`.
        #

        # C-related tools
        toolchain-tools-cc := arm-clang llvm-clang gnu-gcc # C compilers
        toolchain-tools-cpp := arm-clang llvm-clang gnu-gcc # C preprocessors

        # Assembly-related tools
        toolchain-tools-as := arm-clang llvm-clang gnu-gcc # Assemblers

        # Linking and object-handling tools
        toolchain-tools-ld := arm-clang arm-link llvm-clang llvm-lld gnu-gcc gnu-ld # Linkers
        toolchain-tools-oc := arm-fromelf llvm-objcopy gnu-objcopy # Object copiers
        toolchain-tools-od := arm-fromelf llvm-objdump gnu-objdump # Object dumpers
        toolchain-tools-ar := arm-ar llvm-ar gnu-ar # Archivers

        # Other tools
        toolchain-tools-dtc := generic-dtc # Device tree compilers
        toolchain-tools-poetry := generic-poetry # Python Poetry package manager

        #
        # Helper functions to identify toolchain tools.
        #
        # The functions defined in this section return a tool identifier when
        # given a path to a binary. We generally check a help or version string
        # to more reliably identify tools than by looking at the path alone
        # (e.g. `gcc` on macOS is actually Apple Clang).
        #
        # Each tool-guessing function (`toolchain-guess-tool-$(tool)`) takes a
        # single argument giving the path to the tool to guess, and returns a
        # non-empty value if the tool corresponds to the tool identifier
        # `$(tool)`:
        #
        #     $(call toolchain-guess-tool-llvm-clang,aarch64-none-elf-gcc) # <empty>
        #     $(call toolchain-guess-tool-gnu-gcc,aarch64-none-elf-gcc) # <non-empty>
        #
        # The `toolchain-guess-tool` function tries to find the corresponding tool
        # identifier for a tool given its path. It takes two arguments:
        #
        #   - $(1): a list of candidate tool identifiers to check
        #   - $(2): the path to the tool to identify
        #
        # If any of the guess functions corresponding to candidate tool
        # identifiers return a non-empty value then the tool identifier of the
        # first function to do so is returned:
        #
        #     $(call toolchain-guess-tool,gnu-gcc llvm-clang,armclang) # <empty>
        #     $(call toolchain-guess-tool,gnu-gcc llvm-clang,clang-14) # llvm-clang
        #     $(call toolchain-guess-tool,gnu-gcc llvm-clang,aarch64-none-elf-gcc-12) # gnu-gcc
        #
        # Tools are checked in the order that they are provided, and the first
        # match is returned.
        #

        # Arm Compiler for Embedded
        toolchain-guess-tool-arm-clang = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Tool: armclang")
        toolchain-guess-tool-arm-link = $(shell $(1) --help 2>&1 <$(nul) | grep -o "Tool: armlink")
        toolchain-guess-tool-arm-fromelf = $(shell $(1) --help 2>&1 <$(nul) | grep -o "Tool: fromelf")
        toolchain-guess-tool-arm-ar = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Tool: armar")

        # LLVM Project
        toolchain-guess-tool-llvm-clang = $(shell $(1) -v 2>&1 <$(nul) | grep -o "clang version")
        toolchain-guess-tool-llvm-lld = $(shell $(1) --help 2>&1 <$(nul) | grep -o "OVERVIEW: lld")
        toolchain-guess-tool-llvm-objcopy = $(shell $(1) --help 2>&1 <$(nul) | grep -o "llvm-objcopy tool")
        toolchain-guess-tool-llvm-objdump = $(shell $(1) --help 2>&1 <$(nul) | grep -o "llvm object file dumper")
        toolchain-guess-tool-llvm-ar = $(shell $(1) --help 2>&1 <$(nul) | grep -o "LLVM Archiver")

        # GNU Compiler Collection & GNU Binary Utilities
        toolchain-guess-tool-gnu-gcc = $(shell $(1) -v 2>&1 <$(nul) | grep -o "gcc version")
        toolchain-guess-tool-gnu-ld = $(shell $(1) -v 2>&1 <$(nul) | grep -o "GNU ld")
        toolchain-guess-tool-gnu-objcopy = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU objcopy")
        toolchain-guess-tool-gnu-objdump = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU objdump")
        toolchain-guess-tool-gnu-ar = $(shell $(1) --version 2>&1 <$(nul) | grep -o "GNU ar")

        # Other tools
        toolchain-guess-tool-generic-dtc = $(shell $(1) --version 2>&1 <$(nul) | grep -o "Version: DTC")
        toolchain-guess-tool-generic-poetry = $(shell $(1) --version 2>&1 <$(nul))

        toolchain-guess-tool = $(if $(2),$(firstword $(foreach candidate,$(1),$\
                $(if $(call toolchain-guess-tool-$(candidate),$(2)),$(candidate)))))

        #
        # Warn the user that a tool could not be identified.
        #
        # Parameters:
        #
        # - $1: The toolchain that the tool belongs to.
        # - $2: The tool class that the tool belongs to.
        #

        define toolchain-warn-unrecognized
                $(warning )
                $(warning The configured $($(1)-name) $(toolchain-tool-class-name-$(2)) could not be identified:)
                $(warning )
                $(warning $(space)   $($(1)-$(2))$(if $($(1)-$(2)-parameter), (via `$($(1)-$(2)-parameter)`)))
                $(warning )
                $(warning The following tools were tried, but either did not exist or could not be identified:)
                $(warning )

                $(foreach default,$($(1)-$(2)-default), \
                        $(warning $(space) - $(default)))

                $(warning )
                $(warning The following tools are supported:)
                $(warning )

                $(foreach tool,$(toolchain-tools-$(2)), \
                        $(warning $(space) - $(toolchain-tool-name-$(tool))))

                $(warning )
                $(warning The build system will treat this $(toolchain-tool-class-name-$(2)) as $(toolchain-tool-name-$($(1)-$(2)-default-id)).)
                $(warning )
        endef

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
        # for its location.
        #
        # If the C compiler cannot provide the location (or the tool class *is*
        # the C compiler), then it is assigned a default value specific for that
        # toolchain.
        #

        toolchain-derive-arm-clang-cpp = $(1)
        toolchain-derive-arm-clang-as = $(1)
        toolchain-derive-arm-clang-ld = # Fall back to `$(toolchain)-ld-default`
        toolchain-derive-arm-clang-oc = # Fall back to `$(toolchain)-oc-default`
        toolchain-derive-arm-clang-od = # Fall back to `$(toolchain)-od-default`
        toolchain-derive-arm-clang-ar = # Fall back to `$(toolchain)-ar-default`

        toolchain-derive-llvm-clang-cpp = $(1)
        toolchain-derive-llvm-clang-as = $(1)
        toolchain-derive-llvm-clang-ld = $(shell $(1) --print-prog-name ld.lld 2>$(nul))
        toolchain-derive-llvm-clang-oc = $(shell $(1) --print-prog-name llvm-objcopy 2>$(nul))
        toolchain-derive-llvm-clang-od = $(shell $(1) --print-prog-name llvm-objdump 2>$(nul))
        toolchain-derive-llvm-clang-ar = $(shell $(1) --print-prog-name llvm-ar 2>$(nul))

        toolchain-derive-gnu-gcc-cpp = $(1)
        toolchain-derive-gnu-gcc-as = $(1)
        toolchain-derive-gnu-gcc-ld = $(1)
        toolchain-derive-gnu-gcc-oc = $(shell $(1) --print-prog-name objcopy 2>$(nul))
        toolchain-derive-gnu-gcc-od = $(shell $(1) --print-prog-name objdump 2>$(nul))
        toolchain-derive-gnu-gcc-ar = $(shell $(1) --print-prog-name ar 2>$(nul))

        toolchain-derive = $(if $3,$(call toolchain-derive-$1-$2,$3))

        #
        # Configure a toolchain.
        #
        # Parameters:
        #
        #   - $1: The toolchain to configure.
        #
        # This function iterates over all tool classes and configures them for
        # the provided toolchain. Toolchain tools are initialized lazily and
        # on-demand based on the first read of the tool path or identifier
        # variables.
        #

        define toolchain-configure
                $$(foreach tool-class,$$(toolchain-tool-classes), \
                        $$(eval $$(call toolchain-configure-tool,$1,$$(tool-class))))
        endef

        #
        # Configure a specific tool within a toolchain.
        #
        # Parameters:
        #
        #   - $1: The toolchain to configure.
        #   - $2: The tool class to configure.
        #

        define toolchain-configure-tool
                $1-$2-configure = $\
                        $$(eval $$(call toolchain-determine-tool,$1,$2))

                #
                # When either of the following variables are read for the first
                # time, the appropriate tool is determined and *both* variables
                # are overwritten with their final values.
                #

                $1-$2 = $$($1-$2-configure)$$($1-$2)
                $1-$2-id = $$($1-$2-configure)$$($1-$2-id)
        endef

        #
        # Determines and identifies a tool.
        #
        # Parameters:
        #
        #   - $1: The toolchain identifier.
        #   - $2: The tool class.
        #
        # Tool identification happens by reading the designated tool parameter
        # to get the user-specified command for the tool (e.g. `CC` or `LD`). If
        # no tool parameter is defined then try to derive the tool from the C
        # compiler.
        #
        # If all else fails, fall back to the default command defined by the
        # toolchain makefile.
        #

        define toolchain-determine-tool
                toolchain-$1-$2-derive-from-cc = $$(if $$(filter-out cc,$2),$\
                        $$(call toolchain-derive,$$($1-cc-id),$2,$$($1-cc)))

                toolchain-$1-$2-shell = $\
                        $$(if $$(call defined,$$($1-$2-parameter)),$\
                                $$($$($1-$2-parameter)),$\
                                $$(or $$(toolchain-$1-$2-derive-from-cc),$\
                                        $$(toolchain-$1-$2-default)))

                toolchain-$1-$2-default = $$(firstword $\
                        $$(foreach default,$$($1-$2-default),$\
                                $$(if $$(call which,$$(default)),$$(default))) $\
                        $$($1-$2-default))

                $1-$2 := $$(if $$(call which,$$(toolchain-$1-$2-shell)),$\
                        $$(call escape-shell,$$(toolchain-$1-$2-shell)),$\
                        $$(toolchain-$1-$2-shell))

                $1-$2-id := $$(if $$($1-$2),$$(or $\
                        $$(call toolchain-guess-tool,$$\
                                $$(toolchain-tools-$2),$$($1-$2)),$\
                        $$($1-$2-default-id)))

                ifeq ($$(or $$($1-$2-id),$$(call bool,$$($1-$2-optional))),)
                        $$(call toolchain-warn-unrecognized,$1,$2)
                endif
        endef

        $(foreach toolchain,$(toolchains), \
                $(eval $(call toolchain-configure,$(toolchain))))
endif
