/* Glom
 *
 * Copyright (C) 2007 Johannes Schmid <johannes.schmid@openismus.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
 
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <string>

#include "layoutwidgetbase.h"

#ifndef DRAGBUTTON_H
#define DRAGBUTTON_H

namespace Glom
{
  
class DragButton : public Gtk::Button
{
  public:
    DragButton(Gtk::Image& image, LayoutWidgetBase::enumType type);  
    ~DragButton();

  
    static gchar* get_target() {return "flowtable";};
  protected:
		virtual void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& drag_context);
    virtual void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, 
                                  Gtk::SelectionData& selection_data, guint, guint);
  
  private:
    LayoutWidgetBase::enumType m_type;
};

}
#endif // DRAGBUTTON_H
