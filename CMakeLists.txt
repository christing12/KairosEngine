cmake_minimum_required(VERSION 3.15)


# EXES
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/bin) # Output directory for shared lib (.DLL)

set(ENGINE_DIR ${CMAKE_SOURCE_DIR}/KairosEngine)
set(EXTERNAL ${CMAKE_SOURCE_DIR}/external)

# something something
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# setting project name variable
set(PROJECT_NAME MeshRenderer)

# VERSION
set(KRS_VERSION_MAJOR 1)
set(KRS_VERSION_MINOR 0)

set(KRS_VERSION ${KRS_VERSION_MAJOR}.${KRS_VERSION_MINOR})

project(${PROJECT_NAME} VERSION ${KRS_VERSION} LANGUAGES CXX)

add_subdirectory(external)
add_subdirectory(AssetLib)
add_subdirectory(AssetBaker)

add_subdirectory(KairosEngine)

add_subdirectory(Applications)

set_directory_properties(PROPERTIES
    VS_STARTUP_PROJECT MeshRenderer
)
