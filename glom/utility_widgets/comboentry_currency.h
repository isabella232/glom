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

#ifndef GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_HH
#define GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_HH

#include "../data_structure/iso_codes.h"
#include <gtkmm/comboboxentry.h>
#include <libglademm.h>

#include <gtkmm/liststore.h>


class ComboEntry_Currency : public Gtk::ComboBoxEntry
{
public:
  ComboEntry_Currency(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~ComboEntry_Currency();

protected:

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

private:
  IsoCodes::Currency m_dummy; //Force the linker to use this. //TODO: Fix this properly.
};

#endif //GLOM_MODE_DESIGN_COMBOENTRY_CURRENCY_HH

