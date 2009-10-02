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

#include <glom/mode_data/notebook_data.h>
#include <libglom/data_structure/glomconversions.h>
#include <glibmm/i18n.h>

namespace Glom
{

Notebook_Data::Notebook_Data()
: m_iPage_Details(0), m_iPage_List(0)
  #ifdef GLOM_ENABLE_MAEMO
  , m_window_maemo_details(0)
  #endif
{
  //Add Pages:
  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_List, _("List")));
  m_iPage_List = 0;

  #ifndef GLOM_ENABLE_MAEMO
  pages().push_back(Gtk::Notebook_Helpers::TabElem(m_Box_Details, _("Details")));
  m_iPage_Details = 1;
  #else
  //On Maemo, we add the box to m_window_maemo_details instead:
  m_window_maemo_details = new Window_BoxHolder(&m_Box_Details, _("Details"));
  
  Gtk::Window* pWindow = get_app_window();
  if(pWindow)
    m_window_maemo_details->set_transient_for(*pWindow);

  m_Box_Details.show_all();
  #endif //GLOM_ENABLE_MAEMO

  // Set accessible name for the notebook, to be able to access it via LDTP
#ifdef GTKMM_ATKMM_ENABLED
  get_accessible()->set_name(_("List Or Details View"));
#endif  


  //Connect signals:

  //Allow List to ask Details to show a record.
  m_Box_List.signal_user_requested_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_list_user_requested_details));

  //Allow Details to ask List to ask Details to show a different record:
  #ifndef GLOM_ENABLE_MAEMO //These navigation buttons are not visible on Maemo.
  m_Box_Details.signal_nav_first().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_first));
  m_Box_Details.signal_nav_prev().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_previous));
  m_Box_Details.signal_nav_next().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_next));
  m_Box_Details.signal_nav_last().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_nav_last));

  //Allow Details to tell List about record deletion:
  m_Box_Details.signal_record_deleted().connect(sigc::mem_fun(m_Box_List, &Box_Data_List::on_details_record_deleted));
  #endif //GLOM_ENABLE_MAEMO
  
  //Allow Details to ask to show a different record in a different table:
  m_Box_Details.signal_requested_related_details().connect(sigc::mem_fun(*this, &Notebook_Data::on_details_user_requested_related_details));

  //Fill composite view:
  add_view(&m_Box_List);
  add_view(&m_Box_Details);

  show_all_children();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //This is hidden by default,
  m_Box_Details.show_layout_toolbar(false);
#endif //GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_MAEMO
  //On Maemo we show the details in a separate window.
  set_show_tabs(false);
#endif //GLOM_ENABLE_MAEMO
}

Notebook_Data::~Notebook_Data()
{
  remove_view(&m_Box_List);
  remove_view(&m_Box_Details);

#ifdef GLOM_ENABLE_MAEMO
  if(m_window_maemo_details)
    delete m_window_maemo_details;
#endif //GLOM_ENABLE_MAEMO
}

bool Notebook_Data::init_db_details(const FoundSet& found_set, const Gnome::Gda::Value& primary_key_value_for_details)
{
  m_table_name = found_set.m_table_name;
  //std::cout << "Notebook_Data::init_db_details: table_name=" << m_table_name << ", primary_key_value_for_details=" << primary_key_value_for_details.to_string() << std::endl;

  const bool details_record_specified = !Conversions::value_is_empty(primary_key_value_for_details);

  bool result = true;
  //where_clause is only used as a result of a find.

  //Performance optimisation:
  //Keep the connection open during all these operations:
  {
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());
#else
    std::auto_ptr<ExceptionConnection> error;
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window(), error);
    // Ignore error, sharedconnection is not used directly within this function
