#include "search.h"

namespace nc::search
{

namespace
{
struct State
{
	char buffer[256] = {};
	std::string last_raw;
	std::string query;
	std::vector<std::string> tokens;
	std::vector<std::string> pending_tags;
	std::vector<bool> scopes;
	int forced = 0;
	int suspended = 0;
	bool last_result = true;
	bool dry = false;
	int hits = 0;
	int visible = 0;
};

State& state()
{
	static State s;
	return s;
}

std::string normalize(std::string_view text)
{
	std::string out;
	out.reserve(text.size());
	bool pending_space = false;
	for (char raw : text)
	{
		unsigned char ch = (unsigned char)raw;
		if (std::isspace(ch))
		{
			pending_space = !out.empty();
			continue;
		}
		if (pending_space)
		{
			out.push_back(' ');
			pending_space = false;
		}
		out.push_back((char)std::tolower(ch));
	}
	return out;
}

bool contains_all_tokens(const std::string& haystack)
{
	for (const auto& token : state().tokens)
		if (haystack.find(token) == std::string::npos)
			return false;
	return true;
}
} // namespace

char* buffer()
{
	return state().buffer;
}

size_t buffer_size()
{
	return sizeof(state().buffer);
}

void set_query(std::string_view text)
{
	State& s = state();
	size_t n = std::min(text.size(), sizeof(s.buffer) - 1);
	memcpy(s.buffer, text.data(), n);
	s.buffer[n] = '\0';
	sync();
}

bool sync()
{
	State& s = state();
	if (s.last_raw == s.buffer)
		return false;
	s.last_raw = s.buffer;
	s.query = normalize(s.buffer);
	s.tokens.clear();
	size_t start = 0;
	while (start < s.query.size())
	{
		size_t end = s.query.find(' ', start);
		if (end == std::string::npos)
			end = s.query.size();
		if (end > start)
			s.tokens.push_back(s.query.substr(start, end - start));
		start = end + 1;
	}
	return true;
}

const std::string& query()
{
	return state().query;
}

bool active()
{
	return !state().tokens.empty();
}

bool matches(std::string_view text)
{
	if (!active())
		return true;
	return contains_all_tokens(normalize(text));
}

bool matches(std::initializer_list<const char*> terms)
{
	if (!active())
		return true;
	std::string combined;
	for (const char* term : terms)
	{
		if (!term)
			continue;
		combined += term;
		combined += ' ';
	}
	return contains_all_tokens(normalize(combined));
}

void tags(std::initializer_list<const char*> keywords)
{
	auto& pending = state().pending_tags;
	for (const char* k : keywords)
		if (k)
			pending.emplace_back(k);
}

bool filter(const char* label)
{
	State& s = state();
	if (!active() || s.suspended > 0)
	{
		s.pending_tags.clear();
		s.last_result = true;
		s.visible++;
		return !s.dry;
	}

	const bool visible_label = has_visible_label(label);
	bool matched;
	if (!visible_label && s.pending_tags.empty())
	{
		// Unlabeled widget: follows the label drawn right before it. Not counted twice.
		matched = s.last_result;
	}
	else
	{
		std::string text;
		if (visible_label)
			text.assign(label, label_end(label));
		for (const auto& tag : s.pending_tags)
		{
			text += ' ';
			text += tag;
		}
		matched = s.forced > 0 || contains_all_tokens(normalize(text));
		if (matched)
			s.hits++;
	}
	s.pending_tags.clear();
	s.last_result = matched;
	if (matched)
		s.visible++;
	return matched && !s.dry;
}

void push_scope(const char* title)
{
	State& s = state();
	bool force = active() && title && matches(std::string_view(title, label_end(title) - title));
	s.scopes.push_back(force);
	if (force)
		s.forced++;
}

void pop_scope()
{
	State& s = state();
	IM_ASSERT(!s.scopes.empty() && "search::pop_scope() without push_scope()");
	if (s.scopes.empty())
		return;
	if (s.scopes.back())
		s.forced--;
	s.scopes.pop_back();
}

void push_suspend()
{
	state().suspended++;
}

void pop_suspend()
{
	State& s = state();
	IM_ASSERT(s.suspended > 0 && "search::pop_suspend() without push_suspend()");
	if (s.suspended > 0)
		s.suspended--;
}

bool dry_run()
{
	return state().dry;
}

void begin_dry_run()
{
	State& s = state();
	s.dry = true;
	s.hits = 0;
	s.last_result = true;
}

int end_dry_run()
{
	State& s = state();
	s.dry = false;
	s.last_result = true;
	int h = s.hits;
	s.hits = 0;
	return h;
}

int visible_count()
{
	return state().visible;
}

} // namespace nc::search
