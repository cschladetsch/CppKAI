<#
.SYNOPSIS
    Temporarily patches Console.cpp to dump the translated continuation's
    code array right before it's executed, rebuilds, then (optionally)
    feeds the a=2 / a+4 Rho repro into the console so the dump can be
    captured.

.DESCRIPTION
    Chasing the Rho "Empty Stack" / "InvalidPathname" bug needs to see
    exactly what Operation::Type / literal objects ended up in the
    Continuation's code array for `a =2` vs `a = 2` vs `a+4`, since static
    reading of the parser/translator hasn't pinned it down. This inserts
    a guarded debug block into Console::Process (right after
    translator->Translate(...) returns, before Execute(cont) runs) that
    prints each code entry's class name and ToString().

    The patch is idempotent (checks for its own marker before inserting)
    and reversible with -Revert (restores from the .bak file it creates).

.PARAMETER RepoPath
    Path to the CppKAI repo root. Defaults to the current directory.

.PARAMETER Revert
    Restore Console.cpp from the .bak backup instead of patching.

.PARAMETER SkipBuild
    Patch the file but don't try to rebuild afterwards.

.EXAMPLE
    .\Add-RhoTraceDump.ps1 -RepoPath C:\Users\chris\local\repos\CppKAI

.EXAMPLE
    .\Add-RhoTraceDump.ps1 -RepoPath C:\Users\chris\local\repos\CppKAI -Revert
#>

[CmdletBinding()]
param(
    [string]$RepoPath = ".",
    [switch]$Revert,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$consolePath = Join-Path $RepoPath "Ext\CppKaiCore\Source\Library\Executor\Source\Console.cpp"
$backupPath  = "$consolePath.rhotrace.bak"

if (-not (Test-Path $consolePath)) {
    Write-Error "Could not find Console.cpp at: $consolePath`nPass -RepoPath pointing at your CppKAI checkout."
}

$marker = "// >>> RHO_TRACE_DUMP (temporary, added by Add-RhoTraceDump.ps1) <<<"

if ($Revert) {
    if (-not (Test-Path $backupPath)) {
        Write-Error "No backup found at $backupPath - nothing to revert."
    }
    Copy-Item -Path $backupPath -Destination $consolePath -Force
    Remove-Item $backupPath
    Write-Host "Reverted Console.cpp from backup." -ForegroundColor Green
    return
}

$content = Get-Content -Path $consolePath -Raw

if ($content.Contains($marker)) {
    Write-Host "Trace dump already present in Console.cpp - skipping patch." -ForegroundColor Yellow
} else {
    # Anchor on the exact block that exists right after translator->Translate()
    # returns and before Execute(cont) runs (Console::Process, ~line 1398-1403).
    $anchor = @"
        if (cont.Exists()) {
            // Set the scope
            cont->SetScope(tree.GetScope());

            // Execute the continuation using our improved Execute method
            Execute(cont);
"@

    if (-not $content.Contains($anchor)) {
        Write-Error "Could not find the expected anchor block in Console.cpp.`nThe file may have changed since this script was written - patch it by hand, or paste the current Console::Process body back so the anchor can be updated."
    }

    $replacement = @"
        if (cont.Exists()) {
            // Set the scope
            cont->SetScope(tree.GetScope());

$marker
            {
                auto dumpCode = cont->GetCode();
                if (dumpCode.Exists()) {
                    std::cerr << "[RHO_TRACE] code size=" << dumpCode->Size()
                              << " for input: `"" << trimmed << "`"" << std::endl;
                    for (int _i = 0; _i < dumpCode->Size(); ++_i) {
                        Object _entry = dumpCode->At(_i);
                        std::cerr << "[RHO_TRACE]   [" << _i << "] "
                                  << (_entry.GetClass()
                                          ? _entry.GetClass()->GetName().ToString()
                                          : std::string("<null>"))
                                  << " = " << _entry.ToString().c_str()
                                  << std::endl;
                    }
                }
            }
            // <<< end RHO_TRACE_DUMP >>>

            // Execute the continuation using our improved Execute method
            Execute(cont);
"@

    Copy-Item -Path $consolePath -Destination $backupPath -Force
    $newContent = $content.Replace($anchor, $replacement)
    Set-Content -Path $consolePath -Value $newContent -NoNewline
    Write-Host "Patched Console.cpp (backup saved to $backupPath)." -ForegroundColor Green
}

if ($SkipBuild) {
    Write-Host "Skipping build (-SkipBuild passed)."
    return
}

Push-Location $RepoPath
try {
    Write-Host "Rebuilding..." -ForegroundColor Cyan
    py run.py build
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed (exit code $LASTEXITCODE). Fix the build error, then re-run with -SkipBuild once it's patched, or run 'py run.py build' yourself."
    }
    Write-Host "Build succeeded." -ForegroundColor Green
    Write-Host ""
    Write-Host "Now run the repro and watch stderr for [RHO_TRACE] lines, e.g.:" -ForegroundColor Cyan
    Write-Host "  py run.py console"
    Write-Host "  rho"
    Write-Host "  a =2"
    Write-Host "  a = 2"
    Write-Host "  a+4"
    Write-Host ""
    Write-Host "When done, revert with:  .\Add-RhoTraceDump.ps1 -RepoPath `"$RepoPath`" -Revert" -ForegroundColor Cyan
} finally {
    Pop-Location
}
