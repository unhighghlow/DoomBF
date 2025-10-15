@echo off
IF NOT EXIST "build" (mkdir build)
IF NOT EXIST "bin" (mkdir bin)
pushd .
cd build
cmake ..
cmake --build .
popd