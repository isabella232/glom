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
 
#ifndef HEADER_BOX_DATABASES
#define HEADER_BOX_DATABASES



#include "../box_db.h"
#include "../utility_widgets/table_columns.h"

class Box_DataBases : public Box_DB
{
public:
  Box_DataBases(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Box_DataBases();

  virtual void set_use_list(bool bVal = true); //if false, just open the current database. e.g. when reloading a document.

  virtual void load_from_document(); //override

  virtual Gtk::Widget* get_default_button();

protected:

  sharedptr<SharedConnection> connect_to_server_with_connection_settings() const;

  typedef std::vector<Glib::ustring> type_vecStrings;
  type_vecStrings get_database_names();

  //Signal handlers:
  virtual void on_button_connect();
  virtual void on_adddel_Add(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_Edit(const Gtk::TreeModel::iterator& row);
  virtual void on_adddel_Delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator& rowEnd);
  virtual void on_adddel_Changed(const Gtk::TreeModel::iterator& row, guint number);

  Glib::RefPtr<Gnome::Glade::Xml> m_refGlade;
  
  //Member widgets:

  //Connection:
  Gtk::Entry* m_Entry_Host;
  Gtk::Entry* m_Entry_User;
  Gtk::Entry* m_Entry_Password;
  Gtk::Expander* m_Expander_Tables;
  Gtk::Button* m_button_connect;

  //Member widgets:
  mutable AddDel_WithButtons m_AddDel; //mutable because its get_ methods aren't const.

 
  bool m_bUseList; // see set_use_list().
  guint m_col_name;
};

#endif //HEADER_BOX_DATABASES
