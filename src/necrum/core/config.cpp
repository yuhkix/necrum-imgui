#include "config.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace nc
{

namespace
{
std::string_view trim(std::string_view s)
{
	while (!s.empty() && std::isspace((unsigned char)s.front()))
		s.remove_prefix(1);
	while (!s.empty() && std::isspace((unsigned char)s.back()))
		s.remove_suffix(1);
	return s;
}

bool parse_float(std::string_view s, float* out)
{
	std::string tmp(trim(s));
	if (tmp.empty())
		return false;
	char* end = nullptr;
	float v = std::strtof(tmp.c_str(), &end);
	if (end == tmp.c_str())
		return false;
	*out = v;
	return true;
}

bool parse_int(std::string_view s, int* out)
{
	std::string tmp(trim(s));
	if (tmp.empty())
		return false;
	char* end = nullptr;
	long v = std::strtol(tmp.c_str(), &end, 10);
	if (end == tmp.c_str())
		return false;
	*out = (int)v;
	return true;
}

std::vector<std::string_view> split(std::string_view s, char sep)
{
	std::vector<std::string_view> out;
	size_t start = 0;
	while (true)
	{
		size_t pos = s.find(sep, start);
		out.push_back(s.substr(start, pos == std::string_view::npos ? std::string_view::npos : pos - start));
		if (pos == std::string_view::npos)
			break;
		start = pos + 1;
	}
	return out;
}

std::string format_float(float v)
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%g", v);
	return buf;
}

std::string escape(const std::string& s)
{
	std::string out;
	for (char c : s)
	{
		if (c == '\\')
			out += "\\\\";
		else if (c == '\n')
			out += "\\n";
		else
			out += c;
	}
	return out;
}

std::string unescape(std::string_view s)
{
	std::string out;
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] == '\\' && i + 1 < s.size())
		{
			++i;
			out += (s[i] == 'n') ? '\n' : s[i];
		}
		else
			out += s[i];
	}
	return out;
}

ImGuiKey key_from_name(std::string_view name)
{
	if (name.empty() || name == "None")
		return ImGuiKey_None;
	for (int k = ImGuiKey_NamedKey_BEGIN; k < ImGuiKey_NamedKey_END; ++k)
	{
		const char* n = ImGui::GetKeyName((ImGuiKey)k);
		if (n && name == n)
			return (ImGuiKey)k;
	}
	return ImGuiKey_None;
}

const char* mode_token(KeybindMode m)
{
	switch (m)
	{
	case KeybindMode::Toggle:
		return "toggle";
	case KeybindMode::Hold:
		return "hold";
	case KeybindMode::Always:
		return "always";
	case KeybindMode::Off:
		return "off";
	}
	return "toggle";
}
} // namespace

void ConfigStore::add(const std::string& key, std::function<std::string()> get,
											std::function<bool(std::string_view)> set)
{
	unbind(key);
	Entry e{key, std::move(get), std::move(set), {}};
	e.default_value = e.get();
	entries_.push_back(std::move(e));
}

void ConfigStore::bind(const std::string& key, bool* v)
{
	add(
			key, [v] { return std::string(*v ? "true" : "false"); },
			[v](std::string_view s)
			{
				s = trim(s);
				if (s == "true" || s == "1")
					*v = true;
				else if (s == "false" || s == "0")
					*v = false;
				else
					return false;
				return true;
			});
}

void ConfigStore::bind(const std::string& key, int* v)
{
	add(key, [v] { return std::to_string(*v); }, [v](std::string_view s) { return parse_int(s, v); });
}

void ConfigStore::bind(const std::string& key, float* v)
{
	add(key, [v] { return format_float(*v); }, [v](std::string_view s) { return parse_float(s, v); });
}

void ConfigStore::bind(const std::string& key, HSV* v)
{
	add(
			key, [v] { return format_float(v->h) + "," + format_float(v->s) + "," + format_float(v->v) + "," + format_float(v->a); },
			[v](std::string_view s)
			{
				auto parts = split(s, ',');
				if (parts.size() < 3)
					return false;
				HSV out = *v;
				if (!parse_float(parts[0], &out.h) || !parse_float(parts[1], &out.s) || !parse_float(parts[2], &out.v))
					return false;
				if (parts.size() > 3)
					parse_float(parts[3], &out.a);
				*v = out;
				return true;
			});
}

