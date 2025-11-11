#
# Copyright (c) 2024-2025, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

space :=
space := $(space) $(space)
comma := ,

null := �

lparen := (
rparen := )

compat-path = $(subst $(space),$(null),$(1))
decompat-path = $(subst $(null), ,$(1))

absolute-path = $(call decompat-path,$(abspath $(call compat-path,$(1))))
real-path = $(call decompat-path,$(realpath $(call compat-path,$(1))))

file-name = $(call decompat-path,$(notdir $(call compat-path,$(1))))
directory-name = $(call decompat-path,$(dir $(call compat-path,$(1))))

escape-shell = '$(subst ','\'',$(1))'

#
# The grouped-target symbol. Grouped targets are not supported on versions of
# GNU Make <= 4.2, which was most recently packaged with Ubuntu 20.04.
#

& := $(if $(filter grouped-target,$(.FEATURES)),&)

#
# Upper-case a string value.
#
# Parameters:
#
#   - $(1): The string to upper-case.
#
# Example usage:
#
#     $(call uppercase,HeLlO wOrLd) # "HELLO WORLD"
#

uppercase = $(shell echo $(call escape-shell,$(1)) | tr '[:lower:]' '[:upper:]')

#
# Lower-case a string value.
#
# Parameters:
#
#   - $(1): The string to lower-case.
#
# Example usage:
#
#     $(call lowercase,HeLlO wOrLd) # "hello world"
#

lowercase = $(shell echo $(call escape-shell,$(1)) | tr '[:upper:]' '[:lower:]')

#
# Determine the "truthiness" of a value.
#
# Parameters:
#
#   - $(1): The value to determine the truthiness of.
#
# A value is considered to be falsy if it is:
#
#   - empty, or
#   - equal to "0", "N", "NO", "F" or "FALSE" after upper-casing.
#
# If the value is truthy then the value is returned as-is, otherwise no value
# is returned.
#
# Example usage:
#
#     truthy := y
#     truthy-bool := $(call bool,$(truthy)) # "y"
#
#     falsy := n
#     falsy-bool := $(call bool,$(falsy)) # <empty>
#

bool = $(filter-out 0 n no f false,$(call lowercase,$(1)))

#
# Determine the "truthiness" of a value, returning 0 or 1.
#
# Parameters:
#
#   - $(1): The value to determine the truthiness of.
#
# A value is considered to be falsy if it is:
#
#   - empty, or
#   - equal to "0", "N", "NO", "F" or "FALSE" after upper-casing.
#
# If the value is truthy then the value is returned as-is, otherwise no value
# is returned.
#
# Example usage:
#
#     truthy := y
#     truthy-bool := $(call bool,$(truthy)) # "1"
#
#     falsy := n
#     falsy-bool := $(call bool,$(falsy)) # "0"
#

bool-01 = $(if $(call bool,$(1)),1,0)

#
# Determine whether a variable is defined or not.
#
# Parameters:
#
#   - $(1): The variable to check.
#
# Example usage:
#
#     xyz-defined := $(call defined,xyz) # <empty>
#
#     xyz :=
#     xyz-defined := $(call defined,xyz) # <non-empty>
#
#     xyz := hello
#     xyz-defined := $(call defined,xyz) # <non-empty>
#

defined = $(call bool,$(filter-out undefined,$(origin $(1))))

#
# Extract include directories from compiler flags and convert them to absolute
# paths.
#
# Parameters:
#
#   - $(1): A list of C compiler flags.
#
# Example:
#
#     includes := $(call include-dirs, -nostdlib -Iinclude-dir) # /absolute/path/to/include-dir
#

