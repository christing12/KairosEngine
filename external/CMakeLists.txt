include(ExternalProject)

add_library(stb_image INTERFACE) # header only library
add_library(json INTERFACE) #json header only library
add_library(lz4 STATIC)
add_library(DirectXTK12 STATIC)

target_include_directories(json INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json)



# -------------- LZ4 COMPRESSION ------------------ ##

target_sources(lz4 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lz4/lz4.cpp
)
target_include_directories(lz4 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lz4)

target_compile_options(lz4 PUBLIC -std=c++17)


# --------------- IMGUI STUFF ----------------------- ##
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

target_sources(imgui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h

    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_dx12.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_win32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_dx12.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)
# ------------------- END --------------------- #


target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)


# ------------- ASSIMP STUFF ------------------
add_library(assimp INTERFACE)


set(assimp_dir ${CMAKE_CURRENT_SOURCE_DIR}/assimp/lib)
set(assimp_dll ${CMAKE_CURRENT_SOURCE_DIR}/assimp/lib/Debug/assimp-vc142-mtd.dll)
set(assimp_lib ${CMAKE_CURRENT_SOURCE_DIR}/assimp/lib/Debug/assimp-vc142-mtd.lib)

target_include_directories(assimp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include)
target_link_libraries(assimp INTERFACE ${assimp_lib})


# ---------------- DIRECTXTK STUFF --------------- 

set(DX12_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/DirectXTK12/include/DirectXTK/SimpleMath.h
    ${CMAKE_CURRENT_SOURCE_DIR}/DirectXTK12/include/DirectXTK/SimpleMath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DirectXTK12/include/DirectXTK/SimpleMath.inl
)

target_sources(DirectXTK12 PRIVATE ${DX12_SRC})
source_group("src" FILES ${DX12_SRC})

target_include_directories(DirectXTK12 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/DirectXTK12/include)
