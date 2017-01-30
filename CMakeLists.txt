cmake_minimum_required(VERSION 2.6)
project(de.codemusings.Octopus)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

pkg_check_modules(SQLITE3 REQUIRED sqlite3)
include_directories(${SQLITE3_INCLUDE_DIRS})
link_directories(${SQLITE3_LIBRARY_DIRS})
add_definitions(${SQLITE3_CFLAGS_OTHER})

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
    src/oct-task-view.c)
target_link_libraries(octopus ${GTK3_LIBRARIES} ${SQLITE3_LIBRARIES})
