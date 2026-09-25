#pragma once

#include "necrum/core/color.h"
#include "necrum/core/input.h"
#include "necrum/layout/layout.h"
#include "necrum/widgets/widgets.h"

#include <memory>

// Retained-mode layer: build a tree of controls once, draw it every frame.
// Each control either owns its value or binds to an external variable:
//
//     auto panel = std::make_shared<nc::retained::Panel>("Aimbot");
//     panel->add<nc::retained::Checkbox>("Enabled", &settings.enabled);   // bound
//     panel->add<nc::retained::Slider>("FOV", 0.0f, 180.0f, 90.0f)        // owned
//          .on_change([] { puts("fov changed"); })
//          .visible_if([&] { return settings.enabled; });
//     ...
//     panel->draw();
namespace nc::retained
{

// Holds a value or points to an external one.
template <typename T>
class Value
{
public:
	Value(T initial = T{}) : own_(std::move(initial)), ptr_(&own_) {}
	Value(T* external) : own_{}, ptr_(external ? external : &own_) {}
	Value(const Value& o) : own_(o.own_), ptr_(o.ptr_ == &o.own_ ? &own_ : o.ptr_) {}
	Value& operator=(const Value& o)
	{
		own_ = o.own_;
		ptr_ = o.ptr_ == &o.own_ ? &own_ : o.ptr_;
		return *this;
	}

	T& get() { return *ptr_; }
	const T& get() const { return *ptr_; }
	T* ptr() { return ptr_; }
	void set(const T& v) { *ptr_ = v; }

private:
	T own_;
	T* ptr_;
};

class Node
{
public:
	virtual ~Node() = default;

	// Draws the node (visibility, search tags and callbacks handled here).
	void draw();

	Node& visible_if(std::function<bool()> fn)
	{
		visible_if_ = std::move(fn);
		return *this;
	}
	Node& enabled_if(std::function<bool()> fn)
	{
		enabled_if_ = std::move(fn);
		return *this;
	}
	Node& on_change(std::function<void()> fn)
	{
		on_change_ = std::move(fn);
		return *this;
	}
	Node& tags(std::initializer_list<const char*> keywords);
	Node& tooltip(std::string text)
	{
		tooltip_ = std::move(text);
		return *this;
	}

protected:
	// Returns true when the value changed.
	virtual bool draw_node() = 0;

private:
	std::function<bool()> visible_if_;
	std::function<bool()> enabled_if_;
	std::function<void()> on_change_;
	std::vector<std::string> tags_;
	std::string tooltip_;
};

using NodePtr = std::shared_ptr<Node>;

class Container : public Node
{
public:
	template <typename T, typename... Args>
	T& add(Args&&... args)
	{
		auto node = std::make_shared<T>(std::forward<Args>(args)...);
		T& ref = *node;
		children_.push_back(std::move(node));
		return ref;
	}
	void add_control(NodePtr node) { children_.push_back(std::move(node)); }
	void clear() { children_.clear(); }
	const std::vector<NodePtr>& children() const { return children_; }

protected:
	bool draw_children();
	std::vector<NodePtr> children_;
};

// ---- Containers -------------------------------------------------------------

class Panel : public Container
{
public:
	explicit Panel(std::string title, ImVec2 size = ImVec2(0, 0), PanelFlags flags = 0)
			: title_(std::move(title)), size_(size), flags_(flags)
	{
	}

protected:
	bool draw_node() override;

private:
	std::string title_;
	ImVec2 size_;
	PanelFlags flags_;
};

// Children are distributed across `count` columns: use column(i).add<...>().
class Columns : public Node
{
public:
	explicit Columns(int count, float gap = 16.0f);
	Container& column(int index);

protected:
	bool draw_node() override;

private:
	struct Column : Container
	{
		bool draw_node() override { return draw_children(); }
	};
	std::vector<std::shared_ptr<Column>> columns_;
	float gap_;
};

// A page root: a plain vertical container (useful as nc::Page draw callback).
class Group : public Container
{
protected:
	bool draw_node() override { return draw_children(); }
};

// ---- Controls ---------------------------------------------------------------

class Label : public Node
{
public:
	explicit Label(std::string text) : text_(std::move(text)) {}

protected:
	bool draw_node() override;

private:
	std::string text_;
};

class Separator : public Node
{
public:
	explicit Separator(std::string caption = {}) : caption_(std::move(caption)) {}

protected:
	bool draw_node() override;

private:
	std::string caption_;
};

class Checkbox : public Node
{
public:
	Checkbox(std::string label, bool initial = false) : label_(std::move(label)), value_(initial) {}
	Checkbox(std::string label, bool* bound) : label_(std::move(label)), value_(bound) {}
	bool value() const { return value_.get(); }
	void set_value(bool v) { value_.set(v); }

protected:
	bool draw_node() override;

