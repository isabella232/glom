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

#include "combo_textglade.h"

namespace Glom
{

Combo_TextGlade::Combo_TextGlade(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::ComboBoxText(cobject)
{
  //Check that this was really created from a GtkComboBoxText in the .glade file,
  //instead of just a GtkComboBox, which would not usually have a model.
  g_assert(get_model());

  //Workaround this GtkComboBoxText bug: https://bugzilla.gnome.org/show_bug.cgi?id=612396
  if(get_entry_text_column() < 0)
    set_entry_text_column(0);
}

void Combo_TextGlade::set_first_active()
{
  auto model = get_model();
  if(!model)
    return;

  auto iter = model->children().begin();
  set_active(iter);
}


} //namespace Glom