include-dirs-pattern := $(call escape-shell,-I\s*("[^"]*"|'[^']*'|\S+))
include-dirs = $(shell \
	printf '%s' $(call escape-shell,$1) | \
	perl -nle 'print $$1 while /'$(include-dirs-pattern)'/g' | \
	xargs realpath \
)

#
# Determine the path to a program.
#
# Parameters:
#
#   - $(1): The program to search for.
#
# Example usage:
#
#     path-to-gcc := $(call which,gcc) # "/usr/bin/gcc"
#

which = $(shell command -v $(call escape-shell,$(1)) 2>/dev/null)

#
# Temporarily bind variables while expanding text (scoped "with").
#
# Creates temporary variable bindings, expands a body of text with those
# bindings in effect, then restores all affected variables to their previous
# values and flavors (or undefines them if they did not exist). This provides a
# "let"-style scope for variable assignments during text expansion.
#
# This function is modelled on the `let` function introduced in GNU Make 4.4:
#
#   https://www.gnu.org/software/make/manual/html_node/Let-Function.html
#
# Binding specifiers (space-separated in `$(1)`):
#
#   - l:name  Bind from the word list in `$(2)`. Bindings are applied
#             left-to-right; the last `l` binding receives all remaining words
#             (which may be empty). If there are more `l` names than words, the
#             excess names are bound to empty values.
#
#   - p:name  Bind from subsequent call arguments (`$(2)`, `$(3)`, `$(4)`, ...),
#             in left-to-right order. If `l` bindings are present, these
#             arguments instead start from `$(3)`, following the word list.
#
#   -   name  Treated as `l:name`.
#
# Parameters:
#
#   - $(1): Space-separated binding specifiers.
#   - $(2): The list value used by `l` bindings, if present.
#   - $(2|3..N-1): Values for `p` bindings, in order (optional).
#   - $(N): The text to expand with the temporary bindings active.
#
# Evaluation and restoration:
#
#   - All function arguments in Make are expanded at the call site. The text
#     must therefore be escaped (write `$$` to produce a literal `$`), or
#     supplied via `$(value ...)` to avoid premature expansion.
#
#   - Whitespace in `l`-style bindings is processed in terms of Make words; if
#     you need to preserve whitespace then prefer `p` bindings.
#
#   - Variables are assigned as simple (`:=`) during the text expansion. After
#     the text is expanded, each variable is restored to its previous state with
#     its original flavor (simple or recursive), or undefined if it did not
#     exist. Origins (e.g., command line, environment) are not preserved.
#
# Examples:
#
#   # Basic list destructuring (two names from a list):
#   $(call with,foo bar,10 20,$$(foo) $$(bar)) # "10 20"
#
#   # Last list binding receives the remainder:
#   $(call with,head tail,1 2 3 4,[$$(head)] [$$(tail)]) # "[1] [2 3 4]"
#
#   # Extra list names bind to empty values:
#   $(call with,x y,9,x=<$$(x)> y=<$$(y)>) # "x=<9> y=<>"
#
#   # Parameter-only bindings start in `$(2)`:
#   $(call with,p:x p:y,foo,bar,$$(x)-$$(y)) # "foo-bar"
#
#   # Parameter bindings start in `$(3)` when list bindings are specified:
#   $(call with,l:lhs p:op l:rhs,10 20,+,$$(lhs) $$(op) $$(rhs)) # "10 + 20"
#
#   # Variables are restored after expansion, with flavor preserved:
#
#   x := outer-x
#   y  = outer-y
#
#   $(info $(call with,x y,inner-x inner-y,$$(x) $$(y))) # "inner-x inner-y"
#
#   $(info $(x) ($(flavor x))) # "outer-x (simple)"
#   $(info $(y) ($(flavor y))) # "outer-y (recursive)"
#
#   # Passing the text via `$(value ...)` to avoid `$$` escaping:
#
#   text = [$(head)] [$(tail)]
#   $(call with,head tail,1 2 3 4,$(value text)) # "[1] [2 3 4]"
#
#   # Nested usage:
#
#   $(call with,a b,foo bar, \
#       $$(call with,c d,baz qux,$$$$(a) $$$$(b) $$$$(c) $$$$(d)))
#   # "foo bar baz qux"
#

with = $(with.ns.push)$(eval $(value with.core))$(with.ns.pop)

with.ns = with.ns.$(with.ns.stack.head)

with.ns.stack :=
with.ns.stack.head = $(words $(with.ns.stack))

with.ns.push = $(eval with.ns.stack += $(with.ns.stack.head))
with.ns.pop = $($(with.ns).result)$(eval $(value with.ns.pop.1))

define with.ns.pop.1 =
        $(foreach variable,$(filter $(with.ns).%,$(.VARIABLES)),$\
                $(eval undefine $(variable)))

        with.ns.stack := $(wordlist 2,$(with.ns.stack.head),$(with.ns.stack))
endef

with.bind.norm = $\
        $(if $(findstring :,$(1)),$\
                $(or $(filter l: p:,$(firstword $(subst :,: ,$(1)))),$\
                        $(error invalid binding specifier: $(1)))$\
                $(or $(filter-out %:,$(word 2,$(subst :,: ,$(1)))),$\
                        $(error invalid binding specifier: $(1))),$\
                l:$(1))

with.bind.kind = $(word 1,$(subst :, ,$(call with.bind.norm,$(1))))
with.bind.name = $(word 2,$(subst :, ,$(call with.bind.norm,$(1))))

define with.core =
        # Parse and record binding list/kinds/names from `$(1)`
        $(with.ns).bind.list := $(foreach b,$(1),$(call with.bind.norm,$(b)))
        $(with.ns).bind.names := $(foreach b,$(1),$(call with.bind.name,$(b)))
        $(with.ns).bind.kinds := $(foreach b,$(1),$(call with.bind.kind,$(b)))

        # Create a 1..=(N_bindings) list of binding indices
        $(with.ns).bind.idx :=
        $(with.ns).bind.next = $(words 0 $($(with.ns).bind.idx))

        $(foreach bind,$($(with.ns).bind.list),$\
                $(eval $(with.ns).bind.idx += $($(with.ns).bind.next)))

        # Create a 2..=(N_arguments) list pointing to the text argument
        $(with.ns).text.idx :=
        $(with.ns).text.next = $(words 1 2 $($(with.ns).text.idx))
        $(with.ns).text = $($($(with.ns).text.next))

        # Snapshot original flavors/values of all variables to be overwritten
        $(foreach bind.name,$($(with.ns).bind.names),$\
        $(foreach bind.name.ns,$(with.ns).bind.names[$(bind.name)],$\
                $(eval $(bind.name.ns).flavor := $(flavor $(bind.name))$\
                $(eval $(bind.name.ns).value = $(value $(bind.name))))))

        # Initialize per-kind buckets (e.g., `l`, `p`)
        $(foreach bind.kind,$(sort $($(with.ns).bind.kinds)),$\
                $(eval $(with.ns).bind.kind[$(bind.kind)] := ))

        # Distribute binding indices into kind buckets
        $(foreach bind.i,$($(with.ns).bind.idx),$\
        $(foreach bind.kind,$(word $(bind.i),$($(with.ns).bind.kinds)),$\
                $(eval $(with.ns).bind.kind[$(bind.kind)] += $(bind.i))))

        # Per-kind setup (e.g., to set up index vectors before binding)
        $(foreach bind.kind,$(sort $($(with.ns).bind.kinds)),$\
        $(foreach bind.kind.ns,$(with.ns).bind.kind[$(bind.kind)],$\
                $(eval $(value with.core.$(bind.kind)))))

        # Perform binding from left to right
        $(foreach bind.i,$($(with.ns).bind.idx),$\
        $(foreach bind.name,$(word $(bind.i),$($(with.ns).bind.names)),$\
        $(foreach bind.kind,$(word $(bind.i),$($(with.ns).bind.kinds)),$\
        $(foreach bind.kind.ns,$(with.ns).bind.kind[$(bind.kind)],$\
                $(eval $(value with.core.$(bind.kind).bind))))))

        # Capture the expansion result from the current text pointer
        $(eval $(with.ns).result := $($(with.ns).text))

        # Restore originals (flavor/value) or undefine if previously absent
        $(foreach bind.name,$($(with.ns).bind.names),$\
        $(foreach bind.name.ns,$(with.ns).bind.names[$(bind.name)],$\
                $(eval $(value with.core.restore))))
endef

define with.core.l =
        # Create a 1..=(N_largs) list capturing the unbound `l` words
        $(bind.kind.ns).words.idx :=
        $(bind.kind.ns).words.next = $(words 1 $($(bind.kind.ns).words.idx))
        $(bind.kind.ns).words = $\
                $(wordlist $($(bind.kind.ns).words.next),$(words $(2)),$(2))

        # Increment the text pointer
        $(with.ns).text.idx += $($(with.ns).text.next)
endef

define with.core.l.bind =
        # Bind this name to the next unbound word
        $(bind.name) := $(firstword $($(bind.kind.ns).words))

        # If this is the last `l` binding, absorb the remaining words
        ifeq ($($(bind.kind.ns).words.next),$(words $($(bind.kind.ns))))
                $(bind.name) := $($(bind.kind.ns).words)
        endif

        # Nudge the word pointer forward
        $(bind.kind.ns).words.idx += $($(bind.kind.ns).words.next)
endef

define with.core.p =
        # Compute the parameter index that `p` bindings start at
        $(bind.kind.ns).param.offset := 1 2

        # When `l` bindings are present, `p` values shift right
        ifneq ($(filter l,$($(with.ns).bind.kinds)),)
                $(bind.kind.ns).param.offset += 3
        endif

        # Create an N_poff..=N_pargs list capturing the unbound `p` arguments
        $(bind.kind.ns).param.idx :=
        $(bind.kind.ns).param.next = $\
                $(words $($(bind.kind.ns).param.offset) $\
                        $($(bind.kind.ns).param.idx))
        $(bind.kind.ns).param = $($(lastword $($(bind.kind.ns).param.idx)))
endef

define with.core.p.bind =
        # Mark the next parameter as bound
        $(bind.kind.ns).param.idx += $($(bind.kind.ns).param.next)

        # Bind this name to the next unbound argument
        $(bind.name) := $($(bind.kind.ns).param)

        # Increment the text pointer
        $(with.ns).text.idx += $($(with.ns).text.next)
endef

define with.core.restore =
        ifeq ($($(bind.name.ns).flavor),simple)
                $(eval $(bind.name) := $(value $(bind.name.ns).value))
        else ifeq ($($(bind.name.ns).flavor),recursive)
                $(eval $(bind.name) = $(value $(bind.name.ns).value))
        else ifeq ($($(bind.name.ns).flavor),undefined)
                undefine $(bind.name)
        endif
endef

#
# Quote a string for safe use as a shell word.
#
# Takes the input string `$(1)` and escapes any single quotes it contains so
# that the result can be safely used as a literal shell argument. The output is
# wrapped in single quotes to ensure that whitespace and special characters are
# preserved exactly when passed to the shell.
#
# This function is useful when constructing shell commands dynamically, since it
# guarantees that arbitrary values are quoted correctly and will not be
# misinterpreted by the shell.
#
# Parameters:
#
#   - $(1): The string to quote for safe shell usage.
#
# Examples:
#
#   $(call shell-quote,foo) # "'foo'"
#   $(call shell-quote,bar baz) # "'bar baz'"
#   $(call shell-quote,foo 'bar baz' qux) # "'foo '\''bar baz'\'' qux'"
#

shell-quote = '$(subst ','\'',$(1))'

