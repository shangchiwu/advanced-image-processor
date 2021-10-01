@echo off

:: make build directory

if not exist build (
    echo Making build directory...
    mkdir build
)
pushd build

:: dectet whether the path has changed

if not exist CMakeCache.txt goto :noClearCache
for /f %%i in ('findstr /L "CMAKE_CACHEFILE_DIR:INTERNAL=" CMakeCache.txt') do set cmakeCachefileDir=%%i
set "cmakeCachefileDir=%cmakeCachefileDir:~29%"
set "cmakeCachefileDir=%cmakeCachefileDir:/=\%"
if /I not %cmakeCachefileDir%==%cd% (
    echo Build directory path has changed. Removing old cache...
    del /F CMakeCache.txt
)
:noClearCache

:: generate CMake build system

if not exist CMakeCache.txt (
    echo Generating CMake Files...
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
)

:: build

echo Building...
cmake --build .

:: exit

popd
exit /b %errorlevel%
