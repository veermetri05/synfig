/* === S Y N F I G ========================================================= */
/*!	\file dock_params.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "docks/dock_params.h"

#include <cassert>

#include <gtkmm/icontheme.h>
#include <glibmm/main.h>

#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/trees/layerparamtreestore.h>
#include <gui/workarea.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Params::Dock_Params():
	Dock_CanvasSpecific("params", _("Parameters"), "parameters_icon"),
	action_group( Gtk::ActionGroup::create("action_group_dock_params") ),
	vadjustment( Gtk::Adjustment::create(0, 0, 1, 1, 1) ),
	filter_box_( Gtk::ORIENTATION_HORIZONTAL, 0 )
{
	set_name("parameters_panel");
	create_filter_bar();
}

Dock_Params::~Dock_Params()
{
	refresh_selected_param_connection.disconnect();
}


void
Dock_Params::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	canvas_view->get_adjustment_group(get_name())->add(vadjustment);
}

void
Dock_Params::refresh_selected_param()
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
	if (!canvas_view) return;

	Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget(get_name()));
	assert(tree_view);

	if (Gtk::TreeModel::iterator iter = tree_view->get_selection()->get_selected()) {
		LayerParamTreeStore::Model model;
		canvas_view->get_work_area()->set_selected_value_node(
			(synfig::ValueNode::Handle)(*iter)[model.value_node] );
	} else {
		canvas_view->get_work_area()->set_selected_value_node(0);
	}
}

void
Dock_Params::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	reset_container();
	refresh_selected_param_connection.disconnect();
	filter_store_connection_.disconnect();

	if (Gtk::Container* filter_parent = dynamic_cast<Gtk::Container*>(filter_box_.get_parent()))
		filter_parent->remove(filter_box_);
	
	if(canvas_view)
	{
		Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(canvas_view->get_ext_widget(get_name()));
		assert(tree_view);

		refresh_selected_param_connection = tree_view->get_selection()->signal_changed().connect(
			sigc::mem_fun(
				*this,
				&Dock_Params::refresh_selected_param ));

		// Connect to the canvas's signal_changed() to detect state changes
		canvas_changed_connection = canvas_view->get_canvas()->signal_changed().connect(
			sigc::mem_fun(
				*this, &Dock_Params::refresh_tree_view));

		tree_view->show();

		// Filter bar pinned above the scrolled parameters tree
		Gtk::ScrolledWindow* scrolled = get_container();
		if (Gtk::Container* parent = dynamic_cast<Gtk::Container*>(scrolled->get_parent()))
			parent->remove(*scrolled);
		attach(*scrolled, 0, 1, 1, 1);
		scrolled->add(*tree_view);

		if (Gtk::Container* filter_parent = dynamic_cast<Gtk::Container*>(filter_box_.get_parent()))
			filter_parent->remove(filter_box_);
		attach(filter_box_, 0, 0, 1, 1);
		filter_box_.show();
		get_container()->set_vadjustment(vadjustment);

		// Apply the persisted filter to this canvas view's store and keep the type list in sync
		if (LayerParamTreeStore* store = get_param_tree_store(canvas_view)) {
			filter_store_connection_ = store->signal_changed().connect(
				sigc::mem_fun(*this, &Dock_Params::refresh_filter_type_combo));
			refresh_filter_type_combo();
			on_param_filter_changed();
		}
	}
}

void
Dock_Params::create_filter_bar()
{
	filter_name_entry_.set_placeholder_text(_("Filter by name"));
	filter_name_entry_.signal_changed().connect(sigc::mem_fun(*this, &Dock_Params::on_param_filter_changed));

	filter_value_entry_.set_placeholder_text(_("Filter by value"));
	filter_value_entry_.signal_changed().connect(sigc::mem_fun(*this, &Dock_Params::on_param_filter_changed));

	filter_type_combo_.append(_("All types"));
	filter_types_.push_back(nullptr);
	filter_type_combo_.set_active(0);
	filter_type_combo_changed_connection_ = filter_type_combo_.signal_changed().connect(sigc::mem_fun(*this, &Dock_Params::on_param_filter_changed));

	filter_animated_check_.set_label(_("Animated"));
	filter_animated_check_.signal_toggled().connect(sigc::mem_fun(*this, &Dock_Params::on_param_filter_changed));

	filter_clear_button_.set_image_from_icon_name("edit-clear", Gtk::IconSize::from_name("synfig-small_icon"));
	filter_clear_button_.set_tooltip_text(_("Clear filter"));
	filter_clear_button_.signal_clicked().connect(sigc::mem_fun(*this, &Dock_Params::on_filter_clear_clicked));

	filter_box_.set_border_width(2);
	filter_box_.pack_start(filter_name_entry_, true, true, 2);
	filter_box_.pack_start(filter_value_entry_, true, true, 2);
	filter_box_.pack_start(filter_type_combo_, false, false, 2);
	filter_box_.pack_start(filter_animated_check_, false, false, 2);
	filter_box_.pack_start(filter_clear_button_, false, false, 2);
	filter_box_.show_all();
}

void
Dock_Params::on_param_filter_changed()
{
	// GTK3 emits changed() for every item hovered while the type popup is
	// open (hover-selection). Applying each transient state would rebuild
	// the tree (and the popup) while hovering - flicker. Defer until the
	// popup closes.
	if (filter_type_combo_.property_popup_shown().get_value())
	{
		if (!popup_watch_connection_.connected())
			popup_watch_connection_ = Glib::signal_timeout().connect(
				sigc::mem_fun(*this, &Dock_Params::on_combo_popup_watch), 25);
		return;
	}
	apply_param_filter();
}

bool
Dock_Params::on_combo_popup_watch()
{
	// Popup still open: keep watching (the timer is stopped when it closes)
	if (filter_type_combo_.property_popup_shown().get_value())
		return true;

	popup_watch_connection_.disconnect();
	apply_param_filter(); // popup closed: apply the final selection
	return false;
}

void
Dock_Params::apply_param_filter()
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
	LayerParamTreeStore* store = get_param_tree_store(canvas_view);
	if (!store)
		return;

	const synfig::Type* type(nullptr);
	const int active = filter_type_combo_.get_active_row_number();
	if (active > 0 && active < (int)filter_types_.size())
		type = filter_types_[active];

	store->set_param_filter(
		filter_name_entry_.get_text(),
		filter_value_entry_.get_text(),
		type,
		filter_animated_check_.get_active());
}

void
Dock_Params::on_filter_clear_clicked()
{
	filter_name_entry_.set_text("");
	filter_value_entry_.set_text("");
	filter_type_combo_.set_active(0);
	filter_animated_check_.set_active(false);
	on_param_filter_changed();
}

void
Dock_Params::refresh_filter_type_combo()
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
	LayerParamTreeStore* store = get_param_tree_store(canvas_view);
	if (!store)
		return;

	std::vector<std::pair<synfig::String, const synfig::Type*>> types;
	store->get_distinct_types(types);

	// If the type list is unchanged, don't touch the combo: repopulating
	// while the popup is open would rebuild it and flicker.
	if (types.size() + 1 == filter_types_.size()) {
		bool same = true;
		for (size_t i = 1; i < filter_types_.size(); ++i) {
			if (filter_types_[i]->description.local_name != types[i - 1].first) {
				same = false;
				break;
			}
		}
		if (same)
			return;
	}

	// Remember the currently selected type (by identity, not by row)
	const synfig::Type* previous_type(nullptr);
	const int previous_active = filter_type_combo_.get_active_row_number();
	if (previous_active > 0 && previous_active < (int)filter_types_.size())
		previous_type = filter_types_[previous_active];

	// Repopulate without emitting signal_changed: remove_all() drops the
	// active row to -1 mid-way, which would otherwise be applied as
	// "clear the type filter" by on_param_filter_changed().
	filter_type_combo_changed_connection_.block();
	filter_type_combo_.remove_all();
	filter_types_.clear();
	filter_type_combo_.append(_("All types"));
	filter_types_.push_back(nullptr);
	for (const auto& type : types) {
		filter_type_combo_.append(type.first);
		filter_types_.push_back(type.second);
	}

	int new_active(0);
	for (size_t i = 1; i < filter_types_.size(); ++i) {
		if (filter_types_[i] == previous_type) {
			new_active = (int)i;
			break;
		}
	}
	filter_type_combo_.set_active(new_active);
	filter_type_combo_changed_connection_.unblock();

	// Apply the (restored) selection once
	on_param_filter_changed();
}

LayerParamTreeStore*
Dock_Params::get_param_tree_store(CanvasView::LooseHandle canvas_view) const
{
	if (!canvas_view)
		return nullptr;
	return Glib::RefPtr<LayerParamTreeStore>::cast_dynamic(canvas_view->get_tree_model("params")).get();
}

void
Dock_Params::refresh_tree_view()
{
	Gtk::TreeView* tree_view = dynamic_cast<Gtk::TreeView*>(get_canvas_view()->get_ext_widget(get_name()));
	if (tree_view)
	{
		tree_view->queue_draw();
	}
}