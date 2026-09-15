#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Applies the CppKAI regression-fix bundle (2026-09-15) to this repo.

.DESCRIPTION
    Extracts CppKAI_regression_fixes.zip's full-file overwrites into the
    repo root, then applies the three git patches (PiLexer, PiTranslator,
    Console). Run this from the repo root (C:\Users\chris\local\repos\CppKAI).
#>

$ErrorActionPreference = "Stop"

$repoRoot = "C:\Users\chris\local\repos\CppKAI"
$zipPath  = Join-Path $repoRoot "Claude outputs\CppKAI_regression_fixes.zip"
$extractDir = Join-Path $repoRoot "Claude outputs\_kai_regression_fixes_extracted"

if (-not (Test-Path $zipPath)) {
    Write-Error "Zip not found at: $zipPath"
    exit 1
}

Set-Location $repoRoot

Write-Host ">>> Extracting $zipPath ..." -ForegroundColor Cyan
if (Test-Path $extractDir) {
    Remove-Item $extractDir -Recurse -Force
}
Expand-Archive -Path $zipPath -DestinationPath $extractDir -Force

# --- 1. Full-file overwrites: copy everything except patches/ and README.txt ---
Write-Host ">>> Copying full-file overwrites into place ..." -ForegroundColor Cyan

$skipTopLevel = @("patches", "README.txt")

Get-ChildItem -Path $extractDir -Force | Where-Object {
    $skipTopLevel -notcontains $_.Name
} | ForEach-Object {
    $dest = Join-Path $repoRoot $_.Name
    Write-Host "    $($_.Name) -> $dest"
    Copy-Item -Path $_.FullName -Destination $repoRoot -Recurse -Force
}

# --- 2. Apply patches ---
Write-Host ">>> Applying patches ..." -ForegroundColor Cyan

$patches = @(
    "PiLexer.patch",
    "PiTranslator.patch"
)

$failedPatches = @()

foreach ($patch in $patches) {
    $patchPath = Join-Path $extractDir "patches\$patch"
    if (-not (Test-Path $patchPath)) {
        Write-Warning "Patch not found: $patchPath (skipping)"
        continue
    }

    Write-Host "    git apply $patch"
    git apply --check $patchPath 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "    '$patch' does not apply cleanly (file may have changed since this patch was written)."
        $failedPatches += $patch
        continue
    }

    git apply $patchPath
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "    '$patch' failed to apply."
        $failedPatches += $patch
    } else {
        Write-Host "    OK" -ForegroundColor Green
    }
}

Write-Host ""
if ($failedPatches.Count -gt 0) {
    Write-Warning "The following patches did NOT apply: $($failedPatches -join ', ')"
    Write-Warning "Paste the current content of the affected file(s) back to Claude to get a regenerated patch."
} else {
    Write-Host ">>> All patches applied successfully." -ForegroundColor Green
}

Write-Host ""
Write-Host ">>> Done. Extracted files kept at: $extractDir (safe to delete once you've verified everything)" -ForegroundColor Cyan
Write-Host ">>> Next steps:" -ForegroundColor Cyan
Write-Host "    py run.py test --clean"
Write-Host "    ctest --output-on-failure"
