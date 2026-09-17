# start-kai.ps1
# Run from CppKAI root or anywhere; starts WebConsole and the web frontend.

param(
    [string]$RepoRoot   = "$HOME\local\repos\CppKAI",
    [string]$KaiPort    = "7272",
    [string]$Lang       = "pi",
    [int]   $Trace      = 0
)

$Console = Join-Path $RepoRoot "Bin\WebConsole.exe"
$WebDir  = Join-Path $RepoRoot "Web"

if (-not (Test-Path $Console)) {
    Write-Error "WebConsole.exe not found at $Console - run: cmake --build build --target WebConsole"
    exit 1
}

$null = Stop-Process -Name "kai-bridge" -Force -ErrorAction SilentlyContinue
$null = Stop-Process -Name "WebConsole" -Force -ErrorAction SilentlyContinue
$null = Stop-Process -Name "node"       -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 300

Write-Host "Starting KAI web stack..." -ForegroundColor Cyan

Start-Process powershell -ArgumentList "-NoExit", "-Command", `
    "& '$Console' --port $KaiPort --lang $Lang --trace $Trace"

Start-Sleep -Milliseconds 800

Start-Process powershell -ArgumentList "-NoExit", "-Command", `
    "Set-Location '$WebDir'; npm run dev"

Write-Host "Open http://localhost:5173" -ForegroundColor Green
