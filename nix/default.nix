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

          nativeBuildInputs = [
            pkgs.dtc
            pkgs.gnumake

            stdenv.cc
          ];

          makeFlags = [
            "AS=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"
            "CPP=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"
            "CC=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"

            "AR=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}ar"}"
            "LD=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"

            "OC=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}objcopy"}"
            "OD=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}objdump"}"

            "BUILD_STRING=nix-flake" # For reproducibility
            "BUILD_MESSAGE_TIMESTAMP=\"1970-01-01T00:00:00Z\""
          ];

          installPhase = ''
            runHook preInstall

            install -D -m0644 "''${PWD}/build/fvp/release/bl1.bin" "''${out}/bl1.bin"
            install -D -m0644 "''${PWD}/build/fvp/release/bl2.bin" "''${out}/bl2.bin"
            install -D -m0644 "''${PWD}/build/fvp/release/bl31.bin" "''${out}/bl31.bin"

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
