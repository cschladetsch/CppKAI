<#
.SYNOPSIS
    Runs clang-tidy (with -fix and your .clang-format style) against one or
    more CppKAI source files, regardless of what directory you're sitting
    in when you call it.

.DESCRIPTION
    Fixes the relative-path trap where running clang-tidy from inside
    `build\` resolves a relative file argument against the wrong directory
    (e.g. `Include\KAI\Network\Node.h` becomes
    `build\Include\KAI\Network\Node.h`, which doesn't exist). This script
    always resolves paths against the repo root, converts them to absolute
    paths before handing them to clang-tidy, and points -p at the build
    directory's compile_commands.json regardless of your current location.

    Android\ and Ext\ (third-party/vendored code) are excluded from every
    recursive scan by default, since they're not yours to tidy and just
    burn hours re-parsing template-heavy vendored headers.

.PARAMETER Path
    One or more files or directories to tidy. A directory is expanded to
    every *.cpp/*.h/*.hpp/*.cc under it recursively. Defaults to the whole
    repo (Source, Include, Test) if omitted.

.PARAMETER RepoRoot
    Root of the CppKAI repo. Defaults to the git repo root containing this
    script (via `git rev-parse --show-toplevel`), so it works regardless of
    where the repo is cloned. Falls back to this script's own directory if
    git isn't available or this isn't a git checkout.

.PARAMETER BuildDir
    Directory containing compile_commands.json. Defaults to <RepoRoot>\build.

.PARAMETER Exclude
    Directory name fragments to exclude from recursive scans (matched
    anywhere in the full path, case-insensitive). Defaults to
    @('Android', 'Ext'). Pass -Exclude @() to disable exclusion entirely,
    or add your own list to replace the default.

.PARAMETER NoFix
    Run clang-tidy in report-only mode (no -fix, no -format-style) instead
    of applying fixes in place.

.PARAMETER Parallel
    Run clang-tidy invocations concurrently instead of one file at a time.
    Each invocation reparses its whole translation unit from scratch, so on
    a template-heavy codebase this is the difference between minutes and
    seconds. Requires PowerShell 7+ (uses ForEach-Object -Parallel) - falls
    back to sequential with a warning on Windows PowerShell 5.1.

.PARAMETER ThrottleLimit
    Max concurrent clang-tidy processes when -Parallel is used. Defaults to
    half the logical processor count, since each clang-tidy process can be
    memory-hungry on template-heavy headers and full-core saturation tends
    to cause swapping rather than a speedup.

.EXAMPLE
    .\tidy.ps1 Include\KAI\Network\Node.h

.EXAMPLE
    .\tidy.ps1 -Path Include\KAI\Network -NoFix

.EXAMPLE
    .\tidy.ps1 -Parallel
        # tidies the whole repo (Source, Include, Test), skipping
        # Android\ and Ext\, concurrently

.EXAMPLE
    .\tidy.ps1 -Parallel -ThrottleLimit 4

.EXAMPLE
    .\tidy.ps1 -Exclude @('Android','Ext','ThirdParty')
#>

[CmdletBinding()]
param(
    [Parameter(Position = 0, ValueFromRemainingArguments = $true)]
    [string[]]$Path,

    [string]$RepoRoot,

    [string]$BuildDir,

    [string[]]$Exclude = @('Android', 'Ext'),

    [switch]$NoFix,

    [switch]$Parallel,

    [int]$ThrottleLimit = [Math]::Max(1, [int]([Environment]::ProcessorCount / 2))
)

$ErrorActionPreference = 'Stop'

# Resolve the repo root without any hardcoded path: prefer git (works
# from any clone location), and fall back to this script's own directory
# (assumes the script lives at the repo root, as run_tidy.ps1 does here)
# if git isn't available or this isn't a checkout.
if (-not $RepoRoot) {
    $scriptDir = $PSScriptRoot
    $gitRoot = $null
    try {
        Push-Location $scriptDir
        $gitRoot = (& git rev-parse --show-toplevel 2>$null)
        Pop-Location
    } catch {
        if ((Get-Location).Path -ne $scriptDir) { Pop-Location -ErrorAction SilentlyContinue }
    }

    if ($LASTEXITCODE -eq 0 -and $gitRoot) {
        # git prints forward slashes even on Windows; normalize to the
        # platform separator.
        $RepoRoot = ($gitRoot -replace '/', [System.IO.Path]::DirectorySeparatorChar)
    } else {
        $RepoRoot = $scriptDir
        Write-Warning "Could not determine repo root via git; defaulting to script directory: $RepoRoot"
    }
}

if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot "build"
}

# Find clang-tidy on PATH rather than assuming a fixed install location.
$clangTidyCmd = Get-Command "clang-tidy" -ErrorAction SilentlyContinue
if (-not $clangTidyCmd) {
    Write-Error "clang-tidy.exe not found on PATH. Install LLVM and ensure its bin directory is on PATH, or add it for this session with: `$env:PATH += ';C:\Program Files\LLVM\bin'"
    exit 1
}
$clangTidy = $clangTidyCmd.Source

