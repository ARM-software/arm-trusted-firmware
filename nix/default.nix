{ self, ... }:

{
  # For each system configured in `flake.nix`, `flake-parts` evaluates this
  # module and supplies it with a system-specific Nixpkgs package set (`pkgs`).
  perSystem =
    { pkgs, lib, ... }:

    let
      mkFirmware = import ./builders/mk-firmware.nix {
        # Use the flake's source snapshot from the Nix store. Note that this
        # does *not* include `.gitignore`'d files or the `.git/` directory.
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
