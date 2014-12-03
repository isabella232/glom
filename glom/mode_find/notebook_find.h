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

#ifndef GLOM_NOTEBOOK_FIND_H
#define GLOM_NOTEBOOK_FIND_H

#include "../mode_data/notebook_data.h"
#include "box_data_list_find.h"
#include "box_data_details_find.h"

namespace Glom
{

class Notebook_Find : public Notebook_Glom
{
public: 
  Notebook_Find();
  virtual ~Notebook_Find();

  bool init_db_details(const Glib::ustring& table_name, const Glib::ustring& layout_platform);

  void set_current_view(Notebook_Data::dataview view);

  /** Emitted when the user has entered a find critera that
   * should be used to find and display records.
   * @param find_criteria The SQL where clause.
   */
  sigc::signal<void, Gnome::Gda::SqlExpr> signal_find_criteria;

private:

  static const Glib::ustring m_pagename_details, m_pagename_list;

  //Member widgets:
  Box_Data_List_Find m_Box_List;
  
  Box_Data_Details_Find m_Box_Details;
};

} //namespace Glom

#endif // GLOM_NOTEBOOK_FIND_H
