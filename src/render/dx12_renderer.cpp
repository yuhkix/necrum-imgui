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
		desc.NumDescriptors = 128; // Increased to support font + multiple web images
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
				if (!this->p_device || m_srv_heap_next_slot >= 128)
					return 0;

				const UINT rowPitch = (width * 4 + 255) & ~255;
				const UINT uploadBufferSize = rowPitch * height;

				// Create texture resource
				D3D12_RESOURCE_DESC texDesc = {};
				texDesc.MipLevels = 1;
				texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				texDesc.Width = width;
				texDesc.Height = height;
				texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				texDesc.DepthOrArraySize = 1;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

				ID3D12Resource* pTexture = nullptr;
				D3D12_HEAP_PROPERTIES defaultHeapProps = {};
				defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
				if (FAILED(p_device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
																										 D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
																										 IID_PPV_ARGS(&pTexture))))
					return 0;

				// Create upload heap
				ID3D12Resource* pUploadHeap = nullptr;
				D3D12_HEAP_PROPERTIES uploadHeapProps = {};
				uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				D3D12_RESOURCE_DESC bufferDesc = {};
				bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				bufferDesc.Alignment = 0;
				bufferDesc.Width = uploadBufferSize;
				bufferDesc.Height = 1;
				bufferDesc.DepthOrArraySize = 1;
				bufferDesc.MipLevels = 1;
				bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
				bufferDesc.SampleDesc.Count = 1;
				bufferDesc.SampleDesc.Quality = 0;
				bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				if (FAILED(p_device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
																										 D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
																										 IID_PPV_ARGS(&pUploadHeap))))
				{
					pTexture->Release();
					return 0;
				}

				// Copy pixels to upload heap with pitch alignment
				void* pMappedData = nullptr;
				if (SUCCEEDED(pUploadHeap->Map(0, nullptr, &pMappedData)))
				{
					for (int y = 0; y < height; ++y)
					{
						memcpy((unsigned char*)pMappedData + y * rowPitch, pixels + y * width * 4, width * 4);
					}
					pUploadHeap->Unmap(0, nullptr);
				}

				// Copy data to texture
				execute_one_shot(
						[&](ID3D12GraphicsCommandList* cmdList)
						{
							D3D12_TEXTURE_COPY_LOCATION src = {};
							src.pResource = pUploadHeap;
							src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
							src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
							src.PlacedFootprint.Footprint.Width = width;
							src.PlacedFootprint.Footprint.Height = height;
							src.PlacedFootprint.Footprint.Depth = 1;
							src.PlacedFootprint.Footprint.RowPitch = rowPitch;

							D3D12_TEXTURE_COPY_LOCATION dst = {};
							dst.pResource = pTexture;
							dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
							dst.SubresourceIndex = 0;

							cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

							D3D12_RESOURCE_BARRIER barrier = {};
							barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
							barrier.Transition.pResource = pTexture;
							barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
							barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
							barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
							cmdList->ResourceBarrier(1, &barrier);
						});

				pUploadHeap->Release();

				// Create SRV
				D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = 1;

				UINT descriptorSize = p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = p_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
				srvHandle.ptr += m_srv_heap_next_slot * descriptorSize;

				p_device->CreateShaderResourceView(pTexture, &srvDesc, srvHandle);

				D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = p_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
				gpuHandle.ptr += m_srv_heap_next_slot * descriptorSize;

				m_srv_heap_next_slot++;
				return (ImTextureID)gpuHandle.ptr;
			});

	initialized = true;
	return true;
}

void DX12Renderer::execute_one_shot(const std::function<void(ID3D12GraphicsCommandList*)>& callback)
{
	ID3D12CommandAllocator* allocator = nullptr;
	if (FAILED(p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
		return;

	ID3D12GraphicsCommandList* cmdList = nullptr;
	if (FAILED(p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&cmdList))))
	{
		allocator->Release();
		return;
	}

	callback(cmdList);

	cmdList->Close();
	p_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);

	// Simple wait for completion
	ID3D12Fence* fence = nullptr;
	p_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	p_pd3dCommandQueue->Signal(fence, 1);
	while (fence->GetCompletedValue() < 1)
		SwitchToThread();

	fence->Release();
	cmdList->Release();
	allocator->Release();
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
