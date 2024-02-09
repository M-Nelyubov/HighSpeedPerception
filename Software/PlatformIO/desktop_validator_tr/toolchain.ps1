Push-Location
cd $PSScriptRoot
mkdir build -ErrorAction SilentlyContinue
cd build
cmake ..
cmake --build .
Pop-Location
. "$PSScriptRoot\build\Debug\targetRed.exe"
