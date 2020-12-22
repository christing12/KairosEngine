cmake_minimum_required(VERSION 3.15)

set(LIB_NAME KairosEngine)

project(${LIB_NAME} LANGUAGES CXX)





set(KRS_CORE_INC
    Engine/Core/Application.h
    Engine/Core/Input.h
    Engine/Core/Window.h
    Engine/Core/BaseTypes.h
    Engine/Core/EngineCore.h
    Engine/Core/EngineMath.h
    Engine/Core/KeyCodes.h
    Engine/Core/MouseCodes.h
    Engine/Core/Log.h
)

set(KRS_CORE_SRC
    Engine/Core/Log.cpp
    Engine/Core/Window.cpp
    Engine/Core/Application.cpp
    Engine/Core/Input.cpp
)

source_group("Core" FILES ${KRS_CORE_INC} ${KRS_CORE_SRC})



set(KRS_EVENT_INC
    Engine/Core/Events/Event.h
    Engine/Core/Events/KeyEvent.h
    Engine/Core/Events/MouseEvent.h
    Engine/Core/Events/WindowEvent.h
)
source_group("Core\\Events" FILES ${KRS_EVENT_INC})





set(KRS_GRAPHICS_CORE_INC
    Buffer.h
    Camera.h
    CommandAllocatorPool.h
    CommandContext.h
    CommandQueue.h
    DescriptorHeap.h
    DynamicDescriptorHeap.h
    EngineFactory.h
    Fence.h
    GraphicsTypes.h
    LinearAllocator.h
    Mesh.h
    PipelineStateObject.h
    RenderDevice.h
    Renderer.h
    Resource.h
    RootSignature.h
    Shader.h
    SwapChain.h
    Texture.h
)

set(KRS_GRAPHICS_CORE_SRC
    Buffer.cpp
    Camera.cpp
    CommandAllocatorPool.cpp
    CommandContext.cpp
    CommandQueue.cpp
    DescriptorHeap.cpp
    DynamicDescriptorHeap.cpp
    EngineFactory.cpp
    Fence.cpp
    LinearAllocator.cpp
    Mesh.cpp
    PipelineStateObject.cpp
    RenderDevice.cpp
    Renderer.cpp
    Resource.cpp
    RootSignature.cpp
    Shader.cpp
    SwapChain.cpp
    Texture.cpp
)

list(TRANSFORM KRS_GRAPHICS_CORE_INC PREPEND "Engine/Graphics/Core/")
list(TRANSFORM KRS_GRAPHICS_CORE_SRC PREPEND "Engine/Graphics/Core/")
source_group("Graphics\\Core" FILES ${KRS_GRAPHICS_CORE_INC} ${KRS_GRAPHICS_CORE_SRC})


set(KRS_CORE_SHADERS
    Constants.hlsl
    PixelShader.hlsl
    VertexShader.hlsl
)
list(TRANSFORM KRS_CORE_SHADERS PREPEND "Engine/Graphics/Shaders/")
source_group("Graphics\\Shaders" FILES ${KRS_CORE_SHADERS})
set_property(SOURCE ${KRS_CORE_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")



# folders for visual studio
set(KRS_GENERAL_INC
    API.h
    Engine/krspch.h
)

set(KRS_GENERAL_SRC
    Engine/krspch.cpp
)
source_group(" " FILES ${KRS_GENERAL_INC} ${KRS_GENERAL_SRC})




set(KRS_ALL_INC
    ${KRS_GENERAL_INC}
    ${KRS_CORE_INC}
    ${KRS_EVENT_INC}
    ${KRS_GRAPHICS_CORE_INC}
    ${KRS_CORE_SHADERS}
    
)

set(KRS_ALL_SRC
    ${KRS_GENERAL_SRC}
    ${KRS_CORE_SRC}
    ${KRS_GRAPHICS_CORE_SRC}
)

add_library(${LIB_NAME} STATIC ${KRS_ALL_INC} ${KRS_ALL_SRC})


set_target_properties(${LIB_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)



# ----------------- LINKED LIBRARIES ---------------------- #
target_link_libraries(${LIB_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}/external/lib/DirectXTK12.lib
    PUBLIC ${CMAKE_SOURCE_DIR}/external/lib/assimp-vc142-mtd.lib
    PUBLIC d3d12.lib
    PUBLIC dxgi.lib
    PUBLIC d3dcompiler.lib
    PUBLIC dxguid.lib
)


# ------------------ ADDITIONAL INCLUDE DIRECTORIES ------------ #

target_include_directories(${LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    PUBLIC ${CMAKE_SOURCE_DIR}/external/include
)

# ---------------- PRECOMPILED HEADERS -------------- #

set_source_files_properties(${SRC} ${KRS_CORE_SRC} ${KRS_GRAPHICS_CORE_SRC}
    PROPERTIES 
        COMPILE_FLAGS /Yu"krspch.h"
)

set_source_files_properties(Engine/krspch.cpp
    PROPERTIES
        COMPILE_FLAGS /Yc"krspch.h"
)


# ------------- PREPROCESS DEFINES ------------- #

target_compile_definitions(${LIB_NAME}
    PUBLIC KRS_DEBUG
    PUBLIC KRS_PLATFORM_WINDOWS
)
