# Apply-SigmaFix.ps1
# Fixes the crash in SigmaTests.Calls on Windows (0xc0000005): a
# use-after-free in SigmaChecker::FunctionBody that MSVC exposes and GCC
# hides. One commit on CppKAI's 'sigma' branch. Run Apply-Sigma.ps1 first.
# The patch is embedded, so there is nothing else to download.
#
# Usage:  pwsh -File .\Apply-SigmaFix.ps1
#         pwsh -File .\Apply-SigmaFix.ps1 -Kai D:\code\CppKAI

param(
    [string]$Kai = "$HOME\local\repos\CppKAI",
    [string]$Branch = "sigma"
)

$ErrorActionPreference = "Stop"

$PatchBase64 = @"
RnJvbSA3ZjUxNWEzNjk2NDIwZTVjNDg5YzhiYzg3NmJiOTRmNmVhNzZmMGZlIE1vbiBTZXAgMTcgMDA6MDA6MDAgMjAwMQpGcm9tOiBDaHJpc3RpYW4gPGNo
cmlzdGlhbi5zY2hsYWRldHNjaEBnbWFpbC5jb20+CkRhdGU6IEZyaSwgMiBPY3QgMjAyNiAwMjoyMzo0NiArMTAwMApTdWJqZWN0OiBbUEFUQ0hdIFNpZ21h
OiBmaXggdXNlLWFmdGVyLWZyZWUgaW4gU2lnbWFDaGVja2VyOjpGdW5jdGlvbkJvZHkKIChjcmFzaCBvbiBNU1ZDKQoKRnVuY3Rpb25Cb2R5IGtlcHQgYSBC
aW5kaW5nKiBpbnRvIHRoZSBnbG9iYWwgc2NvcGUsIHRoZW4gcHVzaGVkIGEgbmV3CnNjb3BlIG9udG8gc2NvcGVzXywgYSBzdGQ6OnZlY3RvciBvZiB1bm9y
ZGVyZWRfbWFwcy4gTVNWQydzCnVub3JkZXJlZF9tYXAgbW92ZSBjb25zdHJ1Y3RvciBpcyBub3Qgbm9leGNlcHQsIHNvIHRoZSB2ZWN0b3IgY29waWVzIHRo
ZQptYXBzIHdoZW4gaXQgZ3Jvd3MgYW5kIGZyZWVzIHRoZSBvcmlnaW5hbHMsIGxlYXZpbmcgdGhlIHBvaW50ZXIKZGFuZ2xpbmc7IFNpZ21hVGVzdHMuQ2Fs
bHMgY3Jhc2hlZCB3aXRoIDB4YzAwMDAwMDUuIGxpYnN0ZGMrKyBtb3ZlcwppbnN0ZWFkLCBzbyBpdCBwYXNzZWQgb24gTGludXguIFJlcHJvZHVjZWQgdW5k
ZXIgQVNhbiB3aXRoIGEgU2NvcGUgd2hvc2UKbW92ZSBpcyBub2V4Y2VwdChmYWxzZSkuCgotIENvcHkgdGhlIHNpZ25hdHVyZSAoc2hhcmVkX3B0cikgYmVm
b3JlIHB1c2hpbmcgdGhlIHNjb3BlLgotIHNjb3Blc18gaXMgbm93IGEgc3RkOjpkZXF1ZSwgd2hpY2ggbmV2ZXIgcmVsb2NhdGVzIGV4aXN0aW5nIHNjb3Bl
cy4KLSBGb3J3YXJkLWRlY2xhcmUgUmVnaXN0cnkgYXMgYSBzdHJ1Y3QsIG1hdGNoaW5nIENwcEthaUNvcmU7IHRoZSBjbGFzcwogIHRhZyBjaGFuZ2VzIGl0
cyBtYW5nbGVkIG5hbWUgdW5kZXIgdGhlIE1pY3Jvc29mdCBBQkkuCgpDby1BdXRob3JlZC1CeTogQ2xhdWRlIE9wdXMgNS41IDxub3JlcGx5QGFudGhyb3Bp
Yy5jb20+CkNsYXVkZS1TZXNzaW9uOiBodHRwczovL2NsYXVkZS5haS9jb2RlL3Nlc3Npb25fMDFCdEJza1A0ZXRqRzlXOWtYdDJoOWhWCi0tLQogSW5jbHVk
ZS9LQUkvTGFuZ3VhZ2UvU2lnbWEvU2lnbWFDaGVja2VyLmggICAgICAgICAgICB8ICA1ICsrKy0tCiAuLi4vTGlicmFyeS9MYW5ndWFnZS9TaWdtYS9Tb3Vy
Y2UvU2lnbWFDaGVja2VyLmNwcCAgIHwgMTIgKysrKysrKystLS0tCiAyIGZpbGVzIGNoYW5nZWQsIDExIGluc2VydGlvbnMoKyksIDYgZGVsZXRpb25zKC0p
CgpkaWZmIC0tZ2l0IGEvSW5jbHVkZS9LQUkvTGFuZ3VhZ2UvU2lnbWEvU2lnbWFDaGVja2VyLmggYi9JbmNsdWRlL0tBSS9MYW5ndWFnZS9TaWdtYS9TaWdt
YUNoZWNrZXIuaAppbmRleCAwYWUzODgwLi4xYTRmOTM3IDEwMDY0NAotLS0gYS9JbmNsdWRlL0tBSS9MYW5ndWFnZS9TaWdtYS9TaWdtYUNoZWNrZXIuaAor
KysgYi9JbmNsdWRlL0tBSS9MYW5ndWFnZS9TaWdtYS9TaWdtYUNoZWNrZXIuaApAQCAtMyw2ICszLDcgQEAKICNpbmNsdWRlIDxLQUkvQ29yZS9Db25maWcv
QmFzZS5oPgogI2luY2x1ZGUgPEtBSS9MYW5ndWFnZS9TaWdtYS9TaWdtYUFzdE5vZGUuaD4KIAorI2luY2x1ZGUgPGRlcXVlPgogI2luY2x1ZGUgPG1lbW9y
eT4KICNpbmNsdWRlIDxzdHJpbmc+CiAjaW5jbHVkZSA8dW5vcmRlcmVkX21hcD4KQEAgLTExLDcgKzEyLDcgQEAKIAogS0FJX0JFR0lOCiAKLWNsYXNzIFJl
Z2lzdHJ5Oworc3RydWN0IFJlZ2lzdHJ5OwogCiBzdHJ1Y3QgU2lnbWFUeXBlOwogdXNpbmcgU2lnbWFUeXBlUHRyID0gc3RkOjpzaGFyZWRfcHRyPGNvbnN0
IFNpZ21hVHlwZT47CkBAIC0xMDYsNyArMTA3LDcgQEAgY2xhc3MgU2lnbWFDaGVja2VyIHsKIAogICAgIFJlZ2lzdHJ5ICpyZWdfID0gbnVsbHB0cjsKICAg
ICBHbG9iYWxzIHNlc3Npb25fOwotICAgIHN0ZDo6dmVjdG9yPFNjb3BlPiBzY29wZXNfOworICAgIHN0ZDo6ZGVxdWU8U2NvcGU+IHNjb3Blc187ICAvLyBk
ZXF1ZTogZ3Jvd2luZyBpdCBuZXZlciBtb3ZlcyBleGlzdGluZyBzY29wZXMKICAgICBzdGQ6OnZlY3RvcjxTaWdtYURpYWdub3N0aWM+IGRpYWdub3N0aWNz
XzsKICAgICBzdGQ6OnVub3JkZXJlZF9zZXQ8Y29uc3QgU2lnbWFBc3ROb2RlICo+IHdpZGVuZWRfOwogICAgIHN0ZDo6dmVjdG9yPE5vZGVQdHI+IGZ1bmN0
aW9uc187CmRpZmYgLS1naXQgYS9Tb3VyY2UvTGlicmFyeS9MYW5ndWFnZS9TaWdtYS9Tb3VyY2UvU2lnbWFDaGVja2VyLmNwcCBiL1NvdXJjZS9MaWJyYXJ5
L0xhbmd1YWdlL1NpZ21hL1NvdXJjZS9TaWdtYUNoZWNrZXIuY3BwCmluZGV4IDQxMjA5MGYuLjUyZWQzMjAgMTAwNjQ0Ci0tLSBhL1NvdXJjZS9MaWJyYXJ5
L0xhbmd1YWdlL1NpZ21hL1NvdXJjZS9TaWdtYUNoZWNrZXIuY3BwCisrKyBiL1NvdXJjZS9MaWJyYXJ5L0xhbmd1YWdlL1NpZ21hL1NvdXJjZS9TaWdtYUNo
ZWNrZXIuY3BwCkBAIC00ODcsMTggKzQ4NywyMiBAQCBib29sIFNpZ21hQ2hlY2tlcjo6QWx3YXlzUmV0dXJucyhjb25zdCBOb2RlUHRyICZub2RlKSB7CiB9
CiAKIHZvaWQgU2lnbWFDaGVja2VyOjpGdW5jdGlvbkJvZHkoY29uc3QgTm9kZVB0ciAmZnVuKSB7Ci0gICAgYXV0byBzaWcgPSBMb29rdXAoZnVuLT5HZXRU
b2tlbigpLlRleHQoKSk7Ci0gICAgaWYgKCFzaWcgfHwgIXNpZy0+dHlwZS0+SXMoS2luZDo6RnVuKSkgcmV0dXJuOworICAgIC8vIENvcHkgdGhlIHNpZ25h
dHVyZTogcHVzaGluZyBhIHNjb3BlIGJlbG93IG1heSByZWFsbG9jYXRlIHNjb3Blc18sIGFuZAorICAgIC8vIGEgcG9pbnRlciBpbnRvIHRoZSBvbGQgZ2xv
YmFsIHNjb3BlIHdvdWxkIHRoZW4gZGFuZ2xlIChNU1ZDIGNvcGllcworICAgIC8vIHVub3JkZXJlZF9tYXBzIHdoZW4gYSB2ZWN0b3IgZ3Jvd3MsIGJlY2F1
c2UgdGhlaXIgbW92ZSBpc24ndCBub2V4Y2VwdCkuCisgICAgY29uc3QgQmluZGluZyAqZm91bmQgPSBMb29rdXAoZnVuLT5HZXRUb2tlbigpLlRleHQoKSk7
CisgICAgaWYgKCFmb3VuZCB8fCAhZm91bmQtPnR5cGUtPklzKEtpbmQ6OkZ1bikpIHJldHVybjsKKyAgICBjb25zdCBTaWdtYVR5cGVQdHIgc2lnID0gZm91
bmQtPnR5cGU7CiAKICAgICBmdW5jdGlvbl8gPSBmdW4uZ2V0KCk7Ci0gICAgcmVzdWx0XyA9IHNpZy0+dHlwZS0+cmVzdWx0OworICAgIHJlc3VsdF8gPSBz
aWctPnJlc3VsdDsKICAgICBzY29wZXNfLmVtcGxhY2VfYmFjaygpOwogCiAgICAgY29uc3QgYXV0byAmcGFyYW1zID0gZnVuLT5HZXRDaGlsZCgwKS0+R2V0
Q2hpbGRyZW4oKTsKICAgICBmb3IgKHNpemVfdCBuID0gMDsgbiA8IHBhcmFtcy5zaXplKCk7ICsrbikgewogICAgICAgICBjb25zdCBzdGQ6OnN0cmluZyBu
YW1lID0gcGFyYW1zW25dLT5HZXRUb2tlbigpLlRleHQoKTsKICAgICAgICAgaWYgKHNjb3Blc18uYmFjaygpLmNvbnRhaW5zKG5hbWUpKSBSZXBvcnQocGFy
YW1zW25dLCBzdGQ6OmZvcm1hdCgiZHVwbGljYXRlIHBhcmFtZXRlciAne30nIiwgbmFtZSkpOwotICAgICAgICBCaW5kKG5hbWUsIHNpZy0+dHlwZS0+YXJn
c1tuXSk7CisgICAgICAgIEJpbmQobmFtZSwgc2lnLT5hcmdzW25dKTsKICAgICB9CiAKICAgICBjb25zdCBhdXRvICZib2R5ID0gZnVuLT5HZXRDaGlsZCgy
KTsKLS0gCjIuNDMuMAoK
"@

