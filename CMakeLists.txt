cmake_minimum_required(VERSION 3.25)
project(DCMediaPlayer VERSION 0.5 LANGUAGES CXX RC)
if(NOT WIN32)
    message(FATAL_ERROR "MediaPlayer requires Windows")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLUGIN_ARCH x64)
else()
    set(PLUGIN_ARCH x86)
endif()
add_library(MediaPlayer SHARED
    src/main.cpp src/Dialog.cpp src/MediaPlayers.cpp src/MediaUtil.cpp src/Plugin.cpp
    src/resource.rc pluginsdk/Config.cpp pluginsdk/Core.cpp pluginsdk/Hooks.cpp
    pluginsdk/Hubs.cpp pluginsdk/Logger.cpp pluginsdk/UI.cpp pluginsdk/Util.cpp)
target_include_directories(MediaPlayer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(MediaPlayer PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
    _WIN32_WINNT=0x0601 WINVER=0x0601)
target_link_libraries(MediaPlayer PRIVATE user32)
set_target_properties(MediaPlayer PROPERTIES PREFIX "" OUTPUT_NAME "MediaPlayer-${PLUGIN_ARCH}")
if(MINGW)
    target_link_options(MediaPlayer PRIVATE -static -static-libgcc -static-libstdc++ -Wl,--kill-at)
endif()
set(DCPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DCPlusPlus-No-MDI" CACHE PATH "DC++ source for ABI tests")
include(CTest)
if(BUILD_TESTING)
    if(NOT EXISTS "${DCPP_SOURCE_DIR}/dcpp/plugins/PluginDefs.h")
        message(FATAL_ERROR "Set DCPP_SOURCE_DIR to your DC++ checkout, or BUILD_TESTING=OFF")
    endif()
    add_executable(compatibility_test tests/compatibility.cpp)
    target_include_directories(compatibility_test PRIVATE "${DCPP_SOURCE_DIR}")
    target_compile_definitions(compatibility_test PRIVATE UNICODE _UNICODE NOMINMAX)
    target_link_libraries(compatibility_test PRIVATE user32)
    if(MINGW)
        target_link_options(compatibility_test PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
    add_dependencies(compatibility_test MediaPlayer)
    add_test(NAME dcplusplus_compatibility COMMAND compatibility_test "$<TARGET_FILE:MediaPlayer>")
endif()
configure_file(packaging/info.xml.in "${CMAKE_CURRENT_BINARY_DIR}/info.xml" @ONLY)
add_custom_target(package_dcext
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/package/$<CONFIG>"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:MediaPlayer>" "${CMAKE_CURRENT_BINARY_DIR}/package/$<CONFIG>/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/info.xml" "${CMAKE_CURRENT_BINARY_DIR}/package/$<CONFIG>/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/packaging/MediaPlayer.ico" "${CMAKE_CURRENT_BINARY_DIR}/package/$<CONFIG>/"
    COMMAND "${CMAKE_COMMAND}" -E chdir "${CMAKE_CURRENT_BINARY_DIR}/package/$<CONFIG>"
        "${CMAKE_COMMAND}" -E tar cf "../../MediaPlayer-${PROJECT_VERSION}-${PLUGIN_ARCH}-$<CONFIG>.dcext" --format=zip
        info.xml "MediaPlayer-${PLUGIN_ARCH}.dll" MediaPlayer.ico
    DEPENDS MediaPlayer VERBATIM)
