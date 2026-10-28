cmake_minimum_required(VERSION 3.20)
project(matchmint_backend C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# DSA source files
set(DSA_SOURCES
    src/dsa/bracket_tree.c
    src/dsa/cyclic_schedule.c
    src/dsa/merge_sort.c
    src/dsa/interval_conflict.c
    src/dsa/audit_list.c
)

# HTTP & Network source files
set(HTTP_SOURCES
    src/response.c
    src/router.c
    src/server.c
    src/json_util.c
)

# Controller source files
set(CONTROLLER_SOURCES
    src/controllers/competition_controller.c
    src/controllers/team_controller.c
    src/controllers/match_controller.c
    src/controllers/result_controller.c
    src/controllers/dashboard_settings_controller.c
)

# Server source files
set(SERVER_SOURCES
    src/main.c
    src/database.c
    ${DSA_SOURCES}
    ${HTTP_SOURCES}
    ${CONTROLLER_SOURCES}
)

# Main Executable target
add_executable(matchmint_server ${SERVER_SOURCES})
target_link_libraries(matchmint_server PRIVATE SQLite3::SQLite3)

if(WIN32)
    target_link_libraries(matchmint_server PRIVATE ws2_32)
else()
    target_link_libraries(matchmint_server PRIVATE m)
endif()

# DSA Unit Test Executable
add_executable(test_dsa
    tests/unit/test_dsa.c
    ${DSA_SOURCES}
)

if(NOT WIN32)
    target_link_libraries(test_dsa PRIVATE m)
endif()

# Server & Router Unit Test Executable
add_executable(test_server
    tests/unit/test_server.c
    ${HTTP_SOURCES}
)

if(WIN32)
    target_link_libraries(test_server PRIVATE ws2_32)
endif()

# Competition & Team API Integration Test Executable
add_executable(test_competition_team_api
    tests/integration/test_competition_team_api.c
    src/database.c
    ${DSA_SOURCES}
    ${HTTP_SOURCES}
    ${CONTROLLER_SOURCES}
)
target_link_libraries(test_competition_team_api PRIVATE SQLite3::SQLite3)

if(WIN32)
    target_link_libraries(test_competition_team_api PRIVATE ws2_32)
else()
    target_link_libraries(test_competition_team_api PRIVATE m)
endif()

# Match & Fixtures API Integration Test Executable
add_executable(test_match_api
    tests/integration/test_match_api.c
    src/database.c
    ${DSA_SOURCES}
    ${HTTP_SOURCES}
    ${CONTROLLER_SOURCES}
)
target_link_libraries(test_match_api PRIVATE SQLite3::SQLite3)

if(WIN32)
    target_link_libraries(test_match_api PRIVATE ws2_32)
else()
    target_link_libraries(test_match_api PRIVATE m)
endif()

# Result, Standings & Rollback API Integration Test Executable
add_executable(test_result_api
    tests/integration/test_result_api.c
    src/database.c
    ${DSA_SOURCES}
    ${HTTP_SOURCES}
    ${CONTROLLER_SOURCES}
)
target_link_libraries(test_result_api PRIVATE SQLite3::SQLite3)

if(WIN32)
    target_link_libraries(test_result_api PRIVATE ws2_32)
else()
    target_link_libraries(test_result_api PRIVATE m)
endif()

# Dashboard, Venues & Settings API Integration Test Executable
add_executable(test_dashboard_settings_api
    tests/integration/test_dashboard_settings_api.c
    src/database.c
    ${DSA_SOURCES}
    ${HTTP_SOURCES}
    ${CONTROLLER_SOURCES}
)
target_link_libraries(test_dashboard_settings_api PRIVATE SQLite3::SQLite3)

if(WIN32)
    target_link_libraries(test_dashboard_settings_api PRIVATE ws2_32)
else()
    target_link_libraries(test_dashboard_settings_api PRIVATE m)
endif()



