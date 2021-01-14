cmake_minimum_required(VERSION 3.15)

set(LIB_NAME KairosEngine)

project(${LIB_NAME} LANGUAGES CXX)

set(KRS_CORE_INC
    Engine/Core/Application.h
    Engine/Core/BaseTypes.h
    Engine/Core/EngineCore.h
    Engine/Core/EngineMath.h
    Engine/Core/KeyCodes.h
    Engine/Core/MouseCodes.h
    Engine/Core/Log.h
    Engine/Core/Filesystem.h
    Engine/Core/ApplicationEntry.h
    Engine/Core/ClassTemplates.h

)

set(KRS_CORE_SRC
    Engine/Core/Log.cpp
    Engine/Core/Application.cpp
    Engine/Core/Filesystem.cpp
    Engine/Core/ApplicationEntry.cpp
)

source_group("Core" FILES ${KRS_CORE_INC} ${KRS_CORE_SRC})



set(KRS_EVENT_INC
    Engine/Core/Events/Event.h
    Engine/Core/Events/KeyEvent.h
    Engine/Core/Events/MouseEvent.h
    Engine/Core/Events/WindowEvent.h
)
source_group("Core\\Events" FILES ${KRS_EVENT_INC})


set(KRS_COMPONENT_INC
    MeshDataComponent.h
    TextureDataComponent.h
)
list(TRANSFORM KRS_COMPONENT_INC PREPEND "Engine/Component/")
source_group("Component" FILES ${KRS_COMPONENT_INC})



set(KRS_GRAPHICS_CORE_INC
    Buffer.h
    BufferManager.h
    CommandAllocatorPool.h
    CommandContext.h
    CommandQueue.h
    DescriptorHeap.h
    DynamicDescriptorHeap.h
    Fence.h
    GraphicsTypes.h
    UploadResourceAllocator.h
    Mesh.h
    PipelineState.h
    RenderDevice.h
    Resource.h
    RootSignature.h
    Shader.h
    SwapChain.h
    Texture.h
    DX12RenderBackend.h
    PipelineStateManager.h
    RenderScene.h
    ShaderCompiler.h
)

set(KRS_GRAPHICS_CORE_SRC
    BufferManager.cpp
    CommandAllocatorPool.cpp
    CommandContext.cpp
    CommandQueue.cpp
    DescriptorHeap.cpp
    DynamicDescriptorHeap.cpp
    Fence.cpp
    UploadResourceAllocator.cpp
    Mesh.cpp
    PipelineState.cpp
    RenderDevice.cpp
    RootSignature.cpp
    Shader.cpp
    SwapChain.cpp
    Texture.cpp
    DX12RenderBackend.cpp
    PipelineStateManager.cpp
    RenderScene.cpp
    ShaderCompiler.cpp
)

list(TRANSFORM KRS_GRAPHICS_CORE_INC PREPEND "Engine/RenderBackend/")
list(TRANSFORM KRS_GRAPHICS_CORE_SRC PREPEND "Engine/RenderBackend/")
source_group("Render Backend" FILES ${KRS_GRAPHICS_CORE_INC} ${KRS_GRAPHICS_CORE_SRC})



set(KRS_EDITOR_INC
    EditorCamera.h
    EngineIMGUI.h
)

set(KRS_EDITOR_SRC
    EditorCamera.cpp
    EngineIMGUI.cpp
)
list(TRANSFORM KRS_EDITOR_INC PREPEND "Engine/Editor/")
list(TRANSFORM KRS_EDITOR_SRC PREPEND "Engine/Editor/")
source_group("Editor" FILES ${KRS_EDITOR_INC} ${KRS_EDITOR_SRC})




set(KRS_CORE_SHADERS
    Constants.hlsl
    EntryPoint.hlsl
    EngineMain.hlsl
    Camera.hlsl
    test.hlsl
)


list(TRANSFORM KRS_CORE_SHADERS PREPEND "../Data/shaders/")
source_group("Shaders" FILES ${KRS_CORE_SHADERS})
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




