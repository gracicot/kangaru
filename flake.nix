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
          frameworks = pkgs.darwin.apple_sdk_11_0.frameworks;
          mkShell = pkgs.mkShell.override { inherit stdenv; };
        in mkShell {
            nativeBuildInputs = with pkgs; [
              llvmPackages_19.llvm
              llvmPackages_19.clang-tools
              llvmPackages_19.lldb
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
            ] ++ lib.optionals stdenv.isDarwin [ darwin.DarwinTools frameworks.ApplicationServices ];

            hardeningDisable = [ "fortify" ];
            VCPKG_ROOT = "${pkgs.vcpkg}/share/vcpkg";
            VCPKG_FORCE_SYSTEM_BINARIES = 1;
          };
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
          #mkShell = pkgs.mkShell.override {stdenv = pkgs.gcc13Stdenv; };
        in {
          default = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_18.libcxxStdenv; };
          ci-clang12 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_12.stdenv; };
          ci-clang13 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_13.libcxxStdenv; };
          ci-clang14 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_14.libcxxStdenv; };
          ci-clang15 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_15.libcxxStdenv; };
          ci-clang16 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_16.stdenv; };
          ci-clang17 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_17.stdenv; };
          ci-clang18 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_18.stdenv; };
          ci-clang19 = makeShell { inherit pkgs; stdenv = pkgs.llvmPackages_19.stdenv; };
          ci-gcc11 = makeShell { inherit pkgs; stdenv = pkgs.gcc11Stdenv; };
          ci-gcc12 = makeShell { inherit pkgs; stdenv = pkgs.gcc12Stdenv; };
          ci-gcc13 = makeShell { inherit pkgs; stdenv = pkgs.gcc13Stdenv; };
          ci-gcc14 = makeShell { inherit pkgs; stdenv = pkgs.gcc14Stdenv; };
          ci-nocc = makeShell { inherit pkgs; stdenv = pkgs.stdenvNoCC; };
        });
    };
}