#endif

    result = m_Box_List.init_db_details(found_set, get_active_layout_platform(get_document())); //TODO: Select the last selected record.

    //Show the previously-shown record, if there is one, if this is not a new found-set (via a new where_clause)
    //so that returning to this table will return the user to the same record:
    Document* document = get_document();
    if(document)
    {
      Gnome::Gda::Value primary_key_for_details;

      if(!details_record_specified)
      {
        //std::cout << "debug: no new_found_set" << std::endl;
        primary_key_for_details = document->get_layout_record_viewed(m_table_name, m_Box_Details.get_layout_name());
      }
      else if(details_record_specified)
      {
        primary_key_for_details = primary_key_value_for_details;
      }
      
      //If the specified (or remembered) primary key value is not in the found set, 
      //then ignore it:
      if(!found_set.m_where_clause.empty() && !get_primary_key_is_in_foundset(found_set, primary_key_for_details))
      {
        primary_key_for_details = Gnome::Gda::Value(); //TODO: We set it to empty just so we can test if for empty.
      }

      if(Conversions::value_is_empty(primary_key_for_details))
      {
        //Make sure that the details view is not empty, if there are any records to show:
        primary_key_for_details = m_Box_List.get_primary_key_value_selected();
        //std::cout << "debug:  m_Box_List.get_primary_key_value_selected()=" << primary_key_for_details.to_string() << std::endl;
        if(Conversions::value_is_empty(primary_key_for_details))
        {
          //std::cout << "debug: calling list.get_primary_key_value_first()" << std::endl;
          primary_key_for_details = m_Box_List.get_primary_key_value_first();
          //std::cout << "  debug:  result=" <<  primary_key_for_details.to_string() << std::endl;
        }
      }

      m_Box_Details.init_db_details(found_set, get_active_layout_platform(get_document()), primary_key_for_details);
    }
    else
      std::cerr << "Notebook_Data::init_db_details(): document is NULL" << std::endl;
  }


  //Block this handler temporarily because we don't need another refresh from the database:
  if(m_connection_switch_page)
    m_connection_switch_page.block();

  //Select the last-viewed layout, or the details layout, if a specific details record was specified:
  const dataview current_view = get_current_view();
  
  if(details_record_specified)
  {
    if(current_view != DATA_VIEW_Details)
      set_current_view(DATA_VIEW_Details);
  }
  else
  {
    //Get information abvout the the last-viewed layout:
    Glib::ustring current_layout;
    if(!details_record_specified)
    {
      Document* document = get_document(); 
      if(document)
        current_layout = document->get_layout_current(m_table_name);
    }

    //Set the layout:
    if( (current_layout.empty() || (current_layout == "list")) && (current_view != DATA_VIEW_List) )
      set_current_view(DATA_VIEW_List);
    else if( (current_layout == "details") && (current_view != DATA_VIEW_Details) )
      set_current_view(DATA_VIEW_Details);
  }

  //Re-enable this handler, so we can respond to notebook page changes:
  if(m_connection_switch_page)
    m_connection_switch_page.unblock();

  return result;
}

FoundSet Notebook_Data::get_found_set() const
{
  return m_Box_List.get_found_set();
}

void Notebook_Data::show_details(const Gnome::Gda::Value& primary_key_value)
{
  //Prevent n_switch_page_handler() from doing the same thing:
  if(m_connection_switch_page)
    m_connection_switch_page.block();
  
  //std::cout << "DEBUG: Notebook_Data::show_details() primary_key_value=" << primary_key_value.to_string() << std::endl;
  m_Box_Details.refresh_data_from_database_with_primary_key(primary_key_value);

#if GLOM_ENABLE_MAEMO
  //Details are shown in a separate window on Maemo,
  //though that window contains the regular m_Box_Details.
  m_window_maemo_details->show();
#else  
  if(get_current_view() != DATA_VIEW_Details)
    set_current_view(DATA_VIEW_Details);
#endif
  
  //Re-enable this handler, so we can respond to notebook page changes:
  if(m_connection_switch_page)
    m_connection_switch_page.unblock();  
}

void Notebook_Data::on_list_user_requested_details(const Gnome::Gda::Value& primary_key_value)
{
  show_details(primary_key_value);
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

FoundSet Notebook_Data::get_found_set_details() const
{
  return m_Box_Details.get_found_set();
}


void Notebook_Data::set_current_view(dataview view)
{
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

#ifndef GLOM_ENABLE_CLIENT_ONLY
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

void Notebook_Data::show_layout_toolbar(bool show)
{
  m_Box_Details.show_layout_toolbar(show);
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

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

void Notebook_Data::on_switch_page_handler(GtkNotebookPage* pPage, guint uiPageNumber)
{
  //Call base class:
  Notebook_Glom::on_switch_page_handler(pPage, uiPageNumber);

  //Remember that currently-viewed layout, so we can show it again when the user comes back to this table from elsewhere:
  Box_Data* box = dynamic_cast<Box_Data*>(get_nth_page(uiPageNumber));
  if(box)
  {
    Document* document = get_document();
    if(document)
      document->set_layout_current(m_table_name, box->get_layout_name());

    //And refresh the list view whenever it is shown, to 
    //a) show any new records that were added via the details view, or via a related portal elsewhere.
    //b) show changed field contents, changed elsewhere.
    if(box == &m_Box_List)
    {
      //std::cout << "debug: switching to list" << std::endl;
      const Gnome::Gda::Value primary_key_selected = m_Box_List.get_primary_key_value_selected();
      m_Box_List.refresh_data_from_database();
      m_Box_List.set_primary_key_value_selected(primary_key_selected);
    }
    else if(box == &m_Box_Details)
    {
      //std::cout << "debug: switching to details" << std::endl;
      const Gnome::Gda::Value primary_key_selected = m_Box_List.get_primary_key_value_selected();
      m_Box_Details.refresh_data_from_database_with_primary_key(primary_key_selected);
    }
  }

}

void Notebook_Data::get_record_counts(gulong& total, gulong& found)
{
  m_Box_List.get_record_counts(total, found);
}

} //namespace Glom
