cmake_minimum_required(VERSION 3.15)

set(ENGINE_PROJECT_NAME KairosEngine)

project(${ENGINE_PROJECT_NAME} LANGUAGES CXX)

# folders for visual studio
set(GENERAL
    KairosAPI.h
    krspch.h
    krspch.cpp
)
source_group(" " FILES ${GENERAL})

# Core folder
set(KRS_CORE_INC
   Application.h
   BaseTypes.h
   EngineCore.h
   MouseCodes.h
   KeyCodes.h
   Log.h
   Input.h
   Window.h
   EngineMath.h
)

set(KRS_CORE_SRC
   Application.cpp
   Log.cpp
   Input.cpp
   Window.cpp
)

list(TRANSFORM KRS_CORE_INC PREPEND "Engine/Core/")
list(TRANSFORM KRS_CORE_SRC PREPEND "Engine/Core/")
source_group("Core" FILES ${KRS_CORE_INC} ${KRS_CORE_SRC})


set(KRS_EVENT_INC
    Event.h
    KeyEvent.h
    MouseEvent.h
    WindowEvent.h
)
list(TRANSFORM KRS_EVENT_INC PREPEND "Engine/Core/Events/")
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



set(KRS_ALL_INC
    ${GENERAL}
    ${KRS_CORE_INC}
    ${KRS_EVENT_INC}
    ${KRS_GRAPHICS_CORE_INC}
)

set(KRS_ALL_SRC
    ${KRS_CORE_SRC}
    ${KRS_GRAPHICS_CORE_SRC}
)

add_library(${ENGINE_PROJECT_NAME} STATIC ${KRS_ALL_INC} ${KRS_ALL_SRC})

set_target_properties(${ENGINE_PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

target_include_directories(${ENGINE_PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Engine
    PUBLIC ${CMAKE_SOURCE_DIR}/external/include
    PUBLIC ${CMAKE_SOURCE_DIR}/external/include/DirectXTK12/Inc
)

target_compile_definitions(${ENGINE_PROJECT_NAME}
    PUBLIC KRS_DEBUG
    PUBLIC KRS_PLATFORM_WINDOWS
)


# link libraries
target_link_libraries(${ENGINE_PROJECT_NAME}
    PUBLIC
        ${EXTNERAL_DIR}/lib/DirectXTK12/DirectXTK12.lib
      #  ${CMAKE_SOURCE_DIR}/external/assimp/lib/assimp-vc142-mtd.lib
      #  ${CMAKE_SOURCE_DIR}/external/WinPixEventRuntime/bin/x64/WinPixEventRuntime.lib
 )

 target_compile_options(${ENGINE_PROJECT_NAME} PRIVATE "/wd4251")
target_compile_options(${ENGINE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/bigobj>)

# Precompiled Header Flags

set_source_files_properties(${KRS_CORE_SRC} ${KRS_GRAPHICS_CORE_SRC}
    PROPERTIES 
        COMPILE_FLAGS /Yu"krspch.h"
)

set_source_files_properties(krspch.cpp 
    PROPERTIES
        COMPILE_FLAGS /Yc"krspch.h"
)

# set_property(SOURCE ${KRS_CORE_SHADERS} PROPERTY VS_SETTINGS "ExcludedFromBuild=true")


install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)
