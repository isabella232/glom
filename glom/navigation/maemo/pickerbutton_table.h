/* Glom
 *
 * Copyright (C) 2009 Murray Cumming
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

#ifndef GLOM_PICKERBUTTON_TABLE_H
#define GLOM_PICKERBUTTON_TABLE_H

#include <hildonmm/picker-button.h>
#include <glom/base_db.h>

namespace Glom
{

/** This widget offers a list of tables in the database,
  * allowing the user to select a table.
  */
class PickerButton_Table
: public Hildon::PickerButton,
  public Base_DB
{
public:
  PickerButton_Table();
  virtual ~PickerButton_Table();

  void set_table_name(const Glib::ustring& table_name);
  Glib::ustring get_table_name() const;

  virtual bool fill_from_database(); //override

private:
  virtual void load_from_document(); //override.

  //Signal handlers:
 
  guint m_colTableName;
  guint m_colTitle;

  Hildon::TouchSelector m_touchselector;

  //Tree model columns:
  //These columns are used by the model that is created by the default constructor
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    { add(m_title); add(m_name); }

    Gtk::TreeModelColumn<Glib::ustring> m_title;
    Gtk::TreeModelColumn<Glib::ustring> m_name;
  };

  ModelColumns m_model_columns;
  Glib::RefPtr<Gtk::ListStore> m_list_store;
};

} //namespace Glom

#endif //GLOM_PICKERBUTTON_TABLE_H

