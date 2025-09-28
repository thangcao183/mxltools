# CMake toolchain file for cross-compiling to Windows
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Specify the cross compiler
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)

# Specify the target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set the resource compiler
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Set executable extension
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

# Qt5 cross-compile settings (if available)
set(QT5_ROOT_PATH "/usr/x86_64-w64-mingw32/lib/qt5")
set(CMAKE_PREFIX_PATH ${QT5_ROOT_PATH})