#
# Parse a shell fragment and extract the N-th word.
#
# Parses the shell fragment given by `$(2)` using the shell's word-splitting and
# quoting rules, then prints the `$(1)`-th shell word in the result. If the
# index is out of range then this function evaluates to an empty string.
#
# This function is useful when working with lists that may contain whitespace or
# quoted values, since it relies on the shell to do the parsing rather than
# Make's own word functions. Whitespace is preserved in the return value.
#
# Parameters:
#
#   - $(1): The 1-based index of the word to extract.
#   - $(2): The shell fragment to parse.
#
# Example usage:
#
#       $(call shell-word,1,foo 'bar baz' qux) # "foo"
#       $(call shell-word,2,foo 'bar baz' qux) # "bar baz"
#       $(call shell-word,3,foo 'bar baz' qux) # "qux"
#       $(call shell-word,4,foo 'bar baz' qux) # <empty>
#

shell-word = $(shell $(shell-word.sh))

define shell-word.sh =
        set -Cefu -- '' $(2);

        n=$(call shell-quote,$(1));

        shift "$${n}";
        printf '%s' "$${1:-}";
endef

#
# Parse a shell fragment and count the number of shell words.
#
# Parses the shell fragment given by `$(1)` using the shell's word-splitting and
# quoting rules, then prints the total number of words in the result.
#
# This function is useful when working with lists that may contain whitespace or
# quoted values, since it relies on the shell to do the parsing rather than
# Make's own word functions.
#
# Parameters:
#
#   - $(1): The shell fragment to parse.
#
# Example usage:
#
#       $(call shell-words,) # "0"
#       $(call shell-words,foo) # "1"
#       $(call shell-words,foo bar baz) # "3"
#       $(call shell-words,foo 'bar baz' qux) # "3"
#

