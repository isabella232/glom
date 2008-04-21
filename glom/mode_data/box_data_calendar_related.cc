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

#include "box_data_calendar_related.h"
#include "dialog_layout_calendar_related.h"
#include <glom/application.h>
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_Calendar_Related::Box_Data_Calendar_Related()
: m_pMenuPopup(0)
{
  set_size_request(400, -1); //An arbitrary default.

  remove(m_AddDel); //TODO: Don't even have this at all.
  m_Alignment.add(m_calendar);
  m_calendar.show();
  
  //Tell the calendar how to get the record details to show:
  m_calendar.set_detail_func( sigc::mem_fun(*this, &Box_Data_Calendar_Related::on_calendar_details) );
  
  setup_menu();
  //m_calendar.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_calendar.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &Box_Data_Calendar_Related::on_calendar_button_press_event) );

  m_layout_name = "list_related_calendar"; //TODO: We need a unique name when 2 portals use the same table.
}

void Box_Data_Calendar_Related::enable_buttons()
{
  //const bool view_details_possible = get_has_suitable_record_to_view_details();
  //m_calendar.set_allow_view_details(view_details_possible); //Don't allow the user to go to a record in a hidden table.
}

bool Box_Data_Calendar_Related::init_db_details(const sharedptr<const LayoutItem_CalendarPortal>& portal, bool show_title)
{
  m_portal = glom_sharedptr_clone(portal);

  LayoutWidgetBase::m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table_name, not used. */); 
  Box_DB_Table::m_table_name = LayoutWidgetBase::m_table_name;

  const Glib::ustring relationship_title = m_portal->get_title_used(Glib::ustring() /* parent title - not relevant */);
  
  if(show_title)
  {
    m_Label.set_markup(Bakery::App_Gtk::util_bold_message(relationship_title));
    m_Label.show();

    m_Alignment.set_padding(Utils::DEFAULT_SPACING_SMALL /* top */, 0, Utils::DEFAULT_SPACING_LARGE /* left */, 0);
  }
  else
  {
    m_Label.set_markup(Glib::ustring());
    m_Label.hide();

    m_Alignment.set_padding(0, 0, 0, 0); //The box itself has padding of 6.
  }

  m_key_field = get_fields_for_table_one_field(LayoutWidgetBase::m_table_name, m_portal->get_to_field_used());

  enable_buttons();

  FoundSet found_set;
  found_set.m_table_name = LayoutWidgetBase::m_table_name;
  return Box_Data_List::init_db_details(found_set); //Calls create_layout() and fill_from_database().
}

