<#
.SYNOPSIS
    Appends kgrep and what to $PROFILE (skipping if already present),
    reloads the profile, then commits and pushes the change in the
    Ps1Profile repo.
#>

[CmdletBinding()]
param(
    [string]$ProfilePath = $PROFILE,
    [string]$RepoDir = (Split-Path $PROFILE -Parent)
)

$ErrorActionPreference = 'Stop'

$existing = Get-Content $ProfilePath -Raw -ErrorAction SilentlyContinue
if ($existing -match '(?m)^\s*function\s+kgrep\b') {
    Write-Warning "kgrep already present in $ProfilePath - skipping append."
} else {

$block = @'

# ============================================================
# Repo grep helper (copies results to clipboard for pasting elsewhere)
# ============================================================

function kgrep {
    <#
    .SYNOPSIS
        Greps a repo recursively and copies the output to the clipboard,
        ready to paste back into a chat. Prints a dot every second while
        the search is running so it doesn't look stuck on a big tree.
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$Pattern,

        [string]$Path = (Get-Location).Path,

        [string[]]$Include = @("*.h", "*.cpp", "*.hpp", "*.cc"),

        [int[]]$Context,

        [switch]$List
    )

    $job = Start-Job -ScriptBlock {
        param($Pattern, $Path, $Include, $Context, $List)

        $params = @{ Pattern = $Pattern }
        if ($Context) { $params["Context"] = $Context }
        if ($List)    { $params["List"] = $true }

        $matches = Get-ChildItem -Recurse -Include $Include -Path $Path -File -ErrorAction SilentlyContinue |
            Select-String @params -ErrorAction SilentlyContinue

        [PSCustomObject]@{
            Count = @($matches).Count
            Text  = ($matches | Out-String)
        }
    } -ArgumentList $Pattern, $Path, $Include, $Context, $List

    Write-Host "Searching for '$Pattern' under $Path" -NoNewline
    while ($job.State -eq 'Running') {
        Write-Host "." -NoNewline
        Start-Sleep -Seconds 1
    }
    Write-Host ""

    $result = Receive-Job -Job $job
    Remove-Job -Job $job

    if (-not $result -or $result.Count -eq 0) {
        Write-Warning "No matches for '$Pattern' under $Path"
        return
    }

    $result.Text | Set-Clipboard
    Write-Host "Copied $($result.Count) match line(s) for '$Pattern' to clipboard."
    Write-Host $result.Text
}

# ============================================================
# Profile / history reminder helper
# ============================================================

$global:WhatCache = $null
$global:WhatCounterPath = Join-Path $env:APPDATA "what_invoke_count.txt"

function Get-WhatInvokeCount {
    if (Test-Path $global:WhatCounterPath) {
        $raw = Get-Content $global:WhatCounterPath -Raw -ErrorAction SilentlyContinue
        $n = 0
        if ([int]::TryParse($raw.Trim(), [ref]$n)) { return $n }
    }
    return 0
}

function Set-WhatInvokeCount([int]$count) {
    Set-Content -Path $global:WhatCounterPath -Value $count -NoNewline
}

