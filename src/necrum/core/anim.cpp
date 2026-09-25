#include "anim.h"

#include <unordered_map>

namespace nc::anim
{

namespace
{
struct Entry
{
	float value = 0.0f;
	int last_frame = 0;
};

struct State
{
	std::unordered_map<ImGuiID, Entry> values;
	float speed = 20.0f;
	float dt = 1.0f / 60.0f;
	int frame = 0;
};

State& state()
{
	static State s;
	return s;
}

constexpr int k_gc_interval = 300;	// frames between sweeps
constexpr int k_gc_max_age = 600;		// entries idle for this many frames are dropped
} // namespace

float speed()
{
	return state().speed;
}

void set_speed(float s)
{
	state().speed = std::max(0.1f, s);
}

float dt()
{
	return state().dt;
}

float approach(float cur, float target, float spd)
{
	if (spd < 0.0f)
		spd = state().speed;
	float t = 1.0f - std::exp(-spd * state().dt);
	float next = cur + (target - cur) * t;
	if (std::fabs(next - target) < 0.0005f)
		next = target;
	return next;
}

float& value(ImGuiID id, float init)
{
	State& s = state();
	auto [it, inserted] = s.values.try_emplace(id);
	if (inserted)
		it->second.value = init;
	it->second.last_frame = s.frame;
	return it->second.value;
}

float animate(ImGuiID id, float target, float spd)
{
	float& v = value(id, target);
	v = approach(v, target, spd);
	return v;
}

ImGuiID key(ImGuiID base, const char* salt)
{
	return ImHashStr(salt, 0, base);
}

float smoothstep(float t)
{
	t = saturate(t);
	return t * t * (3.0f - 2.0f * t);
}

float smoothstep(float edge0, float edge1, float x)
{
	return smoothstep((x - edge0) / (edge1 - edge0));
}

float ease_out_cubic(float t)
{
	float inv = 1.0f - saturate(t);
	return 1.0f - inv * inv * inv;
}

float ease_in_out_cubic(float t)
{
	t = saturate(t);
	return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f;
}

float ease_out_back(float t)
{
	constexpr float c1 = 1.70158f;
	constexpr float c3 = c1 + 1.0f;
	t = saturate(t) - 1.0f;
	return 1.0f + c3 * t * t * t + c1 * t * t;
}

void new_frame(float delta_time)
{
	State& s = state();
	s.dt = clamp(delta_time, 1.0f / 1000.0f, 0.1f);
	s.frame++;

	if (s.frame % k_gc_interval != 0)
		return;
	for (auto it = s.values.begin(); it != s.values.end();)
	{
		if (s.frame - it->second.last_frame > k_gc_max_age)
			it = s.values.erase(it);
		else
			++it;
	}
}

} // namespace nc::anim
