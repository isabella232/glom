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

#include "notebook_data.h"
#include <glibmm/i18n.h>

Notebook_Data::Notebook_Data()
{
  //Add Pages:
  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_List, _("List")));
  m_iPage_List = 0;

  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_Details, _("Details")));
  m_iPage_Details = 1;


  //Connect signals:

  //Allow List to ask Details to show a record.
  m_Box_List.signal_user_requested_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_list_user_requested_details));

  //Allow Details's Related Records to ask Details to show a record.
  //m_Box_Details.signal_user_requested_related_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_Details_user_requested_related_details));

    
  //Allow Details to ask List to ask Details to show a different record:
  m_Box_Details.signal_nav_first().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_first));
  m_Box_Details.signal_nav_prev().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_previous));
  m_Box_Details.signal_nav_next().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_next));
  m_Box_Details.signal_nav_last().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_last));

  //Allow Details to tell List about record deletion:
  m_Box_Details.signal_record_deleted().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_Details_record_deleted));

  //Fill composite view:
  add_view(&m_Box_List);
  add_view(&m_Box_Details);

  show_all_children();
}

Notebook_Data::~Notebook_Data()
{
  remove_view(&m_Box_List);
  remove_view(&m_Box_Details);
}

void Notebook_Data::init_db_details(const Glib::ustring& strTableName, const Glib::ustring& strWhereClause)
{
  //strWhereClause is only used as a result of a find.

  //Performance optimisation:
  //Keep the connection open during all these operations:
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server();

    m_Box_List.init_db_details(strTableName, strWhereClause);
    m_Box_Details.init_db_details(strTableName, m_Box_List.get_primary_key_value_selected());
  }

  //Select List as default:
  set_current_page(m_iPage_List);
}

void Notebook_Data::on_list_user_requested_details(Gnome::Gda::Value primary_key_value)
{
  m_Box_Details.refresh_data_from_database(primary_key_value);
  set_current_page(m_iPage_Details);
}

void Notebook_Data::on_Details_user_requested_related_details(Glib::ustring strTableName, Gnome::Gda::Value primary_key_value)
{
  //Show a different table:
  init_db_details(strTableName);

  //Show the specific record:
  on_list_user_requested_details(primary_key_value);
}

void Notebook_Data::select_page_for_find_results()
{
  if(m_Box_List.get_records_count() > 1)
  {
    set_current_page(m_iPage_List);
  }
  else
  {
    set_current_page(m_iPage_Details);
  }
}

void Notebook_Data::do_menu_developer_layout()
{
  int iPageCurrent = get_current_page();

  Gtk::Widget* pChild  = get_nth_page(iPageCurrent);
  if(pChild)
  {
    Box_Data* pBox = dynamic_cast<Box_Data*>(pChild);
    if(pBox)
      pBox->show_layout_dialog();
  } 
}

void Notebook_Data::do_menu_file_print()
{
  int iPageCurrent = get_current_page();

  Gtk::Widget* pChild  = get_nth_page(iPageCurrent);
  if(pChild)
  {
    Box_Data* pBox = dynamic_cast<Box_Data*>(pChild);
    if(pBox)
      pBox->print_layout();
  } 
}

enum dataview
{
  DATA_VIEW_Details,
  DATA_VIEW_List
};

Notebook_Data::dataview Notebook_Data::get_current_view() const
{
  const int current_page = get_current_page();

  dataview result = DATA_VIEW_Details;
  if(current_page == 1)
    result = DATA_VIEW_List;

  return result;
}

