#include "ui_retained.h"
#include "ui_framework.h"
#include "../ext/imgui/imgui.h"

namespace ui
{

// Panel implementation
Panel::Panel(const std::string& title) : title_(title)
{
}

void Panel::AddControl(std::shared_ptr<Control> control)
{
	controls_.push_back(control);
}

void Panel::draw()
{
	if (is_search_pass_)
		return;

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 cur = ImGui::GetCursorScreenPos();
	float aw = ImGui::GetContentRegionAvail().x;

	// Dynamic height estimation based on control count
	float ph = 32.0f + static_cast<float>(controls_.size()) * 30.0f + 20.0f;
	if (ph < 100.0f)
		ph = 100.0f; // Minimal height

	float th = 28.0f; // header height matching panel.cpp

	draw_panel(dl, cur, ImVec2(cur.x + aw, cur.y + ph), title_.c_str());

	ImGui::SetCursorScreenPos(ImVec2(cur.x, cur.y + th));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);

	std::string child_id = "##panel_child_" + title_;
	if (ImGui::BeginChild(child_id.c_str(), ImVec2(aw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding,
												ImGuiWindowFlags_None))
	{
		ImGui::PushItemWidth(-1.0f);
		for (auto& control : controls_)
		{
			control->draw();
			ImGui::Spacing();
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(2);

	// Advance cursor for next element
	ImGui::SetCursorScreenPos(ImVec2(cur.x, cur.y + ph + 8.0f));
}

// Label implementation
Label::Label(const std::string& text) : text_(text)
{
}
void Label::draw()
{
	ctrl_label(text_.c_str());
}

// Checkbox implementation
Checkbox::Checkbox(const std::string& label, bool default_val) : label_(label), value_(default_val)
{
}
void Checkbox::draw()
{
	flat_checkbox(label_.c_str(), &value_);
}

// Slider implementation
Slider::Slider(const std::string& label, float min, float max, float default_val)
		: label_(label), min_(min), max_(max), value_(default_val)
{
}
void Slider::draw()
{
	flat_slider(label_.c_str(), &value_, min_, max_);
}

// IntSlider implementation
IntSlider::IntSlider(const std::string& label, int min, int max, int default_val)
		: label_(label), min_(min), max_(max), value_(default_val)
{
}
void IntSlider::draw()
{
	flat_slider(label_.c_str(), &value_, min_, max_);
}

// RangeSlider implementation
RangeSlider::RangeSlider(const std::string& label, float min, float max, float default_lo, float default_hi)
		: label_(label), min_(min), max_(max), low_(default_lo), high_(default_hi)
{
}
void RangeSlider::draw()
{
	range_slider(label_.c_str(), &low_, &high_, min_, max_);
}

// ColorPicker implementation
ColorPicker::ColorPicker(const std::string& label) : label_(label), h_(0.0f), s_(1.0f), v_(1.0f)
{
}
void ColorPicker::draw()
{
	flat_color_picker(label_.c_str(), &h_, &s_, &v_);
}

// Dropdown implementation
Dropdown::Dropdown(const std::string& label, const std::vector<std::string>& items, int default_sel)
		: label_(label), items_(items), selected_(default_sel)
{
	for (const auto& item : items_)
		item_ptrs_.push_back(item.c_str());
}
void Dropdown::draw()
{
	flat_dropdown_single(label_.c_str(), &selected_, item_ptrs_.data(), (int)item_ptrs_.size());
}

// MultiDropdown implementation
MultiDropdown::MultiDropdown(const std::string& label, const std::vector<std::string>& items)
		: label_(label), items_(items), count_(items.size())
{
	selected_ = std::make_unique<bool[]>(count_);
	for (size_t i = 0; i < count_; ++i)
		selected_[i] = false;

	for (const auto& item : items_)
		item_ptrs_.push_back(item.c_str());
}
void MultiDropdown::draw()
{
	flat_dropdown_multi(label_.c_str(), selected_.get(), item_ptrs_.data(), (int)item_ptrs_.size());
}
bool MultiDropdown::get_selected(int index) const
{
	return (index >= 0 && index < (int)count_) ? selected_[index] : false;
}
void MultiDropdown::set_selected(int index, bool val)
{
	if (index >= 0 && index < (int)count_)
		selected_[index] = val;
}

// Listbox implementation
Listbox::Listbox(const std::string& label, const std::vector<std::string>& items, float height, int default_sel)
		: label_(label), items_(items), selected_(default_sel), height_(height)
{
	for (const auto& item : items_)
		item_ptrs_.push_back(item.c_str());
}
void Listbox::draw()
{
	flat_listbox(label_.c_str(), &selected_, item_ptrs_.data(), (int)item_ptrs_.size(), height_);
}

// Hotkey implementation
Hotkey::Hotkey(const std::string& label) : label_(label), key_(0), mode_(0)
{
}
void Hotkey::draw()
{
	hotkey_inline(label_.c_str(), &key_, &mode_);
}

// AnimGraph implementation
AnimGraph::AnimGraph(const std::string& label, float width, float height, float speed)
		: label_(label), width_(width), height_(height), speed_(speed)
{
}
void AnimGraph::draw()
{
	anim_graph(label_.c_str(), width_, height_, speed_);
}

} // namespace ui
