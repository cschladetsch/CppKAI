
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
 
.PARAMETER Path
    One or more files or directories to tidy. A directory is expanded to
    every *.cpp/*.h/*.hpp/*.cc under it recursively. Defaults to the whole
    repo (Source, Include, Test) if omitted.
 
.PARAMETER RepoRoot
    Root of the CppKAI repo. Defaults to C:\Users\chris\local\repos\CppKAI.
 
.PARAMETER BuildDir
    Directory containing compile_commands.json. Defaults to <RepoRoot>\build.
 
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
    the number of logical processors.
 
.EXAMPLE
    .\tidy.ps1 Include\KAI\Network\Node.h
 
.EXAMPLE
    .\tidy.ps1 -Path Include\KAI\Network -NoFix
 
.EXAMPLE
    .\tidy.ps1 -Parallel
        # tidies the whole repo (Source, Include, Test) concurrently
 
.EXAMPLE
    .\tidy.ps1 -Parallel -ThrottleLimit 4
#>
 
[CmdletBinding()]
param(
    [Parameter(Position = 0, ValueFromRemainingArguments = $true)]
    [string[]]$Path,
 
    [string]$RepoRoot = "C:\Users\chris\local\repos\CppKAI",
 
    [string]$BuildDir,
 
    [switch]$NoFix,
 
    [switch]$Parallel,
 
    [int]$ThrottleLimit = [Environment]::ProcessorCount
)
 
$ErrorActionPreference = 'Stop'
 
if (-not $BuildDir) {
    $BuildDir = Join-Path $RepoRoot "build"
}
 
$clangTidy = "C:\Program Files\LLVM\bin\clang-tidy.exe"
if (-not (Test-Path $clangTidy)) {
    Write-Error "clang-tidy.exe not found at $clangTidy"
    exit 1
}
 
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
 
if (-not $Path -or $Path.Count -eq 0) {
    $Path = @("Source", "Include", "Test")
}
 
$sourceExtensions = @("*.cpp", "*.h", "*.hpp", "*.cc")
 
$files = @()
foreach ($p in $Path) {
    $resolved = Resolve-RepoPath $p
    if (-not (Test-Path $resolved)) {
        Write-Warning "Path not found, skipping: $resolved"
        continue
    }
    $item = Get-Item $resolved
    if ($item.PSIsContainer) {
        $files += Get-ChildItem -Recurse -Include $sourceExtensions -Path $resolved -File
    } else {
        $files += $item
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
 
Write-Host "Running clang-tidy on $($files.Count) file(s), -p $BuildDir$(if (-not $NoFix) { ', applying fixes with .clang-format style' })$(if ($usingParallel) { ", parallel x$ThrottleLimit" })"
 
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
        if ($r.Output) { Write-Host $r.Output }
        if ($r.ExitCode -ne 0) { $failed += $r.File }
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
 
        & $clangTidy @tidyArgs
        if ($LASTEXITCODE -ne 0) {
            $failed += $file.FullName
        }
    }
}
 
Write-Host ""
if ($failed.Count -gt 0) {
    Write-Warning "$($failed.Count) file(s) reported issues or failed:"
    $failed | ForEach-Object { Write-Warning "  $_" }
} else {
    Write-Host "All files processed cleanly." -ForegroundColor Green
}
