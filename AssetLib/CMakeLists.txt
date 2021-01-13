cmake_minimum_required(VERSION 3.15)

project(assetlib LANGUAGES CXX)


set(HEADERS
    AssetLoader.h
    TextureAsset.h
    MeshAsset.h
)
source_group("inc" FILES ${HEADERS})

set(SRC
    AssetLoader.cpp
    TextureAsset.cpp
    MeshAsset.cpp

)
source_group("src" FILES ${SRC})


add_library(assetlib STATIC ${HEADERS} ${SRC})

target_include_directories(assetlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(assetlib PRIVATE json lz4)

set_target_properties(assetlib
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)