bool Box_Data_Calendar_Related::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  std::cout << "DEBUG: Box_Data_Calendar_Related::refresh_data_from_database_with_foreign_key()" << std::endl;
  
  
  m_key_value = foreign_key_value;

  if(!m_key_field)
    std::cout << "DEBUG: Box_Data_Calendar_Related::refresh_data_from_database_with_foreign_key(): m_key_field is NULL" << std::endl;
  
  if(Conversions::value_is_empty(m_key_value))
    std::cout << "DEBUG: Box_Data_Calendar_Related::refresh_data_from_database_with_foreign_key(): m_key_value is empty." << std::endl;
  
  
  if(m_key_field)
  {
    if(!Conversions::value_is_empty(m_key_value))
    {
      sharedptr<Relationship> relationship = m_portal->get_relationship();

      // Notice that, in the case that this is a portal to doubly-related records,
      // The WHERE clause mentions the first-related table (though by the alias defined in extra_join)
      // and we add an extra JOIN to mention the second-related table.

      Glib::ustring where_clause_to_table_name = relationship->get_to_table();
      sharedptr<Field> where_clause_to_key_field = m_key_field;

      m_found_set.m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table - not relevant */);
      
      sharedptr<const Relationship> relationship_related = m_portal->get_related_relationship();
      if(relationship_related)
      {
         //Add the extra JOIN:
         sharedptr<UsesRelationship> uses_rel_temp = sharedptr<UsesRelationship>::create();
         uses_rel_temp->set_relationship(relationship);
         //found_set.m_extra_join = uses_rel_temp->get_sql_join_alias_definition();
         m_found_set.m_extra_join = "LEFT OUTER JOIN \"" + relationship->get_to_table() + "\" AS \"" + uses_rel_temp->get_sql_join_alias_name() + "\" ON (\"" + uses_rel_temp->get_sql_join_alias_name() + "\".\"" + relationship_related->get_from_field() + "\" = \"" + relationship_related->get_to_table() + "\".\"" + relationship_related->get_to_field() + "\")";

         //Add an extra GROUP BY to ensure that we get no repeated records from the doubly-related table:
         m_found_set.m_extra_group_by = "GROUP BY \"" + m_found_set.m_table_name + "\".\"" + m_key_field->get_name() + "\"";

         //Adjust the WHERE clause appropriately for the extra JOIN:
         where_clause_to_table_name = uses_rel_temp->get_sql_join_alias_name();

         const Glib::ustring to_field_name = uses_rel_temp->get_to_field_used();
         where_clause_to_key_field = get_fields_for_table_one_field(relationship->get_to_table(), to_field_name);
         //std::cout << "extra_join=" << found_set.m_extra_join << std::endl;
 
         //std::cout << "extra_join where_clause_to_key_field=" << where_clause_to_key_field->get_name() << std::endl;
      }

      m_found_set.m_where_clause = "\"" + where_clause_to_table_name + "\".\"" + relationship->get_to_field() + "\" = " + where_clause_to_key_field->sql(m_key_value);
      return true;
    }
    else
    {
      //If there is no from key value then no records can be shown:
      //TODO
      return true;
    }
  }
  else
  {
    //If there is no to field then this relationship specifies all records in the table.
    m_found_set.m_where_clause = Glib::ustring();
    return true;
  }
  
  //The actual data is retrieved in the detail_func handler.
}

bool Box_Data_Calendar_Related::fill_from_database()
{
  bool result = false;
  bool allow_add = true;

  if(m_key_field && m_found_set.m_where_clause.empty()) //There's a key field, but no value.
  {
    //No Foreign Key value, so just show the field names:

    result = Box_DB_Table::fill_from_database();

    //create_layout();

    fill_end();
  }
  else
  {
    result = Box_Data_List::fill_from_database();


    //Is there already one record here?
    if(m_has_one_or_more_records) //This was set by Box_Data_List::fill_from_database().
    {
      //Is the to_field unique? If so, there can not be more than one.
      if(m_key_field && m_key_field->get_unique_key()) //automatically true if it is a primary key
        allow_add = false;
    }

    //TODO: Disable add if the from_field already has a value and the to_field is auto-incrementing because
    //- we cannot override the auto-increment in the to_field.
    //- we cannot change the value in the from_field to the new auto_increment value in the to_field.
  }

  //Prevent addition of new records if that is what the relationship specifies:
  if(allow_add && m_portal->get_relationship())
    allow_add = m_portal->get_relationship()->get_auto_create();

  //TODO: m_calendar.set_allow_add(allow_add);

  return result;
}

