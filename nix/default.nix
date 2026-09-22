{ self, ... }:

{
  perSystem =
    { pkgs, lib, ... }:

    let
      mkFirmware = import ./builders/mk-firmware.nix {
        src = self.outPath;

        # TF-A is a freestanding firmware, and provides its own C standard
        # library and compiler runtime libraries, so use a bare-metal AArch64
        # cross-toolchain without runtime libraries.
        stdenv = pkgs.pkgsCross.aarch64-embedded.stdenvNoLibs;

        inherit pkgs;
        inherit lib;
      };
    in

    {
      packages = {
        default = mkFirmware {
          nativeBuildInputs = [
            pkgs.dtc # Device Tree Compiler
          ];

          installPhase = ''
            runHook preInstall

            install -D -m 0644 -t "''${out}" \
              build/fvp/release/{bl1,bl2,bl31}.bin

            install -D -m 0644 -t "''${out}/dtbs" \
              build/fvp/release/fdts/fvp-base-gicv3-psci.dtb \
              build/fvp/release/fdts/fvp_{fw,tb_fw,soc_fw,nt_fw}_config.dtb

            for stage in bl1 bl2 bl31; do
              install -D -m 0644 -t "''${debug}/''${stage}" \
                "build/fvp/release/''${stage}/''${stage}".{dump,elf,map}
            done

            runHook postInstall
          '';
        };
      };

      formatter = pkgs.nixfmt-tree;
    };
}
