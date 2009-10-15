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

#include <glom/mode_data/box_data_portal.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/application.h>
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_Portal::Box_Data_Portal()
#ifdef GLOM_ENABLE_MAEMO
: m_maemo_appmenubutton_add(Gtk::Hildon::SIZE_AUTO, Hildon::BUTTON_ARRANGEMENT_VERTICAL)
#endif
{
  set_size_request(400, -1); //An arbitrary default.

  //m_Frame.set_label_widget(m_Label_Related);
  m_Frame.set_shadow_type(Gtk::SHADOW_NONE);

  m_Frame.add(m_Alignment);
  m_Frame.show();

  m_Frame.set_label_widget(m_Label);
  m_Label.show();

  //The AddDel or Calendar is added to this:
  m_Alignment.set_padding(Utils::DEFAULT_SPACING_SMALL /* top */, 0, Utils::DEFAULT_SPACING_LARGE /* left */, 0);
  m_Alignment.show();

  add(m_Frame);

  m_layout_name = "list_portal"; //Replaced by derived classes.
  
  #ifdef GLOM_ENABLE_MAEMO
  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  signal_realize().connect(sigc::mem_fun(*this, &Box_Data_Portal::on_realize));
  signal_unrealize().connect(sigc::mem_fun(*this, &Box_Data_Portal::on_unrealize));  
  #endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  m_maemo_appmenubutton_add.signal_clicked().connect(
    sigc::mem_fun(*this, &Box_Data_Portal::on_maemo_appmenubutton_add));
  #endif //GLOM_ENABLE_MAEMO
}

void Box_Data_Portal::on_maemo_appmenubutton_add()
{
  do_add_record();
}

#ifdef GLOM_ENABLE_MAEMO
void Box_Data_Portal::on_realize()
{
  // Add an Add Related Something button to the application's AppMenu.
  // This will be removed when the portal is hidden.
  //TODO: Use the ustring compose thingy. murrayc.
  //TODO: Allow the designer to specify a singluar form for tables (and portals), 
  //so we can say Add Related Something instead of Somethings: Add Related.
  const Glib::ustring title = 
    Glib::ustring::compose(_("%1: Add Related"), get_title());
  m_maemo_appmenubutton_add.set_title(title);
  m_maemo_appmenubutton_add.set_value(_("Add related record"));
  App_Glom* app = App_Glom::get_application();
  g_assert(app);
  if(app)
  {
    Hildon::AppMenu* appmenu = app->get_maemo_appmenu();
    g_assert(appmenu);
    if(appmenu)
    {
      m_maemo_appmenubutton_add.show();
      appmenu->append(m_maemo_appmenubutton_add);
    }
  }
}

void Box_Data_Portal::on_unrealize()
{
  // Remove the AppMenu button when the portal is no longer shown: 
  App_Glom* app = App_Glom::get_application();
  g_assert(app);
  if(app)
  {
    Hildon::AppMenu* appmenu = app->get_maemo_appmenu();
    g_assert(appmenu);
    if(appmenu)
    {
      m_maemo_appmenubutton_add.hide();
      Gtk::Container* container = appmenu;
      container->remove(m_maemo_appmenubutton_add);
    }
  }
}
#endif //GLOM_ENABLE_MAEMO

bool Box_Data_Portal::init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title)
{
  m_portal = glom_sharedptr_clone(portal);

  Glib::ustring parent_table;
  if(m_portal)
    parent_table = m_portal->get_from_table();
      
  return init_db_details(parent_table, show_title);
}

Glib::ustring Box_Data_Portal::get_title() const
{
  //TODO: This same code is in box_data_related_list.cc. Remove the duplication.
  Glib::ustring relationship_title;
  if(m_portal && m_portal->get_has_relationship_name())
    relationship_title = m_portal->get_title_used(Glib::ustring() /* parent title - not relevant */);
  else
  {
    //Note to translators: This text is shown instead of a table title, when the table has not yet been chosen.
    relationship_title = _("Undefined Table");
  }
  
  return relationship_title;
}

