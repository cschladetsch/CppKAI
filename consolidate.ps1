# consolidate.ps1
# Moves KaiWeb into CppKAI/Web/ and deletes the KaiBridge repo.
# Run from anywhere. Edit $RepoRoot if your repos are elsewhere.

param(
    [string]$RepoRoot = "$HOME\local\repos"
)

$KaiWeb    = Join-Path $RepoRoot "KaiWeb"
$KaiBridge = Join-Path $RepoRoot "KaiBridge"
$CppKAI    = Join-Path $RepoRoot "CppKAI"
$WebDest   = Join-Path $CppKAI "Web"

if (-not (Test-Path $KaiWeb))  { Write-Error "KaiWeb not found at $KaiWeb";  exit 1 }
if (-not (Test-Path $CppKAI))  { Write-Error "CppKAI not found at $CppKAI";  exit 1 }

if (Test-Path $WebDest) {
    Write-Error "CppKAI\Web already exists - remove it first"
    exit 1
}

Write-Host "Moving KaiWeb -> CppKAI\Web ..." -ForegroundColor Cyan
Move-Item -Path $KaiWeb -Destination $WebDest

if (Test-Path $KaiBridge) {
    Write-Host "Removing KaiBridge ..." -ForegroundColor Cyan
    Remove-Item -Recurse -Force $KaiBridge
}

# Update start-kai.ps1 if it exists in the new location
$script = Join-Path $WebDest "start-kai.ps1"
if (Test-Path $script) {
    $content = Get-Content $script -Raw
    $content = $content -replace 'KaiWeb', 'CppKAI\\Web'
    Set-Content $script $content
}

Write-Host "Done. CppKAI\Web is ready." -ForegroundColor Green
Write-Host "cd $WebDest && npm install && .\start-kai.ps1"
