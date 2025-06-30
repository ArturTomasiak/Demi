@echo off
setlocal enabledelayedexpansion

set COMPILER=clang-cl
set OUTPUT=exe\DemiEditor.exe

set SOURCES=
for /R src %%f in (*.c) do (
    set sources=!sources! "%%f"
)
set GLADSRC="C:\libraries\glad\src\gl.c" "C:\libraries\glad\src\wgl.c"

set INCLUDES=/I"C:\libraries\freetype\include\freetype2" /I"C:\libraries\glad\include"
set LIBRARIES="C:\libraries\freetype\lib\freetype.lib"

set LINKER_FLAGS=/link opengl32.lib user32.lib shcore.lib comdlg32.lib gdi32.lib dwmapi.lib
set COMPILER_FLAGS=/TC /W3 /Ox /MT
set DEFINES=/D_CRT_SECURE_NO_WARNINGS /DNDEBUG
where %COMPILER% >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: %COMPILER% not found.
    exit /b 1
)

echo Compiling...
%COMPILER% %GLADSRC% %SOURCES% %COMPILER_FLAGS% -o %OUTPUT% %DEFINES% %INCLUDES% %LIBRARIES% %LINKER_FLAGS%
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b 1
)

endlocal