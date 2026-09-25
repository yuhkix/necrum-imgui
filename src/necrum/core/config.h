#pragma once

#include "color.h"
#include "input.h"

// Key/value persistence for UI state.
//
// Bind variables once (usually right where they are declared/registered) and
// the store can save, load and reset them. The on-disk format is plain text:
//
//     # necrum config
//     ragebot.enabled = true
//     ragebot.hitchance = 47
//     theme.accent = 265.91,0.69,1,1
//     binds.double_tap = F,toggle
namespace nc
{

class ConfigStore
{
public:
	void bind(const std::string& key, bool* v);
	void bind(const std::string& key, int* v);
	void bind(const std::string& key, float* v);
	void bind(const std::string& key, HSV* v);
	void bind(const std::string& key, Keybind* v);
	void bind(const std::string& key, std::string* v);
	void bind(const std::string& key, bool* flags, int count); // multi-select arrays
	void bind(const std::string& key, std::function<std::string()> save, std::function<bool(std::string_view)> load);

	void unbind(const std::string& key);
	bool contains(const std::string& key) const;
	size_t size() const { return entries_.size(); }

	std::string serialize() const;
	int deserialize(std::string_view text); // returns number of keys applied

	bool save(const std::string& path) const;
	bool load(const std::string& path);

	// Restores every bound value to what it was when bind() was called.
	void reset();

private:
	struct Entry
	{
		std::string key;
		std::function<std::string()> get;
		std::function<bool(std::string_view)> set;
		std::string default_value;
	};
	void add(const std::string& key, std::function<std::string()> get, std::function<bool(std::string_view)> set);
	std::vector<Entry> entries_;
};

// Global store used by the built-in config manager.
ConfigStore& config();

namespace config_files
{
// Lists "<name>" for every "<dir>/<name><ext>" file, sorted.
std::vector<std::string> list(const std::string& dir, const std::string& ext = ".cfg");
std::string path(const std::string& dir, const std::string& name, const std::string& ext = ".cfg");
bool remove(const std::string& path);
bool ensure_dir(const std::string& dir);
} // namespace config_files

} // namespace nc
