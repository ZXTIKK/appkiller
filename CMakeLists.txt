cmake_minimum_required(VERSION 3.21)
project(APPKILLER)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/APPKILLER")

include(FetchContent)

FetchContent_Declare(
        jsonLib
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(jsonLib)

add_executable(${PROJECT_NAME} WIN32
        main.cpp
        lib/trayRender.h
        src/trayRender.cpp
        lib/readSettings.h
        src/readSettings.cpp
        app.rc
        lib/utils.h
)

if(WIN32 AND MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>>,copy_if_different,echo>
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/settings.json")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/settings.json"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)