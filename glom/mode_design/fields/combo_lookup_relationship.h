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

#ifndef GLOM_MODE_DESIGN_COMBO_LOOKUP_RELATIONSHIP_HH
#define GLOM_MODE_DESIGN_COMBO_LOOKUP_RELATIONSHIP_HH

#include <gtkmm/combobox.h>
#include <libglademm.h>

#include <gtkmm/liststore.h>

class Combo_LookupRelationship : public Gtk::ComboBox
{
public:
  Combo_LookupRelationship(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Combo_LookupRelationship();

  void append_text(const Glib::ustring& name, const Glib::ustring& from_field);
  Glib::ustring get_active_text() const;

  void clear_text();
  void set_active_text(const Glib::ustring& name);

protected:

  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class TextModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    TextModelColumns()
    { add(m_col_name); add(m_col_from_field); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    Gtk::TreeModelColumn<Glib::ustring> m_col_from_field; //Extra information to help the user to choose.
  };

  TextModelColumns m_text_columns;
  Glib::RefPtr<Gtk::ListStore> m_model;

};

#endif //GLOM_MODE_DESIGN_COMBO_LOOKUP_RELATIONSHIP_HH

