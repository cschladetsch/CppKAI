param(
    [string]$RepoRoot = "$HOME\local\repos\CppKAI",
    [string]$KaiPort  = "7272",
    [string]$Lang     = "pi",
    [int]   $Trace    = 0
)

$Console = Join-Path $RepoRoot "Bin\WebConsole.exe"
$WebDir  = Join-Path $RepoRoot "Web"

if (-not (Test-Path $Console)) {
    Write-Error "WebConsole.exe not found at $Console"
    exit 1
}

$null = Stop-Process -Name "kai-bridge" -Force -ErrorAction SilentlyContinue
$null = Stop-Process -Name "WebConsole" -Force -ErrorAction SilentlyContinue
$null = Stop-Process -Name "node"       -Force -ErrorAction SilentlyContinue
Start-Sleep -Milliseconds 500

# Start WebConsole
Start-Process powershell -ArgumentList "-NoExit","-Command","& '$Console' --port $KaiPort --lang $Lang --trace $Trace"
Start-Sleep -Milliseconds 1000

# Start bridge + vite (bridge compiles first, then vite starts)
Start-Process powershell -ArgumentList "-NoExit","-Command","Set-Location '$WebDir'; npm run dev"

# Wait for bridge to compile and vite to start before opening browser
Write-Host "Waiting for bridge to compile..." -ForegroundColor Yellow
Start-Sleep -Seconds 30
Start-Process "http://localhost:5173"
Write-Host "Done." -ForegroundColor Green
