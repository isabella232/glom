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

#include <glom/mode_data/box_data_portal.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <libglom/sql_utils.h>
#include <libglom/utils.h>
#include <libglom/layout_utils.h>
#include <glom/glade_utils.h>
#include <glom/frame_glom.h> //For show_ok_dialog()
#include <glom/utils_ui.h> //For bold_message()).
#include <glom/appwindow.h>


#include <glibmm/i18n.h>

namespace Glom
{

Box_Data_Portal::Box_Data_Portal()
  : m_find_mode(false)
{
  //m_Frame.set_label_widget(m_Label_Related);
  m_Frame.set_shadow_type(Gtk::SHADOW_NONE);
  m_Frame.show();

  m_Frame.set_label_widget(m_Label);
  m_Label.show();

  //The AddDel or Calendar is added to this:
  //They must use child.set_margin_start(Utils::to_utype(UiUtils::DefaultSpacings::LARGE)) and
  //child.set_margin_top(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));
  add(m_Frame);

  m_layout_name = "list_portal"; //Replaced by derived classes.
}

void Box_Data_Portal::make_record_related(const Gnome::Gda::Value& related_record_primary_key_value)
{
  auto field_primary_key = get_field_primary_key();

  //Create the link by setting the foreign key
  if(!m_key_field)
  {
    std::cerr << G_STRFUNC << ": m_key_field was null.\n";
  }

  if(Conversions::value_is_empty(m_key_value))
  {
    std::cerr << G_STRFUNC << ": m_key_value was empty.\n";
  }

  const auto portal = get_portal();
  if (!portal)
  {
    std::cerr << G_STRFUNC << ": portal is null\n";
    return;
  }

  const auto target_table = portal->get_table_used(Glib::ustring() /* not relevant */);
  auto builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_UPDATE);
    builder->set_table(target_table);
    builder->add_field_value_as_value(m_key_field->get_name(), m_key_value);
    builder->set_where(
      builder->add_cond(Gnome::Gda::SQL_OPERATOR_TYPE_EQ,
        builder->add_field_id(field_primary_key->get_name(), target_table),
        builder->add_expr_as_value(related_record_primary_key_value)));

  //std::cout << "debug: " << G_STRFUNC << ": setting value in db=" << primary_key_value.to_string() << std::endl;
  const auto test = DbUtils::query_execute(builder);
  if(!test)
  {
    std::cerr << G_STRFUNC << ": SQL query failed.\n";
  }
}

bool Box_Data_Portal::init_db_details(const std::shared_ptr<const LayoutItem_Portal>& portal, bool show_title)
{
  auto portal_stored = glom_sharedptr_clone(portal);
  set_layout_item(portal_stored, "" /* TODO */);

  Glib::ustring parent_table;
  if(portal_stored)
    parent_table = portal_stored->get_from_table();

  return init_db_details(parent_table, show_title);
}

Glib::ustring Box_Data_Portal::get_title(const Glib::ustring& locale) const
{
  auto portal = get_portal();
  if(portal)
    return portal->get_title_or_name(locale);
  else
  {
    //Note to translators: This text is shown instead of a table title, when the table has not yet been chosen.
    return _("Undefined Table");
  }
}

Glib::ustring Box_Data_Portal::get_title_singular(const Glib::ustring& locale) const
{
  Glib::ustring relationship_title;
  const auto portal = get_portal();
  if(portal && portal->get_has_relationship_name())
    relationship_title = portal->get_title_singular_used(Glib::ustring() /* parent title - not relevant */, locale);
  else
  {
    //Note to translators: This text is shown instead of a table title, when the table has not yet been chosen.
    relationship_title = _("Undefined Table");
  }

  return relationship_title;
}

