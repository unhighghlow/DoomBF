{
  description = "DoomBF";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
    # Target the embedded riscv32 profile
    crossPkgs = import nixpkgs {
      inherit system;
      crossSystem = pkgs.lib.systems.examples.riscv32-embedded // {
        # Forces standard 32-bit integer base with soft float
        gcc.arch = "rv32i";
        gcc.abi = "ilp32";
      };
    };
    mkShellWith = pkg: pkgs.mkShell {
      packages = [
        pkg
        pkgs.gcc
        pkgs.gnumake
        pkgs.libX11.dev
        pkgs.libX11
        pkgs.lightning
        (pkgs.python3.withPackages (p: with p; [
          pyelftools
          capstone
        ]))
      ];
    };
    riscv32-gcc-bin = pkgs.stdenvNoCC.mkDerivation {
      name = "riscv32-elf-ubuntu-24.04-gcc";
      nativeBuildInputs = [ pkgs.autoPatchelfHook ];
      buildInputs = with pkgs; [
        glib
        ncurses
        python312
        zlib
        zstd
        libmpc
        mpfr
        gmp
        expat
        isl_0_23
      ];
      src = pkgs.fetchurl {
        url = "http://ftp.debian.org/debian/pool/main/g/gcc-riscv64-unknown-elf/gcc-riscv64-unknown-elf_15.2.0-23_amd64.deb";
        hash = "sha256-hu1y6LVsZdCjvXgjakWFSH3p23L8V+2o7Kusd1wuTbk=";
      };
      unpackPhase = ''
        ${pkgs.dpkg}/bin/dpkg -x $src .
      '';
      installPhase = ''
        cp -r usr $out
      '';
    };
  in {
    devShells.${system} = {
      default = mkShellWith riscv32-gcc-bin;
      src = mkShellWith crossPkgs.stdenv.cc;
    };
  };
}
