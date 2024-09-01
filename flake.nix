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
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = nixpkgsFor.${system};
          vcpkg = pkgs.vcpkg;
          frameworks = pkgs.darwin.apple_sdk_11_0.frameworks;
          mkShell = pkgs.mkShell.override {stdenv = pkgs.llvmPackages_19.stdenv;};
        in {
          default = mkShell {
            nativeBuildInputs = with pkgs; [
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
        });
    };
}
