#include "krspch.h"
#include "CommandAllocatorPool.h"
#include "RenderDevice.h"

namespace Kairos {
    CommandAllocatorPool::CommandAllocatorPool(RenderDevice* pDevice, CommandType type)
        : m_Device(pDevice)
        , m_Type(type)
    {
        KRS_CORE_INFO("Command Allocator Pool has been started");
    }

    CommandAllocatorPool::~CommandAllocatorPool()
    {
    }

    void CommandAllocatorPool::RequestAvailableAllocator(Uint64 fenceValue, ID3D12CommandAllocator** allocator)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);

        // if there is at least one command alloc and its fence value is in the past then use it
        if (!m_FreedAllocators.empty() && m_FreedAllocators.front().FenceValue <= fenceValue) {
            auto& entry = m_FreedAllocators.front();
            *allocator = entry.CommandAllocator.Detach();
            auto hr = (*allocator)->Reset();
            KRS_CORE_ASSERT(SUCCEEDED(hr), "issue with resetting already allocated command allocator");
            m_FreedAllocators.pop();
        }
        else {
            auto hr = m_Device->D3DDevice()->CreateCommandAllocator(D3DCommandType(m_Type), IID_PPV_ARGS(allocator));
            KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with creating a new command allocator from ID3D12Device");
            m_AllocatorPool.emplace_back(*allocator);
        }
    }


    void CommandAllocatorPool::Shutdown() {
        while (!m_FreedAllocators.empty()) {
            m_FreedAllocators.pop();
        }
        m_AllocatorPool.clear();
    }


    void CommandAllocatorPool::FreeAllocator(Uint64 fenceValue, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_FreedAllocators.emplace(FencedCommandAllocator{ fenceValue, allocator });
    }
}