set(KRS_FRAMEWORK_INC
    Scene.h
    Camera.h

)
set(KRS_FRAMEWORK_SRC
    Scene.cpp
    Camera.cpp
)
list(TRANSFORM KRS_FRAMEWORK_INC PREPEND "Engine/Framework/")
list(TRANSFORM KRS_FRAMEWORK_SRC PREPEND "Engine/Framework/")
source_group("Framework" FILES ${KRS_FRAMEWORK_INC} ${KRS_FRAMEWORK_SRC})


set(KRS_UTIL_INC
    JSONUtil.h
)

set(KRS_UTIL_SRC
    JSONUtil.cpp
)
list(TRANSFORM KRS_UTIL_INC PREPEND "Engine/Util/")
list(TRANSFORM KRS_UTIL_SRC PREPEND "Engine/Util/")
source_group("Util" FILES ${KRS_UTIL_INC} ${KRS_UTIL_SRC})




set(KRS_SYSTEM_INC
    Engine.h
    WinWindowSystem.h
    D3DWindowSurface.h
    InputSystem.h
    GUISystem.h
    CVarSystem.h
)
set(KRS_SYSTEM_SRC
    Engine.cpp
    WinWindowSystem.cpp
    D3DWindowSurface.cpp
    InputSystem.cpp
    GUISystem.cpp
    CVarSystem.cpp
)
list(TRANSFORM KRS_SYSTEM_INC PREPEND "Engine/Systems/")
list(TRANSFORM KRS_SYSTEM_SRC PREPEND "Engine/Systems/")
source_group("Systems" FILES ${KRS_SYSTEM_INC} ${KRS_SYSTEM_SRC})

set(KRS_SYSTEM_INTERFACE
    IEngine.h
    IRenderBackend.h
    ISystem.h
    IEventSystem.h
    IWindowSurface.h
    IWindowSystem.h
    IInputSystem.h
    IGUISystem.h
    ICVarSystem.h
)
list(TRANSFORM KRS_SYSTEM_INTERFACE PREPEND "Engine/Systems/Interface/")
source_group("Systems\\Interface" FILES ${KRS_SYSTEM_INTERFACE})

set(KRS_ALL_INC
    ${KRS_GENERAL_INC}
    ${KRS_CORE_INC}
    ${KRS_EVENT_INC}
    ${KRS_GRAPHICS_CORE_INC}
    ${KRS_CORE_SHADERS}
    ${KRS_EDITOR_INC}
    ${KRS_FRAMEWORK_INC}
    ${KRS_UTIL_INC}
    ${KRS_SYSTEM_INC}
    ${KRS_SYSTEM_INTERFACE}
    ${KRS_COMPONENT_INC}
    
)

set(KRS_ALL_SRC
    ${KRS_GENERAL_SRC}
    ${KRS_CORE_SRC}
    ${KRS_GRAPHICS_CORE_SRC}
    ${KRS_EDITOR_SRC}
    ${KRS_FRAMEWORK_SRC}
    ${KRS_UTIL_SRC}
    ${KRS_SYSTEM_SRC}
)

add_library(${LIB_NAME} STATIC ${KRS_ALL_INC} ${KRS_ALL_SRC})


set_target_properties(${LIB_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)



# ----------------- LINKED LIBRARIES ---------------------- #
target_link_libraries(${LIB_NAME} PUBLIC DirectXTK12 assetlib imgui stb_image json assimp
    Shlwapi.lib
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
    WinPixEventRuntime
    shader_compiler
)


# ------------------ ADDITIONAL INCLUDE DIRECTORIES ------------ #

target_include_directories(${LIB_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    PUBLIC ${CMAKE_SOURCE_DIR}/external
)

# ---------------- PRECOMPILED HEADERS -------------- #

set_source_files_properties(${SRC} ${KRS_CORE_SRC} ${KRS_GRAPHICS_CORE_SRC} ${KRS_EDITOR_SRC} ${KRS_FRAMEWORK_SRC} ${KRS_UTIL_SRC} ${KRS_SYSTEM_SRC}
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

target_compile_definitions(${LIB_NAME}
    PUBLIC -DUNICODE
    PUBLIC -D_UNICODE
)