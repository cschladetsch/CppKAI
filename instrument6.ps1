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
Write-Host "Found Suspend case at line $($suspendLine+1)"

function InsertAfterInRange($list, [int]$startIdx, [int]$endIdx, [string]$matchTrim, [string]$insertText, [string]$marker) {
    for ($i = $startIdx; $i -lt $endIdx -and $i -lt $list.Count; $i++) {
        if ($list[$i].Trim() -eq $matchTrim) {
            if ($i + 1 -lt $list.Count -and $list[$i+1].Contains($marker)) {
                Write-Host "SKIP: already instrumented after '$matchTrim'"
                return $endIdx
            }
            $list.Insert($i + 1, $insertText)
            Write-Host "OK: inserted after '$matchTrim' at line $($i+2)"
            return $endIdx + 1
        }
    }
    Write-Host "FAIL: could not find '$matchTrim' in range"
    return $endIdx
}

$endIdx = $suspendLine + 40
$endIdx = InsertAfterInRange $lines $suspendLine $endIdx "continuation_ = NewContinuation(funcObj);" '            std::cerr << "[Suspend] after NewContinuation, continuation_.Exists()=" << continuation_.Exists() << std::endl;' "[Suspend] after NewContinuation"
$endIdx = InsertAfterInRange $lines $suspendLine $endIdx "continuation_->InitialStackDepth = data_->Size();" '                std::cerr << "[Suspend] after InitialStackDepth OK" << std::endl;' "[Suspend] after InitialStackDepth"
$endIdx = InsertAfterInRange $lines $suspendLine $endIdx "continuation_->SetScope(New<void>());" '                std::cerr << "[Suspend] after SetScope OK, args.Exists()=" << continuation_->args.Exists() << std::endl;' "[Suspend] after SetScope"
$endIdx = InsertAfterInRange $lines $suspendLine $endIdx "continuation_->Enter(this);" '                std::cerr << "[Suspend] after Enter OK" << std::endl;' "[Suspend] after Enter"

Set-Content $f $lines
Write-Host ""
Write-Host "Rebuilding..."
cmake --build "$base\build" --config Debug 2>&1 | Select-Object -Last 20
