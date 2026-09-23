$base = "C:\Users\chris\local\repos\CppKAI"
$f = "$base\Ext\CppKaiCore\Source\Library\Executor\Source\ExecutorPerform.cpp"
$lines = [System.Collections.Generic.List[string]](Get-Content $f)

$anchorIdx = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i].Contains("[ECI-loop] stack before")) {
        $anchorIdx = $i
        break
    }
}
if ($anchorIdx -eq -1) { Write-Host "FAIL: could not find [ECI-loop] stack before anchor"; exit }
Write-Host "Found ECI-loop anchor at line $($anchorIdx+1)"

$targetIdx = -1
for ($i = $anchorIdx; $i -lt $anchorIdx + 10 -and $i -lt $lines.Count; $i++) {
    if ($lines[$i].Trim() -eq "Eval(obj);") {
        $targetIdx = $i
        break
    }
}
if ($targetIdx -eq -1) { Write-Host "FAIL: could not find Eval(obj); near anchor"; exit }
Write-Host "Found Eval(obj); at line $($targetIdx+1)"

if ($lines[$targetIdx+1] -and $lines[$targetIdx+1].Contains("[ECI-loop] after Eval(obj) OK")) {
    Write-Host "SKIP: after-Eval trace already present"
} else {
    $lines.Insert($targetIdx + 1, '                std::cerr << "[ECI-loop] after Eval(obj) OK, continuation_ != cont: " << (continuation_ != cont) << ", replace_=" << replace_ << std::endl;')
    Write-Host "OK: inserted after-Eval trace"
}

$popIdx = -1
for ($i = $targetIdx; $i -lt $targetIdx + 15 -and $i -lt $lines.Count; $i++) {
    if ($lines[$i].Trim() -eq "context_->Pop();") {
        $popIdx = $i
        break
    }
}
if ($popIdx -eq -1) { Write-Host "FAIL: could not find context_->Pop() in range"; exit }
Write-Host "Found context_->Pop(); at line $($popIdx+1)"

if ($lines[$popIdx-1] -and $lines[$popIdx-1].Contains("[ECI-loop] before context_->Pop")) {
    Write-Host "SKIP: before-Pop trace already present"
} else {
    $lines.Insert($popIdx, '                            std::cerr << "[ECI-loop] before context_->Pop, context_->Size()=" << context_->Size() << std::endl;')
    Write-Host "OK: inserted before-Pop trace"
    $popIdx++
}

if ($lines[$popIdx+1] -and $lines[$popIdx+1].Contains("[ECI-loop] after context_->Pop OK")) {
    Write-Host "SKIP: after-Pop trace already present"
} else {
    $lines.Insert($popIdx + 1, '                            std::cerr << "[ECI-loop] after context_->Pop OK" << std::endl;')
    Write-Host "OK: inserted after-Pop trace"
}

Set-Content $f $lines
Write-Host ""
Write-Host "Rebuilding..."
cmake --build "$base\build" --config Debug 2>&1 | Select-Object -Last 20
