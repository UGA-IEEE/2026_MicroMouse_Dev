# Mustafa Khan - MicroMouse Firmware

This project builds STM32F103 firmware using CMake and the Arm GNU Embedded toolchain.

## 1. Install dependencies

Run the installer script from this folder.

```powershell
# Windows
powershell -ExecutionPolicy Bypass -File .\scripts\install_dependencies.ps1
```

```bash
# Linux/macOS
bash ./scripts/install_dependencies.sh
```

What the installer does:
- Installs missing required tools (`cmake`, `arm-none-eabi-gcc`, and `make`) using the system package manager when possible.
- Reports any dependency that still needs manual installation.
- Installs `openocd` when available (optional, only needed for flash target).

## 2. Configure

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake
```

## 3. Build

```powershell
cmake --build build
```

## 4. Useful targets

```powershell
# Firmware size report
cmake --build build --target size

# Disassembly output (build/firmware.asm)
cmake --build build --target disasm

# Flash over ST-Link (requires OpenOCD)
cmake --build build --target flash
```

## 5. Clean local artifacts

```powershell
Remove-Item -Recurse -Force .\build
```

Local build outputs, binaries, and editor files are ignored in `.gitignore` so the folder stays ready for GitHub commits.
