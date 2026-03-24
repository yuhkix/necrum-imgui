#pragma once
#include "../pch.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

namespace renderer
{

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

class DX12Renderer
{
public:
	bool init(IDXGISwapChain3* swap_chain, ID3D12CommandQueue* command_queue);
	void begin_frame();
	void end_frame(IDXGISwapChain3* swap_chain);
	void shutdown();

	bool is_initialized() const { return initialized; }

private:
	HWND h_hwnd = nullptr;
	ID3D12Device* p_device = nullptr;
	ID3D12DescriptorHeap* p_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap* p_pd3dSrvDescHeap = nullptr;
	ID3D12CommandQueue* p_pd3dCommandQueue = nullptr;
	ID3D12GraphicsCommandList* p_pd3dCommandList = nullptr;
	ID3D12Fence* p_fence = nullptr;
	HANDLE h_fence_event = nullptr;
	UINT64 n_fence_last_signaled_value = 0;
	
	std::vector<FrameContext> m_frame_contexts;
	std::vector<ID3D12Resource*> m_main_render_target_resources;
	UINT m_buffer_count = 0;
	UINT m_frame_index = 0;

	bool initialized = false;

	void create_render_target(IDXGISwapChain3* swap_chain);
	void cleanup_render_target();
	void wait_for_last_submitted_frame();
	FrameContext* wait_for_next_frame_resources();
};

} // namespace renderer
