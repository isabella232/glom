/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#include <libglom/report_builder.h>
#include <libglom/utils.h>
#include <libglom/data_structure/glomconversions.h>
#include <libglom/db_utils.h>
#include <libglom/xsl_utils.h>
#include <libglom/xml_utils.h>
#include <iostream>
#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

namespace Glom
{

ReportBuilder::ReportBuilder(const std::locale& locale)
: m_document(nullptr),
  m_locale(locale)
{
  m_locale_id = Utils::locale_simplify(locale.name());
}

bool ReportBuilder::report_build_headerfooter(const FoundSet& found_set, xmlpp::Element& parent_node, const std::shared_ptr<LayoutGroup>& group)
{
  //Add XML node:
  auto node = parent_node.add_child_element(group->get_report_part_id());

  //Add child parts:
  type_vecLayoutItems itemsToGet;
  for(const auto& item : group->m_list_items)
  {
    auto item_text = std::dynamic_pointer_cast<LayoutItem_Text>(item);
    if(item_text)
    {
      if(!report_build_records_text(found_set, *node, item_text))
      {
        std::cerr << G_STRFUNC << ": report_build_records_text() failed." << std::endl;
        return false;
      }
    }
    else
    {
      auto item_image = std::dynamic_pointer_cast<LayoutItem_Image>(item);
      if(item_image)
      {
        if(!report_build_records_image(found_set, *node, item_image))
        {
          std::cerr << G_STRFUNC << ": report_build_records_image() failed." << std::endl;
          return false;
        }
      }
      else
      {
        auto pField = std::dynamic_pointer_cast<LayoutItem_Field>(item);
        if(pField)
        {
          guint col_index = 0; //ignored.
          if(!report_build_records_field(found_set, *node, pField, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index))
          {
            std::cerr << G_STRFUNC << ": report_build_records_field() failed." << std::endl;
            return false;
          }
        }
        else
        {
          auto vertical_group = std::dynamic_pointer_cast<LayoutItem_VerticalGroup>(item);
          if(vertical_group)
          {
            //Reuse (a bit hacky) this function for the header and footer:
            guint col_index = 0; //Ignored, because the model is null.
            if(!report_build_records_vertical_group(found_set, *node, vertical_group, Glib::RefPtr<Gnome::Gda::DataModel>(), 0, col_index))
            {
              std::cerr << G_STRFUNC << ": report_build_records_vertical_group() failed." << std::endl;
              return false;
            }
          }
        }
      }
    }
  }

  return true;
}

bool ReportBuilder::report_build_summary(const FoundSet& found_set, xmlpp::Element& parent_node, const std::shared_ptr<LayoutItem_Summary>& summary)
{
  //Add XML node:
  auto node = parent_node.add_child_element(summary->get_report_part_id());

  //Get fields
  type_vecLayoutItems itemsToGet;
  for(const auto& item : summary->m_list_items)
  {
    auto pGroupBy = std::dynamic_pointer_cast<LayoutItem_GroupBy>(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      if(!report_build_groupby(found_set, *node, pGroupBy))
      {
        std::cerr << G_STRFUNC << ": report_build_groupby() failed." << std::endl;
        return false;
      }
    }
    else
    {
      auto pSummary = std::dynamic_pointer_cast<LayoutItem_Summary>(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        if(!report_build_summary(found_set, *node, pSummary))
        {
          std::cerr << G_STRFUNC << ": report_build_summary() failed." << std::endl;
          return false;
        }
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Make sure that the FoundSet has no ORDER BY, because
    //a) That makes no sense for a single summary row result.
    //b) That would require us to mention the ORDER BY field in the GROUP BY clause or in an aggregate function.
    FoundSet found_set_used = found_set;
    found_set_used.m_sort_clause.clear();

    //Rows, with data:
    if(!report_build_records(found_set_used, *node, itemsToGet))
    {
      std::cerr << G_STRFUNC << ": report_build_records() failed." << std::endl;
      return false;
    }
  }

  return true;
}



bool ReportBuilder::report_build_groupby_children(const FoundSet& found_set, xmlpp::Element& node, const std::shared_ptr<LayoutItem_GroupBy>& group_by)
{
  //Get data and add child rows:
  type_vecLayoutItems itemsToGet;
  for(const auto& item : group_by->m_list_items)
  {
    auto pGroupBy = std::dynamic_pointer_cast<LayoutItem_GroupBy>(item);
    if(pGroupBy)
    {
      //Recurse, adding a sub-groupby block:
      if(!report_build_groupby(found_set, node, pGroupBy))
      {
        std::cerr << G_STRFUNC << ": report_build_groupby() failed." << std::endl;
        return false;
      }
    }
    else
    {
      auto pSummary = std::dynamic_pointer_cast<LayoutItem_Summary>(item);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        if(!report_build_summary(found_set, node, pSummary))
        {
          std::cerr << G_STRFUNC << ": report_build_summary() failed." << std::endl;
          return false;
        }
      }
      else
      {
        itemsToGet.push_back( glom_sharedptr_clone(item) );
      }
    }
  }

  if(!itemsToGet.empty())
  {
    //Rows, with data:
    FoundSet found_set_records = found_set;
    found_set_records.m_sort_clause = group_by->get_fields_sort_by();
    if(!report_build_records(found_set_records, node, itemsToGet))
    {
      std::cerr << G_STRFUNC << ": report_build_records() failed." << std::endl;
      return false;
    }
  }

  return true;
}

bool ReportBuilder::report_build_groupby(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const std::shared_ptr<LayoutItem_GroupBy>& group_by)
{
  //Get the possible heading values.
  if(group_by->get_has_field_group_by())
  {
    auto field_group_by = group_by->get_field_group_by();
    DbUtils::layout_item_fill_field_details(get_document(), found_set_parent.m_table_name, field_group_by);

    //Get the possible group values, ignoring repeats by using GROUP BY.
    const auto group_field_table_name = field_group_by->get_table_used(found_set_parent.m_table_name);

    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder =
      Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->select_add_field(field_group_by->get_name(), group_field_table_name);
    builder->select_add_target(group_field_table_name);

    if(!found_set_parent.m_where_clause.empty())
    {
      builder->set_where( builder->import_expression(found_set_parent.m_where_clause) );
    }

    builder->select_group_by( builder->add_field_id(field_group_by->get_name(), group_field_table_name) ); //TODO: And restrict to the current found set.

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(builder);
    if(!datamodel)
    {
      std::cerr << G_STRFUNC << ": The SQL query failed." << std::endl;
      return false; 
    }
    else
    {
      const guint rows_count = datamodel->get_n_rows();
      for(guint row = 0; row < rows_count; ++row)
      {
        const auto group_value = datamodel->get_value_at(0 /* col*/, row); //TODO: Catch exceptions.


        //Add XML node:
        auto nodeGroupBy = parent_node.add_child_element(group_by->get_report_part_id());
        XmlUtils::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());

        nodeGroupBy->set_attribute("group_field", field_group_by->get_title_or_name(m_locale_id));
        nodeGroupBy->set_attribute("group_value",
          Conversions::get_text_for_gda_value(field_group_by->get_glom_type(), group_value, m_locale, field_group_by->get_formatting_used().m_numeric_format) );

        //TODO: Use a SQL parameter instead of using sql().
        Gnome::Gda::SqlExpr where_clause =
          Utils::build_simple_where_expression(group_field_table_name, field_group_by->get_full_field_details(), group_value);

        if(!found_set_parent.m_where_clause.empty())
        {
          where_clause = Utils::build_combined_where_expression(where_clause, found_set_parent.m_where_clause,
            Gnome::Gda::SQL_OPERATOR_TYPE_AND);
        }

        FoundSet found_set_records = found_set_parent;
        found_set_records.m_where_clause = where_clause;

        //Secondary fields. For instance, the Contact Name, in addition to the Contact ID that we group by.
        if(!(group_by->get_secondary_fields()->m_list_items.empty()))
        {
          auto nodeSecondaryFields = nodeGroupBy->add_child_element("secondary_fields");

          type_vecLayoutItems itemsToGet;
          for(const auto& item : group_by->get_secondary_fields()->m_list_items)
          {
            itemsToGet.push_back( glom_sharedptr_clone(item) );
          }

          if(!itemsToGet.empty())
          {
            if(!report_build_records(found_set_records, *nodeSecondaryFields, itemsToGet, true /* one record only */))
            {
              std::cerr << G_STRFUNC << ": report_build_records() failed." << std::endl;
              return false;
            }
          }
        }

        //Get data and add child rows:
        if(!report_build_groupby_children(found_set_records, *nodeGroupBy, group_by))
        {
          std::cerr << G_STRFUNC << ": report_build_groupby_children() failed." << std::endl;
          return false;
        }
      }
    }
  }
  else
  {
    //There is no group-by field, so output all the found records.
    //For instance, the user could use the GroupBy part just to specify a sort, though that would be a bit of a hack:
    auto nodeGroupBy = parent_node.add_child_element(group_by->get_report_part_id()); //We need this to create the HTML table.
    XmlUtils::set_node_attribute_value_as_decimal_double(nodeGroupBy, "border_width", group_by->get_border_width());
    return report_build_groupby_children(found_set_parent, *nodeGroupBy, group_by);
  }

  return true;
}

bool ReportBuilder::report_build_records_get_fields(const FoundSet& found_set, const std::shared_ptr<LayoutGroup>& group, type_vecLayoutFields& items)
{
  for(const auto& item : group->m_list_items)
  {
    auto pVerticalGroup = std::dynamic_pointer_cast<LayoutItem_VerticalGroup>(item);
    if(pVerticalGroup)
    {
      if(!report_build_records_get_fields(found_set, pVerticalGroup, items))
      {
        std::cerr << G_STRFUNC << ": report_build_records_get_fields() failed." << std::endl;
        return false;
      }
    }
    else
    {
      auto pField = std::dynamic_pointer_cast<LayoutItem_Field>(item);
      if(pField)
        items.push_back(pField);
    }
  }

  return true;
}

bool ReportBuilder::report_build_records(const FoundSet& found_set, xmlpp::Element& parent_node, const type_vecLayoutItems& items, bool one_record_only)
{
  if(!items.empty())
  {
    //Add Field headings:
    for(const auto& layout_item : items)
    {
      auto layoutitem_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);

      //This adds a field heading (and therefore, column) for fields, or for a vertical group.
      auto nodeFieldHeading = parent_node.add_child_element("field_heading");
      if(layoutitem_field && layoutitem_field->get_glom_type() == Field::glom_field_type::NUMERIC)
        nodeFieldHeading->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

      nodeFieldHeading->set_attribute("name", layout_item->get_name()); //Not really necessary, but maybe useful.
      nodeFieldHeading->set_attribute("title", layout_item->get_title_or_name(m_locale_id));
    }

    //Get list of fields to get from the database.
    Utils::type_vecLayoutFields fieldsToGet;
    for(const auto& layout_item : items)
    {
      auto layoutitem_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
      if(layoutitem_field)
        fieldsToGet.push_back(layoutitem_field);
      else
      {
        auto vertical_group = std::dynamic_pointer_cast<LayoutItem_VerticalGroup>(layout_item);
        if(vertical_group)
        {
          //Get all the fields in this group:
          if(!report_build_records_get_fields(found_set, vertical_group, fieldsToGet))
          {
            std::cerr << G_STRFUNC << ": report_build_records_get_fields() failed." << std::endl;
            return false;
          }
        }
      }
    }

    //For instance, when we just want to get a name corresponding to a contact ID, and want to ignore duplicates.
    guint limit = 0; //no limit.
    if(one_record_only)
      limit = 1;

    Glib::RefPtr<Gnome::Gda::SqlBuilder> sql_query = Utils::build_sql_select_with_where_clause(found_set.m_table_name,
      fieldsToGet,
      found_set.m_where_clause, std::shared_ptr<const Relationship>() /* extra_join */, found_set.m_sort_clause,
      limit);

    Glib::RefPtr<Gnome::Gda::DataModel> datamodel = DbUtils::query_execute_select(sql_query);
    if(!datamodel)
    {
      std::cerr << G_STRFUNC << ": The SLQ query failed." << std::endl;
      return false; 
    }
    else
    {
      const guint rows_count = datamodel->get_n_rows();

      for(guint row = 0; row < rows_count; ++row)
      {
        auto nodeRow = parent_node.add_child_element("row");

        guint colField = 0;
        for(const auto& item : items)
        {
          auto field = std::dynamic_pointer_cast<LayoutItem_Field>(item);
          if(field)
          {
            if(!report_build_records_field(found_set, *nodeRow, field, datamodel, row, colField))
            {
              std::cerr << G_STRFUNC << ": report_build_records_field() failed." << std::endl;
              return false;
            }
          }
          else
          {
            auto item_text = std::dynamic_pointer_cast<LayoutItem_Text>(item);
            if(item_text)
            {
              if(!report_build_records_text(found_set, *nodeRow, item_text))
              {
                std::cerr << G_STRFUNC << ": report_build_records_text() failed." << std::endl;
                return false;
              }
            }
            else
            {
              auto item_verticalgroup = std::dynamic_pointer_cast<LayoutItem_VerticalGroup>(item);
              if(item_verticalgroup)
              {
                if(!report_build_records_vertical_group(found_set, *nodeRow, item_verticalgroup, datamodel, row, colField))
                {
                  std::cerr << G_STRFUNC << ": report_build_records_vertical_group() failed." << std::endl;
                  return false;
                }
              }
            }
          }
        }
      }

    }

    //If there are no records, show zero
    //if(!rows_found && show_null_row)
  }

