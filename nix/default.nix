{ self, ... }:

{
  perSystem =
    { pkgs, lib, ... }:

    let
      src = self.outPath;
      stdenv = pkgs.pkgsCross.aarch64-embedded.stdenvNoLibs;

      # Compute the package version by extracting the `VERSION_*` variable
      # assignment values in the TF-A `Makefile`.
      versionSource = builtins.readFile "${src}/Makefile";
      versionComponent =
        component:

        builtins.head (
          # `builtins.match` uses POSIX extended regexes, so this pattern is
          # unfortunately a bit verbose. Multiline mode is also disabled so we
          # don't have access to the usual `^`/`$` anchors; use `.*\n` instead.
          builtins.match ".*\nVERSION_${component}[[:blank:]]*:=[[:blank:]]*([0-9]+)[[:blank:]]*\n.*"
            versionSource
        );

      version = lib.concatMapStringsSep "." versionComponent [
        "MAJOR"
        "MINOR"
        "PATCH"
      ];
    in

    {
      packages = {
        default = stdenv.mkDerivation {
          pname = "trusted-firmware-a";

          inherit version;
          inherit src;

          dontConfigure = true;

          enableParallelBuilding = false; # TF-A's build system is not reliably parallel-safe
          hardeningDisable = [ "all" ]; # TF-A's build system blindly overrides hardening options

          depsBuildBuild = [
            pkgs.stdenv.cc # Nix-wrapped C compiler targeting the host system
          ];

          nativeBuildInputs = [
            pkgs.dtc # Device Tree Compiler
            pkgs.gnumake # GNU Make

            stdenv.cc # Nix-wrapped C compiler targeting the target system
          ];

          makeFlags = [
            # Override TF-A's default build message with a static one to preserve
            # byte-for-byte reproducibility in generated firmware binaries.
            "BUILD_STRING=nix-flake" # `.git/` is not copied with the source tree
            "BUILD_MESSAGE_TIMESTAMP=\"1970-01-01T00:00:00Z\""

            # TF-A discovers toolchain tools by asking the C compiler for their
            # paths, but these paths can bypass Nix's wrappers, which supply
            # toolchain flags. Use the tools selected by `stdenv` instead.
            #
            # Make expands `$(...)` in these arguments; the variables they refer to
            # are provided automatically by `stdenv`.
            #
            # TODO: Align TF-A with GNU variable naming conventions, which Nix is
            # already aligned with: `{AS,LD}`, `CC{AS,LD}`, and `OBJ{COPY,DUMP}`.
            "AS=$(CC)"
            "CPP=$(CC)"
            "LD=$(CC)"

            "OC=$(OBJCOPY)"
            "OD=$(OBJDUMP)"

            "HOSTAS=$(CC_FOR_BUILD)"
            "HOSTCPP=$(CC_FOR_BUILD)"
            "HOSTCC=$(CC_FOR_BUILD)"
            "HOSTLD=$(CC_FOR_BUILD)"

            "HOSTOC=$(OBJCOPY_FOR_BUILD)"
            "HOSTOD=$(OBJDUMP_FOR_BUILD)"
          ];

          installPhase = ''
            runHook preInstall

            install -D -m 0644 -t "''${out}" \
              build/fvp/release/{bl1,bl2,bl31}.bin

            install -D -m 0644 -t "''${out}/dtbs" \
              build/fvp/release/fdts/fvp-base-gicv3-psci.dtb \
              build/fvp/release/fdts/fvp_{fw,tb_fw,soc_fw,nt_fw}_config.dtb

            runHook postInstall
          '';

          # The `__structuredAttrs` attribute is a special built-in Nix
          # attribute which allows Nixpkgs' standard builder to receive lists
          # as Bash arrays instead of as space-separated strings.
          #
          # In this derivation, we enable it in order to preserve whitespace in
          # values passed to `makeFlags`, so they aren't split along by word.
          #
          # See: https://nix.dev/manual/nix/stable/language/advanced-attributes.html#adv-attr-structuredAttrs
          __structuredAttrs = true;
        };
      };

      formatter = pkgs.nixfmt-tree;
    };
}