shell-words = $(shell $(shell-words.sh))
shell-words.sh = set -Cefu -- $(1); printf '%s' "$$\#";

#
# Parse a shell fragment and extract a sequence of shell words.
#
# Parses the shell fragment given by `$(1)` using the shell's word-splitting and
# quoting rules, then extracts the words from index `$(2)` up to but not
# including index `$(3)`. Each extracted shell word is returned sanitized for
# safe use in the shell.
#
# If `$(3)` is omitted, it defaults to one past the total number of words in the
# string, allowing you to express "all words starting from `$(2)`".
#
# This function is useful for safely selecting and passing subsequences of
# shell-parsed arguments into other shell commands, ensuring correct handling
# of whitespace and special characters.
#
# Parameters:
#
#   - $(1): The shell fragment to parse.
#   - $(2): The 1-based start index of the slice (default: 1).
#   - $(3): The 1-based end index of the slice (exclusive, optional).
#
# Example usage:
#
#       $(call shell-slice,foo 'bar baz' qux)     # "'foo' 'bar baz' 'qux'"
#       $(call shell-slice,foo 'bar baz' qux,1,3) # "'foo' 'bar baz'"
#       $(call shell-slice,foo 'bar baz' qux,2)   # "'bar baz' 'qux'"
#       $(call shell-slice,foo 'bar baz' qux,2,4) # "'bar baz' 'qux'"
#       $(call shell-slice,foo 'bar baz' qux,2,5) # "'bar baz' 'qux'"
#