void Box_Data_Calendar_Related::on_record_added(const Gnome::Gda::Value& primary_key_value, const Gtk::TreeModel::iterator& row)
{
  //primary_key_value is a new autogenerated or human-entered key for the row.
  //It has already been added to the database.

  if(!row)
    return;

  Gnome::Gda::Value key_value;

  if(m_key_field)
  {
    //m_key_field is the field in this table that must match another field in the parent table.
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_full_field_details(m_key_field);
    //TODO: key_value = m_calendar.get_value(row, layout_item);
  }

  Box_Data_List::on_record_added(key_value, row); //adds blank row.

  //Make sure that the new related record is related,
  //by setting the foreign key:
  //If it's not auto-generated.
  if(!Conversions::value_is_empty(key_value)) //If there is already a value.
  {
    //It was auto-generated. Tell the parent about it, so it can make a link.
    signal_record_added.emit(key_value);
  }
  else if(Conversions::value_is_empty(m_key_value))
  {
    g_warning("Box_Data_Calendar_Related::on_record_added(): m_key_value is NULL.");
  }
  else
  {
    sharedptr<Field> field_primary_key; //TODO: = m_calendar.get_key_field();

    //Create the link by setting the foreign key
    if(m_key_field)
    {
      Glib::ustring strQuery = "UPDATE \"" + m_portal->get_table_used(Glib::ustring() /* not relevant */) + "\"";
      strQuery += " SET \"" +  /* get_table_name() + "." +*/ m_key_field->get_name() + "\" = " + m_key_field->sql(m_key_value);
      strQuery += " WHERE \"" + get_table_name() + "\".\"" + field_primary_key->get_name() + "\" = " + field_primary_key->sql(primary_key_value);
      const bool test = query_execute(strQuery, get_app_window());
      if(test)
      {
        //Show it on the view, if it's visible:
        sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
        layout_item->set_full_field_details(field_primary_key);

        //TODO: m_calendar.set_value(row, layout_item, m_key_value);
      }
    }

    //on_adddel_user_changed(row, iKey); //Update the database.
  }
}

void Box_Data_Calendar_Related::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Allow the parent record (Details view) to recalculate aggregations:
  signal_record_changed().emit(m_portal->get_relationship_name());
}

