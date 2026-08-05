/* === S Y N F I G ========================================================= */
/*!	\file docks/dock_params.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_DOCK_PARAMS_H
#define __SYNFIG_STUDIO_DOCK_PARAMS_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/actiongroup.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gui/docks/dock_canvasspecific.h>

#include <synfig/type.h>

#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class LayerParamTreeStore;

class Dock_Params : public Dock_CanvasSpecific
{
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	Glib::RefPtr<Gtk::Adjustment> vadjustment;
	sigc::connection refresh_selected_param_connection;
	sigc::connection canvas_changed_connection;

	//! Filter bar shown above the parameters tree
	Gtk::Box filter_box_;
	Gtk::Entry filter_name_entry_;
	Gtk::Entry filter_value_entry_;
	Gtk::ComboBoxText filter_type_combo_;
	Gtk::CheckButton filter_animated_check_;
	Gtk::Button filter_clear_button_;

	//! Value types listed in filter_type_combo_, index-aligned with combo rows (row 0 = nullptr = any type)
	std::vector<const synfig::Type*> filter_types_;

	//! Connection to the current store's signal_changed(), used to refresh the type list
	sigc::connection filter_store_connection_;

	//! Connection to filter_type_combo_'s signal_changed(), blocked while repopulating
	sigc::connection filter_type_combo_changed_connection_;

	//! One-shot timer that applies the filter once the type popup closes
	sigc::connection popup_watch_connection_;

protected:
	virtual void init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);
	virtual void changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view);

	void refresh_tree_view();

	void refresh_selected_param();

private:
	void create_filter_bar();
	void on_param_filter_changed();
	//! Polls while the type popup is open; applies the filter once it closes
	bool on_combo_popup_watch();
	//! Push the current filter widget state into the store
	void apply_param_filter();
	void on_filter_clear_clicked();
	void refresh_filter_type_combo();
	LayerParamTreeStore* get_param_tree_store(etl::loose_handle<CanvasView> canvas_view) const;

public:
	Dock_Params();
	~Dock_Params();
}; // END of Dock_Params

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