	std::string label_;
	Value<bool> value_;
};

class Toggle : public Checkbox
{
public:
	using Checkbox::Checkbox;

protected:
	bool draw_node() override;
};

class Slider : public Node
{
public:
	Slider(std::string label, float min, float max, float initial = 0.0f, std::string fmt = "%.2f", float scale = 1.0f)
			: label_(std::move(label)), min_(min), max_(max), value_(initial), fmt_(std::move(fmt)), scale_(scale)
	{
	}
	Slider(std::string label, float min, float max, float* bound, std::string fmt = "%.2f", float scale = 1.0f)
			: label_(std::move(label)), min_(min), max_(max), value_(bound), fmt_(std::move(fmt)), scale_(scale)
	{
	}
	float value() const { return value_.get(); }
	void set_value(float v) { value_.set(v); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	float min_, max_;
	Value<float> value_;
	std::string fmt_;
	float scale_;
};

class IntSlider : public Node
{
public:
	IntSlider(std::string label, int min, int max, int initial = 0, std::string fmt = "%d")
			: label_(std::move(label)), min_(min), max_(max), value_(initial), fmt_(std::move(fmt))
	{
	}
	IntSlider(std::string label, int min, int max, int* bound, std::string fmt = "%d")
			: label_(std::move(label)), min_(min), max_(max), value_(bound), fmt_(std::move(fmt))
	{
	}
	int value() const { return value_.get(); }
	void set_value(int v) { value_.set(v); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	int min_, max_;
	Value<int> value_;
	std::string fmt_;
};

class RangeSlider : public Node
{
public:
	RangeSlider(std::string label, float min, float max, float lo = 0.0f, float hi = 1.0f, std::string fmt = "%.2f - %.2f")
			: label_(std::move(label)), min_(min), max_(max), lo_(lo), hi_(hi), fmt_(std::move(fmt))
	{
	}
	float low() const { return lo_; }
	float high() const { return hi_; }

protected:
	bool draw_node() override;

private:
	std::string label_;
	float min_, max_, lo_, hi_;
	std::string fmt_;
};

class ColorPicker : public Node
{
public:
	explicit ColorPicker(std::string label, HSV initial = HSV{}) : label_(std::move(label)), value_(initial), reset_(initial) {}
	ColorPicker(std::string label, HSV* bound) : label_(std::move(label)), value_(bound), reset_(*bound) {}
	const HSV& value() const { return value_.get(); }
	ImU32 color() const { return value_.get().u32(); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	Value<HSV> value_;
	HSV reset_;
};

class Combo : public Node
{
public:
	Combo(std::string label, std::vector<std::string> items, int initial = 0)
			: label_(std::move(label)), items_(std::move(items)), value_(initial)
	{
	}
	Combo(std::string label, std::vector<std::string> items, int* bound)
			: label_(std::move(label)), items_(std::move(items)), value_(bound)
	{
	}
	int selected() const { return value_.get(); }
	void set_selected(int i) { value_.set(i); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	std::vector<std::string> items_;
	Value<int> value_;
};
using Dropdown = Combo;

class MultiCombo : public Node
{
public:
	MultiCombo(std::string label, std::vector<std::string> items);
	bool selected(int index) const;
	void set_selected(int index, bool v);

protected:
	bool draw_node() override;

private:
	std::string label_;
	std::vector<std::string> items_;
	std::vector<const char*> ptrs_;
	std::unique_ptr<bool[]> flags_;
};
using MultiDropdown = MultiCombo;

class Listbox : public Node
{
public:
	Listbox(std::string label, std::vector<std::string> items, float height = 120.0f, int initial = 0)
			: label_(std::move(label)), items_(std::move(items)), height_(height), value_(initial)
	{
	}
	int selected() const { return value_.get(); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	std::vector<std::string> items_;
	float height_;
	Value<int> value_;
};

class KeybindControl : public Node
{
public:
	explicit KeybindControl(std::string label, Keybind initial = {}, bool track = true);
	KeybindControl(std::string label, Keybind* bound, bool track = true);
	~KeybindControl() override;
	KeybindControl(const KeybindControl&) = delete;
	KeybindControl& operator=(const KeybindControl&) = delete;
	const Keybind& bind() const { return value_.get(); }
	bool active() const { return value_.get().active(); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	Value<Keybind> value_;
	bool tracked_;
};
using Hotkey = KeybindControl;

class Button : public Node
{
public:
	Button(std::string label, std::function<void()> on_click, ButtonStyle style = ButtonStyle::Default)
			: label_(std::move(label)), on_click_(std::move(on_click)), style_(style)
	{
	}

protected:
	bool draw_node() override;

private:
	std::string label_;
	std::function<void()> on_click_;
	ButtonStyle style_;
};

class TextInput : public Node
{
public:
	TextInput(std::string label, std::string initial = {}, std::string hint = {})
			: label_(std::move(label)), value_(std::move(initial)), hint_(std::move(hint))
	{
	}
	const std::string& value() const { return value_.get(); }

protected:
	bool draw_node() override;

private:
	std::string label_;
	Value<std::string> value_;
	std::string hint_;
};

// Escape hatch: any immediate-mode code as a node. Return true for "changed".
class Custom : public Node
{
public:
	explicit Custom(std::function<bool()> fn) : fn_(std::move(fn)) {}

protected:
	bool draw_node() override { return fn_ ? fn_() : false; }

private:
	std::function<bool()> fn_;
};

} // namespace nc::retained
