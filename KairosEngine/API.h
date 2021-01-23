#pragma once

#include "Engine/Core/Log.h"
#include "Engine/Core/MouseCodes.h"
#include "Engine/Core/KeyCodes.h"
#include "Engine/Core/Application.h"
#include "Engine/Core/Filesystem.h"
#include "Engine/Core/EngineMath.h"

#include "Engine/Core/Events/KeyEvent.h"
#include "Engine/Core/Events/MouseEvent.h"
#include "Engine/Core/Events/WindowEvent.h"

#include "Engine/Editor/EngineIMGUI.h"
#include "Engine/Editor/EditorCamera.h"
#include "Engine/Framework/Camera.h"

#include "Engine/RenderBackend/RHI/GraphicsTypes.h"
#include "Engine/RenderBackend/RHI/Buffer.h"
#include "Engine/RenderBackend/RHI/CommandContext.h"
#include "Engine/RenderBackend/RHI/DescriptorHeap.h"
#include "Engine/RenderBackend/RHI/Fence.h"
#include "Engine/RenderBackend/RHI/PipelineState.h"
#include "Engine/RenderBackend/RHI/RenderDevice.h"
#include "Engine/RenderBackend/RHI/RootSignature.h"
#include "Engine/RenderBackend/RHI/Shader.h"
#include "Engine/RenderBackend/RHI/SwapChain.h"
#include "Engine/RenderBackend/RHI/Texture.h"


#include "Engine/RenderBackend/Scene/Scene.h"
#include "Engine/RenderBackend/Scene/Mesh.h"
#include "Engine/RenderBackend/Scene/MeshComponent.h"
#include "Engine/RenderBackend/Scene/Material.h"

//#include "Engine/RenderBackend/RenderScene.h"

// ------------------------- SOME NEW STUFF
#include "Engine/RenderBackend/DX12RenderBackend.h"
#include "Engine/Core/ApplicationEntry.h"
#include "Engine/Systems/WinWindowSystem.h"
#include "Engine/Systems/GUISystem.h"
#include "Engine/Systems/InputSystem.h"

#include "Engine/Systems/Engine.h"


