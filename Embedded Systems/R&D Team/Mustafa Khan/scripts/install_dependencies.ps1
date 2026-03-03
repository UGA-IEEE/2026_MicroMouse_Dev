param(
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

function Write-Step {
    param(
        [string]$Message,
        [string]$Color = 'Cyan'
    )

    Write-Host $Message -ForegroundColor $Color
}

function Test-Cmd {
    param([string]$Name)
    return [bool](Get-Command $Name -ErrorAction SilentlyContinue)
}

function Install-WithWinget {
    param(
        [string]$PackageId,
        [string]$DisplayName
    )

    if (-not (Test-Cmd winget)) {
        Write-Step "[MISSING] winget not found. Install $DisplayName manually." 'Yellow'
        return $false
    }

    if ($DryRun) {
        Write-Step "[DRY RUN] winget install --id $PackageId -e --accept-package-agreements --accept-source-agreements" 'Gray'
        return $true
    }

    Write-Step "Installing $DisplayName via winget..."
    winget install --id $PackageId -e --accept-package-agreements --accept-source-agreements
    return $LASTEXITCODE -eq 0
}

Write-Step 'Installing dependencies for Mustafa Khan firmware project'
Write-Step '---------------------------------------------------------'

$results = @()

# CMake
if (Test-Cmd cmake) {
    $results += [pscustomobject]@{ Name = 'CMake'; Status = 'Already installed'; Manual = $false }
} else {
    $ok = Install-WithWinget -PackageId 'Kitware.CMake' -DisplayName 'CMake'
    $results += [pscustomobject]@{ Name = 'CMake'; Status = $(if ($ok) { 'Installed' } else { 'Manual install required' }); Manual = (-not $ok) }
}

# Arm GNU toolchain
if (Test-Cmd 'arm-none-eabi-gcc') {
    $results += [pscustomobject]@{ Name = 'Arm GNU Toolchain'; Status = 'Already installed'; Manual = $false }
} else {
    $ok = Install-WithWinget -PackageId 'Arm.GNUToolchain' -DisplayName 'Arm GNU Toolchain'
    $results += [pscustomobject]@{ Name = 'Arm GNU Toolchain'; Status = $(if ($ok) { 'Installed' } else { 'Manual install required' }); Manual = (-not $ok) }
}

# Make (for Unix Makefiles generator)
$hasMake = (Test-Cmd make) -or (Test-Cmd mingw32-make)
if ($hasMake) {
    $results += [pscustomobject]@{ Name = 'GNU Make'; Status = 'Already installed'; Manual = $false }
} else {
    # Winget package names for make vary; try MSYS2 first.
    $ok = Install-WithWinget -PackageId 'MSYS2.MSYS2' -DisplayName 'MSYS2 (provides make)'
    $results += [pscustomobject]@{ Name = 'GNU Make'; Status = $(if ($ok) { 'Install MSYS2 done; run pacman -S --needed make' } else { 'Manual install required' }); Manual = (-not $ok) }
}

# Optional OpenOCD
if (Test-Cmd openocd) {
    $results += [pscustomobject]@{ Name = 'OpenOCD (optional)'; Status = 'Already installed'; Manual = $false }
} else {
    if ($DryRun) {
        $ok = $true
    } else {
        $ok = Install-WithWinget -PackageId 'GNUARMEmbedded.OpenOCD' -DisplayName 'OpenOCD'
    }
    $results += [pscustomobject]@{ Name = 'OpenOCD (optional)'; Status = $(if ($ok) { 'Installed or attempted install' } else { 'Optional manual install' }); Manual = $false }
}

Write-Host ''
Write-Step 'Dependency installation summary:'
$results | ForEach-Object {
    $color = if ($_.Manual) { 'Yellow' } else { 'Green' }
    Write-Step ("- {0}: {1}" -f $_.Name, $_.Status) $color
}

$manual = $results | Where-Object { $_.Manual }
if ($manual.Count -gt 0) {
    Write-Host ''
    Write-Step 'Some dependencies still require manual installation.' 'Yellow'
    Write-Step 'After installing, reopen your terminal and run: cmake --version; arm-none-eabi-gcc --version' 'Yellow'
    exit 1
}

Write-Host ''
Write-Step 'All required dependencies are installed or queued for install.' 'Green'
exit 0
