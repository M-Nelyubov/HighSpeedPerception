Push-Location
cd $PSScriptRoot
mkdir build -ErrorAction SilentlyContinue
cd build
rm * -Recurse
cmake ..
cmake --build .
Pop-Location
. "$PSScriptRoot\build\Debug\targetRed.exe"
