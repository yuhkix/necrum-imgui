#include "web_image.h"
#include "render/renderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <windows.h>
#include <wininet.h>
#include <d3d11.h>

#pragma comment(lib, "wininet.lib")

namespace web_image
{
WebImageCache::WebImageCache()
{
	placeholder_ = create_gray_placeholder();
	worker_thread_ = std::thread(&WebImageCache::download_thread_func, this);
}

WebImageCache::~WebImageCache()
{
	{
		std::lock_guard<std::mutex> lock(cache_mutex_);
		shutdown_ = true;
	}
	cv_.notify_all();
	if (worker_thread_.joinable())
		worker_thread_.join();

	clear_cache();
}

ImTextureID WebImageCache::get_web_image(const std::string& url)
{
	if (url.empty())
		return placeholder_;

	std::lock_guard<std::mutex> lock(cache_mutex_);

	auto it = cache_.find(url);
	if (it != cache_.end())
	{
		// Check if we have pixel data waiting to be turned into a texture
		if (it->second.pixels && !it->second.loaded && !it->second.failed)
		{
			it->second.texture = create_texture(it->second.pixels, it->second.width, it->second.height);
			stbi_image_free(it->second.pixels);
			it->second.pixels = nullptr;

			if (it->second.texture)
				it->second.loaded = true;
			else
				it->second.failed = true;
		}

		if (it->second.loaded)
			return it->second.texture;
		if (it->second.failed)
			return placeholder_;

		return placeholder_;
	}

	// Not in cache, start downloading
	CacheEntry entry;
	entry.url = url;
	entry.downloading = true;
	cache_[url] = entry;

	pending_requests_.push(url);
	cv_.notify_one();

	return placeholder_;
}

bool WebImageCache::is_image_loaded(const std::string& url)
{
	std::lock_guard<std::mutex> lock(cache_mutex_);
	auto it = cache_.find(url);
	if (it != cache_.end())
		return it->second.loaded;
	return false;
}

ImTextureID WebImageCache::get_placeholder() const
{
	return placeholder_;
}

void WebImageCache::clear_cache()
{
	std::lock_guard<std::mutex> lock(cache_mutex_);
	for (auto& pair : cache_)
	{
		if (pair.second.texture && pair.second.texture != placeholder_)
		{
			auto srv = (ID3D11ShaderResourceView*)pair.second.texture;
			srv->Release();
		}
		if (pair.second.pixels)
		{
			stbi_image_free(pair.second.pixels);
		}
	}
	cache_.clear();
}

void WebImageCache::download_thread_func()
{
	while (true)
	{
		std::string url;
		{
			std::unique_lock<std::mutex> lock(cache_mutex_);
			cv_.wait(lock, [this] { return !pending_requests_.empty() || shutdown_; });

			if (shutdown_)
				break;

			url = pending_requests_.front();
			pending_requests_.pop();
		}

		std::vector<unsigned char> data;
		if (download_image(url, data))
		{
			int w, h, comp;
			unsigned char* pixels = stbi_load_from_memory(data.data(), (int)data.size(), &w, &h, &comp, 4);

			std::lock_guard<std::mutex> lock(cache_mutex_);
			auto it = cache_.find(url);
			if (it != cache_.end())
			{
				if (pixels)
				{
					it->second.pixels = pixels;
					it->second.width = w;
					it->second.height = h;
				}
				else
				{
					it->second.failed = true;
				}
				it->second.downloading = false;
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(cache_mutex_);
			auto it = cache_.find(url);
			if (it != cache_.end())
			{
				it->second.failed = true;
				it->second.downloading = false;
			}
		}
	}
}

bool WebImageCache::download_image(const std::string& url, std::vector<unsigned char>& out_data)
{
	HINTERNET hInternet = InternetOpenA("RubyAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet)
		return false;

	HINTERNET hUrl = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hUrl)
	{
		InternetCloseHandle(hInternet);
		return false;
	}

	unsigned char buffer[4096];
	DWORD bytesRead = 0;
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
	{
		out_data.insert(out_data.end(), buffer, buffer + bytesRead);
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	return !out_data.empty();
}

ImTextureID WebImageCache::create_texture(unsigned char* pixels, int width, int height)
{
	auto device = renderer::Renderer::get_device();
	if (!device)
		return 0;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* texture = nullptr;
	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem = pixels;
	subResource.SysMemPitch = width * 4;

	if (FAILED(device->CreateTexture2D(&desc, &subResource, &texture)))
		return 0;

	ID3D11ShaderResourceView* srv = nullptr;
	if (FAILED(device->CreateShaderResourceView(texture, nullptr, &srv)))
	{
		texture->Release();
		return 0;
	}

	texture->Release();
	return (ImTextureID)srv;
}

ImTextureID WebImageCache::create_gray_placeholder()
{
	unsigned char pixels[4] = {40, 40, 45, 255};
	return create_texture(pixels, 1, 1);
}

void shutdown()
{
	WebImageCache::get_instance().clear_cache();
}
} // namespace web_image