Box_Data_Calendar_Related::type_vecLayoutFields Box_Data_Calendar_Related::get_fields_to_show() const
{
  const Document_Glom* document = get_document();
  if(document)
  {
    Document_Glom::type_list_layout_groups mapGroups;
    mapGroups.push_back(m_portal);

    sharedptr<const Relationship> relationship = m_portal->get_relationship();
    if(relationship)
    {
      type_vecLayoutFields result = get_table_fields_to_show_for_sequence(m_portal->get_table_used(Glib::ustring() /* not relevant */), mapGroups);

      //If the relationship does not allow editing, then mark all these fields as non-editable:
      if(!(m_portal->get_relationship_used_allows_edit()))
      {
        for(type_vecLayoutFields::iterator iter = result.begin(); iter != result.end(); ++iter)
        {
          sharedptr<LayoutItem_Field> item = *iter;
          if(item)
            item->set_editable(false);
        }
      }

      return result;
    }
  }

  return type_vecLayoutFields();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_Calendar_Related::on_dialog_layout_hide()
{
  Dialog_Layout_Calendar_Related* dialog_related = dynamic_cast<Dialog_Layout_Calendar_Related*>(m_pDialogLayout);
  g_assert(dialog_related != NULL);
  m_portal = dialog_related->get_portal_layout();


  //Update the UI:
  sharedptr<LayoutItem_CalendarPortal> derived_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(m_portal);
  init_db_details(derived_portal); 

  Box_Data::on_dialog_layout_hide();

  sharedptr<LayoutItem_CalendarPortal> pLayoutItem = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(get_layout_item());
  if(pLayoutItem)
  {
    sharedptr<LayoutItem_CalendarPortal> derived_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(m_portal);
    if(derived_portal)
      *pLayoutItem = *derived_portal;
    
    signal_layout_changed().emit(); //TODO: Check whether it has really changed.
  }
}
#endif // !GLOM_ENABLE_CLIENT_ONLY


bool Box_Data_Calendar_Related::get_has_suitable_record_to_view_details() const
{
  Glib::ustring navigation_table_name;
  sharedptr<const UsesRelationship> navigation_relationship;
  get_suitable_table_to_view_details(navigation_table_name, navigation_relationship);

  return !(navigation_table_name.empty());
}

void Box_Data_Calendar_Related::get_suitable_table_to_view_details(Glib::ustring& table_name, sharedptr<const UsesRelationship>& relationship) const
{
 
  //Initialize output parameters:
  table_name = Glib::ustring();

  //Check whether a relationship was specified:
  bool navigation_relationship_main = false;
  sharedptr<const UsesRelationship> navigation_relationship = m_portal->get_navigation_relationship_specific(navigation_relationship_main);
  if(!navigation_relationship_main && !navigation_relationship)
  {
    //std::cout << "debug: decide automatically." << std::endl;
    //Decide automatically:
    navigation_relationship_main = false;
    navigation_relationship = get_portal_navigation_relationship_automatic(m_portal, navigation_relationship_main);
    //std::cout << "debug: auto main=" << navigation_relationship_main << ", navigation_relationship=" << (navigation_relationship ? navigation_relationship->get_name() : navigation_relationship->get_relationship()->get_name()) << std::endl;
  }
  //else
  //  std::cout << "debug: get_suitable_table_to_view_details(): Using specific nav." << std::endl;

  const Document_Glom* document = get_document();
  if(!document)
    return;


  //Get the navigation table name from the chosen relationship:
  const Glib::ustring directly_related_table_name = m_portal->get_table_used(Glib::ustring() /* not relevant */);
 
  Glib::ustring navigation_table_name;
  if(navigation_relationship_main)
  {
    navigation_table_name = directly_related_table_name;
  }
  else if(navigation_relationship)
  {
    navigation_table_name = navigation_relationship->get_table_used(directly_related_table_name);
  }

  if(navigation_table_name.empty())
  {
    std::cerr << "Box_Data_Calendar_Related::get_suitable_table_to_view_details(): navigation_table_name is empty." << std::endl;
    return;
  }

  if(document->get_table_is_hidden(navigation_table_name))
  {
    std::cerr << "Box_Data_Calendar_Related::get_suitable_table_to_view_details(): navigation_table_name indicates a hidden table: " << navigation_table_name << std::endl;
    return;
  }

  table_name = navigation_table_name;
  relationship = navigation_relationship;
}

void Box_Data_Calendar_Related::get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();
  table_primary_key_value = Gnome::Gda::Value();

  Glib::ustring navigation_table_name;
  sharedptr<const UsesRelationship> navigation_relationship;
  get_suitable_table_to_view_details(navigation_table_name, navigation_relationship);
 
  if(navigation_table_name.empty())
    return;

  //Get the primary key of that table:
  sharedptr<Field> navigation_table_primary_key = get_field_primary_key_for_table(navigation_table_name);

  //Build a layout item to get the field's value:
  sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
  layout_item->set_full_field_details(navigation_table_primary_key);

  if(navigation_relationship)
  {
    layout_item->set_relationship( navigation_relationship->get_relationship() );
    //std::cout << "debug: navigation_relationship->get_relationship()= " << navigation_relationship->get_relationship()->get_name() << std::endl;
    layout_item->set_related_relationship( navigation_relationship->get_related_relationship() );
  }

  //Get the value of the navigation related primary key:
  type_vecLayoutFields fieldsToGet;
  fieldsToGet.push_back(layout_item);

  const Glib::ustring query; //TODO: = Utils::build_sql_select_with_key(m_portal->get_table_used(Glib::ustring() /* not relevant */), fieldsToGet, m_calendar.get_key_field(), primary_key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);


  bool value_found = true;
  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    //Set the output parameters:
    table_name = navigation_table_name;
    table_primary_key_value = data_model->get_value_at(0, 0);

    //std::cout << "Box_Data_Calendar_Related::get_suitable_record_to_view_details(): table_primary_key_value=" << table_primary_key_value.to_string() << std::endl;

    //The value is empty when there there is no record to match the key in the related table:
    //For instance, if an invoice lines record mentions a product id, but the product does not exist in the products table.
    if(Conversions::value_is_empty(table_primary_key_value))
    {
      value_found = false;
      std::cout << "debug: Box_Data_Calendar_Related::get_suitable_record_to_view_details(): SQL query returned empty primary key." << std::endl;
    }
  }
  else
  {
    value_found = false;
    std::cout << "debug: Box_Data_Calendar_Related::get_suitable_record_to_view_details(): SQL query returned no suitable primary key." << std::endl;
  }

  if(!value_found)
  {
    //Clear the output parameters:
    table_name = Glib::ustring();
    table_primary_key_value = Gnome::Gda::Value();

    Gtk::Window* window = const_cast<Gtk::Window*>(get_app_window());
    if(window)
      Frame_Glom::show_ok_dialog(_("No Corresponding Record Exists"), _("No record with this value exists. Therefore navigation to the related record is not possible."), *window, Gtk::MESSAGE_WARNING); //TODO: Make it more clear to the user exactly what record, what field, and what value, we are talking about.
  }
}

