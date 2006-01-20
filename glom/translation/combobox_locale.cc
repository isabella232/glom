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

#include "combobox_locale.h"
#include <gtk/gtkcomboboxentry.h>
#include "../data_structure/iso_codes.h"

ComboBox_Locale::ComboBox_Locale(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& /* refGlade */)
: Gtk::ComboBox(cobject)
{
  m_model = Gtk::ListStore::create(m_model_columns);

  //Fill the model:
  const IsoCodes::type_list_locales list_locales = IsoCodes::get_list_of_locales();
  for(IsoCodes::type_list_locales::const_iterator iter = list_locales.begin(); iter != list_locales.end(); ++iter)
  {
    Gtk::TreeModel::iterator tree_iter = m_model->append();
    Gtk::TreeModel::Row row = *tree_iter;

    const IsoCodes::Locale& the_locale = *iter;
    row[m_model_columns.m_identifier] = the_locale.m_identifier;
    row[m_model_columns.m_name] = the_locale.m_name;
  }

  set_model(m_model);
  pack_start(m_model_columns.m_identifier); //TODO: Hide this.

  //Show this too.
  pack_start(m_model_columns.m_name);
}


ComboBox_Locale::~ComboBox_Locale()
{

}




