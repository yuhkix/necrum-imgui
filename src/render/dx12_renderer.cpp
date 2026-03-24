#include "dx12_renderer.h"
#include "../menu/theme.h"
#include "../core/web_image.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_dx12.h"
#include "../ext/imgui/backends/imgui_impl_win32.h"

namespace renderer
{

bool DX12Renderer::init(IDXGISwapChain3* swap_chain, ID3D12CommandQueue* command_queue)
{
	if (initialized)
		return true;

	DXGI_SWAP_CHAIN_DESC desc;
	if (FAILED(swap_chain->GetDesc(&desc)))
		return false;

	h_hwnd = desc.OutputWindow;
	m_buffer_count = desc.BufferCount;
	p_pd3dCommandQueue = command_queue;

	if (FAILED(swap_chain->GetDevice(IID_PPV_ARGS(&p_device))))
		return false;

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = m_buffer_count;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		if (p_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&p_pd3dRtvDescHeap)) != S_OK)
			return false;

		SIZE_T rtvDescriptorSize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = p_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < m_buffer_count; i++)
		{
			ID3D12Resource* pBackBuffer = nullptr;
			swap_chain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
			p_device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);
			m_main_render_target_resources.push_back(pBackBuffer);
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1024;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (p_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&p_pd3dSrvDescHeap)) != S_OK)
			return false;
	}

	for (UINT i = 0; i < m_buffer_count; i++)
	{
		ID3D12CommandAllocator* allocator = nullptr;
		if (p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)) != S_OK)
			return false;
		m_frame_contexts.push_back({allocator, 0});
	}

	if (p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frame_contexts[0].CommandAllocator, nullptr,
																	IID_PPV_ARGS(&p_pd3dCommandList)) != S_OK ||
			p_pd3dCommandList->Close() != S_OK)
		return false;

	if (p_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence)) != S_OK)
		return false;

	h_fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (h_fence_event == nullptr)
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	if (!ImGui_ImplWin32_Init(h_hwnd))
		return false;

	if (!ImGui_ImplDX12_Init(p_device, m_buffer_count, DXGI_FORMAT_R8G8B8A8_UNORM, p_pd3dSrvDescHeap,
													 p_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
													 p_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
		return false;

	theme::Apply();

	web_image::set_texture_create_callback(
			[this](unsigned char* pixels, int width, int height) -> ImTextureID
			{
				// Simplified texture creation for DX12 would go here
				return 0;
			});

	initialized = true;
	return true;
}

void DX12Renderer::cleanup_render_target()
{
	wait_for_last_submitted_frame();

	for (auto res : m_main_render_target_resources)
		if (res)
			res->Release();
	m_main_render_target_resources.clear();

	if (p_pd3dRtvDescHeap)
	{
		p_pd3dRtvDescHeap->Release();
		p_pd3dRtvDescHeap = nullptr;
	}
}

void DX12Renderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX12Renderer::end_frame(IDXGISwapChain3* swap_chain)
{
	if (!initialized)
		return;

	ImGui::Render();

	FrameContext* frameCtx = wait_for_next_frame_resources();
	m_frame_index = swap_chain->GetCurrentBackBufferIndex();

	frameCtx->CommandAllocator->Reset();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_main_render_target_resources[m_frame_index];
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	p_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
	p_pd3dCommandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = p_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_frame_index * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	p_pd3dCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	p_pd3dCommandList->SetDescriptorHeaps(1, &p_pd3dSrvDescHeap);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), p_pd3dCommandList);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	p_pd3dCommandList->ResourceBarrier(1, &barrier);
	p_pd3dCommandList->Close();

	p_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&p_pd3dCommandList);

	UINT64 fenceValue = n_fence_last_signaled_value + 1;
	p_pd3dCommandQueue->Signal(p_fence, fenceValue);
	n_fence_last_signaled_value = fenceValue;
	frameCtx->FenceValue = fenceValue;
}

void DX12Renderer::wait_for_last_submitted_frame()
{
	if (p_fence && h_fence_event)
	{
		if (p_fence->GetCompletedValue() < n_fence_last_signaled_value)
		{
			p_fence->SetEventOnCompletion(n_fence_last_signaled_value, h_fence_event);
			WaitForSingleObject(h_fence_event, INFINITE);
		}
	}
}

FrameContext* DX12Renderer::wait_for_next_frame_resources()
{
	UINT nextFrameIndex = m_frame_index + 1;
	m_frame_index = nextFrameIndex % m_buffer_count;

	FrameContext* frameCtx = &m_frame_contexts[m_frame_index];
	UINT64 fenceValue = frameCtx->FenceValue;
	if (fenceValue != 0 && p_fence->GetCompletedValue() < fenceValue)
	{
		p_fence->SetEventOnCompletion(fenceValue, h_fence_event);
		WaitForSingleObject(h_fence_event, INFINITE);
	}

	return frameCtx;
}

void DX12Renderer::shutdown()
{
	if (!initialized)
		return;

	wait_for_last_submitted_frame();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_render_target();

	for (auto& ctx : m_frame_contexts)
		if (ctx.CommandAllocator)
			ctx.CommandAllocator->Release();
	m_frame_contexts.clear();

	if (p_pd3dCommandList)
	{
		p_pd3dCommandList->Release();
		p_pd3dCommandList = nullptr;
	}
	if (p_pd3dSrvDescHeap)
	{
		p_pd3dSrvDescHeap->Release();
		p_pd3dSrvDescHeap = nullptr;
	}
	if (p_fence)
	{
		p_fence->Release();
		p_fence = nullptr;
	}
	if (h_fence_event)
	{
		CloseHandle(h_fence_event);
		h_fence_event = nullptr;
	}
	if (p_device)
	{
		p_device->Release();
		p_device = nullptr;
	}

	initialized = false;
}

} // namespace renderer