shell-slice = $(shell $(shell-slice.sh))

define shell-slice.sh =
        set -Cefu -- $(1);

        n=$(if $(2),$(call shell-quote,$(2)),1);
        m=$(if $(3),$(call shell-quote,$(3)),$$(($$# + 1)));

        printf '%s\n' "$$@" $\
                | sed -n "$${n},$${m}{ $${m}!p }; $${m}q" $\
                | sed "s/'/'\\\\''/g; s/^/'/; s/\$$/'/";
endef

#
# Join shell words with a custom delimiter.
#
# Parses the shell fragment given by `$(1)` using the shell's word-splitting and
# quoting rules, then joins the resulting words together with the delimiter
# specified by `$(2)`. If no delimiter is provided, no delimiter is used.
#
# This function is useful for safely rejoining a sequence of shell-parsed
# arguments into a single string with controlled separators, ensuring that
# whitespace and quoting are preserved correctly.
#
# Parameters:
#
#   - $(1): The shell fragment to parse and join.
#   - $(2): The delimiter to insert between words (optional).
#
# Example usage:
#
#       $(call shell-join,foo 'bar baz' qux) # "foobar bazqux"
#       $(call shell-join,foo 'bar baz' qux,:) # "foo:bar baz:qux"
#       $(call shell-join,foo 'bar baz' qux,;) # "foo;bar baz;qux"
#

shell-join = $(shell $(shell-join.sh))

define shell-join.sh =
        set -Cefu -- $(1);

        delimiter=$(call shell-quote,$(2));

        printf '%s' "$${1:-}";
        shift 1;

        while [ "$$#" -gt 0 ]; do
                printf '%s%s' "$${delimiter}" "$${1}";
                shift 1;
        done
endef

#
# Apply a function to each shell word in a fragment.
#
# Parses the shell fragment given by `$(2)` into words using the shell's
# word-splitting and quoting rules. For each word, the function `$(1)` is
# invoked with the word as its first argument and the 1-based index of the word
# as its second argument. The results are concatenated and returned, separated
# by whitespace.
#
# This function is useful when you want to process each shell word from a
# fragment through another function, while preserving correct handling of
# whitespace and quoting.
#
# Parameters:
#
#   - $(1): The function to apply to each word.
#   - $(2): The shell fragment to parse into words.
#
# Example usage:
#
#       $(call shell-map,words,foo 'bar baz' qux) # "1 2 1"
#       $(call shell-map,uppercase,foo 'bar baz' qux) # "FOO BAR BAZ QUX"
#
#       shout = $(1)!
#       $(call shell-map,shout,foo 'bar baz' qux) # "foo! bar baz! qux!"
#
#       make-binary = /bin/$(1)
#       $(call shell-map,make-binary,cp "ls" 'sh') # "/bin/cp /bin/ls /bin/sh"
#
#       index-label = $(1):$(2)
#       $(call shell-map,index-label,foo 'bar baz') # "foo:1 bar baz:2"
#

shell-map = $(call with,,$(shell $(shell-map.sh)))

define shell-map.sh =
        set -Cefu -- $(2);

        function=$(call shell-quote,$(1));
        index=1;

        for argument in "$$@"; do
                sanitized=$$(printf '%s' "$${argument}" $\
                        | sed -e 's/[$$]/$$$$/g; s/,/$${comma}/g' $\
                                -e 's/(/$${lparen}/g; s/)/$${rparen}/g');

                printf '$$(call %s,%s,%s)\n' $\
                        "$${function}" "$${sanitized}" "$${index}";

                index=$$((index + 1));
        done
endef

#
# Resolve a program name or shell fragment to a safely quoted shell command.
#
# Attempts to locate the program given by `$(1)` on the system `PATH`. If the
# program is found, its name is returned wrapped in single quotes so it can be
# used safely in a shell command. If the program cannot be found, then the
# argument is instead parsed as a shell fragment and returned as a sanitized
# sequence of words, ensuring whitespace and quoting are preserved correctly.
#
# This function is useful when dynamically constructing shell command lines
# that may include either well-known executables or arbitrary user-supplied
# fragments. It guarantees that the result is safe to embed in shell commands,
# regardless of whether it resolves to a `PATH` entry or a literal fragment.
#
# Parameters:
#
#   - $(1): The program name or shell fragment to resolve.
#
# Example usage:
#
#       $(call shell-program,sh)    # "'sh'"
#       $(call shell-program,sh -c) # "'sh' '-c'"
#
#       # If the program exists and is executable:
#
#       $(call shell-program,/foo bar/sh)      # "'/foo bar/sh'"
#       $(call shell-program,"/foo bar/sh" -c) # "'/foo bar/sh' '-c'"
#
#       # If the program does not exist or is not executable:
#
#       $(call shell-program,/foo bar/sh)    # "'/foo' 'bar/sh'"
#       $(call shell-program,/foo bar/sh -c) # "'/foo' 'bar/sh' '-c'"
#

shell-program = $\
        $(if $(call which,$(1)),$\
                $(call shell-quote,$(1)),$\
                $(call shell-slice,$(1)))
