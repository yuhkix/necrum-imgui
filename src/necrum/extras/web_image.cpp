#include "web_image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wininet.h>
#ifdef _MSC_VER
#pragma comment(lib, "wininet.lib")
#endif
#endif

namespace nc::web_image
{

namespace
{
struct Entry
{
	ImTextureID texture = 0;
	unsigned char* pixels = nullptr; // decoded, waiting for texture creation on the render thread
	int width = 0;
	int height = 0;
	bool loaded = false;
	bool failed = false;
};

// Intentionally leaked: the worker thread must never observe a destroyed state
// during static destruction (e.g. when a DLL is unloaded).
struct State
{
	std::mutex mutex;
	std::condition_variable cv;
	std::unordered_map<std::string, Entry> cache;
	std::queue<std::string> pending;
	std::thread worker;
	bool running = false;
	bool stop = false;
	CreateTexture create;
	ReleaseTexture release;
	ImTextureID placeholder = 0;
	bool placeholder_tried = false;
};

State& state()
{
	static State* s = new State();
	return *s;
}

bool download(const std::string& url, std::vector<unsigned char>& out)
{
#ifdef _WIN32
	HINTERNET inet = InternetOpenA("necrum", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (!inet)
		return false;
	HINTERNET handle = InternetOpenUrlA(inet, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
	if (!handle)
	{
		InternetCloseHandle(inet);
		return false;
	}
	unsigned char buffer[4096];
	DWORD read = 0;
	while (InternetReadFile(handle, buffer, sizeof(buffer), &read) && read > 0)
		out.insert(out.end(), buffer, buffer + read);
	InternetCloseHandle(handle);
	InternetCloseHandle(inet);
	return !out.empty();
#else
	(void)url;
	(void)out;
	return false;
#endif
}

void worker_main()
{
	State& s = state();
	while (true)
	{
		std::string url;
		{
			std::unique_lock<std::mutex> lock(s.mutex);
			s.cv.wait(lock, [&] { return s.stop || !s.pending.empty(); });
			if (s.stop)
				return;
			url = s.pending.front();
			s.pending.pop();
		}

		std::vector<unsigned char> data;
		unsigned char* pixels = nullptr;
		int w = 0, h = 0, comp = 0;
		if (download(url, data))
			pixels = stbi_load_from_memory(data.data(), (int)data.size(), &w, &h, &comp, 4);

		std::lock_guard<std::mutex> lock(s.mutex);
		auto it = s.cache.find(url);
		if (it == s.cache.end())
		{
			if (pixels)
				stbi_image_free(pixels);
			continue;
		}
		if (pixels)
		{
			it->second.pixels = pixels;
			it->second.width = w;
			it->second.height = h;
		}
		else
			it->second.failed = true;
	}
}

void ensure_worker(State& s)
{
	if (s.running)
		return;
	s.stop = false;
	s.worker = std::thread(worker_main);
	s.running = true;
}

ImTextureID placeholder(State& s)
{
	if (!s.placeholder_tried && s.create)
	{
		const unsigned char gray[4] = {40, 40, 45, 255};
		s.placeholder = s.create(gray, 1, 1);
		s.placeholder_tried = true;
	}
	return s.placeholder;
}
} // namespace

void set_texture_callbacks(CreateTexture create, ReleaseTexture release)
{
	State& s = state();
	std::lock_guard<std::mutex> lock(s.mutex);
	s.create = std::move(create);
	s.release = std::move(release);
	s.placeholder = 0;
	s.placeholder_tried = false;
}

ImTextureID get(const std::string& url)
{
	State& s = state();
	if (url.empty())
		return placeholder(s);

	std::lock_guard<std::mutex> lock(s.mutex);
	auto it = s.cache.find(url);
	if (it == s.cache.end())
	{
		s.cache.emplace(url, Entry{});
		ensure_worker(s);
		s.pending.push(url);
		s.cv.notify_one();
		return placeholder(s);
	}

	Entry& e = it->second;
	if (e.pixels && !e.loaded && !e.failed)
	{
		e.texture = s.create ? s.create(e.pixels, e.width, e.height) : 0;
		stbi_image_free(e.pixels);
		e.pixels = nullptr;
		if (e.texture)
			e.loaded = true;
		else if (s.create)
			e.failed = true;
	}
	return e.loaded ? e.texture : placeholder(s);
}

bool is_loaded(const std::string& url)
{
	State& s = state();
	std::lock_guard<std::mutex> lock(s.mutex);
	auto it = s.cache.find(url);
	return it != s.cache.end() && it->second.loaded;
}

bool has_failed(const std::string& url)
{
	State& s = state();
	std::lock_guard<std::mutex> lock(s.mutex);
	auto it = s.cache.find(url);
	return it != s.cache.end() && it->second.failed;
}

void clear()
{
	State& s = state();
	std::lock_guard<std::mutex> lock(s.mutex);
	for (auto& [url, e] : s.cache)
	{
		if (e.texture && s.release)
			s.release(e.texture);
		if (e.pixels)
			stbi_image_free(e.pixels);
	}
	s.cache.clear();
	if (s.placeholder && s.release)
		s.release(s.placeholder);
	s.placeholder = 0;
	s.placeholder_tried = false;
}

void shutdown()
{
	State& s = state();
	{
		std::lock_guard<std::mutex> lock(s.mutex);
		s.stop = true;
	}
	s.cv.notify_all();
	if (s.worker.joinable())
		s.worker.join();
	s.running = false;
	clear();
}

} // namespace nc::web_image
