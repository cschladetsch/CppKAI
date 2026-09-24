# Entrypoint for the cppkai-build image.
# Configures (if needed) and builds CppKAI exactly the way this session's
# manual builds did: CMake + Ninja, Debug config.
$ErrorActionPreference = "Stop"

$repo = "C:\src"
$buildDir = Join-Path $repo "build"

if (-not (Test-Path $buildDir)) {
    Write-Host "No existing build/ directory -- running initial CMake configure..."
    cmake -S $repo -B $buildDir -G Ninja `
        -DCMAKE_BUILD_TYPE=Debug `
        -DCMAKE_CXX_COMPILER="C:/Program Files/LLVM/bin/clang++.EXE" `
        -DBOOST_ROOT="$env:BOOST_ROOT"
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configure failed with exit code $LASTEXITCODE"
        exit $LASTEXITCODE
    }
}

Write-Host "Building (Debug config)..."
cmake --build $buildDir --config Debug
$buildExit = $LASTEXITCODE

if ($buildExit -ne 0) {
    Write-Error "Build failed with exit code $buildExit"
    exit $buildExit
}

Write-Host ""
Write-Host "Build succeeded. Running ctest..."
Push-Location $buildDir
ctest -C Debug --output-on-failure
$testExit = $LASTEXITCODE
Pop-Location

exit $testExit