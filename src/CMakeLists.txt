cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME bose-connect-app-linux)

# Set the project name and version.
project(${PROJECT_NAME} VERSION 1.0)

# Specify the C standard.
set(CMAKE_)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)


# ------------------------------------------------------
# Option
# ------------------------------------------------------
option(WITH_GUI "Build with GUI (default)" ON)


# ------------------------------------------------------
# Check Pkg Config.
# ------------------------------------------------------
find_package(PkgConfig REQUIRED)

if (NOT PKG_CONFIG_FOUND)
    message(FATAL_ERROR "The 'pkgconf' package is not found.")
endif ()


# ------------------------------------------------------
# Check BlueZ
# ------------------------------------------------------
pkg_search_module(BLUEZ REQUIRED bluez)

if (NOT BLUEZ_FOUND)
    message(FATAL_ERROR "Please install your distro's bluez development libraries.")
elseif (NOT BLUEZ_VERSION)
    message(FATAL_ERROR "The version of Bluez is not correct: ${BLUEZ_VERSION}.")
endif ()

pkg_check_modules(BLUEZ REQUIRED bluez)
include_directories(${BLUEZ_INCLUDE_DIR})
link_directories(${BLUEZ_LIBRARIES})

if(WITH_GUI)

# ------------------------------------------------------
# Check GTK4
# ------------------------------------------------------
pkg_search_module(GTK4 REQUIRED IMPORTED_TARGET gtk4)

if (NOT GTK4_FOUND)
    message(FATAL_ERROR "Please install your distro's GTK4 development libraries.")
elseif (NOT GTK4_VERSION)
    message(FATAL_ERROR "The version of GTK4 is not correct: ${GTK4_VERSION}.")
endif ()

pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
include_directories(${GTK4_INCLUDE_DIRS})


endif()

include(FetchContent)
FetchContent_Declare(gattlib
    GIT_REPOSITORY https://github.com/labapart/gattlib/
    GIT_TAG 1580056bce260f0d8ad7ccc8b105c34057cd1fbd
)
FetchContent_MakeAvailable(gattlib)

FetchContent_GetProperties(gattlib
    SOURCE_DIR GATTLIB_SOURCE_DIR
    BINARY_DIR GATTLIB_BINARY_DIR
)

message(STATUS ${GATTLIB_BINARY_DIR})
include_directories(${GATTLIB_BINARY_DIR}/dbus)
link_directories(${GATTLIB_BINARY_DIR})

# ------------------------------------------------------
# Add the executable.
# ------------------------------------------------------

FILE(GLOB REGULAR_SOURCES main.c library/cli.c library/cli.h library/based.c library/based.h library/bluetooth.c library/bluetooth.h library/util.c library/util.h)
FILE(GLOB GUI_SOURCES gtk/bluetooth.c gtk/gtk.c gtk/connection_panel.c gtk/info_panel.c)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_definitions(PROGRAM_NAME="${PROJECT_NAME}")
if(WITH_GUI)
add_compile_definitions(WITH_GUI=1)
endif()

if(WITH_GUI)
add_executable(
    ${PROJECT_NAME}
    ${REGULAR_SOURCES}
    ${GUI_SOURCES}
)
target_link_libraries(${PROJECT_NAME} bluetooth PkgConfig::GTK4 m gattlib)
else()
add_executable(
    ${PROJECT_NAME}
    ${REGULAR_SOURCES}
)
target_link_libraries(${PROJECT_NAME} bluetooth gattlib)
endif()
get_target_property(PROJECT_SOURCES ${PROJECT_NAME} SOURCES)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)


# ------------------------------------------------------
# CLang Format: Autoformatting
# ------------------------------------------------------
if (AUTOFORMATTING)
    add_custom_command(
            COMMENT "\n\nAutoformatting: CLang Format\n"
            TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND
            cd ${CMAKE_SOURCE_DIR} &&
            clang-format
            "--sort-includes"
            "--verbose"
            "--Werror"
            "-i"
            ${PROJECT_SOURCES}
            VERBATIM
    )
endif ()


# ------------------------------------------------------
# CLang Format
# ------------------------------------------------------
if (VALIDATE_QA)
    add_custom_command(
            COMMENT "\n\nScanning: CLang Format\n"
            TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND
            cd ${CMAKE_SOURCE_DIR} &&
            clang-format
            "--Werror"
            "--dry-run"
            ${PROJECT_SOURCES}
            VERBATIM
    )
endif ()

# ------------------------------------------------------
# CLang Tidy
# ------------------------------------------------------
if (VALIDATE_QA)
    add_custom_command(
            COMMENT "\n\nScanning: CLang Tidy\n"
            TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND
            cd ${CMAKE_SOURCE_DIR} &&
            clang-tidy
            -p ${CMAKE_BINARY_DIR}
            ${PROJECT_SOURCES}
            VERBATIM
    )
endif ()


# ------------------------------------------------------
# CPP Check
# ------------------------------------------------------
set(CPPCHECK_TEMPLATE "")
string(APPEND CPPCHECK_TEMPLATE "{file}:{line}:{column}\\n")
string(APPEND CPPCHECK_TEMPLATE "  Message: {message}\\n")
string(APPEND CPPCHECK_TEMPLATE "    Check: CWE-{cwe}\\n")
string(APPEND CPPCHECK_TEMPLATE "     Type: {severity} -> {id}\\n")
string(APPEND CPPCHECK_TEMPLATE "     Line: {line}:{column}\\n")
string(APPEND CPPCHECK_TEMPLATE "    Stack: {callstack}\\n")
string(APPEND CPPCHECK_TEMPLATE "{code}\\n")

set(CPPCHECK_TEMPLATE_LOCATION "")
string(APPEND CPPCHECK_TEMPLATE_LOCATION "{file}:{line}:{column}\\n")
string(APPEND CPPCHECK_TEMPLATE_LOCATION "   Note: {info}\\n")
string(APPEND CPPCHECK_TEMPLATE_LOCATION "   Line: {line}:{column}\\n")
string(APPEND CPPCHECK_TEMPLATE_LOCATION "{code}\\n")

if (VALIDATE_QA)
    add_custom_command(
            COMMENT "\n\nScanning: CPP Check\n"
            TARGET ${PROJECT_NAME}
            PRE_BUILD
            COMMAND cppcheck
            "--language=c"
            "--std=c11"
            "--platform=unix64"
            "--suppress=missingIncludeSystem"
            "--enable=all"
            "--inconclusive"
            "--force"
            "--max-ctu-depth=1000000"
            "--library=boost.cfg"
            "--library=cppcheck-lib.cfg"
            "--library=cppunit.cfg"
            "--library=gnu.cfg"
            "--library=libcerror.cfg"
            "--library=posix.cfg"
            "--library=std.cfg"
            "--template=${CPPCHECK_TEMPLATE}"
            "--template-location=${CPPCHECK_TEMPLATE_LOCATION}"
            "--project=${CMAKE_BINARY_DIR}/compile_commands.json"
            "--error-exitcode=1"
            VERBATIM
    )
endif ()
