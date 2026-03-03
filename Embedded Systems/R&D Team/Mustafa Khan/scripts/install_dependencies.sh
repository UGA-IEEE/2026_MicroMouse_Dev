#!/usr/bin/env bash
set -euo pipefail

log() {
  echo "$1"
}

exists() {
  command -v "$1" >/dev/null 2>&1
}

PKG_MANAGER=""
if exists apt-get; then
  PKG_MANAGER="apt"
elif exists dnf; then
  PKG_MANAGER="dnf"
elif exists yum; then
  PKG_MANAGER="yum"
elif exists brew; then
  PKG_MANAGER="brew"
fi

install_pkg() {
  local pkg="$1"

  case "$PKG_MANAGER" in
    apt)
      sudo apt-get update
      sudo apt-get install -y "$pkg"
      ;;
    dnf)
      sudo dnf install -y "$pkg"
      ;;
    yum)
      sudo yum install -y "$pkg"
      ;;
    brew)
      brew install "$pkg"
      ;;
    *)
      return 1
      ;;
  esac
}

log "Installing dependencies for Mustafa Khan firmware project"
log "---------------------------------------------------------"

manual_needed=0

if exists cmake; then
  log "- CMake: already installed"
else
  if install_pkg cmake; then
    log "- CMake: installed"
  else
    log "- CMake: manual install required"
    manual_needed=1
  fi
fi

if exists arm-none-eabi-gcc; then
  log "- Arm GNU Toolchain: already installed"
else
  if [ "$PKG_MANAGER" = "brew" ]; then
    if brew install --cask gcc-arm-embedded; then
      log "- Arm GNU Toolchain: installed"
    else
      log "- Arm GNU Toolchain: manual install required"
      manual_needed=1
    fi
  elif [ "$PKG_MANAGER" = "apt" ]; then
    if install_pkg gcc-arm-none-eabi; then
      log "- Arm GNU Toolchain: installed"
    else
      log "- Arm GNU Toolchain: manual install required"
      manual_needed=1
    fi
  else
    log "- Arm GNU Toolchain: install package varies by distro; manual install required"
    manual_needed=1
  fi
fi

if exists make || exists gmake; then
  log "- GNU Make: already installed"
else
  if install_pkg make; then
    log "- GNU Make: installed"
  else
    log "- GNU Make: manual install required"
    manual_needed=1
  fi
fi

if exists openocd; then
  log "- OpenOCD (optional): already installed"
else
  if install_pkg openocd; then
    log "- OpenOCD (optional): installed"
  else
    log "- OpenOCD (optional): manual install recommended if you use flash target"
  fi
fi

if [ "$manual_needed" -gt 0 ]; then
  echo
  log "Some required dependencies still need manual installation."
  exit 1
fi

echo
log "All required dependencies are installed."
exit 0
