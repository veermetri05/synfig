/* === S Y N F I G ========================================================= */
/*!	\file event_layerclick.h
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

#ifndef __SYNFIG_EVENT_LAYERCLICK_H
#define __SYNFIG_EVENT_LAYERCLICK_H

/* === H E A D E R S ======================================================= */

#include <gui/event_mouse.h>
#include <synfig/layer.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

struct EventLayerClick : public Smach::event
{
	synfig::Point pos;
	MouseButton button;
	synfig::Layer::LooseHandle layer;
	Gdk::ModifierType modifier;

	EventLayerClick(synfig::Layer::LooseHandle layer, MouseButton button, const synfig::Point& pos, Gdk::ModifierType modifier=Gdk::ModifierType(0)):
		Smach::event(EVENT_WORKAREA_LAYER_CLICKED),
		pos(pos),
		button(button),
		layer(layer),
		modifier(modifier)
	{ }
}; // END of EventLayerClick

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