bool Box_Data_Portal::refresh_data_from_database_with_foreign_key(const Gnome::Gda::Value& foreign_key_value)
{
  m_key_value = foreign_key_value;
  //std::cout << "debug: " << G_STRFUNC << ": m_key_value=" << m_key_value.to_string() << std::endl;

  const auto portal = get_portal();
  if(m_key_field && portal)
  {
    if(!Conversions::value_is_empty(m_key_value))
    {
      FoundSet found_set;
      set_found_set_where_clause_for_portal(found_set, portal, m_key_value);

      //std::cout << "debug: " << G_STRFUNC << ": where_clause=" << found_set.m_where_clause << std::endl;
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
    found_set.m_where_clause = Gnome::Gda::SqlExpr();
    return Box_Data::refresh_data_from_database_with_where_clause(found_set);
  }
}

std::shared_ptr<LayoutItem_Portal> Box_Data_Portal::get_portal()
{
  return std::dynamic_pointer_cast<LayoutItem_Portal>(get_layout_item());
}

std::shared_ptr<const LayoutItem_Portal> Box_Data_Portal::get_portal() const
{
  return std::dynamic_pointer_cast<const LayoutItem_Portal>(get_layout_item());
}

std::shared_ptr<const Field> Box_Data_Portal::get_key_field() const
{
  return m_key_field;
}

//TODO: refactor: Remove this because it is never called?
void Box_Data_Portal::on_record_deleted(const Gnome::Gda::Value& /* primary_key_value */)
{
  const auto portal = get_portal();
  if (!portal)
  {
    std::cerr << G_STRFUNC << ": portal is null\n";
    return;
  }

  //Allow the parent record (Details view) to recalculate aggregations:
  signal_portal_record_changed().emit(portal->get_relationship_name());
}

void Box_Data_Portal::on_record_added(const Gnome::Gda::Value& /* primary_key_value */, const Gtk::TreeModel::iterator& /* row */)
{
  const auto portal = get_portal();
  if (!portal)
  {
    std::cerr << G_STRFUNC << ": portal is null\n";
    return;
  }

  //Allow the parent record (Details view) to recalculate aggregations:
  signal_portal_record_changed().emit(portal->get_relationship_name());
}

Box_Data_Portal::type_vecConstLayoutFields Box_Data_Portal::get_fields_to_show() const
{
  const auto document = get_document();
  const auto portal = get_portal();
  if(document && portal)
  {
    Document::type_list_const_layout_groups mapGroups;
    mapGroups.emplace_back(portal);

    auto relationship = portal->get_relationship();
    if(relationship)
    {
      auto result = Utils::get_table_fields_to_show_for_sequence(document, portal->get_table_used(Glib::ustring() /* not relevant */), mapGroups);

      //If the relationship does not allow editing, then mark all these fields as non-editable:
      //TODO: Prevent this in some other way:
      /*
      if(!(portal->get_relationship_used_allows_edit()))
      {
        for(const auto& item : result)
        {
          auto item = *iter;
          if(item)
            item->set_editable(false);
        }
      }
      */

      return result;
    }
  }

  return type_vecConstLayoutFields();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Box_Data_Portal::on_dialog_layout_hide()
{
  //Overridden in derived classes.
}
#endif // !GLOM_ENABLE_CLIENT_ONLY



bool Box_Data_Portal::get_has_suitable_record_to_view_details() const
{
  const auto portal = get_portal();
  if (!portal)
  {
    return false;
  }

  const auto document = get_document();
  if(!document)
    return false;
    
  Glib::ustring navigation_table_name;
  std::shared_ptr<const UsesRelationship> navigation_relationship; //Ignored.
  portal->get_suitable_table_to_view_details(navigation_table_name, navigation_relationship, document);

  return !(navigation_table_name.empty());
}

void Box_Data_Portal::get_suitable_record_to_view_details(const Gnome::Gda::Value& primary_key_value, Glib::ustring& table_name, Gnome::Gda::Value& table_primary_key_value) const
{
  //Initialize output parameters:
  table_name = Glib::ustring();
  table_primary_key_value = Gnome::Gda::Value();

  const auto portal = get_portal();
  if(!portal)
    return;

  const auto document = get_document();
  if(!document)
    return;
    
  Glib::ustring navigation_table_name;
  std::shared_ptr<const UsesRelationship> navigation_relationship;
  portal->get_suitable_table_to_view_details(navigation_table_name, navigation_relationship, document);
  
  //if(navigation_relationship && navigation_relationship->get_relationship())
  //  std::cout << "debug: navigation_relationship=" << navigation_relationship->get_relationship()->get_name() << std::endl;
  //if(navigation_relationship && navigation_relationship->get_related_relationship())
  //  std::cout << "debug: navigation_related_relationship=" << navigation_relationship->get_related_relationship()->get_name() << std::endl;

  if(navigation_table_name.empty())
    return;

  //Get the primary key of that table:
  auto navigation_table_primary_key = get_field_primary_key_for_table(navigation_table_name);

  //Build a layout item to get the field's value:
  auto layout_item = std::make_shared<LayoutItem_Field>();
  layout_item->set_full_field_details(navigation_table_primary_key);

  if(navigation_relationship)
  {
    layout_item->set_relationship( navigation_relationship->get_relationship() );
    //std::cout << "debug: navigation_relationship->get_relationship()= " << navigation_relationship->get_relationship()->get_name() << std::endl;
    layout_item->set_related_relationship( navigation_relationship->get_related_relationship() );
  }

  //Get the value of the navigation related primary key:
  type_vecLayoutFields fieldsToGet;
  fieldsToGet.emplace_back(layout_item);

  //For instance "invoice_line_id" if this is a portal to an "invoice_lines" table:
  const auto related_table = portal->get_table_used(Glib::ustring() /* not relevant */);
  auto key_field = get_field_primary_key_for_table(related_table);
  //std::cout << "DEBUG: related table=" << related_table << ", whose primary_key=" << key_field->get_name() << ", with value=" << primary_key_value.to_string() << "getting value for: " << layout_item->get_layout_display_name() << std::endl;

  auto query = SqlUtils::build_sql_select_with_key(related_table, fieldsToGet, key_field, primary_key_value);
  auto data_model = DbUtils::query_execute_select(query);


  bool value_found = true;
  if(data_model && data_model->get_n_rows() && data_model->get_n_columns())
  {
    //Set the output parameters:
    table_name = navigation_table_name;
    table_primary_key_value = data_model->get_value_at(0, 0);
    //std::cout << "debug: " << G_STRFUNC << ": table_primary_key_value=" << table_primary_key_value.to_string() << std::endl;

    //The value is empty when there there is no record to match the key in the related table:
    //For instance, if an invoice lines record mentions a product id, but the product does not exist in the products table.
    if(Conversions::value_is_empty(table_primary_key_value))
    {
      value_found = false;
      std::cout << "debug: " << G_STRFUNC << ": SQL query returned empty primary key.\n";
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

    auto window = const_cast<Gtk::Window*>(get_app_window());
    if(window)
      Frame_Glom::show_ok_dialog(_("No Corresponding Record Exists"), _("No record with this value exists. Therefore navigation to the related record is not possible."), *window, Gtk::MESSAGE_WARNING); //TODO: Make it more clear to the user exactly what record, what field, and what value, we are talking about.
  }
}

Document::type_list_layout_groups Box_Data_Portal::create_layout_get_layout()
{
  Document::type_list_layout_groups result;

  const auto portal = get_portal();
  if(portal)
    result.emplace_back(portal);

  return result;
}

std::shared_ptr<Field> Box_Data_Portal::get_field_primary_key() const
{
  return m_key_field;
}

Box_Data_Portal::type_signal_portal_record_changed Box_Data_Portal::signal_portal_record_changed()
{
  return m_signal_portal_record_changed;
}

void Box_Data_Portal::set_find_mode(bool val)
{
  m_find_mode = val;
}

Glib::ustring Box_Data_Portal::get_parent_table() const
{
  return m_parent_table;
}

void Box_Data_Portal::set_parent_table(const Glib::ustring& parent_table)
{
  m_parent_table = parent_table;
}


} //namespace Glom
