{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.gcc-arm-embedded    # ARM cross-compiler toolchain (arm-none-eabi-gcc etc.)
    pkgs.stlink              # for st-flash upload/debug
    pkgs.openocd             # optional but makes build scripts easier
    pkgs.usbutils
  ];
}
