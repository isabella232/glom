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

#include "box_data_portal.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include <glom/libglom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <bakery/App/App_Gtk.h> //For util_bold_message().
#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_Portal::Box_Data_Portal()
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
}


bool Box_Data_Portal::init_db_details(const sharedptr<const LayoutItem_Portal>& portal, bool show_title)
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

bool Box_Data_Portal::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  m_key_value = foreign_key_value;

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

      FoundSet found_set = m_found_set;
      found_set.m_table_name = m_portal->get_table_used(Glib::ustring() /* parent table - not relevant */);
      
      sharedptr<const Relationship> relationship_related = m_portal->get_related_relationship();
      if(relationship_related)
      {
         //Add the extra JOIN:
         sharedptr<UsesRelationship> uses_rel_temp = sharedptr<UsesRelationship>::create();
         uses_rel_temp->set_relationship(relationship);
         //found_set.m_extra_join = uses_rel_temp->get_sql_join_alias_definition();
         found_set.m_extra_join = "LEFT OUTER JOIN \"" + relationship->get_to_table() + "\" AS \"" + uses_rel_temp->get_sql_join_alias_name() + "\" ON (\"" + uses_rel_temp->get_sql_join_alias_name() + "\".\"" + relationship_related->get_from_field() + "\" = \"" + relationship_related->get_to_table() + "\".\"" + relationship_related->get_to_field() + "\")";

         //Add an extra GROUP BY to ensure that we get no repeated records from the doubly-related table:
         found_set.m_extra_group_by = "GROUP BY \"" + found_set.m_table_name + "\".\"" + m_key_field->get_name() + "\"";

         //Adjust the WHERE clause appropriately for the extra JOIN:
         where_clause_to_table_name = uses_rel_temp->get_sql_join_alias_name();

         Glib::ustring to_field_name = uses_rel_temp->get_to_field_used();
         where_clause_to_key_field = get_fields_for_table_one_field(relationship->get_to_table(), to_field_name);
         //std::cout << "extra_join=" << found_set.m_extra_join << std::endl;
 
         //std::cout << "extra_join where_clause_to_key_field=" << where_clause_to_key_field->get_name() << std::endl;
      }

      found_set.m_where_clause = "\"" + where_clause_to_table_name + "\".\"" + relationship->get_to_field() + "\" = " + where_clause_to_key_field->sql(m_key_value);


      //g_warning("refresh_data_from_database(): where_clause=%s", where_clause.c_str());
      return Box_Data_List::refresh_data_from_database_with_where_clause(found_set);
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
    return Box_Data_List::refresh_data_from_database_with_where_clause(found_set);
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

//void Box_Data_Portal::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
//{
//  //Allow the parent record (Details view) to recalculate aggregations:
//  signal_record_changed().emit(m_portal->get_relationship_name());
//}

Box_Data_Portal::type_vecLayoutFields Box_Data_Portal::get_fields_to_show() const
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

Box_Data_Portal::type_signal_record_changed Box_Data_Portal::signal_record_changed()
{
  return m_signal_record_changed;
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
    std::cerr << "Box_Data_Portal::get_suitable_table_to_view_details(): navigation_table_name is empty." << std::endl;
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

  const Glib::ustring query = Utils::build_sql_select_with_key(m_portal->get_table_used(Glib::ustring() /* not relevant */), fieldsToGet, get_key_field(), primary_key_value);
  Glib::RefPtr<Gnome::Gda::DataModel> data_model = query_execute(query);


  bool value_found = true;
  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    //Set the output parameters:
    table_name = navigation_table_name;
    table_primary_key_value = data_model->get_value_at(0, 0);

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
    std::cout << "debug: Box_Data_Portal::get_suitable_record_to_view_details(): SQL query returned no suitable primary key." << std::endl;
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

Document_Glom::type_list_layout_groups Box_Data_Portal::create_layout_get_layout()
{
  Document_Glom::type_list_layout_groups result;

  if(m_portal)
    result.push_back(m_portal);
  
  return result;
}

} //namespace Glom
