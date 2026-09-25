// Headless smoke test: runs an nc::App for a few thousand frames without any
// GPU/window backend, feeding pseudo-random mouse and keyboard input.
// Any ImGui assertion (unbalanced Begin/End, Push/Pop, ID misuse...) aborts
// the process, so a zero exit code means the UI survived the fuzzing.
//
// Build: xmake build necrum_headless   (links whichever example app is selected)
// Run:   necrum_headless [frames] [seed]

#include "necrum/necrum.h"

#include <cstdlib>
#include <filesystem>
#include <random>

namespace
{

void pump_textures()
{
	for (ImTextureData* tex : ImGui::GetPlatformIO().Textures)
	{
		if (tex->Status == ImTextureStatus_WantCreate)
		{
			tex->SetTexID((ImTextureID)1);
			tex->SetStatus(ImTextureStatus_OK);
		}
		else if (tex->Status == ImTextureStatus_WantUpdates)
			tex->SetStatus(ImTextureStatus_OK);
		else if (tex->Status == ImTextureStatus_WantDestroy)
		{
			tex->SetTexID(ImTextureID_Invalid);
			tex->SetStatus(ImTextureStatus_Destroyed);
		}
	}
}

struct Fuzzer
{
	std::mt19937 rng;
	ImVec2 display;
	ImVec2 mouse{400.0f, 300.0f};

	float rand01() { return std::uniform_real_distribution<float>(0.0f, 1.0f)(rng); }
	int rand_int(int lo, int hi) { return std::uniform_int_distribution<int>(lo, hi)(rng); }

	void feed(ImGuiIO& io, int frame)
	{
		// Mostly wander around the centre, where the main window lives.
		if (rand01() < 0.6f)
		{
			ImVec2 center(display.x * 0.5f, display.y * 0.5f);
			mouse = ImVec2(center.x + (rand01() - 0.5f) * 720.0f, center.y + (rand01() - 0.5f) * 500.0f);
		}
		else
			mouse = ImVec2(rand01() * display.x, rand01() * display.y);
		io.AddMousePosEvent(mouse.x, mouse.y);

		int action = rand_int(0, 99);
		if (action < 30)
		{
			io.AddMouseButtonEvent(0, true);
			io.AddMouseButtonEvent(0, false);
		}
		else if (action < 36)
		{
			io.AddMouseButtonEvent(1, true);
			io.AddMouseButtonEvent(1, false);
		}
		else if (action < 42)
			io.AddMouseButtonEvent(0, rand01() < 0.5f); // drags
		else if (action < 46)
			io.AddMouseWheelEvent(0.0f, rand01() < 0.5f ? -1.0f : 1.0f);
		else if (action < 50)
		{
			static const ImGuiKey keys[] = {ImGuiKey_Escape, ImGuiKey_F,	 ImGuiKey_G,				 ImGuiKey_Enter,
																			ImGuiKey_A,			 ImGuiKey_Tab, ImGuiKey_LeftArrow, ImGuiKey_RightArrow};
			ImGuiKey k = keys[rand_int(0, IM_ARRAYSIZE(keys) - 1)];
			io.AddKeyEvent(k, true);
			io.AddKeyEvent(k, false);
		}
		else if (action < 52)
			io.AddInputCharactersUTF8(rand01() < 0.5f ? "a" : "7");

		// Periodically exercise search and menu toggling.
		if (frame % 400 == 100)
			nc::search::set_query(frame % 800 == 100 ? "slider" : "color");
		if (frame % 400 == 250)
			nc::search::set_query("zzzz no match");
		if (frame % 400 == 300)
			nc::search::set_query("");
		if (frame % 1000 == 999)
		{
			io.AddKeyEvent(ImGuiKey_Insert, true);
			io.AddKeyEvent(ImGuiKey_Insert, false);
		}
	}
};

bool config_roundtrip()
{
	auto& cfg = nc::config();
	const std::string path = (std::filesystem::temp_directory_path() / "necrum_headless.cfg").string();
	std::string before = cfg.serialize();
	if (!cfg.save(path))
		return false;
	cfg.reset();
	if (!cfg.load(path))
		return false;
	std::filesystem::remove(path);
	return cfg.serialize() == before;
}

} // namespace

int main(int argc, char** argv)
{
	const int frames = argc > 1 ? std::atoi(argv[1]) : 3000;
	const unsigned seed = argc > 2 ? (unsigned)std::atoi(argv[2]) : 1234u;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(1600.0f, 900.0f);
	io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
	nc::host::setup(io);

	Fuzzer fuzz{std::mt19937(seed), io.DisplaySize};
	for (int frame = 0; frame < frames; ++frame)
	{
		io.DeltaTime = 1.0f / 60.0f;
		fuzz.feed(io, frame);
		ImGui::NewFrame();
		nc::host::frame();
		ImGui::Render();
		pump_textures();
	}

	bool cfg_ok = config_roundtrip();
	nc::host::shutdown();
	ImGui::DestroyContext();

	printf("necrum headless: %d frames, seed %u, config roundtrip %s\n", frames, seed, cfg_ok ? "ok" : "FAILED");
	return cfg_ok ? 0 : 1;
}
