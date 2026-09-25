#include "internal.h"

namespace nc
{

using namespace detail;

namespace
{
struct InputState
{
	bool hovered = false;
	bool active = false;

	void capture()
	{
		hovered = ImGui::IsItemHovered();
		active = ImGui::IsItemActive();
	}
};

// Themed frame around a stock ImGui input. `body` submits the input widget and
// calls state.capture() right after it. The frame is drawn first using last
// frame's animation state (a one-frame lag is imperceptible and avoids
// splitting the draw list).
template <typename Fn>
bool framed_input(const char* label, float right_reserve, Fn&& body)
{
	const Theme& t = theme();
	ImGuiID id = ImGui::GetID("##in");
	float w = stacked_label(label);
	float h = t.control_height + 2.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();

	float& hov = anim::value(anim::key(id, "hov"));
	float& foc = anim::value(anim::key(id, "foc"));
	field_frame(ImGui::GetWindowDrawList(), pos, ImVec2(pos.x + w, pos.y + h), hov, foc);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, (h - ImGui::GetFontSize()) * 0.5f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, styled(t.text_dim));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, with_alpha(t.accent_col, 0.35f));
	ImGui::SetNextItemWidth(w - right_reserve);
	InputState state;
	bool result = body(state);
	ImGui::PopStyleColor(6);
	ImGui::PopStyleVar();

	hov = anim::approach(hov, state.hovered ? 1.0f : 0.0f);
	foc = anim::approach(foc, state.active ? 1.0f : 0.0f);
	return result;
}

int string_resize_cb(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto* str = static_cast<std::string*>(data->UserData);
		str->resize((size_t)data->BufTextLen);
		data->Buf = str->data();
	}
	return 0;
}

// Small "-" / "+" stepper buttons on the right of numeric inputs.
int stepper(float h)
{
	const Theme& t = theme();
	int dir = 0;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	for (int i = 0; i < 2; ++i)
	{
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::PushID(i);
		ImVec2 p = ImGui::GetCursorScreenPos();
		if (ImGui::InvisibleButton("##step", ImVec2(h, h)))
			dir = i == 0 ? -1 : 1;
		float hov = anim::animate(ImGui::GetID("hov"), ImGui::IsItemHovered());
		ImGui::PopID();
		ImVec2 c(p.x + h * 0.5f, p.y + h * 0.5f);
		ImU32 col = styled(lerp_color(t.text_dim, t.accent_col, hov));
		dl->AddLine(ImVec2(c.x - 3.5f, c.y), ImVec2(c.x + 3.5f, c.y), col, 1.5f);
		if (i == 1)
			dl->AddLine(ImVec2(c.x, c.y - 3.5f), ImVec2(c.x, c.y + 3.5f), col, 1.5f);
	}
	return dir;
}
} // namespace

bool input_text(const char* label, char* buf, size_t buf_size, const char* hint, ImGuiInputTextFlags flags)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	bool changed = framed_input(label, 0.0f,
															[&](InputState& st)
															{
																bool c = ImGui::InputTextWithHint("##in", hint ? hint : "", buf, buf_size, flags);
																st.capture();
																return c;
															});
	ImGui::PopID();
	return changed;
}

bool input_text(const char* label, std::string* str, const char* hint, ImGuiInputTextFlags flags)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	flags |= ImGuiInputTextFlags_CallbackResize;
	bool changed = framed_input(label, 0.0f,
															[&](InputState& st)
															{
																bool c = ImGui::InputTextWithHint("##in", hint ? hint : "", str->data(), str->capacity() + 1,
																																	flags, string_resize_cb, str);
																st.capture();
																return c;
															});
	ImGui::PopID();
	return changed;
}

bool input_int(const char* label, int* v, int step, int min, int max)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	const float h = theme().control_height + 2.0f;
	bool changed = framed_input(label, h * 2.0f,
															[&](InputState& st)
															{
																bool c = ImGui::InputScalar("##in", ImGuiDataType_S32, v, nullptr, nullptr, "%d");
																st.capture();
																int dir = stepper(h);
																if (dir)
																{
																	*v += dir * step;
																	c = true;
																}
																return c;
															});
	if (changed)
		*v = clamp(*v, min, max);
	ImGui::PopID();
	return changed;
}

bool input_float(const char* label, float* v, float step, float min, float max, const char* fmt)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	const float h = theme().control_height + 2.0f;
	bool changed = framed_input(label, h * 2.0f,
															[&](InputState& st)
															{
																bool c = ImGui::InputScalar("##in", ImGuiDataType_Float, v, nullptr, nullptr, fmt);
																st.capture();
																int dir = stepper(h);
																if (dir)
																{
																	*v += dir * step;
																	c = true;
																}
																return c;
															});
	if (changed)
		*v = clamp(*v, min, max);
	ImGui::PopID();
	return changed;
}

} // namespace nc
