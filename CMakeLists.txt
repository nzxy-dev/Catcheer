cmake_minimum_required(VERSION 3.10)
project(Catcheer VERSION 1.4.0 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# COMMON INCLUDES
# -----------------------------
include_directories(lib)
include_directories(lib/api-service)

# -----------------------------
# COMMON SOURCES
# -----------------------------
set(COMMON_SOURCES
    lib/api-service/api_service_shared.cpp
    lib/api-service/config_loader.cpp
)

# -----------------------------
# API SOURCES
# -----------------------------
if(WIN32)
    set(API_SOURCES lib/api-service/api_service_windows.cpp)
else()
    set(API_SOURCES lib/api-service/api_service_linux.cpp)
endif()

# -----------------------------
# WINDOWS BUILD (WebView2)
# -----------------------------
if(WIN32)
    message(STATUS "Building Catcheer for Windows (WebView2)")

    set(WIN_SOURCES
        ${COMMON_SOURCES}
        ${API_SOURCES}
        lib/main.cpp
        lib/core/windows_webview2/webview2.cpp
        lib/core/windows_webview2/icon_webview2.cpp
        lib/core/windows_webview2/performance_windows.cpp
        resource.rc
    )

    add_executable(Catcheer WIN32 ${WIN_SOURCES})

    target_compile_definitions(Catcheer PRIVATE
        _UNICODE
        UNICODE
        OS_WINDOWS
    )



#-------------includes
target_include_directories(Catcheer PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/core/windows_webview2/includes
)

#--------------libs
#target_link_directories(Catcheer PRIVATE
#    ${CMAKE_SOURCE_DIR}/lib/core/windows_webview2/libs
#)

#-----------------system libs
target_link_libraries(Catcheer PRIVATE
    User32
    Shell32
    Ole32
    Shlwapi
    "${CMAKE_SOURCE_DIR}/lib/core/windows_webview2/libs/WebView2Loader.lib"
)

# -----------------------------
# LINUX BUILD (GTK + WebKitGTK)
# -----------------------------
else()
    message(STATUS "Building Catcheer for Linux (GTK + WebKitGTK)")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)

    set(LINUX_SOURCES
        ${COMMON_SOURCES}
        ${API_SOURCES}
        lib/core/linux_gtk/gtk.cpp
        lib/core/linux_gtk/icon_gtk.cpp
        lib/core/linux_gtk/performance_linux.cpp
    )

    add_executable(catcheer ${LINUX_SOURCES})

    target_include_directories(catcheer PRIVATE
        ${GTK3_INCLUDE_DIRS}
        ${WEBKIT_INCLUDE_DIRS}
    )

    target_link_libraries(catcheer PRIVATE
        ${GTK3_LIBRARIES}
        ${WEBKIT_LIBRARIES}
    )

    target_compile_definitions(catcheer PRIVATE OS_LINUX)
endif()
