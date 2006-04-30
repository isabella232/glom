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
#include <glom/libglom/data_structure/glomconversions.h>
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

  //Allow Details to ask List to ask Details to show a different record:
  m_Box_Details.signal_nav_first().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_first));
  m_Box_Details.signal_nav_prev().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_previous));
  m_Box_Details.signal_nav_next().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_next));
  m_Box_Details.signal_nav_last().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_last));

  //Allow Details to tell List about record deletion:
  m_Box_Details.signal_record_deleted().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_record_deleted));

  //Allow Details to ask to show a different record in a different table:
  m_Box_Details.signal_requested_related_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_details_user_requested_related_details));

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

bool Notebook_Data::init_db_details(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value_for_details)
{
  m_table_name = found_set.m_table_name;
  const bool details_record_specified = !GlomConversions::value_is_empty(primary_key_value_for_details);

  bool result = false;
  //where_clause is only used as a result of a find.

  //Performance optimisation:
  //Keep the connection open during all these operations:
  {
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());

    const FoundSet old_found_set = m_Box_List.get_found_set();
    //std::cout << "  old_where_clause=" << old_where_clause << std::endl;
    //std::cout << "  where_clause=" << where_clause << std::endl;
    const bool new_find_set = !(found_set == old_found_set);
    result = m_Box_List.init_db_details(found_set);
    //m_Box_List.load_from_document();

    //Show the previously-shown record, if there is one, if this is not a new found-set (via a new where_clause)
    //so that returning to this table will return the user to the same record:
    Document_Glom* document = get_document();
    if(document)
    {
      Gnome::Gda::Value primary_key_for_details;

      if(!new_find_set && !details_record_specified)
      {
        //std::cout << "debug: no new_found_set" << std::endl;
        primary_key_for_details = document->get_layout_record_viewed(m_table_name, m_Box_Details.get_layout_name());
      }
      else if(details_record_specified)
      {
        primary_key_for_details = primary_key_value_for_details;
      }
      else
      {
         //std::cout << "debug: new_found_set" << std::endl;
      }

      if(GlomConversions::value_is_empty(primary_key_for_details))
      {
        //Make sure that the details view is not empty, if there are any records to show:
        primary_key_for_details = m_Box_List.get_primary_key_value_selected();
        //std::cout << "debug:  m_Box_List.get_primary_key_value_selected()=" << primary_key_for_details.to_string() << std::endl;
        if(GlomConversions::value_is_empty(primary_key_for_details))
        {
          //std::cout << "debug: calling list.get_primary_key_value_first()" << std::endl;
          primary_key_for_details = m_Box_List.get_primary_key_value_first();
          //std::cout << "  debug:  result=" <<  primary_key_for_details.to_string() << std::endl;
        }
      }

      m_Box_Details.init_db_details(found_set, primary_key_for_details);
    }
  }

  //Select the last-viewed layout, or the details layout, if a specific details record was specified:
  if(details_record_specified)
  {
    set_current_page(m_iPage_Details);
  }
  else
  {
    //Select the last-viewed layout:
    bool found = false;
    Document_Glom* document = get_document();
    if(document)
    {
      const Glib::ustring current_layout = get_document()->get_layout_current(m_table_name);
      if(!current_layout.empty())
      {
        const int count = get_n_pages();
        int page = 0;
        while(!found && (page < count))
        {
          Box_Data* box = dynamic_cast<Box_Data*>(get_nth_page(page));
          if(box && (box->get_layout_name() == current_layout))
            found = true;
          else
            ++page;
        }

        if(found)
        {
          set_current_page(page);
        }
      }
    }

    if(!found)
    {
      //Select List as default:
     set_current_page(m_iPage_List);
    }
  }

  return result;
}

FoundSet Notebook_Data::get_found_set() const
{
  return m_Box_List.get_found_set();
}

void Notebook_Data::on_list_user_requested_details(const Gnome::Gda::Value& primary_key_value)
{
  //std::cout << "Notebook_Data::on_list_user_requested_details" << std::endl;
  m_Box_Details.refresh_data_from_database_with_primary_key(primary_key_value);
  set_current_page(m_iPage_Details);
}

void Notebook_Data::on_details_user_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value)
{
  signal_record_details_requested().emit(table_name, primary_key_value);

  /*
  //Show a different table:
  init_db_details(m_table_name);

  //Show the specific record:
  on_list_user_requested_details(primary_key_value);
  */
}

void Notebook_Data::set_current_view(dataview view)
{
  std::cout << "Notebook_Data::set_current_view" << std::endl;

  if(view == DATA_VIEW_List)
    set_current_page(m_iPage_List);
  else
    set_current_page(m_iPage_Details);
}

void Notebook_Data::select_page_for_find_results()
{
  if(m_Box_List.get_showing_multiple_records())
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
  if(current_page == (int)m_iPage_List)
    result = DATA_VIEW_List;

  return result;
}

Notebook_Data::type_signal_record_details_requested Notebook_Data::signal_record_details_requested()
{
  return m_signal_record_details_requested;
}

void Notebook_Data::show_details(const Gnome::Gda::Value& primary_key_value)
{
  //Reuse this implementation:
  on_list_user_requested_details(primary_key_value);
}

void Notebook_Data::on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber)
{
  //Call base class:
  Notebook_Glom::on_switch_page_handler(pPage, uiPageNumber);

  //Remember that currently-viewed layout, so we can show it again when the user comes back to this table from elsewhere:
  Box_Data* box = dynamic_cast<Box_Data*>(get_nth_page(uiPageNumber));
  if(box)
  {
    Document_Glom* document = get_document();
    if(document)
      document->set_layout_current(m_table_name, box->get_layout_name());

    //And refresh the list view whenever it is shown, to 
    //a) show any new records that were added via the details view, or via a related portal elsewhere.
    //b) show changed field contents, changed elsewhere.
    //TODO_Performance: This causes double refreshes (with database retrieval) when doing finds. We probably want to distinguish between user page-switches and programmatic page-switches.
    if(box == &m_Box_List)
    {
      Gnome::Gda::Value primary_key_selected = m_Box_List.get_primary_key_value_selected();
      m_Box_List.refresh_data_from_database();
      m_Box_List.set_primary_key_value_selected(primary_key_selected);
    }
  }

}

void Notebook_Data::get_record_counts(gulong& total, gulong& found)
{
  m_Box_List.get_record_counts(total, found);
}