function what {
    param(
        [int]$Top = 15,
        [string]$Filter,
        [switch]$Force
    )

    $count = (Get-WhatInvokeCount) + 1
    Set-WhatInvokeCount $count

    $cacheKey = "$Top|$Filter"
    $needsRefresh = $Force -or
                    (-not $global:WhatCache) -or
                    ($global:WhatCache.Key -ne $cacheKey) -or
                    ($count % 50 -eq 0)

    if (-not $needsRefresh) {
        Write-Host $global:WhatCache.Output
        Write-Host ""
        Write-Host "(cached - call #$count total, refreshes every 50 calls or with -Force)" -ForegroundColor DarkGray
        return
    }

    $sb = New-Object System.Text.StringBuilder
    function Add-Line([string]$line = "") { [void]$sb.AppendLine($line) }

    Add-Line "=== Functions & aliases in `$PROFILE ==="
    if (Test-Path $PROFILE) {
        $profileText = Get-Content $PROFILE -Raw
        $funcMatches = [regex]::Matches($profileText, '(?m)^\s*function\s+([A-Za-z0-9_-]+)')
        $aliasMatches = [regex]::Matches($profileText, '(?m)^\s*(?:Set-Alias|New-Alias)\s+(?:-Name\s+)?([A-Za-z0-9_-]+)')

        $names = @()
        $names += $funcMatches | ForEach-Object { [PSCustomObject]@{ Type = "function"; Name = $_.Groups[1].Value } }
        $names += $aliasMatches | ForEach-Object { [PSCustomObject]@{ Type = "alias";    Name = $_.Groups[1].Value } }

        if ($Filter) { $names = $names | Where-Object { $_.Name -like "*$Filter*" } }

        if ($names.Count -eq 0) {
            Add-Line "  (none found$(if ($Filter) { " matching '$Filter'" }))"
        } else {
            $names | Sort-Object Type, Name | ForEach-Object {
                $tag = if ($_.Name -match 'clip|clipboard|copy') { " [clipboard]" } else { "" }
                Add-Line ("  {0,-10} {1}{2}" -f $_.Type, $_.Name, $tag)
            }
        }
    } else {
        Add-Line "  No `$PROFILE file found at $PROFILE"
    }

    Add-Line ""
    Add-Line "=== Top $Top commands from history ==="

    $historyPath = (Get-PSReadLineOption).HistorySavePath
    if (-not (Test-Path $historyPath)) {
        Add-Line "  No history file found at $historyPath"
    } else {
        $lines = Get-Content $historyPath -ErrorAction SilentlyContinue | Where-Object { $_.Trim() -ne "" }
        $counts = $lines | ForEach-Object {
            $trimmed = $_.Trim()
            if ($trimmed -match '^([A-Za-z][A-Za-z0-9_.\\-]*)') { $matches[1] }
        } | Where-Object { $_ } | Group-Object | Sort-Object Count -Descending

        if ($Filter) { $counts = $counts | Where-Object { $_.Name -like "*$Filter*" } }

        $counts | Select-Object -First $Top | ForEach-Object {
            $tag = if ($_.Name -match 'clip|clipboard|copy') { " [clipboard]" } else { "" }
            Add-Line ("  {0,4}x  {1}{2}" -f $_.Count, $_.Name, $tag)
        }

        Add-Line ""
        Add-Line "=== Clipboard-related history (Set-Clipboard / Get-Clipboard / clip) ==="
        $clipLines = $lines | Where-Object { $_ -match 'Set-Clipboard|Get-Clipboard|\bclip\b|clip\.exe' } |
            Select-Object -Unique | Select-Object -Last $Top

        if ($clipLines) { $clipLines | ForEach-Object { Add-Line "  $_" } }
        else { Add-Line "  (none found in history)" }
    }

    $output = $sb.ToString().TrimEnd()
    $global:WhatCache = [PSCustomObject]@{ Key = $cacheKey; Output = $output }

    Write-Host $output
    Write-Host ""
    Write-Host "(refreshed - call #$count total, next refresh in $(50 - ($count % 50)) calls)" -ForegroundColor DarkGray
}
'@

    Add-Content -Path $ProfilePath -Value $block
    Write-Host "Appended kgrep and what to $ProfilePath" -ForegroundColor Green
}

# Reload so this session has them immediately.
. $ProfilePath

# Commit and push from the profile's repo directory.
Push-Location $RepoDir
try {
    git add (Split-Path $ProfilePath -Leaf)
    git commit -m "Add kgrep and what helper functions"
    if ($LASTEXITCODE -eq 0) {
        git push
        Write-Host "Committed and pushed." -ForegroundColor Green
    } else {
        Write-Warning "Nothing to commit (file may already match HEAD)."
    }
}
finally {
    Pop-Location
}