  return true;
}

bool ReportBuilder::report_build_records_field(const FoundSet& found_set, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Field>& field, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& colField, bool vertical)
{
  const auto field_type = field->get_glom_type();

  auto nodeField = nodeParent.add_child_element(field->get_report_part_id());
  if(field_type == Field::glom_field_type::NUMERIC)
    nodeField->set_attribute("field_type", "numeric"); //TODO: More sophisticated formatting.

  if(vertical)
    nodeField->set_attribute("vertical", "true");

  Gnome::Gda::Value value;

  if(!datamodel) //We call this for headers and footers too.
  {
    //In this case it can only be a system preferences field.
    //So let's get that data here:
    Glib::RefPtr<Gnome::Gda::SqlBuilder> builder = Gnome::Gda::SqlBuilder::create(Gnome::Gda::SQL_STATEMENT_SELECT);
    builder->set_table(field->get_table_used(found_set.m_table_name));
    builder->select_add_field(field->get_name(), found_set.m_table_name);
    builder->select_set_limit(1);
    Glib::RefPtr<Gnome::Gda::DataModel> datamodel_syspref = DbUtils::query_execute_select(builder);

    if(!datamodel_syspref)
    {
      std::cerr << G_STRFUNC << ": The SQL query failed." << std::endl;
      return false;
    }

    value = datamodel_syspref->get_value_at(colField, row); //TODO: Catch exceptions.
    colField = 0;
  }
  else
  {
    value = datamodel->get_value_at(colField, row); //TODO: Catch exceptions.
  }

  nodeField->set_attribute("title", field->get_title_or_name(m_locale_id)); //Not always used, but useful.

  //Handle the value:
  if(field_type == Field::glom_field_type::IMAGE)
     nodeField->set_attribute("image_uri", Utils::create_local_image_uri(value));
  else
  {
    Glib::ustring text_value = Conversions::get_text_for_gda_value(field_type, value, m_locale, field->get_formatting_used().m_numeric_format);

    //The Postgres summary functions return NULL when summarising NULL records, but 0 is more sensible:
    if(text_value.empty() && std::dynamic_pointer_cast<const LayoutItem_FieldSummary>(field) && (field_type == Field::glom_field_type::NUMERIC))
    {
      //Use get_text_for_gda_value() instead of "0" so we get the correct numerical formatting:
      const auto value_zero = Conversions::parse_value(0);
      text_value = Conversions::get_text_for_gda_value(field_type, value_zero, m_locale, field->get_formatting_used().m_numeric_format);
    }

    nodeField->set_attribute("value", text_value);
  }

  ++colField;

  return true;
}

