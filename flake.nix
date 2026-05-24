{
  description = "Dev shell for pixel engine";

  # Nixpkgs / NixOS version to use.
  inputs.nixpkgs.url = "nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      # System types to support.
      supportedSystems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];

      # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      # Nixpkgs instantiated for supported system types.
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; });

      makeShell = { pkgs, stdenv, llvmPackages }:
        let
          mkShell = pkgs.mkShell.override { inherit stdenv; };
        in mkShell {
            nativeBuildInputs = if llvmPackages == null then [] else with pkgs; [
              llvmPackages.clang-tools
            ];
            buildInputs = with pkgs; [
              cmake
              ninja
              vcpkg

              # to make vcpkg work
              autoconf
              automake
              autoconf-archive
              pkg-config-unwrapped
              bash
              cacert
              coreutils
              curl
              gnumake
              gzip
              openssh
              perl
              zip
              zstd
            ];

            hardeningDisable = [ "fortify" ];
            VCPKG_ROOT = "${pkgs.vcpkg}/share/vcpkg";
            VCPKG_FORCE_SYSTEM_BINARIES = 1;
          };
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
        in {
          default = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_21.stdenv; llvmPackages = pkgs.llvmPackages_21; };
          ci-clang18 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_18.libcxxStdenv; llvmPackages = pkgs.llvmPackages_18; };
          ci-clang19 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_19.stdenv; llvmPackages = pkgs.llvmPackages_19; };
          ci-clang20 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_20.stdenv; llvmPackages = pkgs.llvmPackages_20; };
          ci-clang21 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_21.stdenv; llvmPackages = pkgs.llvmPackages_21; };
          ci-clang22 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_22.stdenv; llvmPackages = pkgs.llvmPackages_22; };
          ci-gcc13 = makeShell { inherit pkgs; stdenv = pkgs.gcc13Stdenv; llvmPackages = null; };
          ci-gcc14 = makeShell { inherit pkgs; stdenv = pkgs.gcc14Stdenv; llvmPackages = null; };
          ci-gcc15 = makeShell { inherit pkgs; stdenv = pkgs.gcc15Stdenv; llvmPackages = null; };
          ci-nocc = makeShell { inherit pkgs; stdenv = pkgs.stdenvNoCC; llvmPackages = null; };
        });
    };
}
