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

Notebook_Data::Notebook_Data()
{
  //Add Pages:
  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_List, gettext("List")));
  m_iPage_List = 0;

  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_Details, gettext("Details")));
  m_iPage_Details = 1;


  //Connect signals:

  //Allow List to ask Details to show a record.
  m_Box_List.signal_user_requested_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_Details_user_requested_details));

  //Allow Details's Related Records to ask Details to show a record.
  m_Box_Details.signal_user_requested_related_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_Details_user_requested_related_details));

    
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


  //Build actions:
  m_actiongroup_special_menus = Gtk::ActionGroup::create("Glom_Mode_Data_Actions");

  m_actiongroup_special_menus->add( Gtk::Action::create("GlomAction_Menu_Developers_Layout_Data", gettext("_Design Layout")),
    sigc::mem_fun(*this, &Notebook_Data::on_menu_Developer_Layout)  );
  
  //Build part of the menu structure, to be merged in by using the "Bakery_MenuPH_Others" placeholder:
  m_special_menus_ui_string =
    "<ui>"
    "  <menubar name='Bakery_MainMenu'>"
    "      <menu action='Glom_Menu_Developer'>"
    "         <placeholder name='Glom_Menu_Developer_PH'>"
    "           <menuitem action='GlomAction_Menu_Developers_Layout_Data' />"
    "        </placeholder>"
    "      </menu>"
    "  </menubar>"
    "</ui>";
    

  show_all_children();
}

Notebook_Data::~Notebook_Data()
{
}

void Notebook_Data::initialize(const Glib::ustring& strDatabaseName, const Glib::ustring& strTableName, const Glib::ustring& strWhereClause)
{
  //strWhereClause is only used as a result of a find.

  m_Box_List.initialize(strDatabaseName, strTableName, strWhereClause);
  m_Box_Details.initialize(strDatabaseName, strTableName, m_Box_List.get_primary_key_value_selected());

  //Select List as default:
  set_current_page(m_iPage_List);
}

void Notebook_Data::on_Details_user_requested_details(Glib::ustring strPrimaryKeyValue)
{
  m_Box_Details.initialize(strPrimaryKeyValue); //Uses existing connection, database, and table.
  set_current_page(m_iPage_Details);
}

void Notebook_Data::on_Details_user_requested_related_details(Glib::ustring strTableName, Glib::ustring strPrimaryKeyValue)
{
  //Show a different table:
  initialize(m_Box_Details.get_databaseName(), strTableName);

  //Show the specific record:
  on_Details_user_requested_details(strPrimaryKeyValue);
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

void Notebook_Data::on_menu_Developer_Layout()
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