bool ReportBuilder::report_build_records_text(const FoundSet& /* found_set */, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Text>& textobject, bool vertical)
{
  //Text object:
  auto nodeField = nodeParent.add_child_element(textobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeField->set_attribute("value", textobject->get_text(m_locale_id));

  if(vertical)
    nodeField->set_attribute("vertical", "true");

  return true;
}

bool ReportBuilder::report_build_records_image(const FoundSet& /* found_set */, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Image>& imageobject, bool vertical)
{
  //Text object:
  auto nodeImage = nodeParent.add_child_element(imageobject->get_report_part_id()); //We reuse this node type for text objects.
  nodeImage->set_attribute("image_uri", imageobject->create_local_image_uri());

  if(vertical)
    nodeImage->set_attribute("vertical", "true");

  return true;
}

bool ReportBuilder::report_build_records_vertical_group(const FoundSet& found_set, xmlpp::Element& parentNode, const std::shared_ptr<LayoutItem_VerticalGroup>& group, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& field_index)
{
  auto nodeGroupVertical = parentNode.add_child_element(group->get_report_part_id());

  for(const auto& item : group->m_list_items)
  {
    auto pVerticalGroup = std::dynamic_pointer_cast<LayoutItem_VerticalGroup>(item);
    if(pVerticalGroup)
    {
      if(!report_build_records_vertical_group(found_set, *nodeGroupVertical, pVerticalGroup, datamodel, row, field_index))
      {
        std::cerr << G_STRFUNC << ": report_build_records_vertical_group() failed." << std::endl;
        return false;
      }
    }
    else
    {
      auto pField = std::dynamic_pointer_cast<LayoutItem_Field>(item);
      if(pField)
      {
        if(!report_build_records_field(found_set, *nodeGroupVertical, pField, datamodel, row, field_index, true /* vertical, so we get a row for each field too. */))
        {
          std::cerr << G_STRFUNC << ": report_build_records_field() failed." << std::endl;
          return false;
        }
      }
      else
      {
        auto pText = std::dynamic_pointer_cast<LayoutItem_Text>(item);
        if(pText)
        {
          if(!report_build_records_text(found_set, *nodeGroupVertical, pText, true))
          {
            std::cerr << G_STRFUNC << ": report_build_records_text() failed." << std::endl;
            return false;
          }
        }
      }
    }
  }

  return true;
}

//TODO: Return a URI
std::string ReportBuilder::report_build_and_save(const FoundSet& found_set, const std::shared_ptr<const Report>& report)
{
  const auto contents = report_build(found_set, report);

 //Save it to a temporary file and show it in a browser:
  const auto temp_uri = Utils::get_temp_file_uri("glom_printout", "html");
  std::cout << G_STRFUNC << ": temp_uri=" << temp_uri << std::endl;

  Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(temp_uri);
  Glib::RefPtr<Gio::FileOutputStream> stream;

  //Create the file if it does not already exist:
  try
  {
    if(file->query_exists())
    {
      stream = file->replace(); //Instead of append_to().
    }
    else
    {
      //By default files created are generally readable by everyone, but if we pass FILE_CREATE_PRIVATE in flags the file will be made readable only to the current user, to the level that is supported on the target filesystem.
      //TODO: Do we want to specify 0660 exactly? (means "this user and his group can read and write this non-executable file".)
      stream = file->create_file();
    }
  }
  catch(const Gio::Error& ex)
  {
    // If the operation was not successful, print the error and abort
    return std::string(); // print_error(ex, output_uri_string);
  }

  //Write the data to the output uri
  gssize bytes_written = 0;
  const auto result_bytes = contents.bytes();
  try
  {
    bytes_written = stream->write(contents.data(), result_bytes);
  }
  catch(const Gio::Error& ex)
  {
    // If the operation was not successful, print the error and abort
    return std::string(); // false; //print_error(ex, output_uri_string);
  }

  if(bytes_written != (gssize)result_bytes)
    return std::string(); //false

  return file->get_path();
}
 

Glib::ustring ReportBuilder::report_build(const FoundSet& found_set, const std::shared_ptr<const Report>& report)
{
  //Create a DOM Document with the XML:
  xmlpp::DomParser dom_parser;;

  auto pDocument = dom_parser.get_document();
  auto nodeRoot = pDocument->get_root_node();
  if(!nodeRoot)
  {
    //Add it if it isn't there already:
    nodeRoot = pDocument->create_root_node("report_print");
  }

  Glib::ustring table_title = get_document()->get_table_title(found_set.m_table_name, m_locale_id);
  if(table_title.empty())
    table_title = found_set.m_table_name;

  nodeRoot->set_attribute("table", table_title);
  if(report->get_show_table_title())
    nodeRoot->set_attribute("show_table_title", "true");


  //The groups:
  auto nodeParent = nodeRoot;


  nodeRoot->set_attribute("title", report->get_title_or_name(m_locale_id));

  type_vecLayoutItems itemsToGet_TopLevel;

  const auto group = report->get_layout_group();
  for(const auto& pPart : group->m_list_items)
  {
    //The Group, and the details for each record in the group:
    auto pGroupBy = std::dynamic_pointer_cast<LayoutItem_GroupBy>(pPart);
    if(pGroupBy)
    {
      if(!report_build_groupby(found_set, *nodeParent, pGroupBy))
      {
        std::cerr << G_STRFUNC << ": report_build_groupby() failed." << std::endl;
        return Glib::ustring();
      }
    }
    else
    {
      auto pSummary = std::dynamic_pointer_cast<LayoutItem_Summary>(pPart);
      if(pSummary)
      {
        //Recurse, adding a summary block:
        if(!report_build_summary(found_set, *nodeParent, pSummary))
        {
          std::cerr << G_STRFUNC << ": report_build_summary() failed." << std::endl;
          return Glib::ustring();
        }
      }
      else
      {
        auto pGroup = std::dynamic_pointer_cast<LayoutGroup>(pPart);
        if(pGroup)
        {
          auto pHeader = std::dynamic_pointer_cast<LayoutItem_Header>(pPart);
          auto pFooter = std::dynamic_pointer_cast<LayoutItem_Footer>(pPart);
          if(pHeader || pFooter)
          {
            //Recurse, adding a summary block:
            if(!report_build_headerfooter(found_set, *nodeParent, pGroup))
            {
              std::cerr << G_STRFUNC << ": report_build_headerfooter() failed." << std::endl;
              return Glib::ustring();
            }
          }
        }
        else
          itemsToGet_TopLevel.push_back( glom_sharedptr_clone(pPart) );
      }
    }
  }

  //Add top-level records, outside of any groupby or summary, if fields have been specified:
  if(!itemsToGet_TopLevel.empty())
  {
    auto nodeGroupBy = nodeParent->add_child_element("ungrouped_records");
    if(!report_build_records(found_set, *nodeGroupBy, itemsToGet_TopLevel))
    {
      std::cerr << G_STRFUNC << ": report_build_records() failed." << std::endl;
      return Glib::ustring();
    }
  }

  return GlomXslUtils::transform(*pDocument, "print_report_to_html.xsl");
}

static void fill_standard_list_report_fill(const std::shared_ptr<Report>& report, const std::shared_ptr<const LayoutGroup>& layout_group)
{
  if(!report)
    return;

  if(!layout_group)
    return;

  for(const auto& item : layout_group->m_list_items)
  {
    if(!item)
      continue;

    const auto unconst = std::const_pointer_cast<LayoutItem>(item); //TODO: Avoid this?
    report->get_layout_group()->add_item(unconst);
  }
}

std::shared_ptr<Report> ReportBuilder::create_standard_list_report(const Document* document, const Glib::ustring& table_name)
{
  auto result = std::make_shared<Report>();
  result->set_name("list");
  //Translators: This is a noun. It is the title of a report.
  result->set_title_original(_("List"));

  const Document::type_list_layout_groups layout_groups = 
    document->get_data_layout_groups("list", table_name); //TODO: layout_platform.
  for(const auto& group : layout_groups)
  {
    if(group)
      fill_standard_list_report_fill(result, group);
  }

  return result;
}

void ReportBuilder::set_document(Document* document)
{
  m_document = document;
}

Document* ReportBuilder::get_document()
{
  return m_document;
}


} //namespace Glom
