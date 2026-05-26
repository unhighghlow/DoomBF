{ pkgs ? import <nixpkgs> {} }:

let
  # Target the embedded riscv32 profile
  crossPkgs = import pkgs.path {
    crossSystem = pkgs.lib.systems.examples.riscv32-embedded // {
      # Forces standard 32-bit integer base with soft float
      gcc.arch = "rv32im";
      gcc.abi = "ilp32";
    };
  };
in
pkgs.mkShell {
  nativeBuildInputs = [
    crossPkgs.stdenv.cc
    pkgs.gcc
    pkgs.gnumake
    pkgs.libX11.dev
    pkgs.libX11
    (pkgs.python3.withPackages (p: with p; [
      pyelftools
      capstone
    ]))
  ];
}
