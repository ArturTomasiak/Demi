cmake_minimum_required(VERSION 3.15)
project(DemiEditor C)

set(GLAD_SRC "" CACHE STRING "Path to glad source files seperated by ; (gl.c and wgl.c)")
set(GLAD_INCLUDE "" CACHE PATH "Path to glad include directory")
set(FREETYPE_INCLUDE "" CACHE PATH "Path to FreeType include/freetype2 directory")
set(FREETYPE_LIB "" CACHE FILEPATH "Path to freetype.lib")

if(NOT GLAD_SRC OR NOT GLAD_INCLUDE OR NOT FREETYPE_INCLUDE OR NOT FREETYPE_LIB)
    message(FATAL_ERROR "You must set GLAD_SRC, GLAD_INCLUDE, FREETYPE_INCLUDE, and FREETYPE_LIB before configuring.")
endif()

set(CMAKE_C_COMPILER clang-cl)

set(COMMON_COMPILE_FLAGS /TC /W3 /Ox /MT /D_CRT_SECURE_NO_WARNINGS)
set(COMMON_LINK_LIBS opengl32.lib user32.lib shcore.lib comdlg32.lib gdi32.lib dwmapi.lib)

include_directories(
    ${GLAD_INCLUDE}
    ${FREETYPE_INCLUDE}
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.c)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/exe")

add_executable(Demi WIN32 ${SRC_FILES} ${GLAD_SRC})
target_compile_options(Demi PRIVATE ${COMMON_COMPILE_FLAGS} /DNDEBUG)
target_link_libraries(Demi PRIVATE ${FREETYPE_LIB} ${COMMON_LINK_LIBS})

add_executable(Debug ${SRC_FILES} ${GLAD_SRC})
target_compile_options(Debug PRIVATE ${COMMON_COMPILE_FLAGS} /Ddemidebug)
target_link_libraries(Debug PRIVATE ${FREETYPE_LIB} ${COMMON_LINK_LIBS})