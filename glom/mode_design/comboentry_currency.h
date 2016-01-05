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

#ifndef GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_H
#define GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_H

#include <gtkmm/combobox.h>
#include <gtkmm/builder.h>

#include <gtkmm/liststore.h>


namespace Glom
{

/// A ComboBox that allows the user to choose, or enter, a currency symbol.
class ComboEntry_Currency : public Gtk::ComboBox
{
public:
  ComboEntry_Currency(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

private:

  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_symbol); add(m_name); }

    Gtk::TreeModelColumn<Glib::ustring> m_symbol;
    Gtk::TreeModelColumn<Glib::ustring> m_name;
  };

  ModelColumns m_model_columns;
  Glib::RefPtr<Gtk::ListStore> m_model;
};

} //namespace Glom

#endif //GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_H
