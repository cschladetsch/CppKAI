# stop-kai.ps1
# Kills kai-bridge, kai-webconsole, and the vite dev server.

Write-Host "Stopping KAI stack..." -ForegroundColor Yellow

$targets = @(
    @{ Name = "WebConsole";  Process = "WebConsole" },
    @{ Name = "kai-bridge";  Process = "kai-bridge" },
    @{ Name = "vite";        Process = "node";  Filter = "vite" }
)

foreach ($t in $targets) {
    $procs = if ($t.Filter) {
        Get-Process -Name $t.Process -ErrorAction SilentlyContinue |
            Where-Object { $_.CommandLine -like "*$($t.Filter)*" }
    } else {
        Get-Process -Name $t.Process -ErrorAction SilentlyContinue
    }

    if ($procs) {
        $procs | Stop-Process -Force
        Write-Host "  Stopped $($t.Name)" -ForegroundColor Gray
    } else {
        Write-Host "  $($t.Name) not running" -ForegroundColor DarkGray
    }
}

Write-Host "Done." -ForegroundColor Green