if (-not (Test-Path (Join-Path $Kai ".git"))) { throw "Not a git repository: $Kai" }
Push-Location $Kai
try {
    if (Test-Path (git rev-parse --git-path rebase-apply)) {
        throw "A git am or rebase is in progress in $Kai. Finish it or run 'git am --abort' first."
    }
    $dirty = git status --porcelain --untracked-files=no --ignore-submodules=all
    if ($dirty) { throw "$Kai has uncommitted changes; commit or stash them first:`n$($dirty -join "`n")" }

    git switch $Branch
    if ($LASTEXITCODE -ne 0) { throw "No '$Branch' branch in $Kai. Run Apply-Sigma.ps1 first." }

    if (-not (git log --oneline --fixed-strings --grep "Sigma: statically typed language that compiles to Rho")) {
        throw "The Sigma commit is not on '$Branch'. Run Apply-Sigma.ps1 first."
    }

    if (git log --oneline --fixed-strings --grep "Sigma: fix use-after-free in SigmaChecker::FunctionBody") {
        Write-Host "Already applied."
    } else {
        $patch = Join-Path ([IO.Path]::GetTempPath()) "kai-sigma-fix.patch"
        [IO.File]::WriteAllBytes($patch, [Convert]::FromBase64String($PatchBase64))
        git am --keep-cr -3 $patch
        if ($LASTEXITCODE -ne 0) {
            Write-Warning "git am stopped. Run 'git status', fix the conflict, 'git add <file>', 'git am --continue'. Or 'git am --abort' to back out."
            exit 1
        }
        Remove-Item $patch -ErrorAction SilentlyContinue
    }

    git log --oneline -3
    Write-Host ""
    Write-Host "Next: rebuild, then run TestSigma:" -ForegroundColor Green
    Write-Host "  cmake --build build --target TestSigma"
    Write-Host "  .\Bin\Test\TestSigma.exe"
}
finally {
    Pop-Location
}
