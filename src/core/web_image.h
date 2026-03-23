#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <condition_variable>
#include <vector>
#include <imgui.h>

namespace web_image
{
class WebImageCache
{
public:
	static WebImageCache& get_instance()
	{
		static WebImageCache instance;
		return instance;
	}

	ImTextureID get_web_image(const std::string& url);
	bool is_image_loaded(const std::string& url);

	void clear_cache();

	ImTextureID get_placeholder() const;

private:
	WebImageCache();
	~WebImageCache();

	struct CacheEntry
	{
		ImTextureID texture;
		bool loaded;
		bool failed;
		bool downloading;
		std::string url;

		// Decoded data (temporary)
		unsigned char* pixels;
		int width, height;

		CacheEntry() : texture(0), loaded(false), failed(false), downloading(false), pixels(nullptr), width(0), height(0) {}
	};

	std::unordered_map<std::string, CacheEntry> cache_;
	std::mutex cache_mutex_;
	ImTextureID placeholder_;

	void download_thread_func();
	bool download_image(const std::string& url, std::vector<unsigned char>& out_data);

	std::queue<std::string> pending_requests_;
	std::thread worker_thread_;
	std::condition_variable cv_;
	bool shutdown_ = false;

	ImTextureID create_texture(unsigned char* pixels, int width, int height);
	ImTextureID create_gray_placeholder();
};

inline ImTextureID get_web_image(const std::string& url)
{
	return WebImageCache::get_instance().get_web_image(url);
}

inline bool is_web_image_loaded(const std::string& url)
{
	return WebImageCache::get_instance().is_image_loaded(url);
}

void shutdown();
} // namespace web_image
