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

#ifndef NOTEBOOK_DATA_H
#define NOTEBOOK_DATA_H

#include "../notebook_glom.h"
#include "box_data_list.h"
#include "box_data_details.h"

class Notebook_Data : public Notebook_Glom
{
public: 
  Notebook_Data();
  virtual ~Notebook_Data();

  //Create the layout for the database structure, and fill it with data.
  virtual bool init_db_details(const Glib::ustring& strTableName, const Glib::ustring& strWhereClause = Glib::ustring());

  ///Show the details for a particular record, without affecting the list view:
  virtual void show_details(const Gnome::Gda::Value& primary_key_value);

  virtual void select_page_for_find_results(); //Details for 1, List for > 1.

  virtual void do_menu_developer_layout(); //override
  virtual void do_menu_file_print(); //override

  enum dataview
  {
    DATA_VIEW_Details,
    DATA_VIEW_List
  };

  virtual dataview get_current_view() const;

  typedef sigc::signal<void, const Glib::ustring&, Gnome::Gda::Value> type_signal_record_details_requested;
  type_signal_record_details_requested signal_record_details_requested();

protected:

  //Signal handlers:
  virtual void on_list_user_requested_details(const Gnome::Gda::Value& primary_key_value);
  void on_Details_user_requested_related_details(const Glib::ustring& strTableName, Gnome::Gda::Value primary_key_value);

  virtual void on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber);

  //Member widgets:
  Box_Data_List m_Box_List;
  Box_Data_Details m_Box_Details;

  guint m_iPage_Details, m_iPage_List;
  Glib::ustring m_table_name;
  type_signal_record_details_requested m_signal_record_details_requested;
};

#endif
