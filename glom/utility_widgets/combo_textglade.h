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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_COMBO_TEXTGLADE_H
#define GLOM_UTILITY_WIDGETS_COMBO_TEXTGLADE_H

#include <gtkmm/comboboxtext.h>
#include <gtkmm/builder.h>

#include <gtkmm/liststore.h>

namespace Glom
{

/** This class just derives from Gtk::ComboBoxText and provides a constructor suitable for libglade's get_widget_derived() template.
 * so we can use its set_first_active() method.
 */
class Combo_TextGlade : public Gtk::ComboBoxText
{
public:
  Combo_TextGlade(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  ///This ensures that something is selected,
  void set_first_active();
};

} //namespace Glom

#endif // GLOM_UTILITY_WIDGETS_COMBO_TEXTGLADE_H
