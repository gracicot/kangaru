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

      makeShell = { pkgs, stdenv }:
        let
          mkShell = pkgs.mkShell.override { inherit stdenv; };
        in mkShell {
            nativeBuildInputs = with pkgs; [
              llvmPackages_21.llvm
              llvmPackages_21.clang-tools
              llvmPackages_21.lldb
            ];
            buildInputs = with pkgs; [
              cmake
              ninja
              vcpkg
              gdb

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
              pkg-config
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
          default = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_21.stdenv; };
          ci-clang18 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_18.stdenv; };
          ci-clang19 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_19.stdenv; };
          ci-clang20 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_20.stdenv; };
          ci-clang21 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_21.stdenv; };
          ci-gcc13 = makeShell { inherit pkgs; stdenv = pkgs.gcc13Stdenv; };
          ci-gcc14 = makeShell { inherit pkgs; stdenv = pkgs.gcc14Stdenv; };
          ci-gcc15 = makeShell { inherit pkgs; stdenv = pkgs.gcc15Stdenv; };
          ci-nocc = makeShell { inherit pkgs; stdenv = pkgs.stdenvNoCC; };
        });
    };
}