//TODO: Is this base class implemenation actually called by anything?
bool Box_Data_Portal::init_db_details(const Glib::ustring& parent_table, bool show_title)
{
  m_parent_table = parent_table;

  if(m_portal)
    LayoutWidgetBase::m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table_name, not used. */); 
  else
    LayoutWidgetBase::m_table_name = Glib::ustring();

  Base_DB_Table::m_table_name = LayoutWidgetBase::m_table_name;

  if(show_title)
  {
    m_Label.set_markup(Utils::bold_message( get_title() ));
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

  FoundSet found_set;
  found_set.m_table_name = LayoutWidgetBase::m_table_name;
  return Box_Data::init_db_details(found_set, m_layout_platform); //Calls create_layout() and fill_from_database().
}

bool Box_Data_Portal::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  m_key_value = foreign_key_value;
  

  if(m_key_field && m_portal)
  {
    if(!Conversions::value_is_empty(m_key_value))
    {
      FoundSet found_set;
      set_found_set_where_clause_for_portal(found_set, m_portal, m_key_value);
   
      //std::cout << "DEBUG: refresh_data_from_database_with_foreign_key(): where_clause=" << found_set.m_where_clause << std::endl;
      return Box_Data::refresh_data_from_database_with_where_clause(found_set);
    }
    else
    {
      //If there is no from key value then no records can be shown:
      refresh_data_from_database_blank();
      return true;
    }
  }
  else
  {
    //If there is no to field then this relationship specifies all records in the table.
    FoundSet found_set = m_found_set;
    found_set.m_where_clause = Glib::ustring();
    return Box_Data::refresh_data_from_database_with_where_clause(found_set);
  }
}

sharedptr<LayoutItem_Portal> Box_Data_Portal::get_portal() const
{
  return m_portal;
}

sharedptr<const Field> Box_Data_Portal::get_key_field() const
{
  return m_key_field;
}

//TODO: refactor: Remove this because it is never called?
void Box_Data_Portal::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
  //Allow the parent record (Details view) to recalculate aggregations:
  signal_portal_record_changed().emit(m_portal->get_relationship_name());
}

void Box_Data_Portal::on_record_added(const Gnome::Gda::Value& /* primary_key_value */, const Gtk::TreeModel::iterator& /* row */)
{
  //Allow the parent record (Details view) to recalculate aggregations:
  signal_portal_record_changed().emit(m_portal->get_relationship_name());
}

