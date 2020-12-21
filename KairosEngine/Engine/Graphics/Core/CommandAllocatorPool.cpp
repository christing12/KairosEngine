#include "krspch.h"
#include "CommandAllocatorPool.h"
#include "RenderDevice.h"

namespace Kairos {
    CommandAllocatorPool::CommandAllocatorPool(RenderDevice* pDevice, D3D12_COMMAND_LIST_TYPE type, const std::wstring& name)
        : m_Device(pDevice)
        , m_CommandType(type)
        , m_Name(name)
    {
        KRS_CORE_INFO("Command Allocator Pool has been started");
    }

    CommandAllocatorPool::~CommandAllocatorPool()
    {

    }

    void CommandAllocatorPool::Shutdown() {
        for (ComPtr<ID3D12CommandAllocator> allo : m_Allocators) {
            allo = nullptr;
        }

        m_Allocators.clear();

        while (!m_CommandAllocators.empty()) {
            CommandAllocatorEntry& entry = m_CommandAllocators.front();
            m_CommandAllocators.pop();
            entry.CommandAllocator = nullptr;
        }
    }

    // gets allocator from the free ones in queue, otherwise get a new one and put it in the vector?
    void CommandAllocatorPool::GetUnusedAllocator(Uint64 fenceValue, ID3D12CommandAllocator** allocator)
    {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        // if there is at least one command alloc and its fence value is in the past then use it
        if (!m_CommandAllocators.empty() && m_CommandAllocators.front().AssociatedFenceValue <= fenceValue) {
            CommandAllocatorEntry& entry = m_CommandAllocators.front();
            if (entry.AssociatedFenceValue <= fenceValue) {
                *allocator = entry.CommandAllocator.Detach();
                KRS_CORE_ASSERT(allocator != nullptr, "Issue wtih allocator");
                auto hr = (*allocator)->Reset();
                KRS_CORE_ASSERT(SUCCEEDED(hr), "Issue with releasing command allocator");
                m_CommandAllocators.pop();
            }
        }
        else {
            auto hr = m_Device->GetD3DDevice()->CreateCommandAllocator(m_CommandType, IID_PPV_ARGS(allocator));
            KRS_CORE_ASSERT(SUCCEEDED(hr), "Faile dto Create New Command Allocator in the pool");
            std::wstring test = m_Name + L"Test";
            (*allocator)->SetName(test.c_str());
            m_Allocators.push_back(*allocator);
        }
    }

    void CommandAllocatorPool::Push(Uint64 fenceValue, ComPtr<ID3D12CommandAllocator> allocator)
    {
        std::lock_guard<std::mutex> lock(m_AllocationMutex);
        m_CommandAllocators.emplace(CommandAllocatorEntry{ fenceValue, allocator });
    }
}