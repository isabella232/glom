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

#ifndef GLOM_TRANSLATION_COMBOBOX_LOCALE_H
#define GLOM_TRANSLATION_COMBOBOX_LOCALE_H

#include <glom/mode_design/iso_codes.h>
#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>

#include <gtkmm/liststore.h>


namespace Glom
{

/// A ComboBox that allows the user to choose a locale.
class ComboBox_Locale : public Gtk::ComboBox
{
public:
  ComboBox_Locale(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  void set_selected_locale(const Glib::ustring& locale);
  Glib::ustring get_selected_locale() const;

private:

  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_identifier); add(m_name); }

    Gtk::TreeModelColumn<Glib::ustring> m_identifier;
    Gtk::TreeModelColumn<Glib::ustring> m_name;
  };

  ModelColumns m_model_columns;
  Glib::RefPtr<Gtk::ListStore> m_model;
};

} //namespace Glom

#endif //GLOM_TRANSLATION_COMBOBOX_LOCALE_H
