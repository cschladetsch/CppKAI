<#
.SYNOPSIS
    Gathers everything relevant to Pi/Rho/Tau syntax and semantics from
    the CppKAI repo - grammar/spec docs, example scripts, and the
    translator source - into one text blob, copied to the clipboard so
    it can be pasted straight into a chat.

.PARAMETER RepoRoot
    Root of the CppKAI repo.

.PARAMETER MaxFileBytes
    Skip individual files larger than this (avoids one huge generated
    file blowing out the whole dump). Default 200KB.

.EXAMPLE
    .\Export-KaiLangDocs.ps1

.EXAMPLE
    .\Export-KaiLangDocs.ps1 -RepoRoot C:\Users\chris\local\repos\CppKAI
#>

[CmdletBinding()]
param(
    [string]$RepoRoot = "C:\Users\chris\local\repos\CppKAI",
    [long]$MaxFileBytes = 200KB
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $RepoRoot)) {
    Write-Error "RepoRoot not found: $RepoRoot"
    exit 1
}

Write-Host "Scanning $RepoRoot for Pi/Rho/Tau syntax & semantics material..." -ForegroundColor Cyan

# ---- 1. Docs/spec/readme files anywhere in the repo ------------------
$docPatterns = @("*README*", "*readme*", "*.md", "*SPEC*", "*spec*", "*GRAMMAR*", "*grammar*")
$docFiles = Get-ChildItem -Recurse -File -Path $RepoRoot -Include $docPatterns -ErrorAction SilentlyContinue |
    Where-Object { $_.Length -le $MaxFileBytes } |
    Where-Object { $_.FullName -notmatch '\\(build|\.git|Bin)\\' }

# ---- 2. Example scripts written IN Pi/Rho/Tau -------------------------
$scriptFiles = Get-ChildItem -Recurse -File -Path $RepoRoot -Include "*.pi", "*.rho", "*.tau" -ErrorAction SilentlyContinue |
    Where-Object { $_.Length -le $MaxFileBytes }

# ---- 3. Translator / grammar source that DEFINES the semantics -------
$sourceNamePatterns = @(
    "*PiTranslator*", "*RhoTranslator*", "*TauTranslator*",
    "*PiGrammar*", "*RhoGrammar*", "*TauGrammar*",
    "*PiParser*", "*RhoParser*", "*TauParser*",
    "*PiLexer*", "*RhoLexer*", "*TauLexer*",
    "*Operation*.h"
)
$translatorFiles = Get-ChildItem -Recurse -File -Path $RepoRoot -Include $sourceNamePatterns -ErrorAction SilentlyContinue |
    Where-Object { $_.Length -le $MaxFileBytes } |
    Where-Object { $_.FullName -notmatch '\\(build|\.git|Bin)\\' }

$allFiles = @($docFiles) + @($scriptFiles) + @($translatorFiles) |
    Sort-Object FullName -Unique

if ($allFiles.Count -eq 0) {
    Write-Warning "No matching files found under $RepoRoot."
    exit 0
}

Write-Host "Found $($allFiles.Count) file(s):" -ForegroundColor Green
$allFiles | ForEach-Object { Write-Host "  $($_.FullName)" -ForegroundColor DarkGray }

$sb = New-Object System.Text.StringBuilder
foreach ($f in $allFiles) {
    [void]$sb.AppendLine("=" * 80)
    [void]$sb.AppendLine("FILE: $($f.FullName)")
    [void]$sb.AppendLine("=" * 80)
    [void]$sb.AppendLine((Get-Content $f.FullName -Raw -ErrorAction SilentlyContinue))
    [void]$sb.AppendLine()
}

$output = $sb.ToString()
$output | Set-Clipboard

$sizeKb = [math]::Round(($output.Length / 1KB), 1)
Write-Host ""
Write-Host "Copied $($allFiles.Count) file(s), $sizeKb KB total, to clipboard." -ForegroundColor Green
Write-Host "Paste it into the chat now." -ForegroundColor Yellow