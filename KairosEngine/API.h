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

#include "Engine/RenderBackend/GraphicsTypes.h"
#include "Engine/RenderBackend/Buffer.h"
#include "Engine/RenderBackend/CommandContext.h"
#include "Engine/RenderBackend/DescriptorHeap.h"
#include "Engine/RenderBackend/DynamicDescriptorHeap.h"
#include "Engine/RenderBackend/Fence.h"
#include "Engine/RenderBackend/Mesh.h"
#include "Engine/RenderBackend/PipelineState.h"
#include "Engine/RenderBackend/RenderDevice.h"
#include "Engine/RenderBackend/RootSignature.h"
#include "Engine/RenderBackend/Shader.h"
#include "Engine/RenderBackend/SwapChain.h"
#include "Engine/RenderBackend/Texture.h"


// ------------------------- SOME NEW STUFF
#include "Engine/RenderBackend/DX12RenderBackend.h"
#include "Engine/Core/ApplicationEntry.h"

#include "Engine/Systems/Engine.h"
#include "Engine/Systems/WinWindowSystem.h"
#include "Engine/Systems/GUISystem.h"
#include "Engine/Systems/InputSystem.h"

