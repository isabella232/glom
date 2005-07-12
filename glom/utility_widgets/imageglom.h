/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
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

#ifndef GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H
#define GLOM_UTILITY_WIDGETS_IMAGE_GLOM_H

#include <gtkmm.h>
#include "../data_structure/field.h"
#include "layoutwidgetfield.h"

class App_Glom;

class ImageGlom
: public Gtk::EventBox,
  public LayoutWidgetField
{
public:
  ImageGlom();

  virtual ~ImageGlom();
  
  virtual void set_value(const Gnome::Gda::Value& value);
  virtual Gnome::Gda::Value get_value() const;

protected:

  virtual bool on_button_press_event(GdkEventButton *event);

  virtual App_Glom* get_application();
  static Glib::RefPtr<Gdk::Pixbuf> scale_keeping_ratio(const Glib::RefPtr<Gdk::Pixbuf> pixbu, int target_height, int target_width);
  
  Gtk::Image m_image;
};

#endif //GLOM_UTILITY_WIDGETS_COMBOENTRY_GLOM_H

