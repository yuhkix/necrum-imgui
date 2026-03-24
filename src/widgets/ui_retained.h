#pragma once
#include "ui_core.h"
#include <memory>
#include <string>
#include <vector>

namespace ui
{

class Control
{
public:
	virtual ~Control() = default;
	virtual void draw() = 0;
};

class Panel : public Control
{
public:
	Panel(const std::string& title);
	void AddControl(std::shared_ptr<Control> control);
	void draw() override;

private:
	std::string title_;
	std::vector<std::shared_ptr<Control>> controls_;
};

class Label : public Control
{
public:
	Label(const std::string& text);
	void draw() override;

private:
	std::string text_;
};

class Checkbox : public Control
{
public:
	Checkbox(const std::string& label, bool default_val = false);
	void draw() override;

	bool get_value() const { return value_; }
	void set_value(bool v) { value_ = v; }

private:
	std::string label_;
	bool value_;
};

using Toggle = Checkbox;

class Slider : public Control
{
public:
	Slider(const std::string& label, float min, float max, float default_val = 0.0f);
	void draw() override;

	float get_value() const { return value_; }
	void set_value(float v) { value_ = v; }

private:
	std::string label_;
	float min_, max_, value_;
};

class IntSlider : public Control
{
public:
	IntSlider(const std::string& label, int min, int max, int default_val = 0);
	void draw() override;

	int get_value() const { return value_; }
	void set_value(int v) { value_ = v; }

private:
	std::string label_;
	int min_, max_, value_;
};

class RangeSlider : public Control
{
public:
	RangeSlider(const std::string& label, float min, float max, float default_lo = 0.0f, float default_hi = 1.0f);
	void draw() override;

	float get_low() const { return low_; }
	float get_high() const { return high_; }

private:
	std::string label_;
	float min_, max_, low_, high_;
};

class ColorPicker : public Control
{
public:
	ColorPicker(const std::string& label);
	void draw() override;

	float* get_hue_ptr() { return &h_; }
	float* get_sat_ptr() { return &s_; }
	float* get_val_ptr() { return &v_; }

private:
	std::string label_;
	float h_, s_, v_;
};

class Dropdown : public Control
{
public:
	Dropdown(const std::string& label, const std::vector<std::string>& items, int default_sel = 0);
	void draw() override;

	int get_selected() const { return selected_; }
	void set_selected(int i) { selected_ = i; }

private:
	std::string label_;
	std::vector<std::string> items_;
	std::vector<const char*> item_ptrs_;
	int selected_;
};

class MultiDropdown : public Control
{
public:
	MultiDropdown(const std::string& label, const std::vector<std::string>& items);
	void draw() override;

	bool get_selected(int index) const;
	void set_selected(int index, bool val);

private:
	std::string label_;
	std::vector<std::string> items_;
	std::vector<const char*> item_ptrs_;
	std::unique_ptr<bool[]> selected_;
	size_t count_;
};

class Listbox : public Control
{
public:
	Listbox(const std::string& label, const std::vector<std::string>& items, float height, int default_sel = 0);
	void draw() override;

	int get_selected() const { return selected_; }
	void set_selected(int i) { selected_ = i; }

private:
	std::string label_;
	std::vector<std::string> items_;
	std::vector<const char*> item_ptrs_;
	int selected_;
	float height_;
};

class Hotkey : public Control
{
public:
	Hotkey(const std::string& label);
	void draw() override;

	int get_key() const { return key_; }
	int get_mode() const { return mode_; }

private:
	std::string label_;
	int key_ = 0;
	int mode_ = 0;
};

class AnimGraph : public Control
{
public:
	AnimGraph(const std::string& label, float width, float height, float speed);
	void draw() override;

private:
	std::string label_;
	float width_, height_, speed_;
};

} // namespace ui
