#pragma once

#include "base.h"

// Built-in feature search.
//
// Every framework widget calls search::filter(label) before drawing. While a
// query is active, widgets whose label (or extra tags) do not contain every
// query token are skipped, so pages become searchable with zero extra code.
//
// The app shell additionally runs a "dry run" of every page when the query
// changes: widgets only count matches without drawing, which lets the shell
// highlight tabs/pages with hits and jump to the best one.
namespace nc::search
{

// Query buffer (bound to the search field).
char* buffer();
size_t buffer_size();

void set_query(std::string_view text);
const std::string& query(); // normalised: lower case, single spaces
bool active();							 // non-empty query

// Must be called after editing buffer() directly. Returns true if the query changed.
bool sync();

// True if every token of the query appears in any of the given strings.
bool matches(std::string_view text);
bool matches(std::initializer_list<const char*> terms);

// Extra keywords for the next widget: nc::search::tags({"aim", "fov"}); nc::slider(...)
void tags(std::initializer_list<const char*> keywords);

// Widget visibility check (used by all widgets). Counts a hit when matching.
// Labels without a visible part ("##id") inherit the result of the previous
// filter() call, so `nc::label("Foo"); nc::combo("##foo", ...)` works as expected.
bool filter(const char* label);

// Everything filtered inside a matching scope is visible (e.g. a panel whose
// title matches shows all of its widgets).
void push_scope(const char* title);
void pop_scope();

// Temporarily disable filtering (popups, dialogs and other chrome that must
// stay usable while a query is active).
void push_suspend();
void pop_suspend();

// Dry run control (used by the shell). Widgets must not draw during a dry run.
bool dry_run();
void begin_dry_run();
int end_dry_run(); // returns hits since begin_dry_run()

// Number of visible widgets since the last reset (used by panels for "no matches" hints).
int visible_count();

} // namespace nc::search
