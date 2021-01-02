
cmake_minimum_required(VERSION 3.15)

add_executable(baker
	asset_main.cpp
)


# set_property(TARGET baker PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:KairosEngine>)


target_include_directories(baker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(baker PUBLIC stb_image json lz4 assetlib assimp DirectXTK12)

set_target_properties(baker
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