Box_Data_Portal::type_vecLayoutFields Box_Data_Portal::get_fields_to_show() const
{
  const Document* document = get_document();
  if(document && m_portal)
  {
    Document::type_list_layout_groups mapGroups;
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
void Box_Data_Portal::on_dialog_layout_hide()
{
  //Overridden in derived classes.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY



bool Box_Data_Portal::get_has_suitable_record_to_view_details() const
{
  Glib::ustring navigation_table_name;
  sharedptr<const UsesRelationship> navigation_relationship;
  get_suitable_table_to_view_details(navigation_table_name, navigation_relationship);

  return !(navigation_table_name.empty());
}

void Box_Data_Portal::get_suitable_table_to_view_details(Glib::ustring& table_name, sharedptr<const UsesRelationship>& relationship) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();

  if(!m_portal)
    return;


  sharedptr<const UsesRelationship> navigation_relationship = m_portal->get_navigation_relationship_specific();

  //Check whether a relationship was specified:
  if(m_portal->get_navigation_type() == LayoutItem_Portal::NAVIGATION_AUTOMATIC)
  {
    //std::cout << "debug: decide automatically." << std::endl;
    //Decide automatically:
    bool navigation_relationship_main = false; // no longer used
    navigation_relationship = get_portal_navigation_relationship_automatic(m_portal, navigation_relationship_main);
    //std::cout << "debug: auto main=" << navigation_relationship_main << ", navigation_relationship=" << (navigation_relationship ? navigation_relationship->get_name() : navigation_relationship->get_relationship()->get_name()) << std::endl;
  }
  //else
  //  std::cout << "debug: get_suitable_table_to_view_details(): Using specific nav." << std::endl;

  const Document* document = get_document();
  if(!document)
    return;


  //Get the navigation table name from the chosen relationship:
  const Glib::ustring directly_related_table_name = m_portal->get_table_used(Glib::ustring() /* not relevant */);

  // The navigation_table_name (and therefore, the table_name output parameter,
  // as well) stays empty if the navrel type was set to none.
  Glib::ustring navigation_table_name;
  if(m_portal->get_navigation_type() == LayoutItem_Portal::NAVIGATION_AUTOMATIC)
  {
    navigation_table_name = directly_related_table_name;
  }
  else if(m_portal->get_navigation_type() == LayoutItem_Portal::NAVIGATION_SPECIFIC)
  {
    navigation_table_name = navigation_relationship->get_table_used(directly_related_table_name);
  }

  if(navigation_table_name.empty())
  {
    //std::cerr << "Box_Data_Portal::get_suitable_table_to_view_details(): navigation_table_name is empty." << std::endl;
    return;
  }

  if(document->get_table_is_hidden(navigation_table_name))
  {
    std::cerr << "Box_Data_Portal::get_suitable_table_to_view_details(): navigation_table_name indicates a hidden table: " << navigation_table_name << std::endl;
    return;
  }

  table_name = navigation_table_name;
  relationship = navigation_relationship;
}

void Box_Data_Portal::get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const
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

  //For instance "invoice_line_id" if this is a portal to an "invoice_lines" table:
  const Glib::ustring related_table = m_portal->get_table_used(Glib::ustring() /* not relevant */);
  sharedptr<const Field> key_field = get_field_primary_key_for_table(related_table);
  //std::cout << "DEBUG: related table=" << related_table << ", whose primary_key=" << key_field->get_name() << std::endl;

  const Glib::ustring query = Utils::build_sql_select_with_key(related_table, fieldsToGet, key_field, primary_key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute_select(query);


  bool value_found = true;
  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    //Set the output parameters:
    table_name = navigation_table_name;
#ifdef GLIBMM_EXCEPTIONS_ENABLED    
    table_primary_key_value = data_model->get_value_at(0, 0);
#else
    std::auto_ptr<Glib::Error> error;    
    table_primary_key_value = data_model->get_value_at(0, 0, error);
#endif    
    //std::cout << "Box_Data_Portal::get_suitable_record_to_view_details(): table_primary_key_value=" << table_primary_key_value.to_string() << std::endl;

    //The value is empty when there there is no record to match the key in the related table:
    //For instance, if an invoice lines record mentions a product id, but the product does not exist in the products table.
    if(Conversions::value_is_empty(table_primary_key_value))
    {
      value_found = false;
      std::cout << "debug: Box_Data_Portal::get_suitable_record_to_view_details(): SQL query returned empty primary key." << std::endl;
    }
  }
  else
  {
    value_found = false;

    std::cout << "DEBUG: Box_Data_Portal::get_suitable_record_to_view_details(): SQL query returned no suitable primary key. table=" 
      << related_table  
      << ", field=" << layout_item->get_layout_display_name() 
      << ", key_field=" << key_field->get_name()
      << ", primary_key_value=" << primary_key_value.to_string() << std::endl;

    std::cout << "  DEBUG: SQL was: " << query << std::endl;
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

Document::type_list_layout_groups Box_Data_Portal::create_layout_get_layout()
{
  Document::type_list_layout_groups result;

  if(m_portal)
    result.push_back(m_portal);
  
  return result;
}

sharedptr<Field> Box_Data_Portal::get_field_primary_key() const
{
  return m_key_field;
}

Box_Data_Portal::type_signal_portal_record_changed Box_Data_Portal::signal_portal_record_changed()
{
  return m_signal_portal_record_changed;
}


} //namespace Glom
