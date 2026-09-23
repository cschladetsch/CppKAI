$base = "C:\Users\chris\local\repos\CppKAI"
$f = "$base\Ext\CppKaiCore\Source\Library\Executor\Source\ExecutorPerform.cpp"
$lines = [System.Collections.Generic.List[string]](Get-Content $f)

$suspendLine = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i].Trim() -eq "case Operation::Suspend: {") {
        $suspendLine = $i
        break
    }
}
if ($suspendLine -eq -1) { Write-Host "FAIL: could not find case Operation::Suspend"; exit }

$anchorIdx = -1
for ($i = $suspendLine; $i -lt $suspendLine + 45 -and $i -lt $lines.Count; $i++) {
    if ($lines[$i].Contains("[Suspend] after Enter OK")) {
        $anchorIdx = $i
        break
    }
}
if ($anchorIdx -eq -1) { Write-Host "FAIL: could not find '[Suspend] after Enter OK' trace"; exit }

for ($i = $anchorIdx; $i -lt $anchorIdx + 15 -and $i -lt $lines.Count; $i++) {
    if ($lines[$i].Trim() -eq "replace_ = true;") {
        if ($lines[$i-1] -and $lines[$i-1].Contains("[Suspend] reached replace_=true line")) {
            Write-Host "SKIP: already instrumented"
            break
        }
        $lines.Insert($i, '            std::cerr << "[Suspend] reached replace_=true line" << std::endl;')
        Write-Host "OK: inserted trace right before replace_=true at line $($i+1)"
        break
    }
}

Set-Content $f $lines
Write-Host ""
Write-Host "Rebuilding..."
cmake --build "$base\build" --config Debug 2>&1 | Select-Object -Last 20