$compileCommands = Join-Path $BuildDir "compile_commands.json"
if (-not (Test-Path $compileCommands)) {
    Write-Warning "No compile_commands.json found at $compileCommands - did you configure cmake with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON?"
}

# Resolve the file/directory arguments into a flat list of absolute file
# paths, treating any relative path as relative to RepoRoot (not the
# caller's current directory) so this behaves the same no matter where
# you run it from.
function Resolve-RepoPath([string]$p) {
    if ([System.IO.Path]::IsPathRooted($p)) {
        return $p
    }
    return Join-Path $RepoRoot $p
}

# True if the full path contains any excluded directory segment, matched
# as a path-separator-bounded fragment (so 'Ext' doesn't accidentally
# match something like 'Extensions').
function Test-ExcludedPath([string]$fullPath, [string[]]$excludeList) {
    if (-not $excludeList -or $excludeList.Count -eq 0) {
        return $false
    }
    $segments = $fullPath -split '[\\/]'
    foreach ($ex in $excludeList) {
        if ($segments -contains $ex) {
            return $true
        }
    }
    return $false
}

if (-not $Path -or $Path.Count -eq 0) {
    $Path = @("Source", "Include", "Test")
}

$sourceExtensions = @('.cpp', '.h', '.hpp', '.cc')

$files = @()
foreach ($p in $Path) {
    $resolved = Resolve-RepoPath $p
    if (-not (Test-Path $resolved)) {
        Write-Warning "Path not found, skipping: $resolved"
        continue
    }
    $item = Get-Item $resolved
    if ($item.PSIsContainer) {
        $files += Get-ChildItem -Recurse -File -Path $resolved |
            Where-Object { $_.Extension -in $sourceExtensions } |
            Where-Object { -not (Test-ExcludedPath $_.FullName $Exclude) }
    } else {
        if (Test-ExcludedPath $item.FullName $Exclude) {
            Write-Warning "Explicitly given file is under an excluded directory, skipping: $($item.FullName)"
        } else {
            $files += $item
        }
    }
}

if ($files.Count -eq 0) {
    Write-Warning "No files resolved - nothing to do."
    exit 0
}

$usingParallel = $Parallel -and ($PSVersionTable.PSVersion.Major -ge 7)
if ($Parallel -and -not $usingParallel) {
    Write-Warning "-Parallel requires PowerShell 7+ (ForEach-Object -Parallel). This is Windows PowerShell $($PSVersionTable.PSVersion) - falling back to sequential. Run this script with 'pwsh' instead of 'powershell' to get parallel execution."
}

$excludeNote = if ($Exclude -and $Exclude.Count -gt 0) { ", excluding $($Exclude -join ', ')" } else { "" }
Write-Host "Running clang-tidy on $($files.Count) file(s)$excludeNote, -p $BuildDir$(if (-not $NoFix) { ', applying fixes with .clang-format style' })$(if ($usingParallel) { ", parallel x$ThrottleLimit" })"

if ($usingParallel) {
    $results = $files | ForEach-Object -Parallel {
        $clangTidy = $using:clangTidy
        $BuildDir = $using:BuildDir
        $NoFix = $using:NoFix
        $file = $_

        $tidyArgs = @("-p", $BuildDir)
        if (-not $NoFix) {
            $tidyArgs += @("-fix", "-format-style=file")
        }
        $tidyArgs += $file.FullName

        $output = & $clangTidy @tidyArgs 2>&1
        [PSCustomObject]@{
            File     = $file.FullName
            ExitCode = $LASTEXITCODE
            Output   = $output -join "`n"
        }
    } -ThrottleLimit $ThrottleLimit

    $failed = @()
    foreach ($r in $results) {
        Write-Host "  $($r.File)" -ForegroundColor DarkGray
        if ($r.Output -match '(?m)^.*error:.*$') {
            Write-Host $r.Output
            $failed += $r.File
        } elseif ($r.Output) {
            Write-Host $r.Output
        }
    }
} else {
    $failed = @()
    foreach ($file in $files) {
        Write-Host "  $($file.FullName)" -ForegroundColor DarkGray

        $tidyArgs = @("-p", $BuildDir)
        if (-not $NoFix) {
            $tidyArgs += @("-fix", "-format-style=file")
        }
        $tidyArgs += $file.FullName

        $output = & $clangTidy @tidyArgs 2>&1
        $output | ForEach-Object { Write-Host $_ }
        if ($output -match '(?m)^.*error:.*$') {
            $failed += $file.FullName
        }
    }
}

Write-Host ""
if ($failed.Count -gt 0) {
    Write-Warning "$($failed.Count) file(s) had compiler errors (not just lint findings):"
    $failed | ForEach-Object { Write-Warning "  $_" }
} else {
    Write-Host "All files processed cleanly." -ForegroundColor Green
}
