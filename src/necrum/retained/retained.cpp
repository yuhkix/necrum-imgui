#include "retained.h"

#include "necrum/core/search.h"

namespace nc::retained
{

void Node::draw()
{
	if (visible_if_ && !visible_if_())
		return;
	const bool disabled = enabled_if_ && !enabled_if_();
	if (disabled)
		ImGui::BeginDisabled();

	if (!tags_.empty())
		for (const auto& tag : tags_)
			search::tags({tag.c_str()});

	bool changed = draw_node();
	if (!tooltip_.empty())
		nc::tooltip("%s", tooltip_.c_str());

	if (disabled)
		ImGui::EndDisabled();
	if (changed && on_change_)
		on_change_();
}

Node& Node::tags(std::initializer_list<const char*> keywords)
{
	for (const char* k : keywords)
		if (k)
			tags_.emplace_back(k);
	return *this;
}

bool Container::draw_children()
{
	for (auto& child : children_)
	{
		ImGui::PushID(child.get());
		child->draw();
		ImGui::PopID();
	}
	return false;
}

bool Panel::draw_node()
{
	begin_panel(title_.c_str(), size_, flags_);
	draw_children();
	end_panel();
	return false;
}

Columns::Columns(int count, float gap) : gap_(gap)
{
	for (int i = 0; i < std::max(1, count); ++i)
		columns_.push_back(std::make_shared<Column>());
}

Container& Columns::column(int index)
{
	return *columns_[clamp(index, 0, (int)columns_.size() - 1)];
}

bool Columns::draw_node()
{
	begin_columns((int)columns_.size(), gap_);
	for (size_t i = 0; i < columns_.size(); ++i)
	{
		if (i > 0)
			next_column();
		ImGui::PushID((int)i);
		columns_[i]->draw();
		ImGui::PopID();
	}
	end_columns();
	return false;
}

bool Label::draw_node()
{
	nc::label(text_.c_str());
	return false;
}

bool Separator::draw_node()
{
	nc::separator(caption_.empty() ? nullptr : caption_.c_str());
	return false;
}

bool Checkbox::draw_node()
{
	return nc::checkbox(label_.c_str(), value_.ptr());
}

bool Toggle::draw_node()
{
	return nc::toggle(label_.c_str(), value_.ptr());
}

bool Slider::draw_node()
{
	return nc::slider(label_.c_str(), value_.ptr(), min_, max_, fmt_.c_str(), scale_);
}

bool IntSlider::draw_node()
{
	return nc::slider(label_.c_str(), value_.ptr(), min_, max_, fmt_.c_str());
}

bool RangeSlider::draw_node()
{
	return nc::range_slider(label_.c_str(), &lo_, &hi_, min_, max_, fmt_.c_str());
}

bool ColorPicker::draw_node()
{
	return nc::color_edit(label_.c_str(), value_.ptr(), &reset_);
}

bool Combo::draw_node()
{
	return nc::combo(label_.c_str(), value_.ptr(), items_);
}

MultiCombo::MultiCombo(std::string label, std::vector<std::string> items)
		: label_(std::move(label)), items_(std::move(items)), flags_(std::make_unique<bool[]>(items_.size()))
{
	for (const auto& s : items_)
		ptrs_.push_back(s.c_str());
}

bool MultiCombo::selected(int index) const
{
	return index >= 0 && index < (int)items_.size() && flags_[index];
}

void MultiCombo::set_selected(int index, bool v)
{
	if (index >= 0 && index < (int)items_.size())
		flags_[index] = v;
}

bool MultiCombo::draw_node()
{
	return nc::multi_combo(label_.c_str(), flags_.get(), ptrs_.data(), (int)ptrs_.size());
}

bool Listbox::draw_node()
{
	return nc::listbox(label_.c_str(), value_.ptr(), items_, height_);
}

KeybindControl::KeybindControl(std::string label, Keybind initial, bool track)
		: label_(std::move(label)), value_(initial), tracked_(track)
{
	if (tracked_)
		keybinds::track(value_.ptr(), label_.c_str());
}

KeybindControl::KeybindControl(std::string label, Keybind* bound, bool track)
		: label_(std::move(label)), value_(bound), tracked_(track)
{
	if (tracked_)
		keybinds::track(value_.ptr(), label_.c_str());
}

KeybindControl::~KeybindControl()
{
	if (tracked_)
		keybinds::untrack(value_.ptr());
}

bool KeybindControl::draw_node()
{
	return nc::keybind(label_.c_str(), value_.ptr());
}

bool Button::draw_node()
{
	bool clicked = nc::button(label_.c_str(), ImVec2(-1.0f, 0.0f), style_);
	if (clicked && on_click_)
		on_click_();
	return clicked;
}

bool TextInput::draw_node()
{
	return nc::input_text(label_.c_str(), value_.ptr(), hint_.empty() ? nullptr : hint_.c_str());
}

} // namespace nc::retained
