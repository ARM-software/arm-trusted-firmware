{ self, ... }:

{
  perSystem =
    { pkgs, lib, ... }:

    let
      stdenv = pkgs.pkgsCross.aarch64-embedded.stdenvNoLibs;
    in

    {
      packages = {
        default = stdenv.mkDerivation {
          pname = "trusted-firmware-a";
          version = "experimental";

          src = self.outPath;

          strictDeps = true;
          dontConfigure = true;

          enableParallelBuilding = false; # TF-A's build system is not reliably parallel-safe
          hardeningDisable = [ "all" ]; # TF-A's build system blindly overrides hardening options

          nativeBuildInputs = [
            pkgs.dtc
            pkgs.gnumake

            stdenv.cc
          ];

          preBuild = ''
            makeFlagsArray+=(
              ${lib.escapeShellArgs [
                "AS=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"
                "CPP=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"
                "CC=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"

                "AR=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}ar"}"
                "LD=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}gcc"}"

                "OC=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}objcopy"}"
                "OD=${lib.getExe' stdenv.cc "${stdenv.cc.targetPrefix}objdump"}"

                "DTC=${lib.getExe pkgs.dtc}"

                "BUILD_STRING=nix-flake" # For reproducibility
                "BUILD_MESSAGE_TIMESTAMP=\"1970-01-01T00:00:00Z\""
              ]}
            )

            buildFlagsArray+=(
              "''${PWD}/build/fvp/release/bl1.bin"
              "''${PWD}/build/fvp/release/bl2.bin"
              "''${PWD}/build/fvp/release/bl31.bin"
            )
          '';

          installPhase = ''
            runHook preInstall

            install -D -m0644 "''${PWD}/build/fvp/release/bl1.bin" "''${out}/bl1.bin"
            install -D -m0644 "''${PWD}/build/fvp/release/bl2.bin" "''${out}/bl2.bin"
            install -D -m0644 "''${PWD}/build/fvp/release/bl31.bin" "''${out}/bl31.bin"

            runHook postInstall
          '';
        };
      };

      formatter = pkgs.nixfmt;
    };
}
