# start-kai.ps1 - run from anywhere
param(
    [string]$RepoRoot   = "$HOME\local\repos",
    [string]$KaiPort    = "7272",
    [string]$BridgePort = "7171",
    [string]$Lang       = "pi",
    [int]   $Trace      = 0
)

$Console   = Join-Path $RepoRoot "CppKAI\Bin\WebConsole.exe"
$BridgeDir = Join-Path $RepoRoot "kai-bridge"
$WebDir    = Join-Path $RepoRoot "kai-web"

if (-not (Test-Path $Console)) {
    Write-Error "WebConsole.exe not found at $Console - build it first"
    exit 1
}

Write-Host "Starting KAI stack..." -ForegroundColor Cyan

Start-Process powershell -ArgumentList "-NoExit", "-Command", "& '$Console' --port $KaiPort --lang $Lang --trace $Trace"
Start-Sleep -Milliseconds 800

Start-Process powershell -ArgumentList "-NoExit", "-Command", "Set-Location '$BridgeDir'; cargo run -- --kai-addr 127.0.0.1:$KaiPort --listen 0.0.0.0:$BridgePort"
Start-Sleep -Milliseconds 800

Start-Process powershell -ArgumentList "-NoExit", "-Command", "Set-Location '$WebDir'; npm run dev"

Write-Host "Done. Open http://localhost:5173" -ForegroundColor Green