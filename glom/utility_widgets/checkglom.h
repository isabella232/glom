/* Glom
 *
 * Copyright (C) 2008 Johannes Schmid
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

#ifndef GLOM_UTILITY_WIDGETS_CHECK_GLOM_H
#define GLOM_UTILITY_WIDGETS_CHECK_GLOM_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <gtkmm.h>
#include <glom/libglom/data_structure/field.h>
#include "layoutwidgetfield.h"
#include <libglademm.h>

namespace Glom
{

class App_Glom;

class CheckGlom
: public Gtk::CheckButton,
  public LayoutWidgetField
{
public:
  explicit CheckGlom(Glib::ustring title);
  virtual ~CheckGlom();

  virtual void set_value(const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_value() const;    

protected:
  void init();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual bool on_button_press_event(GdkEventButton *event); //override
#endif // !GLOM_ENABLE_CLIENT_ONLY

  virtual App_Glom* get_application();
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_CHECK_GLOM_H

