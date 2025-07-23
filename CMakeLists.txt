cmake_minimum_required(VERSION 3.1)

project(amoebash)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Nice hierarchical structure in MSVC
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Detect OS
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(IS_OS_MAC 1)
    set(MACOSX_BUNDLE TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_OS_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(IS_OS_WINDOWS 1)
else()
    message(FATAL_ERROR "OS ${CMAKE_SYSTEM_NAME} was not recognized")
endif()

# Generate the shader folder location to the header
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/ext/project_path.hpp")

# Find source files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# Icon files
set(ICON_ICNS "${CMAKE_CURRENT_SOURCE_DIR}/MyIcon.icns")  # macOS icon
set(ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/MyIcon.ico")    # Windows icon
set(WIN_RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/windows_icon.rc")  # Windows resource script

# External libraries for macOS
if(IS_OS_MAC)
    include_directories(/usr/local/include)
    link_directories(/usr/local/lib)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

# Add executable and set icon depending on platform
if(IS_OS_MAC)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCE_FILES})

    # Set the icon filename (just the filename, no path)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MACOSX_BUNDLE_ICON_FILE MyIcon.icns)

    # Add the icon to the bundle's Resources folder
    set_source_files_properties(${ICON_ICNS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${PROJECT_NAME} PRIVATE ${ICON_ICNS})

elseif(IS_OS_WINDOWS)
    # Add Windows resource file for icon embedding
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${WIN_RC_FILE})
else()
    # Linux and others
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC src/)

# Added this so policy CMP0065 doesn't scream
set_target_properties(${PROJECT_NAME} PROPERTIES ENABLE_EXPORTS 0)

# External header-only libraries in ext/
target_include_directories(${PROJECT_NAME} PUBLIC ext/stb_image/)
target_include_directories(${PROJECT_NAME} PUBLIC ext/gl3w)

# Find OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_gl_LIBRARY})
endif()

# Find Freetype
find_package(freetype REQUIRED)
if(FREETYPE_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${FREETYPE_LIBRARIES})
endif()

# GLM
set(glm_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/cmake/glm)
find_package(glm REQUIRED)

# Platform-specific dependencies
if(IS_OS_LINUX OR IS_OS_MAC)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    pkg_search_module(SDL2 REQUIRED sdl2)
    pkg_search_module(SDL2MIXER REQUIRED SDL2_mixer)

    if(IS_OS_MAC)
        find_library(COCOA_LIBRARY Cocoa)
        find_library(CF_LIBRARY CoreFoundation)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY} ${CF_LIBRARY})
    endif()

    target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")

elseif(IS_OS_WINDOWS)
    set(GLFW_FOUND TRUE)
    set(SDL2_FOUND TRUE)

    set(GLFW_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include")
    set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/include/SDL")

    set(GLFW_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3dll-x64.lib")
    set(SDL2_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.lib")
    set(SDL2MIXER_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.lib")

    set(GLFW_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/lib/glfw3-x64.dll")
    set(SDL_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2-x64.dll")
    set(SDLMIXER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/ext/sdl/lib/SDL2_mixer-x64.dll")

    set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include")
    set(FREETYPE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/release static/vs2015-2022/win64/freetype.lib")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLFW_DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDLMIXER_DLL}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2_mixer.dll")

    add_compile_options(/w4 /we4715 /EHsc /we4239)
endif()

# Error if missing critical libs
if(NOT GLFW_FOUND OR NOT SDL2_FOUND)
    if(NOT GLFW_FOUND)
        message(FATAL_ERROR "Can't find GLFW.")
    else()
        message(FATAL_ERROR "Can't find SDL.")
    endif()
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/ext/freetype/include")

target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW_LIBRARIES} ${SDL2_LIBRARIES} ${SDL2MIXER_LIBRARIES} glm::glm ${FREETYPE_LIBRARY})

# Linux-specific linkage
if(IS_OS_LINUX)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${CMAKE_DL_LIBS})
endif()

# Memory Sanitizer
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/Zi /RTC1)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
