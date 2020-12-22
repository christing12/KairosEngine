cmake_minimum_required(VERSION 3.15)


set(APP_NAME MeshRenderer)


set(APP_INC
    Source/MeshRenderer.h
)

set(APP_SRC
    Source/MeshRenderer.cpp
    Source/main.cpp
)

source_group("Source" FILES ${APP_INC} ${APP_SRC})

set(PCH
    apppch.h
    apppch.cpp
)
source_group(" " FILES ${PCH})

set(APP_ALL_INC
    ${APP_INC}
    ${PCH}
)

set(APP_ALL_SRC
    ${APP_SRC}
)

add_executable(${APP_NAME} ${APP_ALL_INC} ${APP_ALL_SRC})

set_target_properties(${APP_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin
        LINK_FLAGS /SUBSYSTEM:CONSOLE
)

target_include_directories(${APP_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/KairosEngine
    PRIVATE ${CMAKE_SOURCE_DIR}/external/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/test
)

target_link_libraries(${APP_NAME}
    PUBLIC test
)


# Precompiled Header Flags

set_source_files_properties(${APP_SRC}
    PROPERTIES 
        COMPILE_FLAGS /Yu"apppch.h"
)

set_source_files_properties(apppch.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"apppch.h"
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

#[[
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/lib/assimp/assimp-vc142-mtd.dll"
       # "${CMAKE_SOURCE_DIR}/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.dll"
        $<TARGET_FILE_DIR:MeshRenderer>
)
]]