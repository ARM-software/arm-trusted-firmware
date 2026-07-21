{
  description = "Secure World boot and runtime firmware for Arm A-Profile platforms";

  inputs = {
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };

    nixpkgs = {
      url = "github:NixOS/nixpkgs/nixos-26.05";
    };
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "aarch64-darwin"
        "aarch64-linux"

        "x86_64-darwin"
        "x86_64-linux"
      ];

      imports = [
        ./nix
      ];
    };
}