void ConfigStore::bind(const std::string& key, Keybind* v)
{
	add(
			key,
			[v]
			{
				const char* name = v->key == ImGuiKey_None ? "None" : ImGui::GetKeyName(v->key);
				return std::string(name) + "," + mode_token(v->mode);
			},
			[v](std::string_view s)
			{
				auto parts = split(s, ',');
				if (parts.empty())
					return false;
				v->key = key_from_name(trim(parts[0]));
				if (parts.size() > 1)
				{
					std::string_view m = trim(parts[1]);
					v->mode = m == "hold"		 ? KeybindMode::Hold
										: m == "always" ? KeybindMode::Always
										: m == "off"		? KeybindMode::Off
																		: KeybindMode::Toggle;
				}
				v->toggled = false;
				return true;
			});
}

void ConfigStore::bind(const std::string& key, std::string* v)
{
	add(
			key, [v] { return escape(*v); },
			[v](std::string_view s)
			{
				*v = unescape(s);
				return true;
			});
}

void ConfigStore::bind(const std::string& key, bool* flags, int count)
{
	add(
			key,
			[flags, count]
			{
				std::string out(count, '0');
				for (int i = 0; i < count; ++i)
					out[i] = flags[i] ? '1' : '0';
				return out;
			},
			[flags, count](std::string_view s)
			{
				s = trim(s);
				for (int i = 0; i < count; ++i)
					flags[i] = i < (int)s.size() && s[i] == '1';
				return true;
			});
}

void ConfigStore::bind(const std::string& key, std::function<std::string()> save,
											 std::function<bool(std::string_view)> load)
{
	add(key, std::move(save), std::move(load));
}

void ConfigStore::unbind(const std::string& key)
{
	entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [&](const Entry& e) { return e.key == key; }),
								 entries_.end());
}

bool ConfigStore::contains(const std::string& key) const
{
	return std::any_of(entries_.begin(), entries_.end(), [&](const Entry& e) { return e.key == key; });
}

std::string ConfigStore::serialize() const
{
	std::string out = "# necrum config\n";
	for (const auto& e : entries_)
	{
		out += e.key;
		out += " = ";
		out += e.get();
		out += '\n';
	}
	return out;
}

int ConfigStore::deserialize(std::string_view text)
{
	int applied = 0;
	for (std::string_view line : split(text, '\n'))
	{
		line = trim(line);
		if (line.empty() || line.front() == '#' || line.front() == ';')
			continue;
		size_t eq = line.find('=');
		if (eq == std::string_view::npos)
			continue;
		std::string_view key = trim(line.substr(0, eq));
		std::string_view value = trim(line.substr(eq + 1));
		for (auto& e : entries_)
		{
			if (e.key == key)
			{
				if (e.set(value))
					applied++;
				break;
			}
		}
	}
	return applied;
}

bool ConfigStore::save(const std::string& path) const
{
	std::ofstream f(path, std::ios::binary | std::ios::trunc);
	if (!f)
		return false;
	f << serialize();
	return (bool)f;
}

bool ConfigStore::load(const std::string& path)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
		return false;
	std::stringstream ss;
	ss << f.rdbuf();
	deserialize(ss.str());
	return true;
}

void ConfigStore::reset()
{
	for (auto& e : entries_)
		e.set(e.default_value);
}

ConfigStore& config()
{
	static ConfigStore store;
	return store;
}

namespace config_files
{

std::vector<std::string> list(const std::string& dir, const std::string& ext)
{
	std::vector<std::string> out;
	std::error_code ec;
	if (!std::filesystem::is_directory(dir, ec))
		return out;
	for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
	{
		if (!entry.is_regular_file(ec))
			continue;
		const auto& p = entry.path();
		if (p.extension().string() == ext)
			out.push_back(p.stem().string());
	}
	std::sort(out.begin(), out.end());
	return out;
}

std::string path(const std::string& dir, const std::string& name, const std::string& ext)
{
	return (std::filesystem::path(dir) / (name + ext)).string();
}

bool remove(const std::string& p)
{
	std::error_code ec;
	return std::filesystem::remove(p, ec);
}

bool ensure_dir(const std::string& dir)
{
	std::error_code ec;
	if (std::filesystem::is_directory(dir, ec))
		return true;
	return std::filesystem::create_directories(dir, ec);
}

} // namespace config_files

} // namespace nc
