# shell.nix
# Dev enviornment for cmake c++ development

# Pin nixpkgs to 23.11
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
    buildInputs = [
        pkgs.cmake
        pkgs.gcc
    ];
}