Document_Glom::type_list_layout_groups Box_Data_Calendar_Related::create_layout_get_layout()
{
  Document_Glom::type_list_layout_groups result;

  if(m_portal)
    result.push_back(m_portal);
  
  return result;
}

Dialog_Layout* Box_Data_Calendar_Related::create_layout_dialog() const
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom_developer.glade", "window_data_layout");
  if(refXml)
  {
    Dialog_Layout_Calendar_Related* dialog = 0;
    refXml->get_widget_derived("window_data_layout", dialog);
    return dialog;
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY

  return NULL;
}

void Box_Data_Calendar_Related::prepare_layout_dialog(Dialog_Layout* dialog)
{
  Dialog_Layout_Calendar_Related* related_dialog = dynamic_cast<Dialog_Layout_Calendar_Related*>(dialog);
  g_assert(related_dialog != NULL);
  
  sharedptr<LayoutItem_CalendarPortal> derived_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(m_portal);
  related_dialog->set_document(m_layout_name, get_document(), derived_portal);
}

Glib::ustring Box_Data_Calendar_Related::on_calendar_details(guint year, guint month, guint day)
{
  sharedptr<LayoutItem_CalendarPortal> derived_portal = sharedptr<LayoutItem_CalendarPortal>::cast_dynamic(m_portal);
    
  
  if(!derived_portal || !derived_portal->get_date_field())
  {
    std::cout << "DEBUG: Box_Data_Calendar_Related::on_calendar_details(): date_field is NULL" << std::endl;
    return Glib::ustring();
  }
  
  Glib::Date date(day, Glib::Date::Month(month+1), year);
  Gnome::Gda::Value date_value(date);

  sharedptr<Relationship> relationship = m_portal->get_relationship();
  Glib::ustring where_clause_to_table_name = relationship->get_to_table();
  
  const Glib::ustring date_field_name = derived_portal->get_date_field()->get_name();
    
  sharedptr<const Relationship> relationship_related = m_portal->get_related_relationship();
  if(relationship_related)
  {
    //Adjust the WHERE clause appropriately for the extra JOIN:
    sharedptr<UsesRelationship> uses_rel_temp = sharedptr<UsesRelationship>::create();
    uses_rel_temp->set_relationship(relationship);
    where_clause_to_table_name = uses_rel_temp->get_sql_join_alias_name();
  }

  //Add an AND to the existing where clause, to get only records with this date, if any:
  const Glib::ustring extra_where_clause = "\"" + where_clause_to_table_name + "\".\"" + derived_portal->get_date_field()->get_name() + "\" = " + derived_portal->get_date_field()->sql(date_value);
  Glib::ustring where_clause;
  if(m_found_set.m_where_clause.empty())
    where_clause = extra_where_clause;
  else
    where_clause = "( " + m_found_set.m_where_clause + " ) AND ( " + extra_where_clause + " )";
  
  const Glib::ustring sql_query = Utils::build_sql_select_with_where_clause(m_found_set.m_table_name, m_FieldsShown, where_clause, m_found_set.m_extra_join, m_found_set.m_sort_clause, m_found_set.m_extra_group_by);
  //std::cout << "DEBUG: sql_query=" << sql_query << std::endl;
  Glib::RefPtr<Gnome::Gda::DataModel> datamodel = query_execute(sql_query, get_app_window());
  if(!(datamodel && datamodel->get_n_rows()))
    return Glib::ustring();
 
  //Execute the query to get the data:
  Glib::ustring result;
  const int row_index = 0;
  int column_index = 0;
  for(type_vecLayoutFields::const_iterator iter = m_FieldsShown.begin(); iter != m_FieldsShown.end(); ++iter)
  {
    sharedptr<LayoutItem> layout_item = *iter;
    
    Glib::ustring text;
    
    //Text for a text item:
    sharedptr<LayoutItem_Text> layout_item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
    if(layout_item_text)
      text = layout_item_text->get_text();
    else
    {  
      //Text for a field:
      sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      if(layout_item_field)
      {
        const Gnome::Gda::Value value = datamodel->get_value_at(row_index, column_index);
        text = Conversions::get_text_for_gda_value(layout_item_field->get_glom_type(), value, layout_item_field->get_formatting_used().m_numeric_format);
        std::cout << "  DEBUG: text=" << text << std::endl;
        ++column_index;
      }
    }
    
    if(!text.empty())
    {
      if(!result.empty())
          result += ", "; //TODO: Internationalization?
      
      result += text;
    }
  }
  
  return result;
}

