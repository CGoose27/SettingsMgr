cmake_minimum_required(VERSION 3.10)

project(SettingsManager VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(SettingsManager Settings.c)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(SettingsManager PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_custom_target(install-directly
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SettingsManager> /usr/bin/
    COMMENT "Copying SettingsManager to /usr/bin..."
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -E remove /usr/bin/SettingsManager
    COMMENT "Removing SettingsManager from /usr/bin..."
)
