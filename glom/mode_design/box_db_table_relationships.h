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

#ifndef BOX_DB_TABLE_RELATIONSHIPS_H
#define BOX_DB_TABLE_RELATIONSHIPS_H

#include "../box_db_table.h"

/**
  *@author Murray Cumming
  */

class Box_DB_Table_Relationships : public Box_DB_Table
{
public: 
  Box_DB_Table_Relationships();
  Box_DB_Table_Relationships(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DB_Table_Relationships();

  void init(); //avoid duplication in constructors.
  
  virtual void save_to_document(); //override.

protected:
  virtual void fill_from_database();

  //Signal handlers:
  virtual void on_AddDel_user_activated(guint row, guint col);
  virtual void on_AddDel_user_changed(guint row, guint col);

  typedef std::vector<Glib::ustring> type_vecStrings;
  static type_vecStrings util_vecStrings_from_Fields(const type_vecFields& fields);

  guint m_colName, m_colFromField, m_colToTable, m_colToField;

  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.
  Gtk::Button m_Button_Guess;
};

#endif
