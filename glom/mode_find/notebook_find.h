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

#ifndef NOTEBOOK_FIND_H
#define NOTEBOOK_FIND_H

#include "../notebook_glom.h"
#include "box_data_list_find.h"
#include "box_data_details_find.h"

/**
  *@author Murray Cumming
  */

class Notebook_Find : public Notebook_Glom
{
public: 
  Notebook_Find();
  virtual ~Notebook_Find();

  virtual void init_db_details(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName);

  //Signals:
  //where_clause.
  sigc::signal<void, Glib::ustring> signal_find;
  
protected:

  //Signal handlers:
  virtual void on_page_find(Glib::ustring strWhereClause);

  //Member widgets:
  Box_Data_List_Find m_Box_List;
  Box_Data_Details_Find m_Box_Details;

};

#endif
