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
#include <glibmm/main.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

static const gchar gNotebookCss[] = "gtkmm__GtkNotebook#glomnotebook { padding: 0 0 0 0; }";


Notebook_Data::Notebook_Data()
:
  m_iPage_Details(0), m_iPage_List(0)
{

  //Hide the GtkNotebook border:
  set_name("glomnotebook");

  GtkStyleContext *style_context;
  GtkCssProvider *provider;
  GError *error = NULL;

  style_context = gtk_widget_get_style_context(GTK_WIDGET(gobj()));
  provider = gtk_css_provider_new();
  if (!gtk_css_provider_load_from_data(provider, gNotebookCss, -1,
                                        &error)) {
    g_error("%s", error->message);
    g_error_free(error);
    return;
  }
  gtk_style_context_add_provider(style_context,
                                  GTK_STYLE_PROVIDER(provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(provider);
   
   
  //Add Pages:
  //Translators: This is a noun. It is a notebook tab title.
  append_page(m_Box_List, _("List"));
  m_iPage_List = 0;

  //Translators: This is a noun. It is a notebook tab title.
  append_page(m_Box_Details, _("Details"));
  m_iPage_Details = 1;

  // Set accessible name for the notebook, to be able to access it via LDTP
#ifdef GTKMM_ATKMM_ENABLED
  //Translators: This is a title, not an action.
  get_accessible()->set_name(_("List Or Details View"));
#endif


  //Connect signals:

  //Allow List to ask Details to show a record.
  m_Box_List.signal_user_requested_details().connect(sigc::mem_fun(*this,
    &Notebook_Data::on_list_user_requested_details));
  
  //Allow the parent widget to detect list selection changes:
  m_Box_List.signal_record_selection_changed().connect(sigc::mem_fun(*this,
    &Notebook_Data::on_list_selection_changed));

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

#ifndef GLOM_ENABLE_CLIENT_ONLY
  //This is hidden by default,
  m_Box_Details.show_layout_toolbar(false);
#endif //GLOM_ENABLE_CLIENT_ONLY
}

Notebook_Data::~Notebook_Data()
{
  remove_view(&m_Box_List);
  remove_view(&m_Box_Details);
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
    sharedptr<SharedConnection> sharedconnection = connect_to_server(get_app_window());

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
      std::cerr << G_STRFUNC << ": document is NULL" << std::endl;
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

  if(get_current_view() != DATA_VIEW_Details)
    set_current_view(DATA_VIEW_Details);

  //Re-enable this handler, so we can respond to notebook page changes:
  if(m_connection_switch_page)
    m_connection_switch_page.unblock();
}

bool Notebook_Data::on_idle_show_details(const Gnome::Gda::Value& primary_key_value)
{
  show_details(primary_key_value);
  return false; //Don't call this idle handler again.
}

void Notebook_Data::on_list_user_requested_details(const Gnome::Gda::Value& primary_key_value)
{
  //Show the details after a delay,
  //to avoid problems with deleting the list GtkCellRenderer while
  //handling its signal.
  Glib::signal_idle().connect(
    sigc::bind(
      sigc::mem_fun(*this, &Notebook_Data::on_idle_show_details),
      primary_key_value));
}

void Notebook_Data::on_list_selection_changed()
{
  m_signal_record_selection_changed.emit();
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

FoundSet Notebook_Data::get_found_set_selected() const
{
  if(get_current_view() == DATA_VIEW_Details)
  {
    return m_Box_Details.get_found_set();
  }
  else
  {
    //Start with something sensible:
    FoundSet found_set = m_Box_List.get_found_set();
    
    const Gnome::Gda::Value primary_key_value_selected = 
      m_Box_List.get_primary_key_value_selected();
    if(Conversions::value_is_empty(primary_key_value_selected))
    {
      //Indicate to the caller that no record is selected:
      found_set.m_where_clause = Gnome::Gda::SqlExpr();
      return found_set;
    }

    const Document* document = get_document();
    if(!document)
    {
      std::cerr << G_STRFUNC << ": document is null" << std::endl;
      found_set.m_where_clause = Gnome::Gda::SqlExpr();
      return found_set;
    }
    
    sharedptr<Field> primary_key_field =
      document->get_field_primary_key(m_table_name);
    found_set.m_where_clause = Utils::build_simple_where_expression(
      m_table_name, primary_key_field,
      primary_key_value_selected);
    return found_set;
  }
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

void Notebook_Data::set_enable_layout_drag_and_drop(bool enable)
{
  m_Box_Details.show_layout_toolbar(enable);
  m_Box_Details.set_enable_drag_and_drop(enable);
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

void Notebook_Data::do_menu_file_print()
{
  const int iPageCurrent = get_current_page();

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

Notebook_Data::type_signal_record_selection_changed Notebook_Data::signal_record_selection_changed()
{
  return m_signal_record_selection_changed;
}

void Notebook_Data::on_switch_page_handler(Gtk::Widget* pPage, guint uiPageNumber)
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

void Notebook_Data::do_menu_file_add_record()
{
  show_details(Gnome::Gda::Value());
  m_Box_Details.do_new_record();
}

} //namespace Glom