void Box_Data_Calendar_Related::setup_menu()
{
  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("ContextMenu", "Context Menu") );

  m_refContextEdit =  Gtk::Action::create("ContextEdit", Gtk::Stock::EDIT);

  m_refActionGroup->add(m_refContextEdit,
    sigc::mem_fun(*this, &Box_Data_Calendar_Related::on_MenuPopup_activate_Edit) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  // Don't add ContextLayout in client only mode because it would never
  // be sensitive anyway
  m_refContextLayout =  Gtk::Action::create("ContextLayout", _("Layout"));
  m_refActionGroup->add(m_refContextLayout,
    sigc::mem_fun(*this, &Box_Data_Calendar_Related::on_MenuPopup_activate_layout) );

  //TODO: This does not work until this widget is in a container in the window:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  //TODO: add_accel_group(m_refUIManager->get_accel_group());

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
#endif
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextEdit'/>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
        "    <menuitem action='ContextLayout'/>"
#endif
        "  </popup>"
        "</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
#else
  std::auto_ptr<Glib::Error> error;
  m_refUIManager->add_ui_from_string(ui_info, error);
  if(error.get() != NULL)
  {
    std::cerr << "building menus failed: " << error->what();
  }
#endif //GLIBMM_EXCEPTIONS_ENABLED

  //Get the menu:
  m_pMenuPopup = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/ContextMenu") ); 
  if(!m_pMenuPopup)
    g_warning("menu not found");

 
#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(pApp)
    m_refContextLayout->set_sensitive(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER);
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

void Box_Data_Calendar_Related::on_calendar_button_press_event(GdkEventButton *event)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }
#endif

  GdkModifierType mods;
  gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
  if(mods & GDK_BUTTON3_MASK)
  {
    //Give user choices of actions on this item:
    m_pMenuPopup->popup(event->button, event->time);
    return; //handled.
  }
  else
  {
    if(event->type == GDK_2BUTTON_PRESS)
    {
      //Double-click means edit.
      //Don't do this usually, because users sometimes double-click by accident when they just want to edit a cell.

      //TODO: If the cell is not editable, handle the double-click as an edit/selection.
      //on_MenuPopup_activate_Edit();
      return; //Not handled.
    }
  }

  return; //Not handled. TODO: Call base class?
}

void
Box_Data_Calendar_Related::on_MenuPopup_activate_Edit()
{
  const Gnome::Gda::Value primary_key_value; //TODO: = m_AddDel.get_value_key(row); //The primary key is in the key.

  signal_user_requested_details().emit(primary_key_value);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_Calendar_Related::on_MenuPopup_activate_layout()
{
  show_layout_dialog();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY


} //namespace Glom
