cmake_minimum_required(VERSION 2.6)
project(de.codemusings.Octopus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JSONGLIB REQUIRED json-glib-1.0)
pkg_check_modules(CURL REQUIRED libcurl)

include_directories(
    ${GTK3_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${JSONGLIB_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)
link_directories(
    ${GTK3_LIBRARY_DIRS}
    ${SQLITE3_LIBRARY_DIRS}
    ${JSONGLIB_LIBRARY_DIRS}
    ${CURL_LIBRARY_DIRS}
)
add_definitions(
    ${GTK3_CFLAGS}
    ${SQLITE3_CFLAGS}
    ${JSONGLIB_CFLAGS}
    ${CURL_CFLAGS}
)

set(CMAKE_C_FLAGS "-Wall -Werror -pedantic -std=c99")

add_custom_command(
    OUTPUT src/oct-resources.h
    COMMAND glib-compile-resources
        --generate-header
        --target src/oct-resources.h
        --c-name oct res/de.codemusings.Octopus.gresource.xml)

add_custom_command(
    OUTPUT src/oct-resources.c
    COMMAND glib-compile-resources
        --generate-source
        --target src/oct-resources.c
        --c-name oct res/de.codemusings.Octopus.gresource.xml)

add_executable(octopus
    src/main.c
    src/octopus.c
    src/oct-cell-renderer-button.c
    src/oct-resources.h
    src/oct-resources.c
    src/oct-settings-popover.c
    src/oct-sync.c
    src/oct-task-view.c
)
target_link_libraries(octopus
    ${GTK3_LIBRARIES}
    ${SQLITE3_LIBRARIES}
    ${JSONGLIB_LIBRARIES}
    ${CURL_LIBRARIES}
)
