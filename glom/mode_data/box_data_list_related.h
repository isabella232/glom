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

#ifndef BOX_DATA_LIST_RELATED_H
#define BOX_DATA_LIST_RELATED_H

#include "box_data_list.h"

/**
  *@author Murray Cumming
  */

class Box_Data_List_Related : public Box_Data_List
{
public: 
  Box_Data_List_Related();
  virtual ~Box_Data_List_Related();

  virtual void initialize(const Glib::ustring& strDatabaseName, const Relationship& relationship, const Glib::ustring& strForeignKeyValue,  const Glib::ustring& from_table_primary_key_value);

  virtual Glib::ustring get_KeyField() const;

  sigc::signal<void, Glib::ustring> signal_record_added;

protected:
  virtual void fill_from_database(); //Override.

  virtual void on_AddDel_user_added(guint row); //Override.
  virtual void on_record_added(const Glib::ustring& strPrimaryKeyValue); //Override. Not a signal handler.

  virtual void enable_buttons();
  
protected:
  Glib::ustring m_strKeyField, m_strKeyValue;
};

#endif
